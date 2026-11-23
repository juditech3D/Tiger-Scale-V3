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

- Settings -> Mode chooses between Tiger Scale (weighing) and TigerPOD: in TigerPOD mode the scale lends its two readers to Tiger Studio over `ws://<scale>/pod?c=<LAN access code>`, speaking the messages of Studio's own reader process (`reader-connected`, `card`, `card-removed`, `read-now` / `read-result`, `write-now` / `write-result`), so Studio can treat each reader as one more TigerPOD. NTAG / Ultralight pages only, pages 0-3 never written, socket closed unless the mode is on screen. The choice is kept in NVS (`podMode`) and a restarted scale comes back in it; the back arrow returns to weighing. The mode's screen sleeps with the same setting as the weigh screen and wakes on a tap, a chip or `/wake` (`runTigerPodMode`, `runModeSettings`, `podSleepTick` in the .ino, new `pod_reader.h`, `PN532Reader::writePage` in the three transports, 11 new i18n keys `I18N_MODE` ... `I18N_POD_HINT`, CJK subset regenerated).
- `GET /wake?c=<code>` on the live port lights a dark screen without pressing anything (an already-lit one only restarts its sleep countdown), for a Wake button in Tiger Studio (`liveHandleRequest`, `runScreensaver`, `gRemoteWakeRequested`).

### Fixed

- A scale whose screen had slept for over an hour sent its heartbeat with an expired token, which was refused, so Studio counted it as gone: the cloud worker now renews the token before each beat (no-op while fresh). The token / heartbeat / command-poll schedule moved out of `loop()` into `cloudScheduleTick()` so the TigerPOD mode, which blocks `loop()`, keeps it running.

### Changed

- The share picture is tigersystem.io's TigerScale card (`https://tigersystem.io/en/tigerscale/opengraph-image`), the same as the Hub's own page, instead of the site's own `og.jpg` (`web-installer/index.html`).
- The docs no longer say the scale is usable without an account: the weigh workflow only starts with a linked TigerSystem account (`IDLE→SCANNING` requires `firebaseAuth`), so the README feature list, `docs/CLOUD.md` ("An account is required to weigh") and `docs/MAKERWORLD.md` say so; the local brand / material lookups stay described (`README.md`, `docs/CLOUD.md`, `docs/MAKERWORLD.md`).
