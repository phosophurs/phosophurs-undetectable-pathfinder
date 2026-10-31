# Phosophurs Undetectable Pathfinder

Experimental Geometry Dash Geode mod project.

## Target
- Windows
- Geometry Dash 2.2081
- Geode SDK 5.11.2

## Build
The GitHub Actions workflow builds a Windows `.geode` artifact on pushes to `main`/ `master` and on manual workflow dispatch.

## Implemented foundation
- Configurable keybinds (default: `4` for autoplay, `Z` for the Geode settings popup).
- Master click-sounds toggle, plus separate keyboard and mouse sound toggles.
- Generated short keyboard-style and mouse-button-style WAV clicks.
- Four subtle waveform variants per sound bank, with optional random variation.
- Gameplay input hook that plays a keyboard or mouse sound based on the currently held Windows mouse-button state.
- Progress-based input variation settings (start percentage, probability, and behaviour mode).

## Development status
This is an early development build, not the completed pathfinder. The current autoplay keybind still changes a status state only. The level-analysis/route solver, actual automated input scheduler, progress-mistake execution, predicted-path renderer, and custom Mega Hack-style tabbed interface remain to be implemented and tested.

The click-audio functions are separated from the input hook so the future pathfinder can explicitly request keyboard or mouse click audio depending on its selected input method.

No anti-cheat bypass or concealment is included.
