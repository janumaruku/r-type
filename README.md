# r-type

A networked R-Type clone built on a small game engine written for it: an authoritative UDP server (`r-type_server`) and a graphical client (`r-type_client`), in C++20.

The developer documentation is in [`docs/`](docs/README.md).

## Build

### Prerequisites

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
