# Browser Detection from the Dashboard - Plan

> **Status**: implemented (2026-10-11) as described below. References: [rendering.md](../reference/rendering.md),
> [dashboard.md](../reference/dashboard.md), [data-model.md](../reference/data-model.md#browser_detection).

Today `detect_epoch()` (`src/utils/detect_epoch.c`) is code: an ordered chain of `User-Agent`
checks with the epoch 3 floors (Chrome 87, Firefox 66, Safari 14.1, iOS 14.5) as constants.
Changing a floor, or teaching it a browser it doesn't know, means editing C and recompiling - while
the stylesheet those floors exist for (`styles_epoch3.css`) is already editable from the dashboard.
An admin who adds `:has()` to that stylesheet has moved the floor and has no way to move the
detector with it.

This plan turns the chain into **data**: an ordered list of rules, stored in MongoDB, edited from
`/dashboard/settings/browsers`, evaluated by a small generic engine. The current chain ships as
the **factory rules**, so nothing changes for a site that never opens the page. The same page holds
**test cases** - real `User-Agent`s with the epoch they must get - which run on every save and show
what an edit would change before it is applied.

---

## 1. Why every check fits one rule shape

Each branch of today's chain is "if the UA contains X (and not Y), read the version after Z, and
pick an epoch by that version":

| Today | contains any | excludes | version after | tiers (from → epoch) | else |
|---|---|---|---|---|---|
| WAP 1.x | `Wap WAP Nokia nokia UP.Browser UP/ Obigo Ericsson SIE- SEC- MOT- LG- Alcatel Panasonic SANYO SHARP PHILIPS` | `AppleWebKit` | - | - | −1 |
| Text browsers | `Lynx Cello Line Mode Browser ELinks w3m` | | - | - | 0 |
| Internet Explorer | `MSIE ` | | `MSIE ` | 4 → 2 | 1 |
| IE 11 | `Trident/` | | - | - | 2 |
| EdgeHTML | `Edge/` | | - | - | 2 |
| iOS | `iPhone iPad iPod` | | ` OS ` | 14.5 → 3 | 2 |
| Firefox | `Firefox/` | | `Firefox/` | 66 → 3 | 2 |
| Chrome | `Chrome/` | | `Chrome/` | 87 → 3 | 2 |
| Safari | `Safari/` | | `Version/` | 14.1 → 3 | 2 |
| Opera (Presto) | `Opera/` | | `Opera/` | 5 → 2 | 1 |
| Netscape | `Netscape` | | `Netscape` | 4 → 2 | 1 |
| Mosaic | `Mosaic` | | - | - | 1 |
| Generic Mozilla | `Mozilla/` | | `Mozilla/` | 3 → 2 | 1 |
| *(no rule matched)* | | | | | **1** (fallback, editable) |

The first rule whose *contains* matches and whose *excludes* don't wins. Its tiers are checked from
the highest `from` down; the first one the version reaches gives the epoch. No version, or none
reached → the rule's *else* epoch.

Version parsing: digits, then an optional `.` or `_` and more digits (`14.1`, iOS's `14_5`); a
single `/` or space right after the token is skipped (`Netscape/7.1`, `Netscape6/6.2`). Missing
minor = 0.

Matching is **case-sensitive substring**, as today (the WAP rule lists `Nokia` and `nokia` on
purpose). No regular expressions: they would make a single bad pattern able to stall every
request thread.

## 2. The dashboard floor (phase 3 folded in)

A tier also carries **`dashboard: true|false`** - whether that browser may use `/login` and
`/dashboard`, whose stylesheet needs more than the public one (`:has()`, `color-mix()`: Chrome 111,
Firefox 121, Safari 16.2). The factory Chrome rule becomes `87 → 3`, `111 → 3 + dashboard`. Today's
gate (`epoch == EPOCH_MODERN`) becomes `epoch == EPOCH_MODERN && dashboard`.

## 3. Data model

One document in a new `browser_detection` collection (single document: the list is edited, saved
and validated as a whole, and its order is part of its meaning):

```js
{
  "_id": "rules",
  "fallback_epoch": 1,
  "rules": [
    { "name": "Chrome", "enabled": true,
      "contains": ["Chrome/"], "excludes": [],
      "version_after": "Chrome/",
      "tiers": [ { "from": "87",  "epoch": 3, "dashboard": false },
                 { "from": "111", "epoch": 3, "dashboard": true } ],
      "epoch": 2 },
    ...
  ],
  "cases": [
    { "name": "Chrome 86", "user_agent": "Mozilla/5.0 ... Chrome/86.0 ...", "epoch": 2 },
    ...
  ],
  "updated_at": ISODate, "updated_by": ObjectId   // users._id
}
```

No document → the factory rules and cases are in effect. Limits, enforced on save: 64 rules,
32 tokens per list, 64 bytes per token, 8 tiers per rule, 300 cases, 1 KiB per `User-Agent`.

## 4. Engine (`src/utils/detect_epoch.c`)

- The factory rules and cases become a `static const` **data table** in C - the only place the
  defaults live. `tests/detect_epoch_test.c` keeps running the factory cases against the factory
  rules through the same engine.
- `detect_epoch(ua)` keeps its signature; new `detect_browser(ua, &result)` returns epoch,
  dashboard flag and the matched rule's index (for the tester).
- The active rule set is an immutable snapshot behind a `pthread_rwlock_t`: request threads take
  the read lock to classify; loading replaces the pointer under the write lock and frees the old
  set. Loaded once at startup (after `mongodb_manager_init()`) and again after every save. **No
  database access per request.** MongoDB down at startup → factory rules, logged.

## 5. Persistence (`src/db/browser_detection.c`)

BSON ↔ the engine's structs, in one converter used both for the stored document and for the JSON
the page posts (libbson's `bson_init_from_json()` - no new dependency). `load`, `save` (upsert),
`reset` (delete the document).

## 6. Dashboard: `/dashboard/settings/browsers` (admin role only)

Linked from Site settings → More settings and the Site group of the dashboard menu.

1. **Rules** - an ordered table: name, enabled, *contains*, *excludes*, *version after*, tiers
   (rows of from / epoch / dashboard), *else*; move up/down, add, delete. Fallback epoch below.
2. **Try a User-Agent** - prefilled with the admin's own; shows the matched rule, epoch and
   dashboard access live, against the rules *as edited*.
3. **Test cases** - name, `User-Agent`, expected epoch; each row shows its current result.
4. **Save** - first asks the server for a preview: cases that would fail, and cases whose epoch
   would change. Saving with failing cases needs an explicit confirmation.
5. **Restore factory rules** - deletes the stored document (confirmation).

All markup lives in templates; the editable rows are `<template>` elements in the page template
that the page's script clones (no markup in C, none built as strings in JS). The initial state is
embedded as JSON in the page.

### Routes

| Method | Path | Does |
|---|---|---|
| `GET` | `/dashboard/settings/browsers` | The page |
| `POST` | `/dashboard/api/browsers/preview` | JSON in (rules, cases, optional UA) → JSON out: validation errors, each case's result under the edited rules and under the active ones, the UA's result. Saves nothing |
| `POST` | `/dashboard/api/browsers` | Validates and saves, reloads the engine. `409` if it would lock out the saving browser (§7) |
| `POST` | `/dashboard/settings/browsers/reset` | Factory rules, reload, `302` back |

## 7. Lock-out protection

The dashboard is served only to browsers the rules grant `dashboard`. A rule saved wrong could send
the admin's own browser elsewhere - leaving no way back in to fix it. So **a save is refused when
the request's own `User-Agent`, classified by the new rules, would lose dashboard access**. The
factory reset needs no such check (the factory rules grant it to every current browser). As a last
resort, deleting the `browser_detection` document from MongoDB restores the factory rules on the
next start.

## 8. Out of scope

- `request_charset.c`'s Cello special case (charset, not epoch).
- `?preview_epoch` and `force_epoch` keep overriding detection exactly as today.
- Recording unmatched `User-Agent`s from real traffic to suggest rules: a natural follow-up (the
  analytics already parse the browser), but it stores raw UAs, so it is its own decision.

## 9. Documentation to update on implementation

`rendering.md` (epoch detection), `routes.md`, `data-model.md`, `dashboard.md`, `security.md`
(lock-out rule), `templates-catalog.md`, the epoch tables in `boat-rudder.md`, `CHANGELOG.md`.
