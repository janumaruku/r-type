# Development handbook

How we build, test and write code on this project. Read it once before your first pull request.

## Error handling policy

| Situation | Mechanism | Example |
| --- | --- | --- |
| A start-up failure the user must fix | Throw an `engine::core::EngineError` subclass. `main()` catches it, logs it, and exits with 84 | Port in use, missing asset, invalid argument |
| Untrusted bytes from the network | `net::Packet` throws `net::PacketError`. The adapter's decode boundary catches it, drops the datagram and counts an offence. Nothing else catches it | Truncated payload, unknown message type, bad enum value |
| Untrusted config files | Return `std::optional` and fall back to the default, with a warning | A key binding that names an unknown key |
| An expected runtime event | A return value or a game event | Client timeout, full server |
| Programmer error | `assert` in debug builds. Fail fast with `EngineError` where the check is cheap | An unregistered component type |
| Anything escaping a per-datagram handler on the server | Caught at the adapter boundary. Kick that session and continue | See [server: robustness](../modules/server.md#robustness-rules) |

Ownership: `std::unique_ptr` for anything owned, references for borrowed objects whose lifetime is guaranteed by the composition root. There is no raw `new`/`delete`, matching the `cppcoreguidelines-owning-memory` and `no-malloc` checks already in `.clang-tidy`.

## Logging

Both binaries log to `std::cerr` through `engine::core::log::Logger` and the `log::` functions ([engine: logging](../modules/engine.md#logging)). Each module or class that logs keeps a `Logger` with a short context name (`"net"`, `"server"`, `"client"`), and each thread names itself with `setThreadName` on one of its `Logger`s at start-up. Every level has a defined use:

- **error** (`Level::EG_ERROR`): an action failed and was abandoned.
- **warn:** something was degraded (tick overrun, dropped datagrams, audio disabled).
- **info:** lifecycle events (start, bind, join, leave, game over, periodic stats).
- **debug:** per-message traces.
- **trace:** per-tick system timings.

Nothing below `warn` is logged in the per-datagram path unless `--log-level debug` is set.

## Configuration files (client only)

| File | Format | Contents |
| --- | --- | --- |
| `config/client.cfg` | `key=value` lines, `#` comments | Volumes, `muted`, `fullscreen`, `hudFontSize`, `highContrast`, `reduceFlashes` ([accessibility](../architecture/accessibility.md#settings)) |
| `config/keybindings.cfg` | JSON: a `config` array of `{ "name", "keys" }` entries | Overrides for the default bindings ([engine: input](../modules/engine.md#input-engine_input)) |

An unknown key or a value that won't parse logs a warning and falls back to the default for that line. A config file never prevents the game from starting.

## Testing

Part 1 has **unit tests only**, written with GoogleTest. Other kinds of tests are listed below for later.

### Unit tests (now)

| What | Examples of cases |
| --- | --- |
| ECS | Every rule of the contract in [engine: ECS](../modules/engine.md#contract): registration, views, commands, stale handles, resources, stages. The journey's "Check yourself" lists ([ECS journey](../reference/ECS_JOURNEY.md)) give the cases |
| Runtime | `FixedTimestep` step counts and the 0.25 s cap |
| `net::Packet` | Round trips of every primitive and container, reads past the end, writes past `MAX_SIZE`, forged list counts ([network: tests](../modules/network.md#tests)) |
| Protocol | Every message round-trips, and every truncated prefix of a valid payload throws `PacketError` |
| Game | Each system on a hand-built registry; `movePlayer` |
| Client logic | `SnapshotAssembler` (complete and incomplete ticks), `EventFilter` (repeated event ids) |

Conventions:

- One test executable per library target (`engine_core_tests`, `engine_ecs_tests`, `network_tests`, `protocol_tests`, `game_tests`…), linked to that target and `GTest::gtest_main`, so a broken module only breaks its own tests.
- Code that only exists in an application (such as the client's `SnapshotAssembler`) is compiled into a static library next to the executable (`client_logic`, linked by `r-type_client`), so its tests can link it too. `main.cpp` stays as thin as possible.
- Tests live in `tests/<module>/`, one `<Class>Tests.cpp` per class under test. Test names read as sentences: `TEST(Packet, ReadPastEndThrows)`.
- CMake registers them with `gtest_discover_tests`, so `ctest --preset linux-debug` runs everything. CI runs it on every pull request.

### Other kinds of tests (later)

A test's kind depends on what it exercises, not on the framework. GoogleTest is a runner with assertions, so it can drive most of these too:

| Kind | What it would check | Can GoogleTest do it? |
| --- | --- | --- |
| Integration | A real server tick loop and 2 scripted clients on loopback: join, move, shoot, then one client goes silent and the other gets PLAYER_LEFT(TIMEOUT) | Yes: start the server loop on a thread inside the test |
| Packet loss | The same, with a test hook in `UdpSocket` dropping 20% of datagrams: the clients' worlds still match the server's after 2 s | Yes, same setup as integration |
| Determinism | Two `GameWorld`s with the same seed and commands produce identical events over 3600 ticks | Yes, it's a plain unit-style test |
| Fuzz | Millions of random or mutated datagrams into the server's decode path: no crash, no memory growth | Partly: a GoogleTest loop with random bytes works; a real fuzzer (libFuzzer, built with Clang) finds far more bugs |
| Functional / end-to-end | The real `r-type_server` and `r-type_client` binaries started as processes | Not well: a script (Python or shell) run by CTest suits it better |
| Manual | 4 clients on Linux and Windows, one unplugged, star speed at 30 and 144 FPS | No: a checklist in `docs/how-to/release-check.md`, run before each defence |

## Benchmarks

Tests show that code is correct; benchmarks show that a design choice was worth it. Part 1 benchmarks the ECS (`ecs_benchmarks`, targets in [engine: ECS](../modules/engine.md#benchmarks)); other modules can add theirs the same way.

- **Tool:** Google Benchmark, fetched by CPM only when `RTYPE_BUILD_BENCHMARKS=ON`, with its own tests, install rules and `-Werror` turned off. Each benchmark executable links `benchmark::benchmark_main`.
- **Layout:** `benchmarks/<module>/`, one `<Topic>Benchmarks.cpp` per area (storage, registry, iteration), shared helpers in a `BenchmarkHelpers.hpp`.
- **Build:** only in Release. Use the `linux-bench` preset (`linux-release` plus `RTYPE_BUILD_BENCHMARKS=ON`), then run `build/linux-bench/benchmarks/<module>/<module>_benchmarks`. Debug and sanitizer numbers are meaningless, and CMake prints a warning if benchmarks are enabled in another build type. Benchmarks are never registered with `ctest`.
- **How to write one, run it, read the output and compare two versions with `compare.py`:** see [Benchmarking with Google Benchmark](../reference/ECS_JOURNEY.md#benchmarking-with-google-benchmark) in the ECS journey.

**Rules for trustworthy numbers** (from the journey):

1. Release builds only, with the flags you ship. When a result matters, measure `-O2` and `-O3`.
2. A quiet machine: check the load average in the output header, close other programs.
3. Repetitions, and read the median and the coefficient of variation, not a single run.
4. Both compilers, GCC and Clang: the same code can differ by 2×.
5. Compare against a hand-written baseline: "1.1× the manual loop" means something, "3 ms" alone doesn't.
6. Benchmark what the claim is about. Counting copies is a test's job, not a benchmark's.

A change that claims a speed-up comes with a before/after comparison (`compare.py benchmarks before.json after.json`, 10 repetitions each) in its pull request.

## Build, dependencies and platforms

- **CMake ≥ 3.28** (the version already pinned in the repository), with `CMakePresets.json` presets `linux-debug`, `linux-release`, `linux-coverage`, `windows-clangcl-debug` and `windows-clangcl-release`. The release presets do not build the tests.
- **Package manager: CPM.cmake** (allowed by the subject), pinned in `cmake/CPM.cmake`. Dependencies are declared in `cmake/Dependencies.cmake`:

| Package | Version | Used by | Notes |
| --- | --- | --- | --- |
| Raylib | 5.5 | `engine_render`, `engine_audio` | Built static from source by CPM (`BUILD_EXAMPLES=OFF`), with its bundled GLFW and miniaudio. Never found with `find_package`, so a copy preinstalled on a machine or CI image is ignored |
| Asio (standalone) | 1.30.2 | `network` | Header-only and ships no CMake file, so we declare an `INTERFACE` target `asio` with its include directory and `ASIO_STANDALONE`, `ASIO_NO_DEPRECATED`, plus `_WIN32_WINNT=0x0A00` on Windows |
| GoogleTest | 1.15.x | test executables | Only when `RTYPE_BUILD_TESTS=ON`. `INSTALL_GTEST=OFF`, and `gtest_force_shared_crt=ON` on Windows so it uses the same C runtime as our code |
| Google Benchmark | 1.9.x | benchmark executables | Only when `RTYPE_BUILD_BENCHMARKS=ON`, with `BENCHMARK_ENABLE_TESTING`, `BENCHMARK_ENABLE_INSTALL` and `BENCHMARK_ENABLE_WERROR` set to `OFF` |

- **Self-contained (requirement T-03).** Apart from the compiler, Linux needs only the low-level system libraries Raylib links against, which the subject explicitly allows: the X11, Xrandr, Xinerama, Xcursor, Xi and OpenGL development packages. Audio needs no development package: miniaudio loads ALSA or PulseAudio at run time. Windows needs Visual Studio 2022 or later with its clang-cl component, and nothing else. A server-only build (`RTYPE_BUILD_CLIENT=OFF`) skips Raylib and needs none of these packages.
- **Compilers:** GCC ≥ 13 or Clang ≥ 17 on Linux, clang-cl on Windows ([ADR 0006](../decisions/0006-clang-cl-on-windows.md)), all in C++20 without compiler extensions, because `std::format` is required. Warnings are errors: `-Wall -Wextra -Werror`, and `/W4 /WX` under clang-cl, through the `project_options` interface target (`cmake/ProjectOptions.cmake`) that every target of ours links.
- **CI cost (from the subject's warning):** set `CPM_SOURCE_CACHE` to a cached directory, so dependencies are downloaded once per cache key rather than on every commit. One job each for Linux and Windows. clang-format and clang-tidy run only on changed files.
- **Packaging:** CPack produces `r-type-<version>-Linux.tar.gz` and `r-type-<version>-win64.zip`, each holding both binaries, `assets/`, the default `config/`, the README and the LICENSE.

## Workflow and conventions

- **Branches:** `main` is always releasable. Work happens on `feat/<ticket>-<slug>`, `fix/…` and `docs/…` branches, merged through PRs with at least one review and green CI. This matches the existing `branch-merge-policy.yml` and Jira automation.
- **Commits:** Conventional Commits (`feat:`, `fix:`, `chore:`, `ci:`, `docs:`, `test:`), as already used in the history.
- **Tags:** `v0.1.0` for the Part 1 delivery. Each internal milestone gets a pre-release tag (`v0.1.0-alpha.N`).
- **Style:** `.clang-format` and `.clang-tidy` (already in the repository) are the source of truth. One class per header. Headers use `#pragma once`.
- **Docs in the same PR:** a change to a module's public API, the protocol or a rule value updates the matching page under `docs/` in the same pull request.

## Documentation

| Deliverable | Location | Notes |
| --- | --- | --- |
| README | `README.md` | Purpose, platforms, dependencies, build and run commands for both OSes, license, authors |
| Developer docs | `docs/architecture/`, `docs/modules/` | One page per module, with the Mermaid diagrams kept as text |
| Protocol RFC | `docs/protocol/RFC-rtype.md` | To be expanded with hex dumps of example packets |
| Decision records | `docs/decisions/` | One page per significant decision: context, decision, consequences |
| How-tos | `docs/how-to/` | Adding a component, adding a message type, adding an enemy (to write) |
| Comparative study | `docs/study/` | See [open decisions](../roadmap.md#open-decisions) |
| Site | GitHub Pages | MkDocs Material builds it from `docs/` (search, outline, Mermaid support) |
