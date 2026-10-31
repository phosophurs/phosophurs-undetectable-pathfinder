# Phosophurs Undetectable Pathfinder

An experimental Windows Geometry Dash mod built with Geode. This project is a work-in-progress input/audio and pathfinding foundation; it is **not yet an automatic level solver**.

## Compatibility
- Geometry Dash: Windows 2.2081
- Geode SDK: 5.11.2
- Build target: Win64

## Current features
- Five-tab in-game menu (Autoplay, Pathfinder, Sounds, Visuals, Settings), opened with **Z** by default.
- Configurable keybinds through Geode's settings UI.
- Autoplay request toggle on **4** by default. At this stage it changes the request/status state only; it does not control the player.
- Separate generated keyboard-style and mouse-style click sounds.
- Four waveform variants per sound bank, with optional random variation.
- In-menu keyboard and mouse sound previews.
- Master, keyboard, mouse, and waveform-variation audio settings.
- Optional random mouse-click offset helper, configurable from 0–12 px. Frame-perfect inputs always receive zero offset; keyboard inputs are unaffected.
- Optional progress-variation settings for future input scheduling.

## Not implemented yet
- Level analysis and route solving.
- Automated gameplay input scheduling.
- Pause/resume of an active automated run.
- Progress-based mistake execution.
- Predicted path rendering and hazard/collision preview.
- Applying the humanised offset helper to live mouse input.

The click-sound hook currently infers mouse input from the Windows mouse-button state. This is a heuristic and cannot reliably identify every input source. The audio preview buttons provide a direct way to test each sound bank.

## Build
Pushes to the main branch trigger the Windows Geode build workflow. A successful run uploads a `Phosophurs-Windows-geode` artifact. You can also run the workflow manually from the repository's Actions tab.

## Safety and integrity
This project does not include anti-cheat bypasses, stealth mechanisms, or detection-evasion features. The name is a project name, not a claim that the mod is undetectable.
