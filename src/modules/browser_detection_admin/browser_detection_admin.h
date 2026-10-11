#ifndef BROWSER_DETECTION_ADMIN_H
#define BROWSER_DETECTION_ADMIN_H

#include <stddef.h>

// /dashboard/settings/browsers - the browser detection rules and test cases
// (develop_docs/plans/browser-detection-admin-plan.md). The page is rendered
// with the active rules embedded as JSON; html/assets/js/browser-detection.js
// edits them and talks to the two JSON endpoints below. Admin role only - the
// router checks that before calling any of these.

// The page content for `epoch` (3), with `user_agent` prefilled in its "Try a
// User-Agent" box. Returns a malloc'd fragment, or NULL on a missing template
// / allocation failure.
char *browser_detection_admin_page(int epoch, const char *user_agent);

// POST /dashboard/api/browsers/preview. `body` is the page's JSON
// ({fallback_epoch, rules, cases, try_user_agent}); nothing is saved. Returns
// a malloc'd JSON body - {error, position} when the rules don't parse or
// validate; else each case's result under the posted rules and under the
// active ones, the try box's result, and whether `request_user_agent` keeps
// the dashboard under the posted rules, and every theme's stylesheet
// features some browser would still be served without (css_compat.h).
// `*status` gets the HTTP status line.
// NULL only on allocation failure.
char *browser_detection_admin_preview(const char *body, size_t body_length,
                                      const char *request_user_agent, const char **status);

// POST /dashboard/api/browsers. Validates and stores the posted rules, then
// puts them into effect. Refused (409) when `request_user_agent` - the
// browser saving - would lose the dashboard under them. Returns a malloc'd
// JSON body ({ok: true} or {error, position}); `*status` gets the status line.
// NULL only on allocation failure.
char *browser_detection_admin_save(const char *body, size_t body_length,
                                   const char *request_user_agent, const char *user_id_hex,
                                   const char **status);

#endif // BROWSER_DETECTION_ADMIN_H
