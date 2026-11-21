# Scale API reference

Everything a TigerScale exposes on the LAN: every HTTP endpoint, the WebSocket
in both directions, what each field means and where the gaps are. This is the
document to hand to anything that talks to a scale directly — Tiger Studio
Manager, a dashboard, a script.

Two companions, deliberately separate:

- [`docs/TELEMETRY.md`](TELEMETRY.md) — what the scale *pushes to the cloud*, the
  Firestore heartbeat. That is the channel Studio Manager reads when the scale is
  not on the same network.
- [`docs/CLOUD.md`](CLOUD.md) — the privacy question: what leaves the device and
  how to wipe it.

This file covers the local channels. Where a field exists on more than one, the
name and meaning are the same.

## Which channel for what

Three channels, and the split is a rule rather than an accident:

| Channel | Carries | Read it when |
|---|---|---|
| Firestore heartbeat | slow state: battery, power, signal, firmware, account, counters | you need it from anywhere, without being on the scale's network |
| WebSocket `/ws` | fast state: live weight, tag being read, workflow phase | you need it now, and you are on the LAN |
| HTTP | commands, and one complete snapshot | you want to *do* something, or resynchronise on demand |

`GET /api/status` is the full snapshot in one request — including the power
fields, so it answers on its own without a socket. That is what it is for.

## Before anything else: there is no authentication

`Access-Control-Allow-Origin: *` is sent on every response, deliberately without
an `Access-Control-Allow-Headers` and without an `OPTIONS` handler. Simple
requests and their responses work — a browser client can read `/api/status`,
`/api/ping`, and the result of `POST /api/tare`. Anything needing a preflight,
which means every JSON-bodied POST such as `/api/calibration` or
`/api/firebase/token`, still fails. On an API with no authentication that
asymmetry is the only guard rail available.

**Every endpoint below is unauthenticated.** Anyone who can reach the scale's IP
can read its state, trigger a tare, change its calibration, log it out of its
account or factory-reset it. That is a deliberate simplification for a device on
a home network, not an oversight — but it means a scale does not belong on an
untrusted network without a VLAN in front of it.

The one exception is the live view on port 81, which asks for the six-character
code from Settings → LAN.

## Base

```
http://<scale-ip>/          or  http://tigerscale-XXXX.local/
ws://<scale-ip>/ws
http://<scale-ip>:81/       live view, code-protected
```

`mdns` in `/api/status` gives the hostname; the last four characters come from
the MAC. The IP is not stable — resolve the mDNS name rather than hard-coding an
address.

## One server

The firmware used to carry two HTTP server implementations, only one of them
compiled in. The dead one has been removed — with it, 30 duplicate route
registrations, six helper functions with no caller left, and about 750 lines
that could never run.

Two of its routes were not duplicates: `GET /api/session` and `GET /api/history`
existed **only** there, and therefore answered 404 on every scale ever shipped.
They were ported onto the live server rather than deleted with the rest.

## Reading state

### `GET /api/status`

The main read. One JSON object, no parameters. Verified response:

```json
{
  "weight": 7,
  "rawWeight": 7.409999847,
  "uid": "", "uid_hex": "", "uid2": "", "uid2_hex": "",
  "uid_left": "", "uid_right": "", "uid_twin": "",
  "wifi": "Stargate",
  "ip": "192.168.20.143",
  "mdns": "tigerscale-XXXX.local",
  "cloud": true,
  "firebaseConfigured": true,
  "firebaseAuth": true,
  "firebaseEmail": "benoit@atome3d.com",
  "firebaseDisplayName": "Open Maker",
  "calibrationFactor": 461.0623474,
  "servoEnabled": false,
  "uptime_ms": 71756188,
  "uptime_s": 71756,
  "fw_version": "3.6.0",
  "fw_git_sha": "dev",
  "ota_status": "idle",
  "ota_progress": 0,
  "ota_latest": "3.6.0",
  "scaleStatus": "idle",
  "wfPhase": "",
  "wfSlope": 0,
  "containerWeight": -1,
  "db_updating": false,
  "db_brands": 120,
  "db_materials": 113,
  "db_checked_s": -1
}
```

