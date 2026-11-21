# TigerScale software connector

The PC side of the connected parcel scale. Optional: the scale's own page,
`http://tigerscale-XXXX.local/colis`, already shows the weight live and copies
it in one click, on any device, with nothing installed. The connector adds the
one thing a web page is not allowed to do: **type the weight into another
program** — the active field, or a program you name.

## Use

1. Download `TigerScaleConnector.exe` from the latest release (the firmware
   install page links to it). It is a single ~35 KB file; nothing is
   installed. Windows SmartScreen shows "Windows protected your PC" the first
   time because the file is not signed: *More info → Run anyway*.
2. It opens `http://127.0.0.1:47800/` in your browser and sits next to the
   clock. On the first run, enter the scale's address or press *Search the
   network*.
3. Open **Parcel scale** on the scale. The page shows the weight, and:
   - **Copy** — to the clipboard;
   - **Paste into the active field** — goes back to the window you were in
     just before the browser and types the weight there;
   - **Paste into the chosen program** — brings that program to the front and
     types the weight (path such as `C:\Program Files\…\App.exe`, or just a
     name such as `notepad`). The program must already be running;
   - **Automatic paste** — once the weight is stable, once per parcel; lift
     the parcel to re-arm;
   - unit (g / kg), decimal separator, unit suffix, and Tab or Enter after.

The tray menu has *Start with Windows* (a `HKCU\…\Run` entry, no admin rights).
When started that way it stays in the tray; double-click the icon to open the
page.

## How it works, and why

```
browser ── http://127.0.0.1:47800 ──► connector ── http://<scale>/colis, /api/parcel, /api/tare ──► scale
                                         │
                                         └─ /paste → SendInput into the target window
```

- **The connector serves the page, rather than the page calling the
  connector.** Chrome and Edge refuse a plain-http page on the local network any
  request to `127.0.0.1` (Local Network Access is limited to secure contexts),
  and the scale serves no https. Loopback to loopback is not restricted, so the
  page is fetched from the scale and served from here, with the scale's two
  endpoints relayed. One page source, in the firmware (`colis_page.h`); it hides
  the Paste controls when it is not served by the connector.
- **Typing uses Unicode key events**, so AZERTY/QWERTY does not matter and the
  clipboard is left untouched.

### What keeps it from being misused

A program that types into other windows on request has to be narrow:

| Limit | Against |
|-------|---------|
| Listens on `127.0.0.1` only | anything else on the network |
| Answers only when `Host` is `127.0.0.1:47800` or `localhost:47800` | DNS rebinding: a page reaching it under another name |
| Accepts a `POST` only with its own page's `Origin` | any other website posting to it from your browser |
| Types only `^[0-9]{1,6}([.,][0-9]{1,3})?( ?(g\|kg))?$` | being turned into a general keystroke injector |
| Never starts a program, only brings a running one forward | a request launching an arbitrary path |
| The scale address is a host or IPv4 (optionally `:port`), never a URL | the relay being pointed at arbitrary paths |

## Build

`build.cmd` uses the C# compiler that ships inside Windows
(`%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe`), so building needs
nothing installed either. That compiler is C# 5: no string interpolation, no
`?.`, no expression-bodied members. CI (`.github/workflows/connector.yml`)
builds it on every pull request that touches it, and attaches the `.exe` to the
release of each `v*` tag. It never creates a release itself, so it cannot move
what the scales' OTA reads.

## Not done

- **Bluetooth keyboard mode** (the scale pairs as a BLE keyboard and types the
  weight itself, nothing on the PC at all). Set aside because the scale is not
  necessarily within Bluetooth range of the PC; worth reconsidering if someone
  wants it with the scale on the desk.
- **Signing** the `.exe`, which would remove the SmartScreen warning. It needs a
  code-signing certificate.
- macOS / Linux. The page's Copy button works there today.
