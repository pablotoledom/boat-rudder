#include "browser_detection_admin.h"
#include "../../db/browser_detection_bson.h"
#include "../../db/cms_browser_detection.h"
#include "../../utils/detect_epoch.h"
#include "../../utils/generate_url_theme.h"
#include "../../utils/http_utils.h"
#include "../../utils/read_file.h"
#include "../../utils/template_utils.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STATUS_OK          "200 OK"
#define STATUS_BAD_REQUEST "400 Bad Request"
#define STATUS_CONFLICT    "409 Conflict"
#define STATUS_ERROR       "500 Internal Server Error"

static char *load_template(const char *subpath_fmt, int epoch) {
    char *path = generate_url_theme(subpath_fmt, epoch);
    char *tpl  = path ? read_file_to_string(path) : NULL;
    free(path);
    return tpl;
}

// html_encode() needs a caller-sized buffer; worst case every byte of `src`
// expands to "&quot;" (6 bytes), so 6x + 1 is always enough.
static char *html_encode_alloc(const char *src) {
    size_t cap = strlen(src) * 6 + 1;
    char *out = malloc(cap);
    if (!out) return NULL;
    html_encode(out, src, cap);
    return out;
}

// `doc` as relaxed extended JSON - plain numbers and booleans for what the
// converter writes - as a malloc'd string (bson_free()'d copy), NULL on
// failure.
static char *bson_to_json(const bson_t *doc) {
    char *json = bson_as_relaxed_extended_json(doc, NULL);
    char *copy = json ? strdup(json) : NULL;
    bson_free(json);
    return copy;
}

static char *error_json(const char *message, size_t position) {
    bson_t doc = BSON_INITIALIZER;
    bson_append_utf8(&doc, "error", -1, message, -1);
    bson_append_int32(&doc, "position", -1, (int32_t)position);
    char *json = bson_to_json(&doc);
    bson_destroy(&doc);
    return json;
}

char *browser_detection_admin_page(int epoch, const char *user_agent) {
    char *tpl = load_template("dashboard/settings/browsers_epoch%d.html", epoch);
    if (!tpl) return NULL;

    int64_t updated_at_ms = 0;
    BrowserDetection *stored = cms_browser_detection_load(&updated_at_ms);
    BrowserDetection *shown  = stored ? stored : browser_detection_active_copy();

    char status[64] = "Factory rules";
    if (stored && updated_at_ms > 0) {
        time_t when = (time_t)(updated_at_ms / 1000);
        struct tm t;
        gmtime_r(&when, &t);
        strftime(status, sizeof(status), "Saved %Y-%m-%d %H:%M UTC", &t);
    }

    char *result = NULL;
    if (shown) {
        bson_t doc = BSON_INITIALIZER;
        browser_detection_to_bson(shown, &doc);
        char *json = bson_to_json(&doc);
        bson_destroy(&doc);

        char *state = json ? html_encode_alloc(json) : NULL;
        char *ua    = html_encode_alloc(user_agent ? user_agent : "");
        if (state && ua) result = render_template(tpl, state, status, ua);
        free(json);
        free(state);
        free(ua);
    }

    free(shown);
    free(tpl);
    return result;
}

// Parses and validates the posted rules into `out`. NULL when they are
// usable, else a malloc'd error JSON (with `*status` set).
static char *parse_posted(const char *body, size_t body_length, BrowserDetection *out,
                          bson_t *doc_out, const char **status) {
    bson_error_t bson_error;
    if (!body || body_length == 0 ||
        !bson_init_from_json(doc_out, body, (ssize_t)body_length, &bson_error)) {
        bson_init(doc_out);
        *status = STATUS_BAD_REQUEST;
        return error_json("The request is not valid JSON.", 0);
    }

    char error[160];
    if (browser_detection_from_bson(doc_out, out, error, sizeof(error)) != 0) {
        *status = STATUS_BAD_REQUEST;
        return error_json(error, 0);
    }

    size_t position;
    const char *invalid = browser_detection_validate(out, &position);
    if (invalid) {
        *status = STATUS_BAD_REQUEST;
        return error_json(invalid, position);
    }
    return NULL;
}

