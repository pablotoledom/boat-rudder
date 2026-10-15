#ifndef MARKDOWN_LITE_H
#define MARKDOWN_LITE_H

// The small Markdown subset CHANGELOG.md is written in, to HTML - for release
// notes shown in the dashboard (modules/updates_admin). Everything in `md` is
// HTML-escaped first; only these constructs produce markup:
//
//   # .. ######   headings, shifted down two levels (h3 .. h6) to sit under
//                 the page's own h1/h2
//   - * +         bullet items, one level; indented lines continue the item
//   blank line    ends a paragraph or an item
//   `code`  **bold**  *emphasis*  [text](url)
//
// A link URL is kept when it is http(s) and turned into text otherwise; a
// relative one ("develop_docs/x.md") is prefixed with `link_base` (e.g.
// "https://github.com/<repo>/blob/<tag>/"), or turned into text when
// `link_base` is NULL. Raw HTML, tables, code fences and nested lists come out
// as escaped text. Returns a malloc'd string, NULL on allocation failure.
char *markdown_to_html(const char *md, const char *link_base);

#endif // MARKDOWN_LITE_H
