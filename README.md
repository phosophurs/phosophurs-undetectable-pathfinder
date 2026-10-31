# Phosophurs Undetectable Pathfinder

Experimental Geometry Dash Geode mod project.

## Target
- Windows
- Geometry Dash 2.2081
- Geode SDK 5.11.2

## Build
The GitHub Actions workflow builds a Windows `.geode` artifact on pushes to `main` / `master` and on manual workflow dispatch.

## Implemented foundation
- Configurable keybinds (default: `4` for autoplay, `Z` for the Geode settings popup).
- Master click-sounds toggle, plus separate keyboard and mouse sound toggles.
- Generated short keyboard-style and mouse-button-style WAV clicks.
- Four subtle waveform variants per sound bank, with optional random variation.
- Gameplay input hook that plays a keyboard or mouse sound based on the currently held Windows mouse-button state.
- Progress-based input variation settings (start percentage, probability, and behaviour mode).
- **Humanised mouse-click offset engine:** optional random positions inside a configurable 0–12 px radius (default 3 px when enabled). Offsets are uniformly distributed inside a circle. Frame-perfect clicks always get a zero offset; keyboard inputs are unaffected.

## Development status
This is an early development build, not the completed pathfinder. The current autoplay keybind still changes a status state only. The level-analysis/route solver, actual automated input scheduler, progress-mistake execution, predicted-path renderer, and custom Mega Hack-style tabbed interface remain to be implemented and tested.

The humanised offset helper is now in the codebase, but it is not yet connected to live gameplay because the mouse-coordinate input scheduler/pathfinder has not been implemented. When that scheduler is added, it must add the returned offset to ordinary mouse coordinates immediately before dispatch, skip the helper for keyboard inputs, and never clamp/retry/correct a click after it has caused a death. Randomised mistakes should remain genuine mistakes.

The click-audio functions are separated from the input hook so the future pathfinder can explicitly request keyboard or mouse click audio depending on its selected input method.

No anti-cheat bypass or concealment is included.