static void append_match(bson_t *parent, const char *key, const BrowserMatch *m) {
    bson_t doc;
    bson_append_document_begin(parent, key, -1, &doc);
    bson_append_int32(&doc, "epoch", -1, m->epoch);
    bson_append_bool(&doc, "dashboard", -1, m->dashboard);
    bson_append_int32(&doc, "rule", -1, m->rule);
    bson_append_document_end(parent, &doc);
}

// Whether the browser saving keeps the dashboard under `d`: it must still be
// epoch 3 with the dashboard flag, as dashboard_allowed() in the router asks.
static int keeps_dashboard(const BrowserDetection *d, const char *user_agent) {
    BrowserMatch m;
    browser_detection_classify(d, user_agent, &m);
    return m.epoch == EPOCH_MODERN && m.dashboard;
}

char *browser_detection_admin_preview(const char *body, size_t body_length,
                                      const char *request_user_agent, const char **status) {
    BrowserDetection *posted = malloc(sizeof(*posted));
    BrowserDetection *active = browser_detection_active_copy();
    if (!posted || !active) {
        free(posted);
        free(active);
        *status = STATUS_ERROR;
        return NULL;
    }

    bson_t doc;
    char *result = parse_posted(body, body_length, posted, &doc, status);
    if (!result) {
        bson_t out = BSON_INITIALIZER, cases;
        bson_append_array_begin(&out, "cases", -1, &cases);
        for (size_t i = 0; i < posted->case_count; i++) {
            char index[16];
            const char *key;
            bson_uint32_to_string((uint32_t)i, &key, index, sizeof(index));
            BrowserMatch now, before;
            browser_detection_classify(posted, posted->cases[i].user_agent, &now);
            browser_detection_classify(active, posted->cases[i].user_agent, &before);

            bson_t item;
            bson_append_document_begin(&cases, key, -1, &item);
            append_match(&item, "result", &now);
            append_match(&item, "active", &before);
            bson_append_document_end(&cases, &item);
        }
        bson_append_array_end(&out, &cases);

        bson_iter_t it;
        if (bson_iter_init_find(&it, &doc, "try_user_agent") && BSON_ITER_HOLDS_UTF8(&it)) {
            BrowserMatch m;
            browser_detection_classify(posted, bson_iter_utf8(&it, NULL), &m);
            append_match(&out, "try", &m);
        }
        bson_append_bool(&out, "keeps_dashboard", -1,
                         keeps_dashboard(posted, request_user_agent ? request_user_agent : ""));

        result = bson_to_json(&out);
        bson_destroy(&out);
        *status = result ? STATUS_OK : STATUS_ERROR;
    }

    bson_destroy(&doc);
    free(posted);
    free(active);
    return result;
}

char *browser_detection_admin_save(const char *body, size_t body_length,
                                   const char *request_user_agent, const char *user_id_hex,
                                   const char **status) {
    BrowserDetection *posted = malloc(sizeof(*posted));
    if (!posted) {
        *status = STATUS_ERROR;
        return NULL;
    }

    bson_t doc;
    char *result = parse_posted(body, body_length, posted, &doc, status);
    bson_destroy(&doc);
    if (result) {
        free(posted);
        return result;
    }

    if (!keeps_dashboard(posted, request_user_agent ? request_user_agent : "")) {
        free(posted);
        *status = STATUS_CONFLICT;
        return error_json("Not saved: with these rules, the browser you are using would lose "
                          "access to the dashboard, and you could not undo it from here.", 0);
    }

    if (cms_browser_detection_save(posted, user_id_hex) != 0) {
        free(posted);
        *status = STATUS_ERROR;
        return error_json("The rules could not be saved to the database.", 0);
    }
    free(posted);
    cms_browser_detection_reload();

    *status = STATUS_OK;
    return strdup("{\"ok\": true}");
}
