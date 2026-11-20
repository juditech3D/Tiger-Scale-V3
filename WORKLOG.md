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

- TigerPOD tool reads Bambu Lab chips: a 4-byte-UID MIFARE Classic is
  authenticated sector by sector with the key A derived from its UID
  (HKDF-SHA256, the Bambu Research Group method, same derivation as Studio's
  `services/bambu-tag.js`), and the data blocks of sectors 0–4 go out in the
  `card` message as `bambuBlocksHex`, for Studio to decode. Read only; any other
  chip's message is unchanged. `PN532Reader::mifareAuthA()` /
  `readMifareBlock()` on all three transports. Files:
  `TigerTagSplashESP32/TigerTagSplashESP32.ino` (§9, PN532Reader),
  `TigerTagSplashESP32/pod_reader.h`, `docs/API.md`.
