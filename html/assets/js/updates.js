// /dashboard/settings/updates (updates_epoch3.html). While an update or a
// rollback runs, polls GET /dashboard/api/updates/progress every 2 s and
// moves the stage list, the message and the log along. The server stops for
// a few seconds while the files are swapped: failed polls then read as "the
// server is restarting" and polling goes on. Once the run is over - or the
// server keeps answering without the progress, as a version without this
// page would - the page reloads, rendered by whatever version runs now.
(function () {
  'use strict';

  var POLL_MS = 2000;
  var GIVE_UP_MS = 30 * 60 * 1000;
  var FINAL = { done: 1, failed: 1, 'rolled-back': 1, none: 1 };

  // One click per form: a second submit would only find the run busy.
  document.querySelectorAll('.boat-rudder-updates__apply').forEach(function (form) {
    form.addEventListener('submit', function () {
      var button = form.querySelector('button[type="submit"]');
      if (button) {
        button.disabled = true;
        button.textContent = 'Starting...';
      }
    });
  });

  var card = document.querySelector('[data-updates-progress]');
  if (!card || card.getAttribute('data-updates-progress') !== 'active') return;

  var message = card.querySelector('.boat-rudder-updates__progress-message');
  var log = card.querySelector('.boat-rudder-updates__log pre');
  var stages = Array.prototype.slice.call(card.querySelectorAll('[data-stage]'));
  var started = Date.now();
  var strange = 0;   // answers that aren't the progress JSON, in a row

  function showStage(name) {
    var current = -1;
    stages.forEach(function (li, i) {
      if (li.getAttribute('data-stage') === name) current = i;
    });
    if (current < 0) return;
    stages.forEach(function (li, i) {
      var state = i < current ? 'done' : i === current ? 'current' : 'pending';
      li.className = 'boat-rudder-updates__stage boat-rudder-updates__stage--' + state;
    });
  }

  function showLog(text) {
    if (!log || log.textContent === text) return;
    var atBottom = log.scrollTop + log.clientHeight >= log.scrollHeight - 4;
    log.textContent = text;
    if (atBottom) log.scrollTop = log.scrollHeight;
  }

  function poll() {
    if (Date.now() - started > GIVE_UP_MS) {
      message.textContent = 'No answer for a long time - reload the page, or check the server.';
      return;
    }
    fetch('/dashboard/api/updates/progress', { credentials: 'same-origin', cache: 'no-store' })
      .then(function (response) {
        if (!response.ok) throw { answered: true };
        return response.json().catch(function () { throw { answered: true }; });
      })
      .then(function (progress) {
        strange = 0;
        if (FINAL[progress.state]) {
          message.textContent = 'Finished - reloading...';
          setTimeout(function () { location.reload(); }, 1000);
          return;
        }
        showStage(progress.stage);
        message.textContent = progress.stage === 'Apply'
          ? 'Swapping the files - the site stops for a few seconds.'
          : progress.message || (progress.stage ? progress.stage + '...' : 'Starting...');
        showLog(progress.log || '');
        setTimeout(poll, POLL_MS);
      })
      .catch(function (error) {
        // Down while restarting. A server that answers, but not with the
        // progress - a version without this endpoint, now running - is
        // asked for the page instead after a few tries.
        if (error && error.answered && ++strange >= 5) {
          location.reload();
          return;
        }
        message.textContent = 'The server is restarting...';
        setTimeout(poll, POLL_MS);
      });
  }

  setTimeout(poll, POLL_MS);
})();
