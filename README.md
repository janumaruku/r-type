# Software Architecture Engineering - R-Type

This is an **Epitech project** called R-Type.

It's a four-week project from the fifth semester of the Epitech **Software Architecture Engineering module** (**G-ING-500**).

## Project Overview

The goal of this project is to **implement a multi-threaded server and a graphical client for a well-known legacy video game called R-Type, using a game engine of your own design**.

The language used is **C++**.

## Epitech Warning

**This is a warning to all epitech students:** 
- **We recall you that being a copycat at EPITECH will not bring you any good.**
- **Copying is totally banned at EPITECH and by not respecting this rule you can be scolded.**

## Prerequisites

| Platform | What to install |
| --- | --- |
| Linux | CMake ≥ 3.28, GCC ≥ 13 or Clang ≥ 17, and the system libraries Raylib links against |
| Windows | CMake ≥ 3.28 and Visual Studio 2022 or later with the "C++ Clang tools for Windows" component (clang-cl) |

On Debian or Ubuntu, the Raylib system libraries are:

```sh
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

Everything else (Raylib, Asio, GoogleTest) is downloaded and built by CMake on the first configure. Nothing needs to be installed for them.

### Commands

Linux:

```sh
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

Windows:

```powershell
cmake --preset windows-clangcl-debug
cmake --build --preset windows-clangcl-debug
ctest --preset windows-clangcl-debug
```

Both binaries are written to the repository root: `r-type_server` and `r-type_client` (`.exe` on Windows). Build files go to `build/<preset name>/`.

The client build also writes `r-type_ecs_example`, a tiny R-Type made only of ECS entities, systems and resources, drawn with Raylib (`apps/ecs_example/`). Move with WASD or the arrows, fire with Space, restart with R after a hit, close with Esc.

The client build also writes `hollow_knight`, a single-room action-platformer demo that reuses the engine ECS, input, runtime, renderer, and snapshot packet encoding. It draws the Knight and enemies from original Hollow Knight sprites and features endless, progressively harder enemy waves; see [sprite sources and copyright notes](assets/hollow_knight/ASSETS.md). Start it with `./hollow_knight` from the repository root. Move with A/D or the arrows, jump with Space, slash with J, dash with Shift, hold F to focus, use E at the bench, and press Esc to pause.

`cmake --list-presets` shows the presets available on your platform:

| Preset | Use |
| --- | --- |
| `linux-debug`, `windows-clangcl-debug` | Daily work. Builds the tests |
| `linux-release`, `windows-clangcl-release` | Optimised binaries, without the tests |
| `linux-coverage` | Clang only. Building it runs the tests and writes the report to `build/linux-coverage/coverage_html/index.html` |

### Options

Pass them at configure time, for example `cmake --preset linux-debug -DRTYPE_BUILD_CLIENT=OFF`.

| Option | Default | Effect |
| --- | --- | --- |
| `RTYPE_BUILD_TESTS` | `ON` | Builds the test executables. When `OFF`, GoogleTest is not downloaded |
| `RTYPE_BUILD_CLIENT` | `ON` | Builds `r-type_client`, `r-type_ecs_example` and Raylib. When `OFF`, only the server is built and the Raylib system libraries are not needed |
| `WITH_COVERAGE` | `OFF` | Instruments the code and adds the `coverage` target. Needs Clang |
| `RTYPE_BUILD_EXAMPLES` | `ON` | Builds the example programs, such as `apps/render_demo` (needs `RTYPE_BUILD_CLIENT`). They stay in the build folder, for example `build/linux-debug/apps/render_demo/render_demo` |

### Examples

`render_demo` opens a window and exercises the client engine the way the client will: window events, rebindable input from `apps/render_demo/assets/keybindings.json`, a letterboxed 1280 × 720 playfield, sprites, rectangles and text sorted by layer. Arrows or WASD move, SPACE or LCTRL fire, Escape quits. It is the manual check for what CI can't see: [apps/render_demo/CHECKLIST.md](apps/render_demo/CHECKLIST.md).

### Download cache

Dependencies are downloaded once per build folder by default. To download them once for all build folders, point `CPM_SOURCE_CACHE` at a directory before configuring:

```sh
export CPM_SOURCE_CACHE=$PWD/.cpm-cache
```

`.cpm-cache/` is ignored by git, and CI uses the same variable.

## Documentation

The developer documentation is in [`docs/`](docs/README.md).

## Authors

| [<img src="https://avatars.githubusercontent.com/u/146741144?v=4?size=85" width=85><br><sub>Jean-Marc Kottin </sub>](https://github.com/janumaruku) |
| :---: |
| [<img src="https://avatars.githubusercontent.com/u/181121247?v=4?size=85" width=85><br><sub>Quentin Dutournier-Ottenin</sub>](https://github.com/Patate-with-computer) |
| [<img src="https://avatars.githubusercontent.com/u/146721380?v=4?size=85" width=85><br><sub> Liam Lequet</sub>](https://github.com/lyzer-px) |

