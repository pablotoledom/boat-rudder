#ifndef CMS_BROWSER_DETECTION_H
#define CMS_BROWSER_DETECTION_H

#include "../utils/detect_epoch.h"
#include <stdint.h>

// db.browser_detection: the browser detection rules and test cases edited
// from /dashboard/settings/browsers - one document, {_id: "rules",
// fallback_epoch, rules: [...], cases: [...], updated_at, updated_by}. No
// document means the factory rules (browser_detection_factory()). See
// develop_docs/plans/browser-detection-admin-plan.md.

// What is stored, as a new heap BrowserDetection (free()), or NULL if
// nothing is stored, MongoDB is not ready, or the stored document does not
// parse/validate (logged). `updated_at_ms` (may be NULL) gets the save time,
// 0 when NULL is returned.
BrowserDetection *cms_browser_detection_load(int64_t *updated_at_ms);

// Stores `d` (already validated) as the rules document, recording who saved
// it. 0 on success, -1 on a DB error or if MongoDB is not ready.
int cms_browser_detection_save(const BrowserDetection *d, const char *user_id_hex);

// Deletes the stored document - the factory rules apply again. 0 on success
// (also when there was nothing to delete), -1 on a DB error.
int cms_browser_detection_reset(void);

// Puts the stored rules - or the factory ones, if none are stored or MongoDB
// is down - into effect for every request (browser_detection_activate()).
// Called at startup and after every save/reset.
void cms_browser_detection_reload(void);

#endif // CMS_BROWSER_DETECTION_H
