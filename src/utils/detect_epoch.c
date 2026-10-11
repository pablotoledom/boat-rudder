#include "detect_epoch.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// ── Factory rules ────────────────────────────────────────────────────────────
//
// What every site runs until an admin saves rules of its own, and what
// "Restore factory rules" goes back to. Written as compact data (NULL-ended
// token lists) and expanded into a BrowserDetection by
// browser_detection_factory().
//
// The epoch 3 floors are set by the newest CSS the public stylesheet relies
// on: `gap` on flex containers (Chrome 84, Firefox 63, Safari 14.1) and
// `inset` (Chrome 87, Firefox 66, Safari 14.1), both in styles_epoch3.css.
// Below them the page falls apart (custom properties, older still, take every
// color with them), so those browsers get epoch 2. The dashboard floors are
// its own stylesheet's: `:has()` and `color-mix()` (Chrome 111, Firefox 121,
// Safari 16.2).
//
// Order matters - most modern UAs carry several engines' tokens: MSIE,
// Trident and EdgeHTML's "Edge/" (which also says Chrome/) come before
// Chrome; iOS before everything WebKit, since every iOS browser is the system
// WebKit and only Safari says "Version/" (CriOS/FxiOS don't), so the OS
// version is the engine's.

#define TOKENS(...) (const char *const[]){ __VA_ARGS__, NULL }
#define NO_TOKENS   (const char *const[]){ NULL }

typedef struct {
    const char *name;
    const char *const *contains;
    const char *const *excludes;
    const char *version_after;
    BrowserTier tiers[BROWSER_TIERS_MAX];
    size_t tier_count;
    int epoch;
    bool dashboard;
} FactoryRule;

static const FactoryRule FACTORY_RULES[] = {
    // WAP 1.x (~1999-2005): the Openwave/Unwired Planet UP.Browser, Nokia's
    // own WAP browser (also in lower case: a real Nokia 6233 on UC Browser
    // sends "nokia6233/..."), Obigo, and the manufacturer-prefix firmware
    // conventions of that generation ("SIE-" Siemens, "SEC-" Samsung, "MOT-"
    // Motorola, "LG-", Ericsson also matching SonyEricsson, and the brands
    // written in caps). Several are still-shipping brands whose Android UAs
    // would collide - excluded by "AppleWebKit", which every modern mobile
    // browser carries and no WAP 1.x one ever did. UP.Browser 3.x is excluded
    // too: it speaks only HDML, never WML, so it is not served at all (it falls
    // to the fallback, which it can't render either) - 4.x to 7.x are WML 1.1.
    { "WAP 1.x",
      TOKENS("Wap", "WAP", "Nokia", "nokia", "UP.Browser", "UP/", "Obigo", "Ericsson",
             "SIE-", "SEC-", "MOT-", "LG-", "Alcatel", "Panasonic", "SANYO", "SHARP",
             "PHILIPS"),
      TOKENS("AppleWebKit", "UP.Browser/3", "UP/3"), "", { { "", 0, false } }, 0, EPOCH_WML, false },

    // Text-mode browsers. Cello is a 1993 graphical browser that is text-only
    // in practice; its pre-Unicode charset is handled by request_charset.c.
    { "Text browsers",
      TOKENS("Lynx", "Cello", "Line Mode Browser", "ELinks", "w3m"),
      NO_TOKENS, "", { { "", 0, false } }, 0, EPOCH_PRESTANDARD, false },

    // MSIE 4+ has CSS1, a:hover and JavaScript - all of epoch 2. Opera 5-9
    // masquerading as MSIE lands here too, and is just as able.
    { "Internet Explorer",
      TOKENS("MSIE "), NO_TOKENS, "MSIE ",
      { { "4", EPOCH_MIDDLE, false } }, 1, EPOCH_EARLY, false },

    { "Internet Explorer 11",
      TOKENS("Trident/"), NO_TOKENS, "", { { "", 0, false } }, 0, EPOCH_MIDDLE, false },

    // EdgeHTML (Edge 12-18): carries Chrome/, never got flex gap or `inset`.
    // Chromium Edge writes "Edg/", which this does not match.
    { "EdgeHTML",
      TOKENS("Edge/"), NO_TOKENS, "", { { "", 0, false } }, 0, EPOCH_MIDDLE, false },

    // "CPU iPhone OS 14_5" / "CPU OS 14_5" (iPad). Safari 14.1 shipped with
    // iOS 14.5, Safari 16.2 with iOS 16.2. iPhone OS 1 wrote no version.
    { "iOS",
      TOKENS("iPhone", "iPad", "iPod"), NO_TOKENS, " OS ",
      { { "14.5", EPOCH_MODERN, false }, { "16.2", EPOCH_MODERN, true } }, 2,
      EPOCH_MIDDLE, false },

    { "Firefox",
      TOKENS("Firefox/"), NO_TOKENS, "Firefox/",
      { { "66", EPOCH_MODERN, false }, { "121", EPOCH_MODERN, true } }, 2,
      EPOCH_MIDDLE, false },

    // Chrome/Chromium and everything built on it (Opera 15+, Samsung
    // Internet, Chromium Edge, Android WebView).
    { "Chrome",
      TOKENS("Chrome/"), NO_TOKENS, "Chrome/",
      { { "87", EPOCH_MODERN, false }, { "111", EPOCH_MODERN, true } }, 2,
      EPOCH_MIDDLE, false },

    // Safari, and the pre-Chrome Android browser (also "Version/... Safari/").
    // Safari 1-2 wrote no "Version/".
    { "Safari",
      TOKENS("Safari/"), NO_TOKENS, "Version/",
      { { "14.1", EPOCH_MODERN, false }, { "16.2", EPOCH_MODERN, true } }, 2,
      EPOCH_MIDDLE, false },

    // Opera 5-12 (Presto) under its own name: "Opera/9.80 (...)".
    { "Opera (Presto)",
      TOKENS("Opera/"), NO_TOKENS, "Opera/",
      { { "5", EPOCH_MIDDLE, false } }, 1, EPOCH_EARLY, false },

    // "Netscape6/6.2", "Netscape/7.1"; no version -> epoch 1.
    { "Netscape",
      TOKENS("Netscape"), NO_TOKENS, "Netscape",
      { { "4", EPOCH_MIDDLE, false } }, 1, EPOCH_EARLY, false },

    { "Mosaic",
      TOKENS("Mosaic"), NO_TOKENS, "", { { "", 0, false } }, 0, EPOCH_EARLY, false },

    // A Mozilla/3+ string none of the engines above claimed (Netscape 3-4,
    // Konqueror/KHTML, an old Gecko without Firefox/, a crawler): an unknown
    // engine, so epoch 2, which renders everywhere.
    { "Generic Mozilla",
      TOKENS("Mozilla/"), NO_TOKENS, "Mozilla/",
      { { "3", EPOCH_MIDDLE, false } }, 1, EPOCH_EARLY, false },
};

