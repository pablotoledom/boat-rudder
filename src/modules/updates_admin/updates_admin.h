#ifndef UPDATES_ADMIN_H
#define UPDATES_ADMIN_H

// /dashboard/settings/updates - the running version, the releases published
// in the GitHub repository `update_repo` (config_loader.h) and their notes,
// and installing one or rolling the last update back
// (develop_docs/plans/update-system-plan.md §7). Admin role only, and the
// router checks the admin's password again before an install or a rollback.
//
// The release list comes from the GitHub API through curl(1) and is kept in
// memory: a page view fetches it again only when it is older than an hour,
// so a dashboard left open stays far below GitHub's 60 requests/hour limit
// for unauthenticated clients.
//
// The server never updates itself: an update stops and restarts its service.
// It writes the request to .boat-rudder/request.json and starts
// `scripts/boat-rudder-update run-request` as a transient systemd unit
// (systemd-run), which reports to .boat-rudder/progress.json; the page polls
// that through updates_admin_progress_json(). Only an install made by
// install.sh - .boat-rudder/ in the working directory - can do this.

// The page content for `epoch` (3), fetching the release list first when it
// has never been fetched or is over an hour old. `error_code` (the `error`
// query parameter, may be NULL) picks a message: "password", "busy",
// "version", "rollback", "unavailable", "launch". Returns a malloc'd
// fragment, or NULL on a missing template / allocation failure.
char *updates_admin_page(int epoch, const char *error_code);

// POST /dashboard/settings/updates/check ("Check now"): fetches the release
// list again, whatever its age.
void updates_admin_check_now(void);

// POST /dashboard/settings/updates/install and …/rollback, once the router
// has checked the password: queues `action` ("install" with `version`, which
// must be a signed release newer than the running one; "rollback" with NULL)
// for `user_email` and starts the updater. Returns NULL when it started, else
// one of the error codes above.
const char *updates_admin_request(const char *action, const char *version, const char *user_email);

// GET /dashboard/api/updates/progress: {state, stage, message, log} of the
// last run (state "none" if there was none). Malloc'd, NULL on allocation
// failure.
char *updates_admin_progress_json(void);

#endif // UPDATES_ADMIN_H
