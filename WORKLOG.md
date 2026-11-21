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

- Heat press memo background: six gradient presets (night, ember, ocean,
  workshop, lavender, forest) or the owner's own photo, cropped to 480x320 and
  converted to RGB565 in the editor's browser, shown with an adjustable dark
  veil. Kept apart from the list in `/presse_bg.json` and `/presse_bg.bin`,
  served by `GET/POST /api/presse-bg` and `POST /api/presse-bgimg`
  (`TigerTagSplashESP32.ino` §9 `presseBgLoad`, §19; `docs/API.md`).
- Three memo thumbnails, `puzzle`, `gem` and `paper`, also guessed from
  product names (`scripts/make-presse-icons.py`, `presse_icons.h`).

### Changed

- Heat press memo screens restyled: see-through cards over the background,
  techniques in three columns from five up with their product count and
  temperature range, products with temperature and time pills, and the time
  as a ring that empties while pressing (`runPresseMemo`, `presseGlass`,
  `presseIconBadge`, `presseChip`, `presseGrid` now takes a column count).
- Heat press memo factory list grows from 12 to 38 products in six
  categories (Sublimation, DTF, HTV, Flock, Rhinestones, Transfer paper),
  from Siser, Sawgrass, Brother and DTF supplier sheets. Written only on a
  scale with no list yet (`presse_page.h` `PRESSE_DEFAULTS_EN` / `_FR`).
- The `/presse` editor is restyled and gains the background panel with a
  live preview, a product count per category, and the peel values "Hot or
  cold" and "See sheet" (`presse_page.h`).

- `README.md` opens with an "Unofficial alternative firmware" warning (GitHub
  alert) and gains a "What this firmware adds or changes" section: the fork's
  base version, its own installer and releases, tested on the -3.5 only, and the
  tools and fixes it adds over the official v3.13.0.
