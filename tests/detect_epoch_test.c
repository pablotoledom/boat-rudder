// The browser detection engine (src/utils/detect_epoch.c): the factory test
// cases against the factory rules - the same data the dashboard's Browser
// detection page starts from - and the engine's own behavior on rules built
// here (tiers, excludes, disabled rules, versions, validation).
//
// Run with: ctest --test-dir build/debug (or the binary directly).

#include "../src/utils/detect_epoch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

static void expect_int(const char *what, int got, int expected) {
    checks++;
    if (got != expected) {
        printf("FAIL %s: expected %d, got %d\n", what, expected, got);
        failures++;
    }
}

static void expect_true(const char *what, int condition) {
    checks++;
    if (!condition) {
        printf("FAIL %s\n", what);
        failures++;
    }
}

static void factory_cases(void) {
    BrowserDetection *d = browser_detection_factory();
    if (!d) {
        printf("FAIL factory: out of memory\n");
        failures++;
        return;
    }
    expect_true("factory rules are valid", browser_detection_validate(d, NULL) == NULL);
    expect_true("factory has test cases", d->case_count > 40);

    for (size_t i = 0; i < d->case_count; i++) {
        const BrowserCase *c = &d->cases[i];
        BrowserMatch m;
        browser_detection_classify(d, c->user_agent, &m);
        checks++;
        if (m.epoch != c->epoch || m.dashboard != c->dashboard) {
            printf("FAIL %-30s expected %2d%s, got %2d%s (rule %d)\n  %s\n", c->name,
                   c->epoch, c->dashboard ? "+dashboard" : "",
                   m.epoch, m.dashboard ? "+dashboard" : "", m.rule, c->user_agent);
            failures++;
        }
    }
    free(d);
}

// The active set starts as the factory one; detect_epoch(NULL) is its fallback.
static void active_set(void) {
    expect_int("detect_epoch(NULL)", detect_epoch(NULL), EPOCH_EARLY);
    expect_int("detect_epoch(Chrome 120)",
               detect_epoch("Mozilla/5.0 (X11) AppleWebKit/537.36 Chrome/120.0 Safari/537.36"),
               EPOCH_MODERN);

    BrowserDetection *d = calloc(1, sizeof(*d));
    d->fallback_epoch = EPOCH_MIDDLE;
    expect_int("activate an empty rule set", browser_detection_activate(d), 0);
    expect_int("empty set: everything is the fallback",
               detect_epoch("Mozilla/5.0 Chrome/120.0"), EPOCH_MIDDLE);

    BrowserDetection *bad = calloc(1, sizeof(*bad));
    bad->fallback_epoch = 7;
    expect_int("activate refuses an invalid set", browser_detection_activate(bad), -1);
    expect_int("...and keeps the previous one", detect_epoch("x"), EPOCH_MIDDLE);

    expect_int("activate(NULL) restores the factory rules", browser_detection_activate(NULL), 0);
    expect_int("factory again", detect_epoch("Lynx/2.8"), EPOCH_PRESTANDARD);
}

static BrowserRule *add_rule(BrowserDetection *d, const char *name, const char *token,
                             const char *version_after, int epoch) {
    BrowserRule *r = &d->rules[d->rule_count++];
    memset(r, 0, sizeof(*r));
    snprintf(r->name, sizeof(r->name), "%s", name);
    r->enabled = true;
    snprintf(r->contains[r->contains_count++], BROWSER_TOKEN_MAX, "%s", token);
    snprintf(r->version_after, sizeof(r->version_after), "%s", version_after);
    r->epoch = epoch;
    return r;
}

static void add_tier(BrowserRule *r, const char *from, int epoch, bool dashboard) {
    BrowserTier *t = &r->tiers[r->tier_count++];
    snprintf(t->from, sizeof(t->from), "%s", from);
    t->epoch = epoch;
    t->dashboard = dashboard;
}

static int epoch_of(const BrowserDetection *d, const char *ua) {
    BrowserMatch m;
    browser_detection_classify(d, ua, &m);
    return m.epoch;
}

