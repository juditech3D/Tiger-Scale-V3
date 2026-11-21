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

### Changed

- Parts counter: opening the tool now shows a "before you start" card, laid out like the resin tool's (empty counting tray on, hands off, confirm); confirming tares, so the wizard starts on the reference parts and has two steps instead of three. TARE stays on screen as a plain re-zero. `TigerTagSplashESP32.ino` (`runPartsCounter`), `i18n.h` (`I18N_CNT_PREP_TITLE/_1/_2/_3/_HINT/_OK` added in all eleven languages, `I18N_CNT_TRAY` removed; `I18N_CNT_SAME` now reads "Same parts as the last weighing", on two lines in a taller button), `font_cjk_14/16/20.c` regenerated.
