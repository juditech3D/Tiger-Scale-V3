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

- Boot lands on a start menu (tool launcher) instead of going straight to the weigh screen: **Tiger Scale** (the spool workflow, unchanged), **Parcel scale** (live weight in grams and TARE, no account or network) and **Calibration & test** (raw HX711 counts, spread, min/max, sample rate, change since a reference, a verdict naming the likely wiring fault, and the existing calibration wizard). Each tool's header returns to the menu; Settings gains a first row, Tools, that reopens it. A new tool is one row in `kLauncherTools`. Seventeen new keys in all eleven languages, the CJK subset regenerated for them, and Montserrat 48 enabled for the parcel weight (`TigerTagSplashESP32/TigerTagSplashESP32.ino`, `TigerTagSplashESP32/i18n.h`, `TigerTagSplashESP32/font_cjk_*.c`, `include/lv_conf.h`).
- A fourth start-menu tool, **Heat press memo**: categories as coloured tiles, then their products (temperature and time), then a detail screen with large temperature and time, pressure, peel and note, and a touch-started countdown with a progress bar that ends on a blinking full-screen alert with the peel reminder and three beeps on the speaker; a touch resets it. The list is `/presse.json` in LittleFS, written with indicative English or French defaults when missing and labelled as such until edited, and edited from a browser at `/presse` through `GET/POST /api/presse` and `POST /api/presse/reset`. Twelve new keys in all eleven languages, the CJK subset regenerated; `playBeeps()` added beside `playSoundTheme()`, sharing its codec bring-up (`TigerTagSplashESP32/TigerTagSplashESP32.ino`, `TigerTagSplashESP32/presse_page.h`, `TigerTagSplashESP32/i18n.h`, `TigerTagSplashESP32/font_cjk_*.c`, `docs/API.md`).

### Changed

- The share picture is tigersystem.io's TigerScale card (`https://tigersystem.io/en/tigerscale/opengraph-image`), the same as the Hub's own page, instead of the site's own `og.jpg` (`web-installer/index.html`).
- The docs no longer say the scale is usable without an account: the weigh workflow only starts with a linked TigerSystem account (`IDLE→SCANNING` requires `firebaseAuth`), so the README feature list, `docs/CLOUD.md` ("An account is required to weigh") and `docs/MAKERWORLD.md` say so; the local brand / material lookups stay described (`README.md`, `docs/CLOUD.md`, `docs/MAKERWORLD.md`).

### Fixed

- The scale reconnects to WiFi on its own after a drop, and comes online when it booted with the router down: `wifiKeepAlive()` starts one attempt with the stored credentials every 30 s while the link is down, from `loop()` and from the start-menu tools' screens, and on success turns modem sleep back off and arms NTP if it never was. Until now the driver's auto-reconnect was off (it returns to the cached BSSID) and nothing replaced it, so a lost connection lasted until a reboot or the WiFi screen (`TigerTagSplashESP32/TigerTagSplashESP32.ino`).
