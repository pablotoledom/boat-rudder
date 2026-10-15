#ifndef CSS_COMPAT_H
#define CSS_COMPAT_H

#include "detect_epoch.h"
#include <stdbool.h>
#include <stddef.h>

// Checks an epoch 3 stylesheet against the browser detection rules: finds
// CSS features with a known first version per engine (`:has()`, `color-mix()`,
// container queries, nesting, ...) and asks the rules whether the version just
// before each one still gets the page - epoch 3 for the public stylesheet,
// dashboard access for the admin one. Each hit is a browser that would get a
// broken page: raise its tier in /dashboard/settings/browsers, or drop the
// feature. Rule-agnostic: it classifies real-looking User-Agents with
// browser_detection_classify() instead of reading tier numbers.

#define CSS_COMPAT_WARNINGS_MAX 24

typedef struct {
    const char *feature;        // ":has()" - static
    char needs[64];             // "Chrome 105, Firefox 121, Safari 15.4"
    char still_served[96];      // "Chrome 104, Firefox 120" - below and still served
} CssCompatWarning;

// Scans `css` (comments, strings and url() skipped) with `rules`. `dashboard`
// selects the admin stylesheet's test (dashboard access) over the public one
// (epoch 3). Fills up to `max` warnings; returns how many.
size_t css_compat_check(const char *css, const BrowserDetection *rules, bool dashboard,
                        CssCompatWarning *out, size_t max);

#endif // CSS_COMPAT_H
