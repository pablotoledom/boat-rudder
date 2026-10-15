#include "request_charset.h"
#include <string.h>

// Per-thread, per-request. See request_charset.h for why this is not a
// parameter.
static __thread int needs_legacy_charset = 0;

void request_charset_set(const char *user_agent, int http09) {
    // The genuinely pre-Unicode readers of epoch 0, matched independently of
    // epoch resolution (force_epoch/preview_epoch never touch this),
    // straight off the request's real header.
    needs_legacy_charset = http09 ||
        (user_agent && (strstr(user_agent, "Cello") || strstr(user_agent, "LineMode")));
}

int request_needs_legacy_charset(void) {
    return needs_legacy_charset;
}
