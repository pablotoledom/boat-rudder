// src/utils/css_compat.c: which features it finds (and where it must not),
// how it reads the rules, and - the check that matters - that both shipped
// themes' stylesheets are within the factory rules' floors.
//
// Run from the repository root (ctest sets WORKING_DIRECTORY).

#include "../src/utils/css_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

static void expect(const char *what, int condition) {
    checks++;
    if (!condition) {
        printf("FAIL %s\n", what);
        failures++;
    }
}

static size_t warnings(const BrowserDetection *rules, const char *css, bool dashboard,
                       CssCompatWarning *out) {
    return css_compat_check(css, rules, dashboard, out, CSS_COMPAT_WARNINGS_MAX);
}

static int warns_about(const BrowserDetection *rules, const char *css, const char *feature) {
    CssCompatWarning w[CSS_COMPAT_WARNINGS_MAX];
    size_t n = warnings(rules, css, false, w);
    for (size_t i = 0; i < n; i++)
        if (strstr(w[i].feature, feature)) return 1;
    return 0;
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)size + 1);
    if (buf && fread(buf, 1, (size_t)size, f) != (size_t)size) { free(buf); buf = NULL; }
    if (buf) buf[size] = '\0';
    fclose(f);
    return buf;
}

static void shipped_stylesheets(const BrowserDetection *factory) {
    static const struct { const char *path; bool dashboard; } SHEETS[] = {
        { "html/themes/dark/styles_epoch3.css", false },
        { "html/themes/light/styles_epoch3.css", false },
        { "html/themes/dark/styles_admin_epoch3.css", true },
        { "html/themes/light/styles_admin_epoch3.css", true },
    };
    for (size_t i = 0; i < sizeof(SHEETS) / sizeof(SHEETS[0]); i++) {
        char *css = read_file(SHEETS[i].path);
        expect(SHEETS[i].path, css != NULL);
        if (!css) continue;
        CssCompatWarning w[CSS_COMPAT_WARNINGS_MAX];
        size_t n = warnings(factory, css, SHEETS[i].dashboard, w);
        for (size_t k = 0; k < n; k++)
            printf("FAIL %s: %s needs %s, still served: %s\n",
                   SHEETS[i].path, w[k].feature, w[k].needs, w[k].still_served);
        checks++;
        if (n) failures++;
        free(css);
    }
}

int main(void) {
    BrowserDetection *factory = browser_detection_factory();

    // Finds them...
    expect(":has() on the public sheet", warns_about(factory, "a:has(b) { color: red }", ":has()"));
    expect("color-mix()", warns_about(factory, "a { color: color-mix(in srgb, red 50%, blue) }", "color-mix"));
    expect("nesting", warns_about(factory, ".a { & .b { color: red } }", "nesting"));
    expect("dvh unit", warns_about(factory, ".a { height: 100dvh }", "dvh"));
    expect("container-type property", warns_about(factory, ".a { container-type: inline-size }", "container"));

    // ...and not where they only look alike.
    expect("inside a comment", !warns_about(factory, "/* a:has(b) */ a { color: red }", ":has()"));
    expect("inside a string", !warns_about(factory, "a::after { content: \":has( & \" }", ":has()") &&
                              !warns_about(factory, "a::after { content: \"&\" }", "nesting"));
    expect("& inside url()", !warns_about(factory, "a { background: url(/x?a=1&b=2) }", "nesting"));
    expect("`scale(` is a transform function, not the property",
           !warns_about(factory, "a { transform: scale(2) }", "scale"));
    expect("`inset` as a box-shadow keyword", !warns_about(factory, "a { box-shadow: inset 0 0 1px }", "inset"));
    expect("a class named like a feature", !warns_about(factory, ".foo-subgrid { color: red }", "subgrid"));
    expect("`dvh` not after a number", !warns_about(factory, ".dvh { color: red }", "dvh"));

    // Within the public floors (Chrome 87, Firefox 66, Safari 14.1): no warning.
    expect("inset is within the floors", !warns_about(factory, "a { inset: 0 }", "inset"));
    // :has() on the admin sheet is within the dashboard floors (Chrome 111...).
    CssCompatWarning w[CSS_COMPAT_WARNINGS_MAX];
    expect(":has() within the dashboard floors", warnings(factory, "a:has(b) {}", true, w) == 0);

    // Who is named: below the floor and still served.
    size_t n = warnings(factory, "a:has(b) {}", false, w);
    expect("one :has() warning", n == 1);
    if (n == 1) {
        expect("names Chrome 104", strstr(w[0].still_served, "Chrome 104") != NULL);
        expect("names Firefox 120", strstr(w[0].still_served, "Firefox 120") != NULL);
        expect("names Safari 15.3 and iOS", strstr(w[0].still_served, "Safari 15.3") != NULL &&
                                           strstr(w[0].still_served, "iOS 15.3") != NULL);
        expect("says what it needs", strcmp(w[0].needs, "Chrome 105, Firefox 121, Safari 15.4") == 0);
    }

    // It reads the rules: raise Chrome's epoch 3 tier to 105 and Chrome
    // drops out of the warning.
    for (size_t i = 0; i < factory->rule_count; i++)
        if (strcmp(factory->rules[i].name, "Chrome") == 0)
            snprintf(factory->rules[i].tiers[0].from, BROWSER_VERSION_MAX, "%s", "105");
    n = warnings(factory, "a:has(b) {}", false, w);
    expect("raised Chrome tier: Chrome no longer named",
           n == 1 && strstr(w[0].still_served, "Chrome") == NULL);
    free(factory);

    factory = browser_detection_factory();
    shipped_stylesheets(factory);
    free(factory);

    printf("%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
