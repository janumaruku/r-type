# R-Type documentation

This folder is the developer documentation for the R-Type project (Epitech, Advanced C++). It covers Part 1: the engine architecture and the first networked prototype.

## Where to start

Read in this order if you are new to the project:

1. [Architecture overview](architecture/overview.md): what the modules are, how they depend on each other, and where the code lives.
2. [Requirements](architecture/requirements.md): what the subject asks for and where each requirement is handled.
3. [Module flows](architecture/flows.md): sequence diagrams of the four flows that cross every module.
4. The module page for the part you are working on.

## Contents

| Page | What it covers |
| --- | --- |
| [architecture/overview.md](architecture/overview.md) | Scope, glossary, layers, dependency rules, build targets, repository layout, naming |
| [architecture/requirements.md](architecture/requirements.md) | Subject requirements and where each one is covered |
| [architecture/flows.md](architecture/flows.md) | Join, input round trip, enemy kill, client crash |
| [architecture/accessibility.md](architecture/accessibility.md) | Motor, visual, audio and cognitive accessibility features |
| [modules/engine.md](modules/engine.md) | `engine_core`, `engine_ecs`, `engine_runtime`, `engine_render`, `engine_input`, `engine_audio` |
| [modules/network.md](modules/network.md) | `net::Packet` serialization and the `net::UdpSocket` helper |
| [modules/game.md](modules/game.md) | Components, systems, builders, rules, `GameWorld` |
| [modules/server.md](modules/server.md) | `r-type_server`: threads, sessions, tick loop |
| [modules/client.md](modules/client.md) | `r-type_client`: scenes, snapshot assembly, interpolation, prediction, presentation |
| [protocol/RFC-rtype.md](protocol/RFC-rtype.md) | The binary UDP protocol, byte by byte. Enough to write a new client |
| [handbook/development.md](handbook/development.md) | Error handling, logging, configuration, testing, build, workflow |
| [decisions/](decisions/) | Architecture decision records: why things are the way they are |
| [reference/ECS_JOURNEY.md](reference/ECS_JOURNEY.md) | How the ECS design was built and measured, step by step, with its traps and tests (background for `engine_ecs`) |
| [roadmap.md](roadmap.md) | Part 1 phases, gates and open decisions |

## Conventions used in these pages

- Diagrams are [Mermaid](https://mermaid.js.org/) blocks, so they render on GitHub and in the docs site, and are reviewed like code.
- Class diagrams show the members a module must have. Private helpers can be added freely.
- MUST, SHOULD and MAY in the protocol RFC are used as defined in RFC 2119.
- When the code and a page disagree, fix whichever is wrong in the same pull request.
