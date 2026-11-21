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

### Fixed

- The Pages deployment no longer fails on an anonymous GitHub API rate limit:
  `scripts/fetch-published-site.py` and `scripts/verify-published-site.py` send
  the job's `GITHUB_TOKEN` to api.github.com (and only there) when it is set,
  and `.github/workflows/pages.yml` sets it. v3.13.0-mt.6's first deployment
  failed on "cannot read the latest release (HTTP 403)".
- `README.md`: the install button and the `git clone` command point to this
  fork (juditech3d.github.io / juditech3D) instead of the official repository.

### Added

- Resin mixing tool (`runResinMix`, `TigerTagSplashESP32.ino` §9): a "before you
  start" card (adapter, empty cup, confirm) that tares on confirmation; presets
  picked from a three-column grid with the up/down rail when they overflow,
  `-`/`+` adjusting the ratio for the current mix only; a working-time countdown
  offered once the dose of B is reached (beeps 5 min before and at the end).
  Presets live in LittleFS `/resine.json` (8 factory presets, EN/FR), edited from
  a browser at `/resine` (`resine_page.h`, the `/presse` editor's pattern) through
  `GET/POST /api/resine` and `POST /api/resine/reset` (`docs/API.md`). The last
  preset is kept in NVS `config/mixP`; the old `config/mixB` is no longer read.
  i18n keys `MIX_PREP_TITLE`, `MIX_PREP_1..3`, `MIX_PREP_HINT`, `MIX_PREP_OK`,
  `MIX_PICK`, `MIX_STIR`, `MIX_TIMER`, `MIX_POT`, `MIX_POT_LEFT`, `MIX_POT_OVER`,
  `MIX_NEW` in all eleven languages; CJK subset fonts regenerated.

### Changed

- `README.md` opens with an "Unofficial alternative firmware" warning (GitHub
  alert) and gains a "What this firmware adds or changes" section: the fork's
  base version, its own installer and releases, tested on the -3.5 only, and the
  tools and fixes it adds over the official v3.13.0.
