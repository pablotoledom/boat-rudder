#include "browser_detection_bson.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int fail(char *error, size_t size, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(error, size, fmt, ap);
    va_end(ap);
    return -1;
}

// ── BSON → struct ────────────────────────────────────────────────────────────

// An integer field: JSON numbers arrive as int32, int64 or a whole double.
static int iter_int(const bson_iter_t *it, int *out) {
    if (BSON_ITER_HOLDS_INT32(it)) { *out = bson_iter_int32(it); return 0; }
    if (BSON_ITER_HOLDS_INT64(it)) {
        int64_t v = bson_iter_int64(it);
        if (v < -1000 || v > 1000) return -1;
        *out = (int)v;
        return 0;
    }
    if (BSON_ITER_HOLDS_DOUBLE(it)) {
        double v = bson_iter_double(it);
        if (v != (double)(int)v) return -1;
        *out = (int)v;
        return 0;
    }
    return -1;
}

static int iter_string(const bson_iter_t *it, char *out, size_t size) {
    if (!BSON_ITER_HOLDS_UTF8(it)) return -1;
    uint32_t len;
    const char *s = bson_iter_utf8(it, &len);
    if (len >= size || strlen(s) != len) return -1;   // too long, or an embedded NUL
    memcpy(out, s, len + 1);
    return 0;
}

// Finds `key` in the document `parent` points into.
static int find_field(const bson_iter_t *parent, const char *key, bson_iter_t *out) {
    bson_iter_t child;
    if (!bson_iter_recurse(parent, &child)) return 0;
    return bson_iter_find(&child, key) ? (*out = child, 1) : 0;
}

static int read_string(const bson_iter_t *doc, const char *key, char *out, size_t size) {
    bson_iter_t it;
    return find_field(doc, key, &it) ? iter_string(&it, out, size) : -1;
}

static int read_int(const bson_iter_t *doc, const char *key, int *out) {
    bson_iter_t it;
    return find_field(doc, key, &it) ? iter_int(&it, out) : -1;
}

static int read_bool(const bson_iter_t *doc, const char *key, bool *out) {
    bson_iter_t it;
    if (!find_field(doc, key, &it) || !BSON_ITER_HOLDS_BOOL(&it)) return -1;
    *out = bson_iter_bool(&it);
    return 0;
}

static int read_tokens(const bson_iter_t *doc, const char *key,
                       char out[][BROWSER_TOKEN_MAX], size_t *count) {
    bson_iter_t it, item;
    if (!find_field(doc, key, &it) || !BSON_ITER_HOLDS_ARRAY(&it) || !bson_iter_recurse(&it, &item))
        return -1;
    *count = 0;
    while (bson_iter_next(&item)) {
        if (*count >= BROWSER_TOKENS_MAX) return -1;
        if (iter_string(&item, out[*count], BROWSER_TOKEN_MAX) != 0) return -1;
        (*count)++;
    }
    return 0;
}

static int read_rule(const bson_iter_t *doc, BrowserRule *r, char *error, size_t size,
                     size_t n) {
    if (!BSON_ITER_HOLDS_DOCUMENT(doc)) return fail(error, size, "rule %zu is not an object", n);
    if (read_string(doc, "name", r->name, sizeof(r->name)) != 0)
        return fail(error, size, "rule %zu: name missing or too long", n);
    if (read_bool(doc, "enabled", &r->enabled) != 0)
        return fail(error, size, "rule %zu: enabled missing", n);
    if (read_tokens(doc, "contains", r->contains, &r->contains_count) != 0)
        return fail(error, size, "rule %zu: contains must be a list of at most %d tokens of "
                    "under %d bytes", n, BROWSER_TOKENS_MAX, BROWSER_TOKEN_MAX);
    if (read_tokens(doc, "excludes", r->excludes, &r->excludes_count) != 0)
        return fail(error, size, "rule %zu: excludes must be a list of at most %d tokens of "
                    "under %d bytes", n, BROWSER_TOKENS_MAX, BROWSER_TOKEN_MAX);
    if (read_string(doc, "version_after", r->version_after, sizeof(r->version_after)) != 0)
        return fail(error, size, "rule %zu: version_after missing or too long", n);
    if (read_int(doc, "epoch", &r->epoch) != 0)
        return fail(error, size, "rule %zu: epoch missing", n);
    if (read_bool(doc, "dashboard", &r->dashboard) != 0)
        return fail(error, size, "rule %zu: dashboard missing", n);

    bson_iter_t it, tier;
    if (!find_field(doc, "tiers", &it) || !BSON_ITER_HOLDS_ARRAY(&it) || !bson_iter_recurse(&it, &tier))
        return fail(error, size, "rule %zu: tiers missing", n);
    r->tier_count = 0;
    while (bson_iter_next(&tier)) {
        if (r->tier_count >= BROWSER_TIERS_MAX)
            return fail(error, size, "rule %zu: at most %d tiers", n, BROWSER_TIERS_MAX);
        BrowserTier *t = &r->tiers[r->tier_count++];
        if (!BSON_ITER_HOLDS_DOCUMENT(&tier) ||
            read_string(&tier, "from", t->from, sizeof(t->from)) != 0 ||
            read_int(&tier, "epoch", &t->epoch) != 0 ||
            read_bool(&tier, "dashboard", &t->dashboard) != 0)
            return fail(error, size, "rule %zu: tier %zu needs from, epoch and dashboard",
                        n, r->tier_count);
    }
    return 0;
}

