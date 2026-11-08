# D-04 — Comparative study of technology choices

**Reviewed:** 2026-11-08
**Scope:** the current C++20/Linux/Windows R-Type prototype and the one-room Hollow Knight engine demo.
**Decision rule:** “best” means the best fit for these constraints, not the universally fastest or most feature-rich option. We have not run head-to-head performance benchmarks for alternative libraries.

## Constraints used in the comparison

- One small team, two desktop targets (Linux and Windows), C++20, CMake, and an existing CI pipeline.
- A 2D game client and a headless authoritative server. Client-only dependencies must not leak into the server.
- Four players, a few dozen enemies and projectiles, 30 snapshots per second, and a 1,200-byte UDP datagram ceiling.
- The goal is an understandable engine that can support another game, not a full editor-driven engine or a general-purpose networking framework.

## Graphics, window, input and audio: Raylib vs SFML vs SDL

| Choice | Strengths for this project | Cost or limitation here |
| --- | --- | --- |
| **Raylib 5.5 — selected** | One small C API covers the window, 2D drawing, input and audio used by the client. It is designed for direct game programming, builds from source in the existing CMake setup, and was already supported by the team’s CI image when the choice was made. The engine wraps it behind `IRenderer`, draw commands, `InputEvent` and `IAudio`. | C API rather than idiomatic C++; one window/context; the team must maintain its own scenes, ECS, caches and resource lifetimes. The backend is OpenGL-based. |
| SFML 3 | A C++ interface split into system, window, graphics, audio and network modules. Its CMake targets handle module dependencies, and its abstractions suit a 2D game. | More of its window/input/audio model would overlap with the engine wrappers, and its network module is not used because the project has its own UDP/protocol layer. Moving the existing renderer and audio backend would not add a required feature. |
| SDL 3 | A lower-level, cross-platform multimedia layer with broad platform access and more backend control. A good choice when device management, platform APIs, or graphics backends are explicit product needs. | It exposes a wider, lower-level surface than this client needs; our engine would still own the 2D rendering API, resource cache, audio abstraction and scene layer. That raises integration work without a current requirement. |

