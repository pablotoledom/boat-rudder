#include "css_compat.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// How a feature shows up in the (lowercased, comment/string-free) text.
typedef enum {
    MATCH_TEXT,         // a plain substring: ":has(", "@layer"
    MATCH_PROPERTY,     // a declaration's property name: "inset" in "{inset: 0"
    MATCH_UNIT,         // a length unit after a number: "dvh" in "100dvh"
    MATCH_WORD,         // a whole word: "subgrid", not ".foo-subgrid"
    MATCH_NESTING,      // "&" - CSS has no other use for it
} MatchKind;

typedef struct {
    const char *feature;
    MatchKind kind;
    const char *needle[3];      // any of them, NULL-ended
    const char *chrome;         // first version with it, per engine
    const char *firefox;
    const char *safari;         // also used as the iOS version
} CssFeature;

// First versions from MDN / caniuse. Ordered oldest first, so the warnings
// read from the most to the least widely supported.
static const CssFeature FEATURES[] = {
    { "custom properties (var())", MATCH_TEXT, { "var(--" }, "49", "31", "10" },
    { "display: grid", MATCH_TEXT, { "display:grid", "display: grid" }, "57", "52", "10.1" },
    { ":focus-within", MATCH_TEXT, { ":focus-within" }, "60", "52", "10.1" },
    { "clamp()", MATCH_TEXT, { "clamp(" }, "79", "75", "13.1" },
    { "@property", MATCH_TEXT, { "@property" }, "85", "128", "16.4" },
    { ":focus-visible", MATCH_TEXT, { ":focus-visible" }, "86", "85", "15.4" },
    { "inset", MATCH_PROPERTY, { "inset" }, "87", "66", "14.1" },
    { "aspect-ratio", MATCH_PROPERTY, { "aspect-ratio" }, "88", "89", "15" },
    { ":is() / :where()", MATCH_TEXT, { ":is(", ":where(" }, "88", "78", "14" },
    { "accent-color", MATCH_PROPERTY, { "accent-color" }, "93", "92", "15.4" },
    { "@layer", MATCH_TEXT, { "@layer" }, "99", "97", "15.4" },
    { "translate / rotate / scale properties", MATCH_PROPERTY,
      { "translate", "rotate", "scale" }, "104", "72", "14.1" },
    { ":has()", MATCH_TEXT, { ":has(" }, "105", "121", "15.4" },
    { "container queries", MATCH_TEXT, { "@container", "container-type:", "container-type :" }, "105", "110", "16" },
    { "viewport units dvh / svh / lvh", MATCH_UNIT, { "dvh", "svh", "lvh" }, "108", "101", "15.4" },
    { "color-mix()", MATCH_TEXT, { "color-mix(" }, "111", "113", "16.2" },
    { "oklch() / oklab()", MATCH_TEXT, { "oklch(", "oklab(" }, "111", "113", "15.4" },
    { "text-wrap", MATCH_PROPERTY, { "text-wrap" }, "114", "121", "17.5" },
    { "subgrid", MATCH_WORD, { "subgrid" }, "117", "71", "16" },
    { "CSS nesting (&)", MATCH_NESTING, { "&" }, "120", "117", "17.2" },
    { "light-dark()", MATCH_TEXT, { "light-dark(" }, "123", "120", "17.5" },
};

// `css` lowercased, with comments, quoted strings and url(...) blanked out -
// what a selector or declaration is made of, nothing quoted inside them.
static char *strip(const char *css) {
    size_t n = strlen(css);
    char *out = malloc(n + 1);
    if (!out) return NULL;
    size_t o = 0;
    for (size_t i = 0; i < n; i++) {
        char c = css[i];
        if (c == '/' && css[i + 1] == '*') {
            const char *end = strstr(css + i + 2, "*/");
            i = end ? (size_t)(end - css) + 1 : n;
            out[o++] = ' ';
        } else if (c == '"' || c == '\'') {
            size_t j = i + 1;
            while (j < n && css[j] != c) j += (css[j] == '\\' && j + 1 < n) ? 2 : 1;
            i = j;
            out[o++] = ' ';
        } else if (strncasecmp(css + i, "url(", 4) == 0) {
            const char *end = strchr(css + i, ')');
            i = end ? (size_t)(end - css) : n;
            out[o++] = ' ';
        } else {
            out[o++] = (char)tolower((unsigned char)c);
        }
    }
    out[o] = '\0';
    return out;
}

static int is_ident_char(char c) {
    return isalnum((unsigned char)c) || c == '-' || c == '_';
}

// `name` as a declaration's property: after "{", ";" or the start (blanks
// between), and followed by ":" - so "inset" in "box-shadow: inset 0" or
// "scale" in "transform: scale(2)" don't count.
static int has_property(const char *text, const char *name) {
    size_t len = strlen(name);
    for (const char *p = strstr(text, name); p; p = strstr(p + 1, name)) {
        if (is_ident_char(p[len])) continue;
        const char *after = p + len;
        while (*after == ' ' || *after == '\t' || *after == '\n' || *after == '\r') after++;
        if (*after != ':') continue;
        const char *before = p;
        while (before > text && (before[-1] == ' ' || before[-1] == '\t' ||
                                 before[-1] == '\n' || before[-1] == '\r'))
            before--;
        if (before == text || before[-1] == '{' || before[-1] == ';') return 1;
    }
    return 0;
}