static int read_case(const bson_iter_t *doc, BrowserCase *c, char *error, size_t size,
                     size_t n) {
    if (!BSON_ITER_HOLDS_DOCUMENT(doc) ||
        read_string(doc, "name", c->name, sizeof(c->name)) != 0 ||
        read_string(doc, "user_agent", c->user_agent, sizeof(c->user_agent)) != 0 ||
        read_int(doc, "epoch", &c->epoch) != 0 ||
        read_bool(doc, "dashboard", &c->dashboard) != 0)
        return fail(error, size, "test case %zu needs name, user_agent (under %d bytes), epoch "
                    "and dashboard", n, BROWSER_UA_MAX);
    return 0;
}

int browser_detection_from_bson(const bson_t *doc, BrowserDetection *out,
                                char *error, size_t error_size) {
    memset(out, 0, sizeof(*out));

    bson_iter_t it;
    if (!bson_iter_init_find(&it, doc, "fallback_epoch") || iter_int(&it, &out->fallback_epoch) != 0)
        return fail(error, error_size, "fallback_epoch missing");

    bson_iter_t item;
    if (!bson_iter_init_find(&it, doc, "rules") || !BSON_ITER_HOLDS_ARRAY(&it) ||
        !bson_iter_recurse(&it, &item))
        return fail(error, error_size, "rules missing");
    while (bson_iter_next(&item)) {
        if (out->rule_count >= BROWSER_RULES_MAX)
            return fail(error, error_size, "at most %d rules", BROWSER_RULES_MAX);
        if (read_rule(&item, &out->rules[out->rule_count], error, error_size,
                      out->rule_count + 1) != 0)
            return -1;
        out->rule_count++;
    }

    if (!bson_iter_init_find(&it, doc, "cases") || !BSON_ITER_HOLDS_ARRAY(&it) ||
        !bson_iter_recurse(&it, &item))
        return fail(error, error_size, "cases missing");
    while (bson_iter_next(&item)) {
        if (out->case_count >= BROWSER_CASES_MAX)
            return fail(error, error_size, "at most %d test cases", BROWSER_CASES_MAX);
        if (read_case(&item, &out->cases[out->case_count], error, error_size,
                      out->case_count + 1) != 0)
            return -1;
        out->case_count++;
    }
    return 0;
}

// ── struct → BSON ────────────────────────────────────────────────────────────

static void append_tokens(bson_t *doc, const char *key,
                          const char list[][BROWSER_TOKEN_MAX], size_t count) {
    bson_t array;
    bson_append_array_begin(doc, key, -1, &array);
    for (size_t i = 0; i < count; i++) {
        char index[16];
        const char *index_key;
        bson_uint32_to_string((uint32_t)i, &index_key, index, sizeof(index));
        bson_append_utf8(&array, index_key, -1, list[i], -1);
    }
    bson_append_array_end(doc, &array);
}

void browser_detection_to_bson(const BrowserDetection *d, bson_t *doc) {
    char index[16];
    const char *index_key;

    bson_append_int32(doc, "fallback_epoch", -1, d->fallback_epoch);

    bson_t rules;
    bson_append_array_begin(doc, "rules", -1, &rules);
    for (size_t i = 0; i < d->rule_count; i++) {
        const BrowserRule *r = &d->rules[i];
        bson_t rule, tiers;
        bson_uint32_to_string((uint32_t)i, &index_key, index, sizeof(index));
        bson_append_document_begin(&rules, index_key, -1, &rule);
        bson_append_utf8(&rule, "name", -1, r->name, -1);
        bson_append_bool(&rule, "enabled", -1, r->enabled);
        append_tokens(&rule, "contains", r->contains, r->contains_count);
        append_tokens(&rule, "excludes", r->excludes, r->excludes_count);
        bson_append_utf8(&rule, "version_after", -1, r->version_after, -1);
        bson_append_array_begin(&rule, "tiers", -1, &tiers);
        for (size_t t = 0; t < r->tier_count; t++) {
            bson_t tier;
            bson_uint32_to_string((uint32_t)t, &index_key, index, sizeof(index));
            bson_append_document_begin(&tiers, index_key, -1, &tier);
            bson_append_utf8(&tier, "from", -1, r->tiers[t].from, -1);
            bson_append_int32(&tier, "epoch", -1, r->tiers[t].epoch);
            bson_append_bool(&tier, "dashboard", -1, r->tiers[t].dashboard);
            bson_append_document_end(&tiers, &tier);
        }
        bson_append_array_end(&rule, &tiers);
        bson_append_int32(&rule, "epoch", -1, r->epoch);
        bson_append_bool(&rule, "dashboard", -1, r->dashboard);
        bson_append_document_end(&rules, &rule);
    }
    bson_append_array_end(doc, &rules);

    bson_t cases;
    bson_append_array_begin(doc, "cases", -1, &cases);
    for (size_t i = 0; i < d->case_count; i++) {
        const BrowserCase *c = &d->cases[i];
        bson_t item;
        bson_uint32_to_string((uint32_t)i, &index_key, index, sizeof(index));
        bson_append_document_begin(&cases, index_key, -1, &item);
        bson_append_utf8(&item, "name", -1, c->name, -1);
        bson_append_utf8(&item, "user_agent", -1, c->user_agent, -1);
        bson_append_int32(&item, "epoch", -1, c->epoch);
        bson_append_bool(&item, "dashboard", -1, c->dashboard);
        bson_append_document_end(&cases, &item);
    }
    bson_append_array_end(doc, &cases);
}
