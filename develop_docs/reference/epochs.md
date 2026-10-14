# Boat Rudder - Epoch Technical Specification

What each epoch actually is - protocol, encoding, markup, CSS and JavaScript level - as measured
from the templates, stylesheets and scripts that render it, and which browsers are sent to it.
This is the reference to check a template change against: a new tag, attribute, CSS feature or
script construct must fit its epoch's column here, or this table (and the detection rules that
route browsers to the epoch) must change with it.

How an epoch is picked: [rendering.md](rendering.md#epochs). The rules themselves, editable from
the dashboard: [dashboard.md](dashboard.md) (`/dashboard/settings/browsers`) and
[data-model.md](data-model.md#browser_detection). Browser lists below are the **factory rules**
(`src/utils/detect_epoch.c`); a site may have saved others.

"Full fidelity from" is the oldest browser that supports *everything* the epoch uses. Older
browsers sent to the same epoch get a degraded but readable page - see each epoch's last row.

## At a glance

| Epoch | Years | Technology | Example browsers |
|---|---|---|---|
| **−1** WAP | 1999-2005 | WML 1.1 · Latin-1 / WBXML · no CSS, no JS | Nokia 7110, UP.Browser 4-7 |
| **0** Text | 1991-1995 (and today's text browsers) | HTTP/0.9+ · HTML 2.0 subset · no CSS, no JS | Line Mode Browser, Lynx |
| **1** Early | 1993-1996 | HTML 3.2 (+ `font face`, cell `bgcolor`) · Latin-1 · tables · no CSS, no JS | NCSA Mosaic, Netscape 2 |
| **2** Middle | 1997-2006 (and modern browsers older than 2021) | HTML 4.01 Transitional · inline CSS1 (+ `a:hover`) · JavaScript 1.0 | Netscape 4, Internet Explorer 5 |
| **3** Modern | 2021-today | HTML5 · CSS of 2020-21 (grid, flex `gap`, `inset`) · ES2015 | Chrome 87+, Firefox 66+, Safari 14.1+ |
| **3** Dashboard | 2023-today | HTML5 · CSS of 2023 (`:has()`, `color-mix()`) · ES5 + `fetch` | Chrome 111+, Firefox 121+, Safari 16.2+ |

The years are those of the browsers the epoch is built for. Two epochs also take browsers from
other eras: epoch 0 today's text browsers (Lynx, w3m), and epoch 2 every modern browser below
epoch 3's floors (Chrome 86 is from October 2020).

---

## Epoch −1 - WAP (`EPOCH_WML`)

| | |
|---|---|
| Protocol | HTTP/1.x; or WSP/WTP over UDP through the built-in WAP gateway ([wap-gateway.md](wap-gateway.md)) |
| Transport | `text/vnd.wap.wml`, body ISO-8859-1. Through the gateway: WBXML 1.1 (public id `0x04`, charset UTF-8) |
| Prolog | `<?xml version="1.0" encoding="ISO-8859-1"?>` + `-//WAPFORUM//DTD WML 1.1//EN` |
| Markup | **WML 1.1**, minimal subset: `wml`, `card(id,title)`, `p(align)`, `a(href)`, `br`, `b`, `img(src,alt)`. No `do`, `go`, `onevent`, `timer`, forms or variables |
| Layout | One `<card>` per page, paginated to 860-byte pages |
| CSS / JS | none / none (no WMLScript) |
| Images | WBMP (a PNG twin exists only for the dashboard's WAP preview) |
| Full fidelity from | Any WAP 1.1 browser (Nokia 7110, UP.Browser 4) |
| Sent here | WAP tokens (Nokia, UP.Browser, Obigo, `SIE-`, `SEC-`, `MOT-`, `LG-`, Ericsson, ...) **without** `AppleWebKit`. UP.Browser 3.x is excluded: it speaks only HDML, which no epoch serves |

## Epoch 0 - Text (`EPOCH_PRESTANDARD`)

| | |
|---|---|
| Protocol | **HTTP/0.9 and up.** A genuine HTTP/0.9 request (no version, no headers) is answered with an RFC 1945 Simple-Response: the document alone, no status line or headers ([rendering.md](rendering.md#legacy-http-compatibility)) |
| Transport | `text/html; charset=UTF-8`. Cello, the CERN Line Mode Browser and HTTP/0.9 clients: bare `text/html` and a Latin-1 body |
| DOCTYPE | none |
| Markup | **Strict HTML 2.0 subset**: `html`, `head`, `title`, `body`, `h1`-`h6`, `p`, `a(href,name)`, `hr`, `br`, `ul`/`li`, `pre`, `b`, `i`. No presentational attributes. Entities: `&lt;` |
| Layout | Linear flow, no tables |
| CSS / JS | none / none |
| Images | none - videos, images and galleries become a QR code drawn in text (Unicode half blocks; plain ASCII for the Latin-1 readers) with a `/qr/<code>` short link |
| Full fidelity from | Lynx 2.x, Cello 1.0, Line Mode Browser |
| Sent here | Lynx, w3m, ELinks, Cello, the CERN Line Mode Browser (`LineMode/`, `W3CLineMode/`), and any HTTP/0.9 request (it has no User-Agent to classify) |
| Older | A browser older than HTML 2.0 (WorldWideWeb, ViolaWWW, Erwise) still reads it: unknown tags are ignored and their text shown, so the tags beyond the 1991-92 set (`html`/`head`/`body`, `hr`, `br`, `b`, `i`, `pre`) cost formatting, never content |

## Epoch 1 - Early (`EPOCH_EARLY`)

| | |
|---|---|
| Protocol | HTTP/1.0 and up |
| Transport | bare `text/html` - no `charset` parameter, which NCSA Mosaic would not match against its MIME viewers - and a Latin-1 body |
| DOCTYPE | none |
| Markup | **HTML 3.2** (`table`/`tr`/`td` with `border`, `cellpadding`, `cellspacing`, `width`, `height`, `valign`; `font size/color`; `body bgcolor/text/link/vlink/alink`; `img hspace/vspace/width/height/align`; `br clear`; `p align`; `u`) plus two **HTML 4.0 Transitional** attributes: `font face` and cell `bgcolor` |
| Layout | Tables |
| CSS / JS | none / none |
| Images | GIF only (the `_micro` variant) |
| Full fidelity from | **Netscape 3 / MSIE 3** |
| Sent here | Mosaic, Netscape 1-2, MSIE ≤ 3, Opera ≤ 4, requests with no User-Agent, and anything no rule matches (the fallback) |
| Older | Netscape 1.1-2 lose fonts and cell colors; Mosaic 2.x table support varies by build |

## Epoch 2 - Middle (`EPOCH_MIDDLE`)

| | |
|---|---|
| Protocol | HTTP/1.0 and up |
| Transport | `text/html; charset=UTF-8`, matched by `<meta http-equiv="Content-Type">` |
| DOCTYPE | `-//W3C//DTD HTML 4.01 Transitional//EN` with no system identifier - modern browsers render it in quirks mode, on purpose |
| Markup | **HTML 4.01 Transitional** plus period extensions - IE: `bordercolor`, `hr color`, `body leftmargin/topmargin`, `rel="shortcut icon"`; Netscape: `body marginwidth/marginheight`; both: `background` on `table`/`td`. `style` and `lang` attributes, `basefont face` |
| Layout | Tables; presentation in attributes, never in CSS box properties (Netscape 4 misapplies them - [rendering.md](rendering.md#epochs)) |
| CSS | **CSS1** in one inline `<style>` (element, `#id` and `.class` selectors; fonts, `line-height`, margins, `text-decoration`) + **CSS2**: `a:hover`, `max-height`, `overflow: scroll` + IE's `expression()`. No external stylesheet |
| JS | **JavaScript 1.0**: a single `onClick="location.href=..."` |
| Images | GIF and baseline JPEG |
| Full fidelity from | **MSIE 5-7** |
| Sent here | Netscape 3-4, MSIE 4-11, EdgeHTML, Opera 5-12 (Presto), Konqueror, any `Mozilla/` string no engine claims, and every modern browser below epoch 3's floors: Firefox 4-65, Chrome ≤ 86, Safari < 14.1, iOS < 14.5, old Android WebViews |
| Older | Netscape 4 / MSIE 4: no hover color, no `bordercolor`; `expression()` (the code block's height) only on IE 5-7. Netscape 3: no CSS at all, attributes still apply |

## Epoch 3 - Modern (`EPOCH_MODERN`), public site

| | |
|---|---|
| Protocol | HTTP/1.1, optional TLS 1.2+ |
| Transport | `text/html; charset=UTF-8` |
| DOCTYPE | `<!DOCTYPE html>`, `<meta charset="UTF-8">`, `<meta name="viewport">` |
| Markup | **HTML5**: `main`, `section`, `article`, `figure`/`figcaption`, `footer`, `nav`, `data-*`, `aria-*`, `loading="lazy"`, `iframe allow/referrerpolicy` |
| Layout | Flexbox and grid, responsive (`@media` breakpoints 499 / 599 / 768 / 960 / 1200 px) |
| CSS | **CSS of 2020-21** (`styles_epoch3.css`): custom properties, grid, **`gap` on flex containers**, **`inset`**, `object-fit`, transitions/transforms, `rem`/`vw`/`vh`, `@font-face` (TTF) |
| JS | **ES2015** syntax (`const`/`let`, arrow functions, rest parameters) + DOM of 2016: `closest()`, `NodeList.forEach`, `Array.from`, `passive` listeners, `KeyboardEvent.key`, Clipboard API with an `execCommand` fallback |
| Images | JPEG, PNG, GIF, SVG |
| Full fidelity from | **Chrome/Edge 87, Firefox 66, Safari 14.1, iOS 14.5** - set by flex `gap` and `inset`. The JS alone would need only Chrome 51 / Firefox 52 / Safari 10 |
| Sent here | Those versions and up |

## Epoch 3 - Dashboard and login

| | |
|---|---|
| Markup | HTML5 + HTML5 forms (`color`, `date`, `range`, `email`, `number`), `details`/`summary`, `<template>` |
| CSS | **CSS of 2023** (`styles_admin_epoch3.css`): **`:has()`**, **`color-mix()`**, `:is()`, `accent-color`, `:focus-visible`, `-webkit-line-clamp` |
| JS | **ES5 syntax** (`var`/`function`) + modern APIs: `fetch`, `Promise`, `FormData`, `URLSearchParams`, `MutationObserver`, `DOMParser`, `replaceWith`, `template.content` |
| Full fidelity from | **Chrome/Edge 111, Firefox 121, Safari / iOS 16.2** |
| Access | Epoch 3 **and** the `dashboard` flag the rules give the real User-Agent (`dashboard_allowed()` in `http_router.c`; `?preview_epoch=3` cannot lift an older browser in). Other epoch 3 browsers see "needs a more recent browser" |

---

## Keeping it true

| Guard | What it catches |
|---|---|
| `tests/css_compat_test.c` | A shipped epoch 3 stylesheet (public or admin, every theme) using CSS that browsers the factory rules still serve it to lack |
| CSS editor notice / *Stylesheet check* | The same, live: on the theme CSS editor against the active rules, and on the Browser detection page against the rules being edited ([css_compat.h](../../src/utils/css_compat.h)) |
| `tests/detect_epoch_test.c` | Every factory test case - one real User-Agent per browser family and version boundary - getting its epoch and dashboard flag |
| Test cases on the Browser detection page | A rule edit changing what a known browser gets, before it is saved |

What is **not** checked automatically: the HTML and JavaScript levels. A template that adds a tag,
attribute or script construct above its epoch's row here is caught only by review against this
document.

When an epoch's level changes on purpose, change, in this order: the templates or stylesheet, the
factory rules and cases in `detect_epoch.c` (and `css_compat.c`'s feature table if a new CSS
feature is involved), this document, and the epoch tables in [boat-rudder.md](../boat-rudder.md)
and [rendering.md](rendering.md).