static int has_word(const char *text, const char *word) {
    size_t len = strlen(word);
    for (const char *p = strstr(text, word); p; p = strstr(p + 1, word))
        if ((p == text || !is_ident_char(p[-1])) && !is_ident_char(p[len])) return 1;
    return 0;
}

// `unit` right after a digit: "100dvh", ".5svh" - not the class ".dvh".
static int has_unit(const char *text, const char *unit) {
    size_t len = strlen(unit);
    for (const char *p = strstr(text, unit); p; p = strstr(p + 1, unit)) {
        if (p > text && isdigit((unsigned char)p[-1]) && !is_ident_char(p[len]))
            return 1;
    }
    return 0;
}

static int uses(const char *text, const CssFeature *f) {
    for (size_t i = 0; i < 3 && f->needle[i]; i++) {
        switch (f->kind) {
            case MATCH_TEXT:
            case MATCH_NESTING:
                if (strstr(text, f->needle[i])) return 1;
                break;
            case MATCH_PROPERTY:
                if (has_property(text, f->needle[i])) return 1;
                break;
            case MATCH_UNIT:
                if (has_unit(text, f->needle[i])) return 1;
                break;
            case MATCH_WORD:
                if (has_word(text, f->needle[i])) return 1;
                break;
        }
    }
    return 0;
}

// The version just before `first`: "105" -> "104", "15.4" -> "15.3",
// "16" -> "15.9", "10.1" -> "10.0". 0 if there is none.
static int previous_version(const char *first, int *major, int *minor) {
    char *end;
    long ma = strtol(first, &end, 10);
    long mi = (*end == '.') ? strtol(end + 1, NULL, 10) : 0;
    if (mi > 0) { *major = (int)ma; *minor = (int)mi - 1; }
    else if (ma > 1) { *major = (int)ma - 1; *minor = 9; }
    else return 0;
    return 1;
}

// Whether a User-Agent of that engine at the version before `first` is
// still served the page by `rules`; when it is, appends "<Name> <version>"
// to `served`.
static void check_engine(const BrowserDetection *rules, bool dashboard, const char *name,
                         const char *first, char *served, size_t size) {
    int major, minor;
    if (!previous_version(first, &major, &minor)) return;

    char ua[256], version[16];
    if (strcmp(name, "Chrome") == 0) {
        snprintf(version, sizeof(version), "%d", major);
        snprintf(ua, sizeof(ua), "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                 "(KHTML, like Gecko) Chrome/%d.0.0.0 Safari/537.36", major);
    } else if (strcmp(name, "Firefox") == 0) {
        snprintf(version, sizeof(version), "%d", major);
        snprintf(ua, sizeof(ua), "Mozilla/5.0 (X11; Linux x86_64; rv:%d.0) Gecko/20100101 "
                 "Firefox/%d.0", major, major);
    } else if (strcmp(name, "Safari") == 0) {
        snprintf(version, sizeof(version), "%d.%d", major, minor);
        snprintf(ua, sizeof(ua), "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) "
                 "AppleWebKit/605.1.15 (KHTML, like Gecko) Version/%d.%d Safari/605.1.15",
                 major, minor);
    } else {  // iOS: its WebKit is Safari's, version for version
        snprintf(version, sizeof(version), "%d.%d", major, minor);
        snprintf(ua, sizeof(ua), "Mozilla/5.0 (iPhone; CPU iPhone OS %d_%d like Mac OS X) "
                 "AppleWebKit/605.1.15 (KHTML, like Gecko) Version/%d.%d Mobile/15E148 "
                 "Safari/604.1", major, minor, major, minor);
    }

    BrowserMatch m;
    browser_detection_classify(rules, ua, &m);
    int served_page = dashboard ? (m.epoch == EPOCH_MODERN && m.dashboard)
                                : m.epoch == EPOCH_MODERN;
    if (!served_page) return;

    size_t used = strlen(served);
    snprintf(served + used, size - used, "%s%s %s", used ? ", " : "", name, version);
}

size_t css_compat_check(const char *css, const BrowserDetection *rules, bool dashboard,
                        CssCompatWarning *out, size_t max) {
    if (!css || !rules || max == 0) return 0;
    char *text = strip(css);
    if (!text) return 0;

    size_t count = 0;
    for (size_t i = 0; i < sizeof(FEATURES) / sizeof(FEATURES[0]) && count < max; i++) {
        const CssFeature *f = &FEATURES[i];
        if (!uses(text, f)) continue;

        CssCompatWarning *w = &out[count];
        w->feature = f->feature;
        w->still_served[0] = '\0';
        check_engine(rules, dashboard, "Chrome", f->chrome, w->still_served, sizeof(w->still_served));
        check_engine(rules, dashboard, "Firefox", f->firefox, w->still_served, sizeof(w->still_served));
        check_engine(rules, dashboard, "Safari", f->safari, w->still_served, sizeof(w->still_served));
        check_engine(rules, dashboard, "iOS", f->safari, w->still_served, sizeof(w->still_served));
        if (!w->still_served[0]) continue;

        snprintf(w->needs, sizeof(w->needs), "Chrome %s, Firefox %s, Safari %s",
                 f->chrome, f->firefox, f->safari);
        count++;
    }
    free(text);
    return count;
}
