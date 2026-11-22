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

- Remote wake without a press: `GET`/`POST :81/wake?c=CODE` on the live-view
  port lights a dark screen (Tiger Scale screensaver or a tool's sleep panel)
  and, on a lit one, only restarts the sleep countdown — it never injects a
  touch, so it cannot press a button when `power_state` is one beat stale.
  `TigerTagSplashESP32.ino` (`gRemoteWakeRequested`, `liveHandleRequest`,
  `runScreensaver`, `toolSleepTick`), `docs/API.md`, `docs/TELEMETRY.md`.

### Fixed

- A scale left on the start menu or in any tool kept sending heartbeats only on
  transitions and was shown offline in Studio while switched on and connected:
  `toolSleepTick()` now runs the periodic heartbeat schedule and the battery
  poll, as `runScreensaver()` does. The cloud worker also refreshes the Firebase
  token before each beat, because the 30-min token tick lives in `loop()`,
  which the screensaver and the tools block for hours. `TigerTagSplashESP32.ino`.
- A scale rebooted and left on the start menu started no web server, no live
  view and no cloud worker (so not one heartbeat) until a tool was picked: the
  launcher opens on `loop()`'s first pass, before those were started. They now
  live in `bootServicesTick()` (with the token tick, the heartbeat schedule and
  the command poll), run by `loop()` and by `toolSleepTick()`.
  `TigerTagSplashESP32.ino`.
- The Pages deployment no longer fails on an anonymous GitHub API rate limit:
  `scripts/fetch-published-site.py` and `scripts/verify-published-site.py` send
  the job's `GITHUB_TOKEN` to api.github.com (and only there) when it is set,
  and `.github/workflows/pages.yml` sets it. v3.13.0-mt.6's first deployment
  failed on "cannot read the latest release (HTTP 403)".
- `README.md`: the install button and the `git clone` command point to this
  fork (juditech3d.github.io / juditech3D) instead of the official repository.

### Changed

- `web-installer/index.html`: new "Already in Tiger Studio Fork" section (`STUDIO_HAS`, keys `st_*`) listing what fork.1-fork.4 ship, with a download button; the Studio items already released leave "Coming soon", which keeps only the macOS build.
- Unofficial-fork warning made unmissable: `README.md` opens with a full
  caution (not published or supported by the TigerTag team, features offered
  upstream may never be merged, problems go to this fork's issues, no
  liability) and its title becomes "Tiger Multi-tools"; `web-installer/index.html`
  gains a warning strip above the header, an "Unofficial fork" badge beside the
  name, the same five-point warning under the hero (keys `strip*`, `badge`,
  `dis_*`, `foot_0`, replacing `notice`) and "(fork non officiel)" in the title.
- `README.md` opens with an "Unofficial alternative firmware" warning (GitHub
  alert) and gains a "What this firmware adds or changes" section: the fork's
  base version, its own installer and releases, tested on the -3.5 only, and the
  tools and fixes it adds over the official v3.13.0.
