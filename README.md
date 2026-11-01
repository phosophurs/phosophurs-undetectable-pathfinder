# Phosophurs Pathfinder

An experimental Windows Geometry Dash mod built with Geode.

## Compatibility
- Geometry Dash: Windows 2.2081
- Geode SDK: 5.11.2
- Build target: Win64

## Working features
- Five-tab in-game menu, opened with **Z** by default.
- Configurable Geode keybinds.
- Attempt input recording, including press and release timestamps.
- Replay of the last completed attempt with **4** by default.
- Replay pause/resume keybind. Pausing shifts the remaining event timestamps so inputs do not all fire immediately on resume.
- Generated keyboard-style and mouse-style click audio, with separate enable switches and in-menu previews.
- Four waveform variants per sound bank, with optional variation.

## Experimental / limited
- Physics route planner: attempts a simple vertical-jump route against a small list of recognised spike and saw object IDs. It requires a calibration run and a restart before recalculation. It is not a general Geometry Dash solver and may fail on unusual hitboxes, speed changes, platforms, slopes, portals, custom objects, or non-cube gameplay. Treat generated routes as experimental and test them in practice mode.
- Mouse-position offset helper exists in source but is not connected to live gameplay input.
- Predicted-path rendering is not implemented.
- Progress-based input variation is not implemented.

## Replay workflow
1. Enter a level and play one attempt. The mod records the gameplay inputs.
2. After dying, restart the level.
3. Press **4** to replay the recorded attempt.
4. Use the configured pause key to pause/resume playback.

The previous attempt is retained across a retry. Leaving the level clears the in-memory route.

## Physics planner workflow
1. Play a short cube attempt so the mod can estimate horizontal movement and jump impulse.
2. Restart the level.
3. At the start line, use the configured recalculation key.
4. If a route is generated, press **4** to play it.

The planner is intentionally conservative and supports only a small subset of standard geometry. A successful build does not mean a route has been validated in-game.

## Click audio
The mod generates its own short WAV samples at runtime. Input-source detection is heuristic, so some inputs may be classified incorrectly. Audio previews are available in the Sounds tab.

## Build
Pushes to `main` trigger the Windows Geode build workflow. A successful run uploads a `Phosophurs-Windows-geode` artifact for 14 days. The repository's Actions tab shows build status and artifacts.

## Integrity
This project does not include anti-cheat bypasses, stealth mechanisms, or detection-evasion features. The project name is not a claim that the mod is undetectable.
