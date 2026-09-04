# cb-pitch-shift — OBS pitch-shift audio filter

An [OBS Studio](https://obsproject.com/) audio filter that transposes a source's
audio up or down by whole semitones **without changing its tempo** — plus a dock
for changing the key with one click during a stream.

Typical use is a karaoke / cover stream: put a backing track on a Browser or Media
source, add this filter, and raise or lower the key to fit the singer — the vocals
on a separate mic source are untouched.

## Features

- **Transpose −12 … +12 semitones**, tempo preserved (a phase-vocoder pitch shift).
- **Bypass at 0 semitones** — no processing and no added latency.
- **Constant ~60 ms audio latency** when active (see the note in the filter). Video is
  not delayed, so add a *Render Delay* filter to the same source if you need exact sync.
- **Dock** ("Pitch Shift Key"): pick the target source, nudge the key with `−` / `+`,
  reset to 0, and read the current value at a glance. When the target's audio isn't
  routed through OBS, the dock flags it (see below).
- **Localized** in English, 繁體中文, 简体中文, 日本語, 한국어, and Español.
- **Windows, macOS and Linux**, OBS **31.1** and newer.

## Install

Download the release for your platform from the *Releases* page, then:

- **Windows** — two options:
  - **Installer** (`…-windows-x64.exe`): double-click and follow the wizard. It is
    **not code-signed**, so Windows SmartScreen shows a one-time "unknown publisher"
    prompt — click *More info → Run anyway*. To remove it later, use *Settings → Apps*
    (or *Control Panel → Programs and Features*) → **Pitch Shift (ChiwaBots.com)** →
    Uninstall.
  - **ZIP** (`…-windows-x64.zip`): copy the `cb-pitch-shift` folder into
    `%ProgramData%\obs-studio\plugins\`. No SmartScreen prompt, but you install and
    remove the files yourself.

  Restart OBS after either.
- **macOS** — open the `.pkg` and follow the installer, then restart OBS. The `.pkg` is
  **not notarized** (free path), so macOS Gatekeeper blocks it on first open — see below.
- **Linux** — two options:
  - **`.deb`** (`sudo apt install ./cb-pitch-shift-*.deb`): installs to `/usr`, which matches
    OBS from the **official OBS PPA** (`ppa:obsproject/obs-studio`) or your distribution's apt
    package. If OBS doesn't pick it up, your OBS is almost certainly installed under a different
    prefix (e.g. a downloaded build under `/usr/local`, or a Flatpak/Snap sandbox) — see below.
  - **Per-user drop-in** (works with *any* non-sandboxed OBS, no `sudo`, survives OBS updates):
    extract the tarball (or copy the `.deb`'s files) into
    `~/.config/obs-studio/plugins/cb-pitch-shift/` so the layout is:

    ```
    ~/.config/obs-studio/plugins/cb-pitch-shift/bin/64bit/cb-pitch-shift.so
    ~/.config/obs-studio/plugins/cb-pitch-shift/data/locale/*.ini
    ```

  Restart OBS after either.

  > **"Installed but OBS doesn't show the filter"** almost always means a path mismatch: OBS
  > scans the plugin dir under **its own** install prefix. Check where OBS keeps its bundled
  > plugins with `dpkg -L obs-studio | grep obs-plugins` — the `.so` has to sit in that same
  > tree. Flatpak/Snap OBS are sandboxed and won't see system-installed plugins at all; use the
  > per-user drop-in, or install OBS from the official PPA.

### macOS: Gatekeeper blocks the `.pkg`

Because the build is unsigned / ad-hoc (no paid Apple Developer account), double-clicking
the downloaded `.pkg` shows *"Apple could not verify … it may contain malware"* with only
**Done** and **Move to Trash** — no *Open* button. This is expected. Click **Done** (not
Move to Trash), then use one of these once:

- **Terminal** — strip the download quarantine, then open the `.pkg` normally:

  ```bash
  xattr -dr com.apple.quarantine ~/Downloads/cb-pitch-shift-*-macos-universal.pkg
  ```

- **or** *System Settings → Privacy & Security → scroll down →* **Open Anyway** next to the
  blocked `.pkg`, authenticate, then open the `.pkg` again.

The installer places `cb-pitch-shift.plugin` in `~/Library/Application Support/obs-studio/plugins/`.
Apple Silicon requires the bundle to carry at least an ad-hoc signature to load at all; the
CI build already applies it, so once the `.pkg` runs the plugin loads in OBS normally.

> To skip this prompt entirely (download-and-run), the `.pkg` would need a paid Apple
> Developer ID signature + notarization — deliberately deferred on the free path.

## Uninstall

- **Windows (installer):** *Settings → Apps* (or *Control Panel → Programs and Features*)
  → **Pitch Shift (ChiwaBots.com)** → Uninstall.
- **Windows (ZIP):** delete the `cb-pitch-shift` folder from `%ProgramData%\obs-studio\plugins\`.
- **macOS:** delete `~/Library/Application Support/obs-studio/plugins/cb-pitch-shift.plugin`.
  A macOS `.pkg` has no built-in uninstaller, so this is a manual step — dragging that one
  bundle to the Trash is the whole uninstall.
- **Linux (.deb):** `sudo apt remove cb-pitch-shift` (or `sudo dpkg -r cb-pitch-shift`).
- **Linux (per-user drop-in):** delete `~/.config/obs-studio/plugins/cb-pitch-shift/`.

Restart OBS afterwards.

## Usage

- **As a filter:** right-click a source → *Filters* → add **Pitch Shift** under audio
  filters, then set the key in semitones.
- **From the dock:** open the *Pitch Shift Key* dock, choose the source, and use `−` / `+`.

> **Important — Browser sources need "Control audio via OBS".**
> This is an audio filter, so it only affects audio that flows through OBS. A Browser
> source sends its sound straight to your speakers by default, so the filter never
> receives it and the pitch shift silently does nothing. Turn on **Control audio via OBS**
> from the source's right-click menu in the Audio Mixer. If the source appears in the
> Audio Mixer, its audio is going through OBS. The dock marks a source with ⚠ and shows
> a hint when its audio isn't routed through OBS.

## Build from source

The build uses the standard [obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate)
tooling: CMake presets download the pinned OBS, obs-deps and Qt6 (see `buildspec.json`)
and the DSP headers on the fly, so you do not need a local OBS build.

Requirements: CMake 3.28+, Git, and a platform toolchain — Visual Studio 2022 (C++ desktop
workload) on Windows, Xcode on macOS, or GCC/Clang + Ninja on Linux.

```bash
# Windows
cmake --preset windows-x64
cmake --build --preset windows-x64

# macOS
cmake --preset macos
cmake --build --preset macos

# Linux
cmake --preset ubuntu-x86_64
cmake --build --preset ubuntu-x86_64
```

Release artifacts for all three platforms are produced by GitHub Actions (see
`.github/workflows/`) on every push.

## Third-party components

- [Signalsmith Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch) and
  [signalsmith-linear](https://github.com/Signalsmith-Audio/linear) — MIT, header-only
  (the pitch-shift DSP), fetched at configure time.
- Qt 6 — used for the dock, linked against the Qt that OBS ships (LGPL/GPL).
- libobs / obs-frontend-api (OBS Studio) — GPL-2.0-or-later.

## License

**GPL-2.0-or-later** — see [LICENSE](LICENSE). This plugin links libobs, which is
distributed under the GNU GPL, so the plugin is released under a compatible license.
