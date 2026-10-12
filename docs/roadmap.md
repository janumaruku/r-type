# Part 1 roadmap

The subject gives Part 1 three weeks. Work is ordered so that the riskiest boundary, the network, is proven first, and a playable slice exists by the end of week 2.

| Week | Phase | Work | Gate (a demo the whole team runs) |
| --- | --- | --- | --- |
| 1 | Foundations | `engine_core`, `engine_ecs`, `engine_runtime`; `net::Packet` and `net::UdpSocket`; protocol messages with their round-trip tests; CPM dependencies, presets, CI on Linux and Windows | Unit tests green. A test client gets WELCOME from a test server on loopback |
| 2 | Vertical slice | `GameWorld`, systems and builders; server tick loop and sessions; client scenes, `SnapshotAssembler` and `ClientWorld`; sprites for ships; input sending and prediction | Two clients move their ships on one server and see each other |
| 3 | Gameplay and polish | Bydos, missiles and collisions; starfield, sounds and HUD; crash handling; Windows build and packaging; README, RFC, docs site | `v0.1.0` tagged: 4 players, Linux and Windows clients mixed, one client killed mid-game |

A gate that fails moves its leftovers to the next week ahead of new work. Week 1 splits cleanly across people: one on core, ECS and runtime, one on `Packet` and `UdpSocket`, one on the protocol messages, one on the build and CI. Each piece has its own unit tests.

## Open decisions

- [x] **Asio flavour.** Standalone Asio 1.30.2, used directly ([ADR 0001](decisions/0001-standalone-asio.md)).
- [x] **Reliability.** State-based snapshots, no acks ([ADR 0002](decisions/0002-state-based-replication.md)).
- [x] **Graphics library.** Raylib 5.5 ([ADR 0004](decisions/0004-raylib.md)). The existing CI image `janumaruku/epitest-with-raylib` already has the system packages Raylib needs.
- [ ] **Comparative study (D-04).** Still to write: Raylib vs SFML vs SDL, Asio vs raw sockets, state-based vs event-based replication, sparse-set ECS vs archetype ECS, CPM vs vcpkg vs Conan, plus a storage and security section.
- [ ] **Sprite rectangles.** Measure the frame rectangles in the provided sheets and fill `assets/sprites/catalog.cfg`.
- [ ] **Snapshot rate.** 30 Hz is the default. Revisit after the week 2 gate if bandwidth or smoothness disappoints.
