#ifndef BROWSER_DETECTION_BSON_H
#define BROWSER_DETECTION_BSON_H

#include "../utils/detect_epoch.h"
#include <bson/bson.h>

// BSON <-> BrowserDetection, for db.browser_detection (cms_browser_detection.h)
// and for the JSON the dashboard's Browser detection page posts. libbson only -
// no MongoDB connection - so tests/ can link it on its own.

// Fills `out` from a document of that shape - the stored one, or the JSON the
// dashboard posts (parsed with bson_init_from_json()). Strict: a missing or
// mistyped field, or a string longer than its buffer, is an error rather than
// a silent default or truncation. Does not run browser_detection_validate().
// 0 on success, -1 with a message in `error`.
int browser_detection_from_bson(const bson_t *doc, BrowserDetection *out,
                                char *error, size_t error_size);

// Appends fallback_epoch, rules and cases to `doc`.
void browser_detection_to_bson(const BrowserDetection *d, bson_t *doc);

#endif // BROWSER_DETECTION_BSON_H
