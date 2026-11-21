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

- Connected parcel scale. The firmware serves `/colis` (`colis_page.h`), the
  weight live with Copy and Tare, and `GET /api/parcel` (`gParcelOpen` set by
  `runParcelScale`). The optional Windows tray program `tools/connector/`
  (`TigerScaleConnector.cs`, `build.cmd`, README) serves that same page at
  `http://127.0.0.1:47800/`, relays the scale's endpoints, and types the weight
  into the active field or a named running program (manual or automatic, once
  per parcel). `.github/workflows/connector.yml` builds it with the compiler
  built into Windows and attaches the `.exe` to each `v*` release.
  `docs/API.md` documents both endpoints. The parcel scale screen shows the
  `/colis` address (white) and points to the connector (`I18N_PARCEL_PC`,
  `I18N_PARCEL_CONNECTOR`), and starts the web server itself when opened
  straight from the start menu; the page served by the scale links the
  connector's download.

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
