# Requirements

Every requirement below comes from the Epitech subject (`../rtype-project.pdf`, pages 4–13). The **Covered in** column says which page handles it, so the defence checklist can be read straight off these tables.

## Functional requirements

| ID | Level | Requirement | Covered in |
| --- | --- | --- | --- |
| F-01 | MUST | Two binaries: `r-type_server` and `r-type_client` | [overview](overview.md), [server](../modules/server.md), [client](../modules/client.md) |
| F-02 | MUST | The game is playable: a starfield, players' ships, waves of Bydos, missiles on both sides | [game](../modules/game.md), [client](../modules/client.md#presentation) |
| F-03 | MUST | Networked: each player runs their own client, and a central server has final authority | [server](../modules/server.md) |
| F-04 | MUST | The server notifies clients when a monster spawns, moves, is destroyed, fires or kills a player, and when another client acts | [RFC §7](../protocol/RFC-rtype.md#7-how-the-protocol-meets-the-subjects-notification-requirement) |
| F-05 | MUST | The server is multithreaded and never blocks waiting on clients | [server: threads](../modules/server.md#threads) |
| F-06 | MUST | The server survives a client crash and tells the other clients in that game | [RFC §9](../protocol/RFC-rtype.md#9-disconnection-and-crash-handling), [server](../modules/server.md#joining-leaving-and-crashing) |
| F-07 | MUST | Malformed packets never crash a program, use excessive memory or compromise the server | [network: Packet](../modules/network.md#netpacket), [RFC §10](../protocol/RFC-rtype.md#10-error-handling), [RFC §11](../protocol/RFC-rtype.md#11-validation-and-malformed-input) |
| F-08 | MUST | Starfield scrolling, entity movement and all time flow are driven by timers, not CPU speed | [engine: runtime](../modules/engine.md#runtime-engine_runtime) |
| F-09 | MUST | Players move with the arrow keys | [engine: input](../modules/engine.md#input-engine_input) |
| F-10 | MUST | Bydos slaves and missiles exist | [game: components](../modules/game.md#components) |
| F-11 | MUST | Monsters spawn randomly on the right of the screen | [game: systems](../modules/game.md#systems-and-tick-order) |
| F-12 | MUST | The 4 players can be told apart | [game: rules](../modules/game.md#rules-and-tuning-values), [accessibility](accessibility.md) |
| F-13 | SHOULD | Basic sound design | [engine: audio](../modules/engine.md#audio-engine_audio), [client](../modules/client.md#presentation) |
| F-14 | MAY | The client runs some logic locally, such as moving its own ship, but the server stays authoritative | [client: prediction](../modules/client.md#frame-loop-interpolation-and-prediction) |

## Technical requirements

| ID | Level | Requirement | Covered in |
| --- | --- | --- | --- |
| T-01 | MUST | C++ (C++20 here), using a build system generator (CMake) | [handbook](../handbook/development.md#build-dependencies-and-platforms) |
| T-02 | MUST | Third-party dependencies come through a package manager (CPM.cmake here). No vendored sources | [handbook](../handbook/development.md#build-dependencies-and-platforms) |
| T-03 | MUST | Self-contained build. Nothing is installed system-wide apart from the compiler, SDK, OpenGL and X11 | [handbook](../handbook/development.md#build-dependencies-and-platforms) |
| T-04 | MUST | Client and server both run on Linux | [handbook](../handbook/development.md#build-dependencies-and-platforms) |
| T-05 | SHOULD | Cross-platform: Windows too, and Linux and Windows clients can play together | [network](../modules/network.md), [handbook](../handbook/development.md#build-dependencies-and-platforms) |
| T-06 | MUST | All in-game communication is a binary protocol over UDP | [RFC](../protocol/RFC-rtype.md) |
| T-07 | MAY | Asio or OS sockets. Raw sockets must be wrapped in proper abstractions | [network](../modules/network.md), [ADR 0001](../decisions/0001-standalone-asio.md) |
| T-08 | MUST | Rendering, Networking and Game Logic are visible, decoupled layers | [overview](overview.md) |
| T-09 | MUST NOT | Use an existing game engine or a library with too broad a scope. SFML, SDL and Raylib are allowed | [engine](../modules/engine.md) |
| T-10 | MUST | A defined workflow: feature branches, PRs, issues, tags, CI | [handbook](../handbook/development.md#workflow-and-conventions) |

## Documentation and accessibility requirements

| ID | Level | Requirement | Covered in |
| --- | --- | --- | --- |
| D-01 | MUST | All documentation is in English | all of `docs/` |
| D-02 | MUST | A README: purpose, dependencies, build, usage, license, authors | `README.md` at the repository root |
| D-03 | MUST | Developer documentation above class level: architecture diagrams, system overviews, how-tos | `docs/` |
| D-04 | MUST | A technical and comparative study: algorithms, storage, security | [roadmap: open decisions](../roadmap.md#open-decisions) (not written yet) |
| D-05 | SHOULD | Protocol documentation complete enough to write a new client (an RFC) | [RFC](../protocol/RFC-rtype.md) |
| D-06 | MUST | Documentation published in a modern format (a static site or wiki) | [handbook](../handbook/development.md#documentation) |
| A-01 | MUST | Accessibility for motor, audio/visual and cognitive disabilities | [accessibility](accessibility.md) |
