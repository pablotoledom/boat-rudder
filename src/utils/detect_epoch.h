#ifndef DETECT_EPOCH_H
#define DETECT_EPOCH_H

#include <stdbool.h>
#include <stddef.h>

// Browser "epochs" used to select retro-compatible HTML templates. Which
// browsers land in each is data - the rules below - not fixed here; the
// notes are what the factory rules do.
#define EPOCH_WML         (-1)  // WAP 1.x devices (WML/WAP browsers)
#define EPOCH_PRESTANDARD   0   // Text-only browsers (Lynx, w3m, ...)
#define EPOCH_EARLY         1   // Mosaic, Netscape 1-2, IE <=3 (no CSS/JS)
#define EPOCH_MIDDLE        2   // Netscape 3-4, IE 4-11, EdgeHTML, Opera 5-12,
                                // and anything below epoch 3's floors
#define EPOCH_MODERN        3   // Chrome 87+, Firefox 66+, Safari 14.1+ / iOS 14.5+

// Number of epochs (-1..3 inclusive) - the size of any array indexed by
// epoch_to_index(), e.g. a theme's per-epoch banner/footer values.
#define EPOCH_COUNT 5

// Maps epoch (-1..3) to a 0..4 array index, or -1 if `epoch` is out of
// that range. Was cms_site_settings_epoch_index() (site-personalization-
// plan.md) before banner/footer moved to being theme-scoped - moved here
// because it maps epochs generically, not anything specific to site
// settings or themes.
int epoch_to_index(int epoch);

// ── Detection rules ──────────────────────────────────────────────────────────
//
// An ordered list of rules (develop_docs/plans/browser-detection-admin-plan.md).
// The first rule whose `contains` has a token found in the User-Agent and whose
// `excludes` has none wins; matching is case-sensitive substring, no patterns.
// If it names `version_after`, the version right after that token (an optional
// '/' or ' ' skipped; "14.1", "14_5", a missing minor is 0) picks the tier with
// the highest `from` it reaches; no version, or no tier reached, gives the
// rule's own epoch/dashboard. No rule matching gives `fallback_epoch`.
//
// `dashboard` is whether the browser may use /login and /dashboard, whose
// stylesheet needs more than the public epoch 3 one.

#define BROWSER_NAME_MAX      64    // bytes, including the NUL
#define BROWSER_TOKEN_MAX     64
#define BROWSER_VERSION_MAX   16
#define BROWSER_UA_MAX        1024
#define BROWSER_TOKENS_MAX    32    // per list
#define BROWSER_TIERS_MAX     8     // per rule
#define BROWSER_RULES_MAX     64
#define BROWSER_CASES_MAX     300

typedef struct {
    char from[BROWSER_VERSION_MAX];     // "87", "14.1"
    int  epoch;
    bool dashboard;
} BrowserTier;

typedef struct {
    char name[BROWSER_NAME_MAX];
    bool enabled;
    char contains[BROWSER_TOKENS_MAX][BROWSER_TOKEN_MAX];
    size_t contains_count;
    char excludes[BROWSER_TOKENS_MAX][BROWSER_TOKEN_MAX];
    size_t excludes_count;
    char version_after[BROWSER_TOKEN_MAX];  // "" = no version, no tiers
    BrowserTier tiers[BROWSER_TIERS_MAX];
    size_t tier_count;
    int  epoch;                             // when no tier applies
    bool dashboard;
} BrowserRule;

// A User-Agent and what it must be classified as - run against the rules
// on every save from the dashboard, and by tests/detect_epoch_test.c.
typedef struct {
    char name[BROWSER_NAME_MAX];
    char user_agent[BROWSER_UA_MAX];
    int  epoch;
    bool dashboard;
} BrowserCase;

typedef struct {
    int fallback_epoch;
    BrowserRule rules[BROWSER_RULES_MAX];
    size_t rule_count;
    BrowserCase cases[BROWSER_CASES_MAX];
    size_t case_count;
} BrowserDetection;

typedef struct {
    int  epoch;
    bool dashboard;
    int  rule;          // index into rules[], -1 = fallback
} BrowserMatch;

// A new heap BrowserDetection holding the factory rules and cases - what is
// in effect while nothing is stored. NULL on allocation failure. Free with
// free().
BrowserDetection *browser_detection_factory(void);

// Checks `d` against the limits and shapes above (epochs in -1..3, tokens
// non-empty, tiers only with a version_after and a numeric `from`, ...).
// NULL when valid, otherwise a static message naming the first problem,
// with its rule/case position written to `*position` (1-based, 0 = none).
const char *browser_detection_validate(const BrowserDetection *d, size_t *position);

// Classifies `user_agent` (NULL or "" is treated as "") by `d`.
void browser_detection_classify(const BrowserDetection *d, const char *user_agent,
                                BrowserMatch *out);

// ── The active rule set ──────────────────────────────────────────────────────
//
// What every request is classified by: the factory rules until
// browser_detection_activate() installs others (at startup, from MongoDB, and
// after each dashboard save). Thread-safe; no database access per request.

// Replaces the active rules with `d`, taking ownership (the previous set is
// freed). `d` must be valid (browser_detection_validate()); NULL restores the
// factory rules. Returns 0, or -1 if `d` is invalid / on allocation failure
// (the active set is then unchanged, and `d` freed).
int browser_detection_activate(BrowserDetection *d);

// A heap copy of the active rules (free() it), NULL on allocation failure.
BrowserDetection *browser_detection_active_copy(void);

// Classifies by the active rules.
void detect_browser(const char *user_agent, BrowserMatch *out);

// The epoch of `user_agent` by the active rules. NULL or empty -> the
// active fallback epoch.
int detect_epoch(const char *user_agent);

// Whether the active rules let `user_agent` use the dashboard.
bool detect_dashboard(const char *user_agent);

#endif // DETECT_EPOCH_H
