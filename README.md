# Phosophurs Pathfinder

An autonomous Geometry Dash TAS/pathfinding mod for **Windows Geometry Dash 2.2081**.

This replaces the previous attempt recorder with a real search engine. It reads the loaded level, searches for an input sequence, then replays that candidate inside Geometry Dash. The game itself verifies whether the route survives; when the model and game disagree, the solver re-anchors on the real player state and searches the remaining section again.

## Gameplay modes

The physics model includes all eight classic modes:

- Cube
- Ship
- Ball
- UFO
- Wave
- Robot
- Spider
- Swing

**Platformer levels are not supported by this solver.** Platformer has free horizontal steering and backtracking, unlike classic auto-scroll levels, so it requires a different search formulation. The mod refuses platformer levels instead of generating a misleading route.

## How to use

1. Install Geode for Windows and place the built `.geode` file in `Geometry Dash/geode/mods/`.
2. Open a classic Geometry Dash level.
3. Open the Pathfinder panel from the level's play menu and choose **Solve**.
4. The solver searches without recording your gameplay or requiring a calibration attempt.
5. When a candidate is found, the game replays it to verify the result. The solver repairs the plan if the real game disagrees with its physics model.

The overlay and hotkeys are described in the upstream `docs/RUNNING.md` file.

## Scope and limitations

- The upstream solver reports cold solves for all 22 official levels on its tested build.
- Custom levels may work, but are not guaranteed. Unsupported triggers, moving objects, or mechanics can make a level fail or take much longer.
- This is a TAS/pathfinding tool. It is not a promise that every custom level can be solved.
- Automated runs are treated as bot/TAS activity; the upstream safety logic blocks achievements, statistics, coins and level records while the bot is driving.

## Build target

- Geometry Dash: **2.2081 (Windows / Steam)**
- Geode SDK: **5.10.1** (the project metadata pins the tested SDK)
- CMake: 3.21+
- Visual Studio 2022 with the C++ desktop workload

Pushes and pull requests build through GitHub Actions. A successful build only proves compilation; in-game solving still needs verification.

## Credits and licence

The autonomous reachability-DP engine and in-game repair architecture are based on [gdsolver](https://github.com/gdsolver/gdsolver), with the Geometry Dash 2.2081 port from [CarlosDev-max/gdsolver-FORK](https://github.com/CarlosDev-max/gdsolver-FORK).

The upstream project is MIT-licensed. Its copyright and licence notice are preserved in [LICENSE](LICENSE). This repository's mod packaging, branding and integration are maintained as Phosophurs Pathfinder.
