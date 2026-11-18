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

- Installer page: an "Also on the scale" section (sleep everywhere, RFID
  sensitivity, updates announced) between the tools and the install steps, and
  the copyright line in the footer (fork, and the original firmware's MIT
  notice). `web-installer/index.html`.
- Installer page: a heat press memo spotlight right after the hero (the tool
  the project started from, with a mock of its screen), the hero reworded around
  several tools on one workshop device, a note that the author tested the 3.5
  board (3.5B untested), a step 5 "Before first use" (calibrate the scale, run
  RFID Auto) and a feedback section linking to the fork's issues (report a
  problem, suggest a tool). `web-installer/index.html`.
