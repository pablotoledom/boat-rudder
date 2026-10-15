#include "markdown_lite.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char  *data;
    size_t len;
    size_t cap;
    int    failed;
} Buf;

static void buf_append(Buf *b, const char *s, size_t n) {
    if (b->failed) return;
    if (b->len + n + 1 > b->cap) {
        size_t cap = b->cap ? b->cap : 256;
        while (cap < b->len + n + 1) cap *= 2;
        char *grown = realloc(b->data, cap);
        if (!grown) { b->failed = 1; return; }
        b->data = grown;
        b->cap  = cap;
    }
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static void buf_puts(Buf *b, const char *s) { buf_append(b, s, strlen(s)); }

static void buf_escaped(Buf *b, const char *s, size_t n) {
    for (size_t i = 0; i < n; i++) {
        switch (s[i]) {
            case '&':  buf_puts(b, "&amp;");  break;
            case '<':  buf_puts(b, "&lt;");   break;
            case '>':  buf_puts(b, "&gt;");   break;
            case '"':  buf_puts(b, "&quot;"); break;
            case '\'': buf_puts(b, "&#39;");  break;
            default:   buf_append(b, s + i, 1);
        }
    }
}

// The first `needle` in s[from, n), or -1.
static long find(const char *s, size_t n, size_t from, const char *needle) {
    size_t k = strlen(needle);
    for (size_t i = from; i + k <= n; i++)
        if (memcmp(s + i, needle, k) == 0) return (long)i;
    return -1;
}

static int is_http(const char *url, size_t n) {
    return (n > 7 && strncmp(url, "http://", 7) == 0) ||
           (n > 8 && strncmp(url, "https://", 8) == 0);
}

static void inline_html(Buf *b, const char *s, size_t n, const char *link_base);

// [text](url) at s[i]: appends the link and returns the length consumed, or 0
// when s[i] doesn't start one.
static size_t link_at(Buf *b, const char *s, size_t n, size_t i, const char *link_base) {
    long close = find(s, n, i + 1, "](");
    if (close < 0 || memchr(s + i + 1, '\n', (size_t)close - i - 1)) return 0;
    long end = find(s, n, (size_t)close + 2, ")");
    if (end < 0) return 0;

    const char *text = s + i + 1;
    size_t text_len  = (size_t)close - i - 1;
    const char *url  = s + close + 2;
    size_t url_len   = (size_t)end - (size_t)close - 2;
    int has_scheme   = memchr(url, ':', url_len) != NULL;
    int relative     = !has_scheme && url_len > 0 && url[0] != '/' && url[0] != '#';

    if (is_http(url, url_len) || (relative && link_base)) {
        buf_puts(b, "<a href=\"");
        if (relative) buf_escaped(b, link_base, strlen(link_base));
        buf_escaped(b, url, url_len);
        buf_puts(b, "\" rel=\"noopener noreferrer\">");
        inline_html(b, text, text_len, link_base);
        buf_puts(b, "</a>");
    } else {
        inline_html(b, text, text_len, link_base);
    }
    return (size_t)end - i + 1;
}

static void inline_html(Buf *b, const char *s, size_t n, const char *link_base) {
    size_t i = 0;
    while (i < n) {
        if (s[i] == '`') {
            long end = find(s, n, i + 1, "`");
            if (end > (long)i + 1) {
                buf_puts(b, "<code>");
                buf_escaped(b, s + i + 1, (size_t)end - i - 1);
                buf_puts(b, "</code>");
                i = (size_t)end + 1;
                continue;
            }
        } else if (s[i] == '*' && i + 1 < n && s[i + 1] == '*') {
            long end = find(s, n, i + 2, "**");
            if (end > (long)i + 2) {
                buf_puts(b, "<strong>");
                inline_html(b, s + i + 2, (size_t)end - i - 2, link_base);
                buf_puts(b, "</strong>");
                i = (size_t)end + 2;
                continue;
            }
        } else if (s[i] == '*' && i + 1 < n && s[i + 1] != ' ') {
            long end = find(s, n, i + 1, "*");
            if (end > (long)i + 1 && s[end - 1] != ' ') {
                buf_puts(b, "<em>");
                inline_html(b, s + i + 1, (size_t)end - i - 1, link_base);
                buf_puts(b, "</em>");
                i = (size_t)end + 1;
                continue;
            }
        } else if (s[i] == '[') {
            size_t used = link_at(b, s, n, i, link_base);
            if (used) { i += used; continue; }
        }
        buf_escaped(b, s + i, 1);
        i++;
    }
}

typedef enum { BLOCK_NONE, BLOCK_PARAGRAPH, BLOCK_ITEM } BlockKind;

typedef struct {
    Buf        *out;
    Buf         text;    // the paragraph or item being collected
    BlockKind   kind;
    int         in_list;
    const char *link_base;
} Renderer;

static void flush_block(Renderer *r) {
    if (r->kind == BLOCK_PARAGRAPH) {
        buf_puts(r->out, "<p>");
        inline_html(r->out, r->text.data, r->text.len, r->link_base);
        buf_puts(r->out, "</p>\n");
    } else if (r->kind == BLOCK_ITEM) {
        buf_puts(r->out, "<li>");
        inline_html(r->out, r->text.data ? r->text.data : "", r->text.len, r->link_base);
        buf_puts(r->out, "</li>\n");
    }
    r->kind     = BLOCK_NONE;
    r->text.len = 0;
    if (r->text.data) r->text.data[0] = '\0';
}

static void close_list(Renderer *r) {
    flush_block(r);
    if (r->in_list) buf_puts(r->out, "</ul>\n");
    r->in_list = 0;
}

static void collect(Renderer *r, const char *s, size_t n) {
    if (r->text.len > 0) buf_append(&r->text, " ", 1);
    buf_append(&r->text, s, n);
}

char *markdown_to_html(const char *md, const char *link_base) {
    Buf out = {0};
    Renderer r = { .out = &out, .link_base = link_base };
    buf_puts(&out, "");   // "" for an empty `md`, not NULL

    const char *line = md ? md : "";
    while (*line) {
        const char *eol = strchr(line, '\n');
        size_t n = eol ? (size_t)(eol - line) : strlen(line);
        const char *next = eol ? eol + 1 : line + n;
        while (n > 0 && (line[n - 1] == '\r' || line[n - 1] == ' ' || line[n - 1] == '\t')) n--;

        size_t indent = 0;
        while (indent < n && line[indent] == ' ') indent++;
        const char *body = line + indent;
        size_t body_len  = n - indent;

        size_t hashes = 0;
        while (hashes < body_len && body[hashes] == '#') hashes++;
        int is_heading = indent == 0 && hashes >= 1 && hashes <= 6 &&
                         hashes < body_len && body[hashes] == ' ';
        int is_item    = indent <= 1 && body_len >= 2 &&
                         (body[0] == '-' || body[0] == '*' || body[0] == '+') && body[1] == ' ';

        if (body_len == 0) {
            flush_block(&r);
        } else if (is_heading) {
            close_list(&r);
            size_t level = hashes + 2 > 6 ? 6 : hashes + 2;
            char tag[4] = { 'h', (char)('0' + level), '>', '\0' };
            buf_puts(&out, "<");
            buf_puts(&out, tag);
            inline_html(&out, body + hashes + 1, body_len - hashes - 1, link_base);
            buf_puts(&out, "</");
            buf_puts(&out, tag);
            buf_puts(&out, "\n");
        } else if (is_item) {
            flush_block(&r);
            if (!r.in_list) { buf_puts(&out, "<ul>\n"); r.in_list = 1; }
            r.kind = BLOCK_ITEM;
            collect(&r, body + 2, body_len - 2);
        } else if (indent > 0 && r.kind == BLOCK_ITEM) {
            collect(&r, body, body_len);
        } else {
            if (r.in_list) close_list(&r);
            r.kind = BLOCK_PARAGRAPH;
            collect(&r, body, body_len);
        }
        line = next;
    }
    close_list(&r);

    free(r.text.data);
    if (out.failed || r.text.failed) { free(out.data); return NULL; }
    return out.data;
}