#define FACTORY_FALLBACK_EPOCH EPOCH_EARLY  // better an old-but-working page

typedef struct {
    const char *name;
    const char *user_agent;
    int epoch;
    bool dashboard;
} FactoryCase;

// One real User-Agent per browser family and version boundary.
static const FactoryCase FACTORY_CASES[] = {
    { "No User-Agent", "", EPOCH_EARLY, false },
    { "Unknown token", "SomeBot", EPOCH_EARLY, false },

    { "Nokia 7110", "Nokia7110/1.0 (05.01)", EPOCH_WML, false },
    { "UP.Browser 4", "SIE-S35/1.0 UP/4.1.8c UP.Browser/4.1.8c-XXXX UP.Link/4.2.2.1", EPOCH_WML, false },
    { "Ericsson R380", "R380 2.0 WAP1.1", EPOCH_WML, false },
    { "UP.Browser 3 (HDML only, not served)", "UP.Browser/3.1-UPG1 UP.Link/3.2", EPOCH_EARLY, false },
    { "UP.Browser 6.1", "SAMSUNG-SGH-E700/BSI UP.Browser/6.1.0.6 (GUI) MMP/1.0", EPOCH_WML, false },
    { "Nokia 6233, UC Browser", "nokia6233/2.0 (05.60) Profile/MIDP-2.0 Configuration/CLDC-1.1 UCWEB/7.9", EPOCH_WML, false },
    { "Nokia Android (WebKit, not WAP)",
      "Mozilla/5.0 (Linux; Android 13; Nokia G21) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
      EPOCH_MODERN, true },

    { "Lynx", "Lynx/2.9.0dev.12 libwww-FM/2.14 SSL-MM/1.4.1 GNUTLS/3.7.8", EPOCH_PRESTANDARD, false },
    { "w3m", "w3m/0.5.3+git20230121", EPOCH_PRESTANDARD, false },
    { "ELinks", "ELinks/0.13.2 (textmode; Linux 6.1.0 x86_64; 80x24-2)", EPOCH_PRESTANDARD, false },

    { "NCSA Mosaic 2.1", "NCSA_Mosaic/2.1 (Windows x86)", EPOCH_EARLY, false },
    { "Netscape 2", "Mozilla/2.02 (Win95; I)", EPOCH_EARLY, false },
    { "Netscape 3", "Mozilla/3.01 (Win95; I)", EPOCH_MIDDLE, false },
    { "Netscape 4.79", "Mozilla/4.79 [en] (X11; U; Linux 2.4.20 i686)", EPOCH_MIDDLE, false },
    { "Netscape 7", "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.4) Gecko/20030624 Netscape/7.1", EPOCH_MIDDLE, false },

    { "MSIE 3", "Mozilla/2.0 (compatible; MSIE 3.02; Windows 95)", EPOCH_EARLY, false },
    { "MSIE 4", "Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)", EPOCH_MIDDLE, false },
    { "MSIE 6", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)", EPOCH_MIDDLE, false },
    { "IE 11", "Mozilla/5.0 (Windows NT 10.0; Trident/7.0; rv:11.0) like Gecko", EPOCH_MIDDLE, false },
    { "EdgeHTML 18",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.102 Safari/537.36 Edge/18.19041",
      EPOCH_MIDDLE, false },
    { "Chromium Edge",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.2210.91",
      EPOCH_MODERN, true },

    { "Opera 6 as MSIE", "Mozilla/4.0 (compatible; MSIE 5.0; Windows 98) Opera 6.0 [en]", EPOCH_MIDDLE, false },
    { "Opera 12 (Presto)", "Opera/9.80 (Windows NT 6.1; WOW64) Presto/2.12.388 Version/12.18", EPOCH_MIDDLE, false },
    { "Opera 106 (Chromium)",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 OPR/106.0.0.0",
      EPOCH_MODERN, true },

    { "Firefox 3.6", "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.28) Gecko/20120306 Firefox/3.6.28", EPOCH_MIDDLE, false },
    { "Firefox 52 ESR", "Mozilla/5.0 (Windows NT 6.1; rv:52.0) Gecko/20100101 Firefox/52.0", EPOCH_MIDDLE, false },
    { "Firefox 65", "Mozilla/5.0 (X11; Linux x86_64; rv:65.0) Gecko/20100101 Firefox/65.0", EPOCH_MIDDLE, false },
    { "Firefox 66", "Mozilla/5.0 (X11; Linux x86_64; rv:66.0) Gecko/20100101 Firefox/66.0", EPOCH_MODERN, false },
    { "Firefox 115 ESR", "Mozilla/5.0 (X11; Linux x86_64; rv:115.0) Gecko/20100101 Firefox/115.0", EPOCH_MODERN, false },
    { "Firefox 128", "Mozilla/5.0 (X11; Linux x86_64; rv:128.0) Gecko/20100101 Firefox/128.0", EPOCH_MODERN, true },

    { "Android 4.4 WebView",
      "Mozilla/5.0 (Linux; Android 4.4.2; Nexus 5 Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/30.0.0.0 Mobile Safari/537.36",
      EPOCH_MIDDLE, false },
    { "Chrome 86",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.198 Safari/537.36",
      EPOCH_MIDDLE, false },
    { "Chrome 87",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.66 Safari/537.36",
      EPOCH_MODERN, false },
    { "Chrome 110",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.5481.178 Safari/537.36",
      EPOCH_MODERN, false },
    { "Chrome 111",
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/111.0.5563.65 Safari/537.36",
      EPOCH_MODERN, true },
    { "Samsung Internet 23",
      "Mozilla/5.0 (Linux; Android 14; SM-S918B) AppleWebKit/537.36 (KHTML, like Gecko) SamsungBrowser/23.0 Chrome/115.0.0.0 Mobile Safari/537.36",
      EPOCH_MODERN, true },

    { "Safari 3", "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_8; en-us) AppleWebKit/531.22.7 (KHTML, like Gecko) Version/3.2.3 Safari/525.28.3", EPOCH_MIDDLE, false },
    { "Safari 2 (no Version/)", "Mozilla/5.0 (Macintosh; U; PPC Mac OS X; en) AppleWebKit/418.8 (KHTML, like Gecko) Safari/419.3", EPOCH_MIDDLE, false },
    { "Safari 14.0",
      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0.3 Safari/605.1.15",
      EPOCH_MIDDLE, false },
    { "Safari 14.1",
      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1 Safari/605.1.15",
      EPOCH_MODERN, false },
    { "Safari 17",
      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Safari/605.1.15",
      EPOCH_MODERN, true },
    { "Android 2.3 browser",
      "Mozilla/5.0 (Linux; U; Android 2.3.6; en-us; Nexus S Build/GRK39F) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1",
      EPOCH_MIDDLE, false },
    { "Konqueror 4", "Mozilla/5.0 (compatible; Konqueror/4.14; Linux) KHTML/4.14.2 (like Gecko)", EPOCH_MIDDLE, false },

    { "iPhone OS 3", "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16", EPOCH_MIDDLE, false },
    { "iOS 14.4 Safari",
      "Mozilla/5.0 (iPhone; CPU iPhone OS 14_4 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0.3 Mobile/15E148 Safari/604.1",
      EPOCH_MIDDLE, false },
    { "iOS 14.5 Safari",
      "Mozilla/5.0 (iPhone; CPU iPhone OS 14_5 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1 Mobile/15E148 Safari/604.1",
      EPOCH_MODERN, false },
    { "iOS 17 Chrome (CriOS)",
      "Mozilla/5.0 (iPhone; CPU iPhone OS 17_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) CriOS/120.0.6099.119 Mobile/15E148 Safari/604.1",
      EPOCH_MODERN, true },
    { "iOS 13 Firefox (FxiOS)",
      "Mozilla/5.0 (iPhone; CPU iPhone OS 13_3 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) FxiOS/22.0 Mobile/15E148 Safari/605.1.15",
      EPOCH_MIDDLE, false },
    { "iPadOS 12",
      "Mozilla/5.0 (iPad; CPU OS 12_5 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/12.1.2 Mobile/15E148 Safari/604.1",
      EPOCH_MIDDLE, false },

    { "Googlebot (legacy UA)", "Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)", EPOCH_MIDDLE, false },
    { "Googlebot (evergreen)",
      "Mozilla/5.0 AppleWebKit/537.36 (KHTML, like Gecko; compatible; Googlebot/2.1; +http://www.google.com/bot.html) Chrome/120.0.6099.216 Safari/537.36",
      EPOCH_MODERN, true },
};

