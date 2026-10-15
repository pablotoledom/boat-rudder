#include "cms_browser_detection.h"
#include "browser_detection_bson.h"
#include "mongodb_manager.h"
#include "../utils/log.h"
#include <mongoc/mongoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BROWSER_DETECTION_COLLECTION "browser_detection"
#define BROWSER_DETECTION_ID         "rules"

// ── MongoDB ──────────────────────────────────────────────────────────────────

BrowserDetection *cms_browser_detection_load(int64_t *updated_at_ms) {
    if (updated_at_ms) *updated_at_ms = 0;

    mongoc_collection_t *collection = mongodb_manager_get_collection(BROWSER_DETECTION_COLLECTION);
    if (!collection) return NULL;

    bson_t *query = BCON_NEW("_id", BCON_UTF8(BROWSER_DETECTION_ID));
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

    BrowserDetection *result = NULL;
    const bson_t *doc;
    if (mongoc_cursor_next(cursor, &doc)) {
        BrowserDetection *d = malloc(sizeof(*d));
        char error[160];
        size_t position;
        const char *invalid = NULL;
        if (d && browser_detection_from_bson(doc, d, error, sizeof(error)) != 0) {
            LOG_ERROR("browser_detection: stored rules unreadable (%s) - using the factory rules", error);
        } else if (d && (invalid = browser_detection_validate(d, &position))) {
            LOG_ERROR("browser_detection: stored rules invalid (%s, item %zu) - using the factory rules",
                      invalid, position);
        } else if (d) {
            result = d;
            d = NULL;
            bson_iter_t it;
            if (updated_at_ms && bson_iter_init_find(&it, doc, "updated_at") &&
                BSON_ITER_HOLDS_DATE_TIME(&it))
                *updated_at_ms = bson_iter_date_time(&it);
        }
        free(d);
    }

    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error))
        LOG_ERROR("browser_detection: load failed: %s", error.message);

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    return result;
}

int cms_browser_detection_save(const BrowserDetection *d, const char *user_id_hex) {
    mongoc_collection_t *collection = mongodb_manager_get_collection(BROWSER_DETECTION_COLLECTION);
    if (!collection) return -1;

    bson_t *document = BCON_NEW("_id", BCON_UTF8(BROWSER_DETECTION_ID));
    browser_detection_to_bson(d, document);
    bson_append_date_time(document, "updated_at", -1, (int64_t)time(NULL) * 1000);
    if (user_id_hex && bson_oid_is_valid(user_id_hex, strlen(user_id_hex))) {
        bson_oid_t oid;
        bson_oid_init_from_string(&oid, user_id_hex);
        bson_append_oid(document, "updated_by", -1, &oid);
    }

    bson_t *query = BCON_NEW("_id", BCON_UTF8(BROWSER_DETECTION_ID));
    bson_t *opts = BCON_NEW("upsert", BCON_BOOL(true));
    bson_error_t error;
    bool ok = mongoc_collection_replace_one(collection, query, document, opts, NULL, &error);
    if (!ok) LOG_ERROR("browser_detection: save failed: %s", error.message);

    bson_destroy(document);
    bson_destroy(query);
    bson_destroy(opts);
    mongoc_collection_destroy(collection);
    return ok ? 0 : -1;
}

int cms_browser_detection_reset(void) {
    mongoc_collection_t *collection = mongodb_manager_get_collection(BROWSER_DETECTION_COLLECTION);
    if (!collection) return -1;

    bson_t *query = BCON_NEW("_id", BCON_UTF8(BROWSER_DETECTION_ID));
    bson_error_t error;
    bool ok = mongoc_collection_delete_one(collection, query, NULL, NULL, &error);
    if (!ok) LOG_ERROR("browser_detection: reset failed: %s", error.message);

    bson_destroy(query);
    mongoc_collection_destroy(collection);
    return ok ? 0 : -1;
}

void cms_browser_detection_reload(void) {
    BrowserDetection *stored = cms_browser_detection_load(NULL);
    if (stored) {
        size_t rule_count = stored->rule_count;   // `stored` belongs to the engine after this
        if (browser_detection_activate(stored) == 0) {
            LOG_INFO("browser_detection: %zu stored rules in effect", rule_count);
            return;
        }
        LOG_ERROR("browser_detection: could not activate the stored rules");
    }
    if (browser_detection_activate(NULL) != 0)
        LOG_ERROR("browser_detection: could not activate the factory rules");
}
