// /dashboard/settings/browsers - edits the browser detection rules and test
// cases (develop_docs/plans/browser-detection-admin-plan.md).
//
// The page arrives with the rules as JSON in #browserDetection's data-state;
// every row is cloned from a <template> in browsers_epoch3.html, so no markup
// is built here. The DOM is the editing state: collect() reads it back into
// the JSON shape the server takes. Every change asks the server for a dry run
// (/dashboard/api/browsers/preview) - the classification lives in C, once -
// and shows each test case's result, the "Try a User-Agent" box's, and any
// validation error. Save posts the same JSON to /dashboard/api/browsers.
(function () {
  'use strict';

  var root = document.getElementById('browserDetection');
  if (!root) return;

  var SPACE = '␣';   // shown for a space inside tokens: spaces count
  var EPOCH_NAMES = { '-1': 'WAP', '0': 'Text', '1': 'Early', '2': 'Middle', '3': 'Modern' };

  var rulesList = document.getElementById('browsersRules');
  var casesBody = document.getElementById('browsersCases');
  var fallback = document.getElementById('browsersFallback');
  var tryUa = document.getElementById('browsersTryUa');
  var tryResult = document.getElementById('browsersTryResult');
  var errorBox = document.getElementById('browsersError');
  var summary = document.getElementById('browsersCasesSummary');
  var dirtyNote = document.getElementById('browsersDirty');

  var initial = null;       // JSON string of the state as loaded
  var lastPreview = null;   // the last successful dry run
  var previewTimer = null;
  var previewSeq = 0;

  function q(sel, el) { return (el || document).querySelector(sel); }
  function qa(sel, el) { return Array.prototype.slice.call((el || document).querySelectorAll(sel)); }

  function clone(id) {
    return document.getElementById(id).content.firstElementChild.cloneNode(true);
  }

  function fillEpochSelects(el) {
    var options = document.getElementById('browsersEpochOptions').content;
    qa('select[data-epoch-select]', el).forEach(function (select) {
      if (!select.options.length) select.appendChild(options.cloneNode(true));
    });
    if (el.matches && el.matches('select[data-epoch-select]') && !el.options.length)
      el.appendChild(options.cloneNode(true));
  }

  function epochLabel(epoch, dashboard) {
    return epoch + ' ' + EPOCH_NAMES[String(epoch)] + (dashboard ? ' + dashboard' : '');
  }

  // ── tokens: a list <-> one per line, spaces shown as SPACE ─────────────────
  function showToken(t) { return t.replace(/ /g, SPACE); }
  function readToken(t) { return t.split(SPACE).join(' '); }
  function tokensToText(list) { return list.map(showToken).join('\n'); }
  function textToTokens(text) {
    return text.split('\n').map(readToken).filter(function (t) { return t !== ''; });
  }

  function field(el, name) { return q('[data-field="' + name + '"]', el); }

  // ── building rows ──────────────────────────────────────────────────────────
  function addTier(ruleEl, tier) {
    var row = clone('browsersTierTemplate');
    fillEpochSelects(row);
    field(row, 'from').value = tier.from;
    field(row, 'epoch').value = String(tier.epoch);
    field(row, 'dashboard').checked = tier.dashboard;
    q('[data-list="tiers"]', ruleEl).appendChild(row);
    return row;
  }

  function addRule(rule, open) {
    var li = clone('browsersRuleTemplate');
    fillEpochSelects(li);
    field(li, 'name').value = rule.name;
    field(li, 'enabled').checked = rule.enabled;
    field(li, 'contains').value = tokensToText(rule.contains);
    field(li, 'excludes').value = tokensToText(rule.excludes);
    field(li, 'version_after').value = showToken(rule.version_after);
    field(li, 'epoch').value = String(rule.epoch);
    field(li, 'dashboard').checked = rule.dashboard;
    rule.tiers.forEach(function (tier) { addTier(li, tier); });
    if (open) q('details', li).open = true;
    rulesList.appendChild(li);
    return li;
  }

  function addCase(item) {
    var row = clone('browsersCaseTemplate');
    fillEpochSelects(row);
    field(row, 'name').value = item.name;
    field(row, 'user_agent').value = item.user_agent;
    field(row, 'epoch').value = String(item.epoch);
    field(row, 'dashboard').checked = item.dashboard;
    casesBody.appendChild(row);
    return row;
  }

  // ── reading the DOM back ───────────────────────────────────────────────────
  function collectRule(li) {
    return {
      name: field(li, 'name').value,
      enabled: field(li, 'enabled').checked,
      contains: textToTokens(field(li, 'contains').value),
      excludes: textToTokens(field(li, 'excludes').value),
      version_after: readToken(field(li, 'version_after').value),
      tiers: qa('[data-list="tiers"] tr', li).map(function (row) {
        return {
          from: field(row, 'from').value.trim(),
          epoch: Number(field(row, 'epoch').value),
          dashboard: field(row, 'dashboard').checked
        };
      }),
      epoch: Number(field(li, 'epoch').value),
      dashboard: field(li, 'dashboard').checked
    };
  }

  function collect() {
    return {
      fallback_epoch: Number(fallback.value),
      rules: qa(':scope > li', rulesList).map(collectRule),
      cases: qa('tr', casesBody).map(function (row) {
        return {
          name: field(row, 'name').value,
          user_agent: field(row, 'user_agent').value,
          epoch: Number(field(row, 'epoch').value),
          dashboard: field(row, 'dashboard').checked
        };
      })
    };
  }

  // ── rule summaries (the <summary> line of each rule) ──────────────────────
  function refreshSummaries() {
    qa(':scope > li', rulesList).forEach(function (li, i) {
      var rule = collectRule(li);
      q('.boat-rudder-browsers__rule-position', li).textContent = (i + 1) + '.';
      q('.boat-rudder-browsers__rule-name', li).textContent = rule.name || '(unnamed)';
      var outline = rule.contains.map(showToken).join(' | ');
      if (rule.excludes.length) outline += '  not ' + rule.excludes.map(showToken).join(' | ');
      var tiers = rule.tiers.slice().map(function (t) {
        return t.from + ' → ' + epochLabel(t.epoch, t.dashboard);
      });
      tiers.push((rule.tiers.length ? 'else ' : '') + epochLabel(rule.epoch, rule.dashboard));
      q('.boat-rudder-browsers__rule-outline', li).textContent = outline + '  ·  ' + tiers.join(', ');
      q('.boat-rudder-browsers__rule-off', li).hidden = rule.enabled;
    });
  }

  // ── results ────────────────────────────────────────────────────────────────
  function ruleName(state, index) {
    return index < 0 ? 'no rule (fallback)' : (state.rules[index].name || 'rule ' + (index + 1));
  }

  function showError(message) {
    errorBox.textContent = message || '';
    errorBox.hidden = !message;
  }

  function showPreview(state, data) {
    lastPreview = data;
    var failing = 0, changed = 0;
    qa('tr', casesBody).forEach(function (row, i) {
      var cell = q('[data-result]', row);
      var expected = state.cases[i];
      var got = data.cases[i].result, before = data.cases[i].active;
      var ok = got.epoch === expected.epoch && got.dashboard === expected.dashboard;
      var differs = got.epoch !== before.epoch || got.dashboard !== before.dashboard;
      if (!ok) failing++;
      if (differs) changed++;
      cell.textContent = (ok ? '✓ ' : '✗ got ') + epochLabel(got.epoch, got.dashboard) +
        ' · ' + ruleName(state, got.rule) +
        (differs ? ' · today: ' + epochLabel(before.epoch, before.dashboard) : '');
      cell.className = 'boat-rudder-browsers__result' +
        (ok ? '' : ' boat-rudder-browsers__result--fail') +
        (differs ? ' boat-rudder-browsers__result--changed' : '');
    });
    summary.textContent = state.cases.length + ' cases' +
      (failing ? ', ' + failing + ' failing' : ', all pass') +
      (changed ? ', ' + changed + ' changed' : '');

    tryResult.textContent = data.try
      ? 'Epoch ' + epochLabel(data.try.epoch, data.try.dashboard) + ' — ' + ruleName(state, data.try.rule)
      : '';
    showError(data.keeps_dashboard ? '' :
      'With these rules, the browser you are using would lose the dashboard - saving is refused.');
  }

  function preview() {
    var state = collect();
    var seq = ++previewSeq;
    var body = JSON.stringify({
      fallback_epoch: state.fallback_epoch, rules: state.rules, cases: state.cases,
      try_user_agent: tryUa.value
    });
    return fetch('/dashboard/api/browsers/preview', {
      method: 'POST', credentials: 'same-origin',
      headers: { 'Content-Type': 'application/json' }, body: body
    }).then(function (res) {
      return res.json().then(function (data) { return { ok: res.ok, data: data }; });
    }).then(function (r) {
      if (seq !== previewSeq) return null;           // a newer edit is on its way
      if (!r.ok) {
        lastPreview = null;
        showError((r.data.position ? '#' + r.data.position + ': ' : '') + r.data.error);
        return null;
      }
      showPreview(state, r.data);
      return r.data;
    }).catch(function () {
      if (seq === previewSeq) showError('Could not reach the server to check the rules.');
      return null;
    });
  }

  function changed() {
    refreshSummaries();
    var dirty = JSON.stringify(collect()) !== initial;
    dirtyNote.hidden = !dirty;
    clearTimeout(previewTimer);
    previewTimer = setTimeout(preview, 300);
  }

  // ── events ─────────────────────────────────────────────────────────────────
  root.addEventListener('input', changed);
  root.addEventListener('change', changed);

  root.addEventListener('click', function (e) {
    var button = e.target.closest('[data-action]');
    if (!button) return;
    var li = button.closest('.boat-rudder-browsers__rule');
    switch (button.dataset.action) {
      case 'up':
        if (li.previousElementSibling) rulesList.insertBefore(li, li.previousElementSibling);
        break;
      case 'down':
        if (li.nextElementSibling) rulesList.insertBefore(li.nextElementSibling, li);
        break;
      case 'delete':
        if (!confirm('Delete the rule "' + (field(li, 'name').value || 'unnamed') + '"?')) return;
        li.remove();
        break;
      case 'add-tier':
        addTier(li, { from: '', epoch: 3, dashboard: false });
        break;
      case 'remove-tier':
        button.closest('tr').remove();
        break;
      case 'remove-case':
        button.closest('tr').remove();
        break;
      default:
        return;
    }
    changed();
  });

  document.getElementById('browsersAddRule').addEventListener('click', function () {
    var li = addRule({ name: 'New rule', enabled: true, contains: [], excludes: [],
                       version_after: '', tiers: [], epoch: 2, dashboard: false }, true);
    field(li, 'contains').focus();
    changed();
  });

  document.getElementById('browsersAddCase').addEventListener('click', function () {
    field(addCase({ name: '', user_agent: '', epoch: 3, dashboard: false }), 'name').focus();
    changed();
  });

  document.getElementById('browsersTryAdd').addEventListener('click', function () {
    var t = lastPreview && lastPreview.try;
    var row = addCase({ name: 'New case', user_agent: tryUa.value,
                        epoch: t ? t.epoch : 3, dashboard: t ? t.dashboard : false });
    field(row, 'name').select();
    changed();
  });

  document.getElementById('browsersDiscard').addEventListener('click', function () {
    if (dirtyNote.hidden || confirm('Discard every unsaved change?')) location.reload();
  });

  q('.boat-rudder-browsers__reset').addEventListener('submit', function (e) {
    if (!confirm('Replace the rules and test cases with the factory ones?')) e.preventDefault();
  });

  document.getElementById('browsersSave').addEventListener('click', function () {
    var button = this;
    clearTimeout(previewTimer);
    preview().then(function (data) {
      if (!data) return;
      var state = collect();
      var failing = data.cases.filter(function (c, i) {
        return c.result.epoch !== state.cases[i].epoch || c.result.dashboard !== state.cases[i].dashboard;
      }).length;
      if (!data.keeps_dashboard) return;
      if (failing && !confirm(failing + ' test case(s) fail with these rules. Save anyway?')) return;

      button.disabled = true;
      return fetch('/dashboard/api/browsers', {
        method: 'POST', credentials: 'same-origin',
        headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(state)
      }).then(function (res) {
        return res.json().then(function (r) {
          if (res.ok) {
            initial = JSON.stringify(state);
            location.reload();
          } else {
            showError((r.position ? '#' + r.position + ': ' : '') + r.error);
          }
        });
      }).catch(function () {
        showError('Could not reach the server - nothing was saved.');
      }).then(function () { button.disabled = false; });
    });
  });

  window.addEventListener('beforeunload', function (e) {
    if (!dirtyNote.hidden) { e.preventDefault(); e.returnValue = ''; }
  });

  // ── start ──────────────────────────────────────────────────────────────────
  var state = JSON.parse(root.dataset.state);
  fillEpochSelects(root);
  fallback.value = String(state.fallback_epoch);
  state.rules.forEach(function (rule) { addRule(rule, false); });
  state.cases.forEach(addCase);
  initial = JSON.stringify(collect());
  refreshSummaries();
  preview();
})();