static void engine(void) {
    BrowserDetection *d = calloc(1, sizeof(*d));
    d->fallback_epoch = EPOCH_EARLY;

    // Tiers listed out of order: the highest one reached wins.
    BrowserRule *x = add_rule(d, "X", "X/", "X/", EPOCH_MIDDLE);
    add_tier(x, "20", EPOCH_MODERN, true);
    add_tier(x, "10.5", EPOCH_MODERN, false);
    expect_int("below every tier", epoch_of(d, "X/9.9"), EPOCH_MIDDLE);
    expect_int("minor below a tier", epoch_of(d, "X/10.4"), EPOCH_MIDDLE);
    expect_int("exactly a tier", epoch_of(d, "X/10.5"), EPOCH_MODERN);
    BrowserMatch m;
    browser_detection_classify(d, "X/25", &m);
    expect_true("highest tier reached gives dashboard", m.dashboard && m.rule == 0);
    browser_detection_classify(d, "X/15", &m);
    expect_true("lower tier: no dashboard", !m.dashboard && m.epoch == EPOCH_MODERN);
    expect_int("token without a version", epoch_of(d, "X/beta"), EPOCH_MIDDLE);

    // Excludes, and the first matching rule winning.
    BrowserRule *y = add_rule(d, "Y", "Y", "", EPOCH_WML);
    snprintf(y->excludes[y->excludes_count++], BROWSER_TOKEN_MAX, "%s", "Kit");
    expect_int("excluded token skips the rule", epoch_of(d, "Y Kit"), EPOCH_EARLY);
    expect_int("not excluded", epoch_of(d, "Y"), EPOCH_WML);
    expect_int("first matching rule wins", epoch_of(d, "Y X/30"), EPOCH_MODERN);

    // Version separators: "/" or " " after the token, "_" for the minor.
    BrowserRule *z = add_rule(d, "Z", "Zed", "Zed", EPOCH_EARLY);
    add_tier(z, "14.5", EPOCH_MODERN, false);
    expect_int("version after a space", epoch_of(d, "Zed 14.5"), EPOCH_MODERN);
    expect_int("version with underscore", epoch_of(d, "Zed14_5"), EPOCH_MODERN);
    expect_int("version after a slash", epoch_of(d, "Zed/14.4"), EPOCH_EARLY);

    // Disabled rules are skipped; no match -> fallback, rule -1.
    x->enabled = false;
    expect_int("disabled rule skipped", epoch_of(d, "X/30"), EPOCH_EARLY);
    browser_detection_classify(d, "nothing", &m);
    expect_true("fallback reports rule -1", m.rule == -1 && !m.dashboard);

    expect_true("valid set", browser_detection_validate(d, NULL) == NULL);

    // Validation, each with the position of the offending rule.
    size_t pos;
    z->tiers[0].epoch = 5;
    expect_true("tier epoch out of range",
                browser_detection_validate(d, &pos) != NULL && pos == 3);
    z->tiers[0].epoch = EPOCH_MODERN;
    snprintf(z->tiers[0].from, sizeof(z->tiers[0].from), "%s", "14.x");
    expect_true("tier version not a number", browser_detection_validate(d, &pos) != NULL);
    snprintf(z->tiers[0].from, sizeof(z->tiers[0].from), "%s", "14.5");
    y->version_after[0] = '\0';
    add_tier(y, "1", EPOCH_MIDDLE, false);
    expect_true("tiers without version_after",
                browser_detection_validate(d, &pos) != NULL && pos == 2);
    y->tier_count = 0;
    y->contains_count = 0;
    expect_true("rule with nothing to match", browser_detection_validate(d, &pos) != NULL);
    y->contains_count = 1;
    snprintf(y->contains[0], BROWSER_TOKEN_MAX, "%s", "a\tb");
    expect_true("control character in a token", browser_detection_validate(d, NULL) != NULL);
    snprintf(y->contains[0], BROWSER_TOKEN_MAX, "%s", "Y");
    d->fallback_epoch = -2;
    expect_true("fallback out of range", browser_detection_validate(d, NULL) != NULL);

    free(d);
}

int main(void) {
    factory_cases();
    active_set();
    engine();
    printf("%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
