# Phosophurs Undetectable Pathfinder

Experimental Geometry Dash Geode mod project.

## Target
- Windows
- Geometry Dash 2.2081
- Geode SDK 5.11.2

## Build
The GitHub Actions workflow builds a Windows `.geode` artifact on pushes to `main`/`master` and on manual workflow dispatch.

The project uses the same Geode build action and CMake layout as the Frame Cycle project.

## Current scaffold
- Configurable keybind settings (defaults: `4` for autoplay, `Z` for menu).
- Settings for progress-based input variation (start %, probability, behaviour).
- Settings for click tone frequency and predicted-path visibility.
- Basic keybind listeners and status notifications.

## Development status
This is the initial project scaffold, not a completed pathfinder. The actual level-analysis/route solver, gameplay input driver, custom Mega Hack-style in-game tabbed menu, path renderer, and waveform audio generator still need implementation and testing. No anti-cheat bypass or concealment is included.
