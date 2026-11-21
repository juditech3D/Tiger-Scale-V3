# Worklog — since the last checkpoint

The single source of truth for everything done since the last commit. Read it at
the start of a session; append to it the moment a change is done, never in a
batch at the end.

The headings match [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), so
at release time this file is synthesised into the `CHANGELOG.md` entry and the
release note in `docs/release-notes/` without being re-derived from the diff.

Keep it clean as you go: describe the **end state**, not the journey. An "Added
X" and a later "Fixed X" from the same cycle collapse into one entry. Anything
reverted disappears entirely — it never shipped. One bullet, one logical change,
always naming the files touched.

Reset this file to the header above at each checkpoint. Nothing is lost: the raw
log lives in git history and the synthesised entry lives in the changelog.

---

### Added

- Timer tool in the start menu (`runTimers`, id `timer`): three timers side by
  side, each a countdown or a stopwatch with an optional name typed on the
  scale's keyboard; ready-made times (8 built in) in NVS `timers`/`list`, edited
  from `/minuterie` (`timer_page.h`, `GET/POST /api/timers`,
  `POST /api/timers/reset`) or grown with "Keep" on the manual screen; a
  chosen time only sets a slot up: each slot starts with its play button and
  stops with its square (back to its full time), the cross empties a slot
  that is not running; the slots' set-ups are kept in NVS `timers`/`slots`, so
  the tool reopens on the same timers, ready and not running. A finished timer
  rings and blinks in its own slot (OK puts it back ready, +1:00 runs one more
  minute); leaving stops every timer and frees the tool's state. `tsKeyboard()` gains optional
  `backTo`, `initial` and `maxLen`. 19 i18n keys (`I18N_TOOL_TIMER`,
  `I18N_TOOL_TIMER_SUB`, `I18N_TMR_*`), CJK faces regenerated,
  `web-installer/tools.js` entry, README tool list.

### Fixed

- The Pages deployment no longer fails on an anonymous GitHub API rate limit:
  `scripts/fetch-published-site.py` and `scripts/verify-published-site.py` send
  the job's `GITHUB_TOKEN` to api.github.com (and only there) when it is set,
  and `.github/workflows/pages.yml` sets it. v3.13.0-mt.6's first deployment
  failed on "cannot read the latest release (HTTP 403)".
- `README.md`: the install button and the `git clone` command point to this
  fork (juditech3d.github.io / juditech3D) instead of the official repository.

### Changed

- `README.md` opens with an "Unofficial alternative firmware" warning (GitHub
  alert) and gains a "What this firmware adds or changes" section: the fork's
  base version, its own installer and releases, tested on the -3.5 only, and the
  tools and fixes it adds over the official v3.13.0.