static void copy_str(char *dst, size_t size, const char *src) {
    size_t n = strlen(src);
    if (n >= size) n = size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static size_t copy_tokens(char out[][BROWSER_TOKEN_MAX], const char *const *tokens) {
    size_t n = 0;
    for (; tokens[n] && n < BROWSER_TOKENS_MAX; n++)
        copy_str(out[n], BROWSER_TOKEN_MAX, tokens[n]);
    return n;
}

BrowserDetection *browser_detection_factory(void) {
    BrowserDetection *d = calloc(1, sizeof(*d));
    if (!d) return NULL;

    d->fallback_epoch = FACTORY_FALLBACK_EPOCH;

    d->rule_count = sizeof(FACTORY_RULES) / sizeof(FACTORY_RULES[0]);
    for (size_t i = 0; i < d->rule_count; i++) {
        const FactoryRule *f = &FACTORY_RULES[i];
        BrowserRule *r = &d->rules[i];
        copy_str(r->name, sizeof(r->name), f->name);
        r->enabled = true;
        r->contains_count = copy_tokens(r->contains, f->contains);
        r->excludes_count = copy_tokens(r->excludes, f->excludes);
        copy_str(r->version_after, sizeof(r->version_after), f->version_after);
        r->tier_count = f->tier_count;
        memcpy(r->tiers, f->tiers, sizeof(r->tiers));
        r->epoch = f->epoch;
        r->dashboard = f->dashboard;
    }

    d->case_count = sizeof(FACTORY_CASES) / sizeof(FACTORY_CASES[0]);
    for (size_t i = 0; i < d->case_count; i++) {
        BrowserCase *c = &d->cases[i];
        copy_str(c->name, sizeof(c->name), FACTORY_CASES[i].name);
        copy_str(c->user_agent, sizeof(c->user_agent), FACTORY_CASES[i].user_agent);
        c->epoch = FACTORY_CASES[i].epoch;
        c->dashboard = FACTORY_CASES[i].dashboard;
    }
    return d;
}

// ── Versions ─────────────────────────────────────────────────────────────────

typedef struct { long major, minor; } Version;

// "major[.minor]" or iOS's "major_minor" at `p`. 0 if `p` holds no digit.
static int parse_version(const char *p, Version *out) {
    if (*p < '0' || *p > '9') return 0;
    char *end;
    out->major = strtol(p, &end, 10);
    out->minor = 0;
    if ((*end == '.' || *end == '_') && end[1] >= '0' && end[1] <= '9')
        out->minor = strtol(end + 1, NULL, 10);
    return 1;
}

static int version_cmp(const Version *a, const Version *b) {
    if (a->major != b->major) return a->major < b->major ? -1 : 1;
    if (a->minor != b->minor) return a->minor < b->minor ? -1 : 1;
    return 0;
}

// A tier's `from`: a version and nothing else.
static int parse_tier_version(const char *s, Version *out) {
    if (!parse_version(s, out)) return 0;
    const char *p = s;
    while (*p >= '0' && *p <= '9') p++;
    if (*p == '.' || *p == '_') {
        p++;
        if (*p < '0' || *p > '9') return 0;
        while (*p >= '0' && *p <= '9') p++;
    }
    return *p == '\0';
}

// ── Validation ───────────────────────────────────────────────────────────────

static int epoch_valid(int epoch) {
    return epoch >= EPOCH_WML && epoch <= EPOCH_MODERN;
}

static int has_nul(const char *s, size_t size) {
    return memchr(s, '\0', size) != NULL;
}

// Printable text only: a control character in a token would never match a
// real header, and has no business in the page that edits it.
static int text_ok(const char *s) {
    for (; *s; s++)
        if ((unsigned char)*s < 0x20 || *s == 0x7f) return 0;
    return 1;
}

static const char *validate_tokens(const char list[][BROWSER_TOKEN_MAX], size_t count) {
    if (count > BROWSER_TOKENS_MAX) return "too many tokens in a list";
    for (size_t i = 0; i < count; i++) {
        if (!has_nul(list[i], BROWSER_TOKEN_MAX)) return "a token is too long";
        if (!list[i][0]) return "a token is empty";
        if (!text_ok(list[i])) return "a token has a control character";
    }
    return NULL;
}

const char *browser_detection_validate(const BrowserDetection *d, size_t *position) {
    size_t unused;
    if (!position) position = &unused;
    *position = 0;

    if (!epoch_valid(d->fallback_epoch)) return "the fallback epoch is not -1..3";
    if (d->rule_count > BROWSER_RULES_MAX) return "too many rules";
    if (d->case_count > BROWSER_CASES_MAX) return "too many test cases";

    for (size_t i = 0; i < d->rule_count; i++) {
        const BrowserRule *r = &d->rules[i];
        const char *error = NULL;
        *position = i + 1;

        if (!has_nul(r->name, sizeof(r->name)) || !text_ok(r->name))
            return "a rule name is too long or has a control character";
        if (r->contains_count == 0) return "a rule has nothing to match (contains is empty)";
        if ((error = validate_tokens(r->contains, r->contains_count))) return error;
        if ((error = validate_tokens(r->excludes, r->excludes_count))) return error;
        if (!has_nul(r->version_after, sizeof(r->version_after)) || !text_ok(r->version_after))
            return "version_after is too long or has a control character";
        if (!epoch_valid(r->epoch)) return "a rule's epoch is not -1..3";
        if (r->tier_count > BROWSER_TIERS_MAX) return "too many tiers";
        if (r->tier_count > 0 && !r->version_after[0])
            return "tiers need a version_after token to read the version from";
        for (size_t t = 0; t < r->tier_count; t++) {
            Version v;
            if (!has_nul(r->tiers[t].from, sizeof(r->tiers[t].from)) ||
                !parse_tier_version(r->tiers[t].from, &v))
                return "a tier's version is not a number like 87 or 14.1";
            if (!epoch_valid(r->tiers[t].epoch)) return "a tier's epoch is not -1..3";
        }
    }

    for (size_t i = 0; i < d->case_count; i++) {
        const BrowserCase *c = &d->cases[i];
        *position = i + 1;
        if (!has_nul(c->name, sizeof(c->name)) || !text_ok(c->name))
            return "a test case name is too long or has a control character";
        if (!has_nul(c->user_agent, sizeof(c->user_agent)) || !text_ok(c->user_agent))
            return "a test case User-Agent is too long or has a control character";
        if (!epoch_valid(c->epoch)) return "a test case's epoch is not -1..3";
    }

    *position = 0;
    return NULL;
}

// ── Classification ───────────────────────────────────────────────────────────

static int contains_any(const char *ua, const char list[][BROWSER_TOKEN_MAX], size_t count) {
    for (size_t i = 0; i < count; i++)
        if (strstr(ua, list[i])) return 1;
    return 0;
}

static void classify_rule(const BrowserRule *r, const char *ua, BrowserMatch *out) {
    out->epoch = r->epoch;
    out->dashboard = r->dashboard;
    if (!r->version_after[0] || r->tier_count == 0) return;

    const char *at = strstr(ua, r->version_after);
    if (!at) return;
    at += strlen(r->version_after);
    if (*at == '/' || *at == ' ') at++;

    Version version;
    if (!parse_version(at, &version)) return;

    const BrowserTier *best = NULL;
    Version best_from = { 0, 0 };
    for (size_t t = 0; t < r->tier_count; t++) {
        Version from;
        if (!parse_tier_version(r->tiers[t].from, &from)) continue;
        if (version_cmp(&version, &from) >= 0 && (!best || version_cmp(&from, &best_from) > 0)) {
            best = &r->tiers[t];
            best_from = from;
        }
    }
    if (best) {
        out->epoch = best->epoch;
        out->dashboard = best->dashboard;
    }
}

void browser_detection_classify(const BrowserDetection *d, const char *user_agent,
                                BrowserMatch *out) {
    const char *ua = user_agent ? user_agent : "";

    for (size_t i = 0; i < d->rule_count; i++) {
        const BrowserRule *r = &d->rules[i];
        if (!r->enabled) continue;
        if (!contains_any(ua, r->contains, r->contains_count)) continue;
        if (contains_any(ua, r->excludes, r->excludes_count)) continue;
        out->rule = (int)i;
        classify_rule(r, ua, out);
        return;
    }
    out->rule = -1;
    out->epoch = d->fallback_epoch;
    out->dashboard = false;
}

// ── The active rule set ──────────────────────────────────────────────────────

static pthread_rwlock_t active_lock = PTHREAD_RWLOCK_INITIALIZER;
static BrowserDetection *active = NULL;     // NULL until first use: factory

// Under the write lock: installs the factory rules if nothing is active.
// Called with no lock held; returns 0, -1 on allocation failure.
static int ensure_active(void) {
    pthread_rwlock_rdlock(&active_lock);
    int ready = active != NULL;
    pthread_rwlock_unlock(&active_lock);
    if (ready) return 0;

    BrowserDetection *factory = browser_detection_factory();
    if (!factory) return -1;
    pthread_rwlock_wrlock(&active_lock);
    if (!active) {
        active = factory;
        factory = NULL;
    }
    pthread_rwlock_unlock(&active_lock);
    free(factory);
    return 0;
}

int browser_detection_activate(BrowserDetection *d) {
    if (!d) d = browser_detection_factory();
    if (!d || browser_detection_validate(d, NULL)) {
        free(d);
        return -1;
    }
    pthread_rwlock_wrlock(&active_lock);
    BrowserDetection *previous = active;
    active = d;
    pthread_rwlock_unlock(&active_lock);
    free(previous);
    return 0;
}

BrowserDetection *browser_detection_active_copy(void) {
    if (ensure_active() != 0) return NULL;
    BrowserDetection *copy = malloc(sizeof(*copy));
    if (!copy) return NULL;
    pthread_rwlock_rdlock(&active_lock);
    memcpy(copy, active, sizeof(*copy));
    pthread_rwlock_unlock(&active_lock);
    return copy;
}

void detect_browser(const char *user_agent, BrowserMatch *out) {
    if (ensure_active() != 0) {
        // Out of memory before any rule set exists: the factory fallback.
        out->epoch = FACTORY_FALLBACK_EPOCH;
        out->dashboard = false;
        out->rule = -1;
        return;
    }
    pthread_rwlock_rdlock(&active_lock);
    browser_detection_classify(active, user_agent, out);
    pthread_rwlock_unlock(&active_lock);
}

int detect_epoch(const char *user_agent) {
    BrowserMatch match;
    detect_browser(user_agent, &match);
    return match.epoch;
}

bool detect_dashboard(const char *user_agent) {
    BrowserMatch match;
    detect_browser(user_agent, &match);
    return match.dashboard;
}

int epoch_to_index(int epoch) {
    if (epoch < EPOCH_WML || epoch > EPOCH_MODERN) return -1;
    return epoch + 1;
}