Raylib is the best fit for the current prototype because it supplies the needed client primitives with the least backend code while leaving the engine’s public API independent. This is an integration-cost decision, not a claim that Raylib is faster than SFML or SDL. The one-window and OpenGL assumptions are acceptable for both present applications; multi-window tooling or a required non-OpenGL backend would reopen the choice. See [ADR 0004](../decisions/0004-raylib.md). Sources: [Raylib project and feature list](https://github.com/raysan5/raylib), [SFML modules and CMake linking](https://www.sfml-dev.org/faq/build-use/), [SDL 3 overview](https://wiki.libsdl.org/SDL3/FrontPage).

## UDP networking: standalone Asio vs Boost.Asio vs OS sockets

| Choice | Strengths | Cost or limitation |
| --- | --- | --- |
| **Standalone Asio 1.30.2 — selected** | Cross-platform UDP sockets, endpoints, resolver and `io_context`, with a consistent C++ asynchronous model. Header-only for the features used and no Boost dependency. The existing `net::UdpSocket` supplies the application’s bounded receive queue. | Networking types appear in the network boundary, and Asio must be fetched and built as part of the project. |
| Boost.Asio | Same author and closely related API, with a broad Boost ecosystem and optional facilities such as SSL and other Boost integrations. | The project does not use the Boost extras. Bringing in Boost.System and a larger dependency/build surface would increase setup cost without improving this UDP use case. |
| Raw OS sockets | Maximum control and no third-party library. | The project supports Windows and Linux; the implementation would need separate Winsock/POSIX setup, error handling, address conversion, readiness/threading and cleanup. That is avoidable platform code, not useful gameplay work. |

Standalone Asio is the best fit because it removes the portability and async-I/O plumbing while keeping dependencies smaller than Boost.Asio. The current usage is intentionally modest: UDP transport, address resolution and a receive thread; no TLS or coroutine stack is required. If the product later needs a wider Boost ecosystem, compare the actual dependency cost again. See [ADR 0001](../decisions/0001-standalone-asio.md). Sources: [Asio documentation](https://think-async.com/Asio/Documentation.html), [Boost.Asio capabilities and integrations](https://www.boost.org/doc/libs/latest/doc/html/boost_asio/overview.html), [POSIX/Linux socket interface](https://man7.org/linux/man-pages/man2/socket.2.html), [Winsock socket function](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket).

## Replication: full state snapshots vs event-only state

This is a protocol choice rather than a library comparison.

| Model | Strengths | Cost or limitation |
| --- | --- | --- |
| **State snapshots — selected** | Each complete snapshot is enough to rebuild the current roster and entity state. A dropped UDP packet is repaired by the next snapshot, and a late-joining client needs no event replay. | Repeats unchanged state and costs more bytes than sending only changes. At the documented planned peak, the server sends roughly 70 KB/s per client. |
| Reliable event-only updates | Efficient when changes are sparse; naturally describes actions such as spawn, destroy and score changes. | Requires sequence/ack/resend and ordering or replay rules. Losing a one-time destroy can leave a ghost entity; late join needs a replay or separate initial state. |
| Hybrid used here | Snapshots remain authoritative; selected events explain presentation or important transitions and are repeated with IDs to ignore duplicates. | Both paths need clear ownership: events must not become a second source of truth. |

Snapshots are the best fit for this small UDP game because a few dozen entities fit the measured bandwidth budget and the recovery behavior stays simple. We keep transient cues separate from authoritative state. If entity count or snapshot rate grows enough to make the measured traffic problematic, delta-compressed snapshots are the next comparison; reliable event-only replication is not justified by the current scale. See [ADR 0002](../decisions/0002-state-based-replication.md) and [RFC §6.5](../protocol/RFC-rtype.md#65-snapshot-0x83-server--client).

## ECS storage: this engine’s sparse sets vs EnTT vs archetypes

| Choice | Strengths | Cost or limitation |
| --- | --- | --- |
| **Our sparse-set ECS — selected** | Components live in per-type sparse sets; views join the smallest storage and erase uses swap-and-pop. The repository owns generations, commands, resources and system stages and has focused correctness tests and benchmark targets. | The team owns implementation, API stability, debugging tools and future optimizations. It does not yet have parallel systems, paged sparse arrays or groups. |
| EnTT sparse-set ECS | Mature C++ implementation of the same broad storage family; could replace custom registry/storage maintenance while retaining familiar per-component pools. | It would give up control of the educational engine’s registry and system/resource model and still require adapters for this repo’s APIs. It is a credible alternative if ECS maintenance becomes a larger cost than engine ownership. |
| Flecs archetype ECS | Entities with the same component signature are stored together, making dense query iteration cache-friendly; supports rich queries and relationships. | Component changes move an entity between archetype tables and bring a different API/model. The current scene changes entity composition infrequently and is too small to justify that migration without profiling evidence. |

The sparse-set design is the best fit for this engine’s current component churn and learning goals. Existing measurements compare the repo’s design with its earlier sparse-array prototype and hand-written joins; they do **not** prove it outperforms EnTT or Flecs. Use the engine benchmarks before changing the implementation. See [ADR 0005](../decisions/0005-ecs-design.md) and the [ECS design journey](../reference/ECS_JOURNEY.md). Sources: [EnTT entity storage](https://github.com/skypjack/entt/blob/main/docs/md/entity.md), [Flecs ECS storage tradeoffs](https://www.flecs.dev/ecs-faq/), [Flecs query model](https://www.flecs.dev/flecs/Queries.html).

## Dependency management: CPM.cmake vs vcpkg vs Conan

| Choice | Strengths | Cost or limitation |
| --- | --- | --- |
| **CPM.cmake — selected** | A small CMake wrapper around `FetchContent`; dependency declarations stay next to targets, accept tags or source archives, and can share a source cache in CI. No separate package-manager bootstrap is required. | Source dependencies are built for each clean build tree; no prebuilt binary cache by default. It relies on usable upstream CMake projects, and tag pins are not the same as immutable source hashes. |
| vcpkg manifest mode | Declarative `vcpkg.json`, version baselines/overrides, triplets and binary caching provide a strong path for team-wide and cross-platform package installs. | Adds a bootstrap/toolchain and manifest/baseline/triplet setup to this CMake-first repo. That is worthwhile if the dependency set or binary-cache needs grow. |
| Conan 2 | Profiles and package recipes model compiler/OS/configuration variants; dependency graphs can select or build matching binary packages. | Requires recipe/profile and remote/cache conventions. More control than this small dependency set needs today. |

CPM is the best fit for the present build because dependencies are few, CMake already drives every target, and CI can reuse a source cache. This choice trades binary package reuse for low setup overhead. Reopen it if clean CI builds dominate time, multiple projects must share binary packages, or immutable transitive dependency graphs become a requirement. Sources: [CPM design and source cache](https://github.com/cpm-cmake/CPM.cmake/blob/master/README.md), [vcpkg manifest mode and version constraints](https://learn.microsoft.com/en-us/vcpkg/concepts/manifest-mode), [Conan 2 dependency graphs](https://docs.conan.io/2/reference/commands/graph/info.html).

## Supporting libraries

| Use | Selected dependency | Why it fits | What would change the choice |
| --- | --- | --- | --- |
| Human-edited settings and sprite catalogs | `nlohmann/json` 3.12.0 | Header-only, STL-like DOM API and straightforward CMake integration; these files are small and are not on the frame loop. | If JSON parsing became a measured CPU/memory hotspot, evaluate RapidJSON or simdjson before replacing it. |
| Server/client command-line options | CLI11 2.5.0 | Typed values, help/error handling and validation without taking a dependency on Boost.Program_options. | If the CLI became a handful of fixed flags, manual `std::from_chars` parsing could be smaller; today the client and server share the parser. |
| Unit tests | GoogleTest 1.15.2 | CMake/CTest discovery, fixtures, typed tests and assertions are already used across the test tree. | Catch2 is a reasonable alternative if its test syntax is preferred; replacing the framework would not improve runtime behavior. |
| Optional microbenchmarks | Google Benchmark 1.9.5 | Gives repeated benchmark runs and output tooling for the ECS/performance work. | Keep it optional; do not add it to normal server/client builds. |

These are convenience and development dependencies, not gameplay requirements. Their versions are pinned in [`cmake/Dependencies.cmake`](../../cmake/Dependencies.cmake). Sources: [nlohmann/json](https://github.com/nlohmann/json), [CLI11](https://github.com/CLIUtils/CLI11), [GoogleTest](https://github.com/google/googletest), [Google Benchmark](https://github.com/google/benchmark).

## Storage and security

### Storage

Match state, entities and sessions are held in server memory and are not persisted. Client preferences use a small JSON file; sprite catalogs and assets are repository files. A database would add deployment, schema, backup and migration work without a current account, inventory or cross-session progression requirement.

For a future **single-machine save file**, SQLite is the better next candidate: it is an in-process, serverless transactional database stored in a file. For **shared accounts or progression across server processes**, PostgreSQL is a better fit because it is a client/server database designed to serve concurrent clients. Neither should be added until the product needs persistence. Sources: [SQLite architecture](https://www.sqlite.org/about.html), [PostgreSQL client/server architecture](https://www.postgresql.org/docs/current/tutorial-arch.html).

### Security boundary

The current server treats UDP input as untrusted: the RFC caps datagrams, validates message lengths, enum values, roster state and entity ranges, and documents per-address rate limits. Session lookup gates game input; queue and entity counts are bounded. This is appropriate for a local course prototype, but the protocol does not authenticate players or encrypt traffic. A public deployment would need an explicit threat model, authenticated sessions, integrity/replay protection, secret handling, and parser fuzzing before being called production-safe. Do not place credentials in the client JSON configuration. See [RFC §11](../protocol/RFC-rtype.md#11-validation-and-malformed-input) and [network module security controls](../modules/network.md).

## Conclusion and review triggers

The existing choices minimize integration and operational cost while meeting the stated two-platform, small-team and small-match requirements. They are therefore the best fit for this project today—not a universal ranking. Revisit the choices with measurements or a changed requirement: a non-OpenGL client backend, much higher entity counts, CI build times that remain high with caching, saved progression, or public internet deployment.