| Field | Meaning |
|---|---|
| `weight` | Displayed weight in grams, rounded and filtered. This is the number on screen. |
| `rawWeight` | Unfiltered load-cell reading. Diagnostics only — it moves constantly. |
| `uid` / `uid_hex` | Tag UID of the spool on the platform, empty when none |
| `uid2` / `uid2_hex` | Second tag, for a twin-tagged spool |
| `uid_left` / `uid_right` | Per-reader UIDs, which physical reader saw what |
| `uid_twin` | The paired tag resolved from the inventory |
| `wifi` | **SSID currently associated** |
| `ip`, `mdns` | Network identity |
| `cloud` | WiFi is connected |
| `firebaseConfigured` / `firebaseAuth` | An account is set up / signed in |
| `firebaseEmail` / `firebaseDisplayName` | The signed-in account |
| `calibrationFactor` | Load-cell scale factor |
| `containerWeight` | Empty-spool weight in grams for the tag on the platform, **`-1` when unknown** — not zero |
| `scaleStatus` | `idle`, `scanning`, `stable`, `sending`, `success`, `error` |
| `wfPhase` / `wfSlope` | Weigh-workflow internals |
| `fw_version` / `fw_git_sha` | Running firmware |
| `ota_status` / `ota_progress` / `ota_latest` | Update state, 0-100, and the newest version seen |
| `uptime_ms` / `uptime_s` | Since boot |
| `db_*` | Local brand/material database: sizes, whether a sync is running, age of the last check in seconds (`-1` = never) |

**`containerWeight: -1` means unknown, not empty.** A dashboard that renders it
as a number will show "-1 g".

### Other reads

| Endpoint | Returns |
|---|---|
| `GET /api/ping` | the literal text `pong`. Liveness only. |
| `GET /api/ota/check` | `{"success":true,"current":"3.6.0","current_sha":"dev","latest":"3.6.0","latest_sha":"7c3f…","latest_littlefs_sha":"6c7f…","update_available":false}` — forces a manifest fetch |
| `GET /api/firebase/status` | `{"configured":true,"auth":true,"email":"…"}` |
| `GET /api/hw/config` | `{"rfidCount":2,"rfidSide":"right","motorConnected":false,"motorEnabled":false,"motorSpeed":0}` |
| `GET /api/rfid/test` | `{"active":false,"reader_left":true,"reader_right":true,"uid_left":null,"uid_right":null}` — reader health |
| `GET /api/session` | the weigh session in flight: `sessionId`, `workflowPhase`, `sendPhase`, `sendCountdown`, `currentWeight`, `containerWeight`, `containerFetched`, `peakWeight`, `currentSlope`, the five UIDs, `autoTarePending`, `rfidLocked`, the lifetime counters, and a `lastSend` object with the previous measurement |
| `GET /api/history` | an in-RAM event ring as a JSON array: `ms`, `ageMs`, `type` (`auto_tare`, `boot_ready`, …), `message`, `uid1`, `uid2`, `weight`, `phase`. Survives nothing — it is cleared on reboot. |
| `GET /api/logs` | the last ~80 log lines held in RAM. **The only way to read logs once the USB cable is out.** |
| `GET /` , `/style.css`, `/app.js` | the scale's own web UI |
| `GET /live` | 302 to the live view on port 81 |

### The screen

Two ways in, for two jobs.

**Port 80, to check a screen.** No code, like every other route here.

| Endpoint | Returns |
|---|---|
| `GET /screen.bmp` | what LVGL last drew, 480x320, as a 24-bit BMP (about 450 KB) |
| `GET /api/tap?x=&y=` | presses the panel at that point; add `x2`/`y2` for a swipe. Answers once the press was taken and the screen has finished changing, so it chains with `/screen.bmp` without a delay |
| `GET /screen.ver` | four bytes of frame counter, to poll cheaply |
| `GET /screen` | the picture in a page: click to tap, drag to swipe |

A press on a dark screen wakes it and is swallowed, as a finger's would be.

**Port 81, to watch it.** The real-time stream behind Settings > LAN > Live view,
guarded by the six-character code from that screen.

