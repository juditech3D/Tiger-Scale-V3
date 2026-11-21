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

### Changed

- `README.md` opens with an "Unofficial alternative firmware" warning (GitHub
  alert) and gains a "What this firmware adds or changes" section: the fork's
  base version, its own installer and releases, tested on the -3.5 only, and the
  tools and fixes it adds over the official v3.13.0.

## Added

- TigerPOD: when one reader holds a Bambu Lab chip and the other sees nothing for 2 s, a
  "Bambu Lab: 1 of 2 chips read" bar appears over the hint with an **RFID Auto** button that
  opens the RFID screen (where Auto picks the power level) and comes back to the TigerPOD.
  Tiger Studio refuses to save a Bambu Lab spool until both of its chips are read; this tells
  the user at the scale what to do. `TigerTagSplashESP32.ino` (`runTigerPod`: `halfBar`,
  `POD_RFID`), `i18n.h` (`I18N_POD_BAMBU_HALF`, 11 languages), `docs/API.md` (TigerPOD socket).