| Endpoint | Returns |
|---|---|
| `GET :81/stream?c=CODE` | `200`, a chunked binary stream of frames, or `403` for a wrong code. Two viewers at most. |
| `POST :81/tap?c=CODE&x=&y=` | `204`; a press (the method is not checked) |
| `GET :81/` or `GET :81/CODE` | the viewer page |

`/stream` and its `403` carry `Access-Control-Allow-Origin: *`, so a page can
fetch a frame directly. The message format, and how to take a single-frame
screenshot, are in [TELEMETRY.md](TELEMETRY.md#lan-live-view-the-screen-itself).

**From the account, without asking anyone.** The heartbeat publishes
`lan_live`, `lan_access_code` and `lan_url` (`http://<ip>:81`, or null when live
view is off or there is no address) in `users/{uid}/scales/{mac}`, so the
owner's own apps can open the stream with no code typed.
`lan_access_code` is a secret: that document is owner-only, and the code must
never be copied anywhere shared, nor into a log.

## Commanding the scale

All of these are `POST`. Bodies are JSON unless noted.

| Endpoint | Body | Effect |
|---|---|---|
| `/api/tare` | none | Zeroes the scale and clears the current tag session. Persists the offset. |
| `/api/calibration` | `{"factor": 461.06}` (or `{"value": …}`) | Sets the load-cell factor |
| `/api/weight` | see below | Pushes a weight to the cloud for the current tag |
| `/api/push-weight` | same | Same handler, kept for the web UI |
| `/api/workflow/stop` | none | Aborts the running weigh workflow |
| `/api/rfid/test` | `{"stop":true}` / `{"reset":true}` | Drives the reader self-test |
| `/api/hw/config` | `{"rfidCount":2,"rfidSide":"right"}` | Reader count and which side is primary |
| `/api/servo-toggle`, `/api/servo/test` | — | No motor exists on V3; these are for the motorised variant |
| `/api/update-db` | none | Triggers a brand/material database sync |
| `/api/ota/check` | — | see reads |
| `/api/ota/update` | `{"firmware_url":"…","littlefs_url":"…"}` | Starts an over-the-air update |
| `/api/firebase/auth` | `{"email":"…","password":"…"}` | Signs in |
| `/api/firebase/token` | `{"idToken":"…","refreshToken":"…","email":"…","displayName":"…"}` | Attaches an existing session |
| `/api/firebase/logout` | none | Clears the stored session |
| `/api/set-apikey` | `{"apiKey":"…"}` | Overrides the Firebase web key |
| `/api/logs` `DELETE`, `/api/logs/clear` | none | Empties the log ring |
| `/api/reset-wifi` | none | **Destructive.** Erases stored WiFi credentials. |
| `/api/factory-reset` | none | **Destructive.** Erases the whole NVS partition — WiFi, account, calibration, language. |

The two destructive ones take no confirmation and no authentication. Anything
that exposes them in a UI should confirm on its own side.

### Heat press memo

The list behind the start menu's heat press memo tool, kept in LittleFS as
`/presse.json` (at most 16 KB). Like everything here it has no authentication.

| Endpoint | Body | Effect |
|---|---|---|
| `GET /presse` | — | The list editor: categories, products, export and import |
| `GET /api/presse` | — | The list as JSON (the factory list when none is saved yet) |
| `POST /api/presse` | the whole list | Replaces it; `400` if it is not an object with a `categories` array. An open memo screen reloads to its categories |
| `POST /api/presse/reset` | none | Puts the factory list back (indicative values) |

`{"settings":{"defaults":false},"categories":[{"name":"DTF","color":"#2980B9","items":[{"name":"Cotton","temp":160,"time":15,"pressure":"Firm","peel":"Cold","note":""}]}]}`
- `temp` 0-250 °C and `time` 0-999 s are clamped on the screen; a time of 0 leaves the timer unavailable.
- `settings.defaults` is `true` only on the factory list, and is what makes the screen call the values indicative; the editor clears it on save.

### Resin presets

The presets behind the start menu's resin mixing tool, kept in LittleFS as
`/resine.json` (at most 8 KB). No authentication either.

| Endpoint | Body | Effect |
|---|---|---|
| `GET /resine` | — | The preset editor: name, colour, ratio, working time, note, export and import |
| `GET /api/resine` | — | The presets as JSON (the factory list when none is saved yet) |
| `POST /api/resine` | the whole list | Replaces it; `400` if it is not an object with a `presets` array. An open mixing screen picks the change up |
| `POST /api/resine/reset` | none | Puts the factory presets back (indicative values) |

`{"settings":{"defaults":false},"presets":[{"name":"Epoxy casting 2:1","color":"#3B8FE0","a":2,"b":1,"pot":40,"note":""}]}`
- `a` : `b` is the ratio **by weight**; a preset where either is not above 0 is skipped by the scale.
- `pot` is the working time in minutes, 0-999; 0 means no timer after the dose.

### Start menu tools

Which tools the start menu shows, and in which order. Tiger Scale is always shown and always
first; a hidden tool is still in the firmware and comes back the moment it is shown again. Kept
in NVS (`config`/`toolsOff`, `config`/`toolsOrder`, both lists of tool ids). Ids: `tigerscale`,
`pod`, `parcel`, `caltest`, `presse`, `count`, `mix`.

| Endpoint | Body | Effect |
|---|---|---|
| `GET /tools` | — | A page to tick and order the tools |
| `GET /api/tools` | — | `{"tools":[{"id":"pod","name":"TigerPOD","sub":"…","shown":true}, …]}`, in menu order |
| `POST /api/tools` | form fields `shown`, `order` (comma-separated ids) | Sets both; an open start menu rebuilds |

The same choice travels over the USB serial port (115200 baud), which is how the web installer
sets it right after a flash, while the scale is still on its first-boot screens:

| Line sent | Answer |
|---|---|
| `TOOLS?` | `TOOLS on=<shown ids, in order> order=<all ids, in order>` |
| `TOOLS=pod,parcel` | Shows those tools, in that order, after Tiger Scale; hides the others. `TOOLS OK on=… order=…` |
| `TOOLS=all` | Shows every tool, order unchanged |

Unknown ids are skipped, so `TOOLS=none` leaves Tiger Scale alone, and the scale then boots
straight to the weigh screen.

## The WebSocket

```
ws://<scale-ip>/ws
```

### Outbound: the scale to you

A JSON object per message. **Delta-compressed**: a field appears only when its
value changed. A full snapshot arrives on connect and every 30 seconds; between
those, expect messages carrying two or three keys.

**An absent field means unchanged, never null.** A client must keep the previous
value.

Live fields, sent whenever they change:

`weight`, `netWeight`, `containerWeight`, `uid`, `uid2`, `uid_left`,
`uid_right`, `uid_twin`, `scaleStatus`, `brand`, `material`, `color`, `cloud`,
`firebaseAuth`, `db_updating`, `battery_present`, `battery_percent`,
`is_charging`, `power_source`, `power_state`, `wifi_signal_dbm`.

Snapshot-only fields, on connect and every 30 s: `firebaseConfigured`,
`firebaseEmail`, `firebaseDisplayName`, `calibrationFactor`, `uptime_s`,
`fw_version`, `db_ok`, `db_checked_s`.

The power fields carry the same meaning as in
[`TELEMETRY.md`](TELEMETRY.md#power-and-battery), with one deliberate difference:
JSON has no typed null in this encoding, so **`battery_percent` is `-1`, not
null, when no cell is fitted**, and `wifi_signal_dbm` is `0` when not connected.
`battery_present` is what you branch on.

`scaleStatus` on the WebSocket is richer than on `/api/status`: during a send it
carries a countdown, as `scanning:3`, `stable:2` or a bare number.

### Inbound: you to the scale

**Nothing.** The handler parses incoming JSON and discards it — the code says so:
*"No WS command types currently needed"*. There are no commands, no
subscriptions, no acknowledgements.

To command a scale, use the HTTP endpoints above. The WebSocket is a one-way
state feed.

## The TigerPOD socket

`ws://<scale>/pod?c=<LAN access code>` lends the scale's two RFID readers to Tiger Studio, the
way a TigerPOD (a USB reader on the computer) is used. It is the one authenticated channel: the
handshake is refused unless the code matches **and** the TigerPOD tool is open on the scale's own
screen; closing the tool drops every client. Studio connects on its own (`services/scale-pod.js`)
to the scales of the account that publish `lan_url` and `lan_access_code`.

| Direction | Message |
|---|---|
| scale → client | `{"type":"reader-connected","reader":"1"}` (and `"2"`), sent on connect |
| scale → client | `{"type":"card","reader":"1","uid":"04a1…","atr":"3b8f…","rawPagesHex":"…"}` — pages 4–39 of an NTAG, `null` for a 4-byte-UID MIFARE Classic. A Bambu Lab chip also carries `"bambuBlocksHex":{"0":"…","1":"…",…}`: the data blocks of sectors 0–4, 32 hex characters each, opened with the key A each sector derives from the UID (HKDF-SHA256). The field is absent on any other chip |
| scale → client | `{"type":"card-removed","reader":"1"}` |
| client → scale | `{"type":"read-now","reader":"1","id":7,"start":4,"length":144}` → `{"type":"read-result","id":7,"ok":true,"rawPagesHex":"…"}` |
| client → scale | `{"type":"write-now","reader":"1","id":8,"verify":true,"pages":[{"index":4,"hexData":"01020304"}]}` → `{"type":"write-result","id":8,"ok":true,"pagesWritten":1,"verified":true,"mismatchPages":[]}` |

Pages below 4 (UID, lock bits, capability container) and above 225 are refused, every page of a
write is checked before the first one is written, and a failure answers `"ok":false` with an
`"error"`. The `atr` is the one an ACR122U reports for that chip family, which is what Studio
tells NTAG from MIFARE Classic by.

## Coverage: where each value lives

| What you want | `/api/status` | WebSocket | Firestore |
|---|---|---|---|
| Weight | yes | yes | `current_weight_g` |
| Net weight | no | `netWeight` | via last measurement |
| Container weight | yes (`-1` = unknown) | yes | `container_weight` on the tag doc |
| Tag UIDs | yes, all five | yes | `current_spool_uid_1` / `_2` |
| Brand / material / colour | no | yes | no |
| **USB or battery** | **no** | `power_source` | `power_source` |
| **Charging** | **no** | `is_charging` | `is_charging` |
| **Charge %** | **no** | `battery_percent` | `battery_percent` |
| **Battery fitted** | **no** | `battery_present` | `battery_present` |
| **Screen state** | **no** | `power_state` | `power_state` |
| WiFi SSID | `wifi` | no | no |
| **WiFi signal** | **no** | `wifi_signal_dbm` | `wifi_signal_dbm` |
| Firmware version | yes | yes | `fw_version` |
| Account | yes | yes | `display_name` |
| Calibration | yes | yes | `calibration_factor` |
| Counters, session stats | no | no | yes |
| LAN live view: on, code, URL | no | no | `lan_live`, `lan_access_code`, `lan_url` |

## Known gaps

These are real, current, and worth deciding on rather than working around.

- **The WebSocket cannot be commanded.** A client that wants to tare has to fall
  back to HTTP, which means holding two connections for one job.
- **No authentication anywhere**, including on the two destructive endpoints. A
  web page cannot *read* what it is not allowed to, but it has always been able
  to *send* a simple POST — `/api/tare`, `/api/reset-wifi`, `/api/factory-reset`
  take no body and are therefore reachable from any site the owner visits. CORS
  changes nothing about that; only an access token would.

## Where this lives in the firmware

| What | Where |
|---|---|
| HTTP routes | §19 Web server, the `server.on(...)` block |
| WebSocket events | §13, `onWsEvent()` |
| WebSocket payload | §15, `buildWsFrame()` |
| Firestore payload | `sendScaleHeartbeat()` |
| Battery and power state | `gBattery`, refreshed by `pollBatteryState()` |

Line numbers are in [`CODEMAP.md`](../CODEMAP.md), which is regenerated — do not
copy them here.
