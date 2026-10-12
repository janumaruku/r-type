# ADR 0005: ECS built on the bootstrap journey, in its final form

- **Status:** accepted
- **Date:** 2026-10-07

## Context

The first draft of the spec described the classic Epitech bootstrap ECS: one `SparseArray<Component>` per type (a vector of optionals indexed by entity), a registry using `std::type_index` and `std::any`, systems run by a separate `SystemPipeline<SimContext>`, and a "mark then flush" kill.

Meanwhile, the team built the bootstrap ECS step by step, with tests and benchmarks, and documented it in the [ECS journey](../reference/ECS_JOURNEY.md). That work measured the limits of the classic design (a `SparseArray` visits every slot up to the highest entity id: 68 MB and 6–9 ms for 1 000 components among 1M ids), moved to sparse sets and views, and planned three further steps: own type erasure (10), generational entities (11), and commands, resources and stages (12).

## Decision

`engine_ecs` implements the journey's design directly in its final form, rather than replaying the bootstrap steps:

- **Sparse sets** with an untyped base (`SparseSetBase`) and a typed storage (`SparseSet<C>`), swap-and-pop erase, and **views** driven by the smallest set (journey steps 8–9).
- **Own type erasure** (step 10): integer type ids and `std::vector<std::unique_ptr<SparseSetBase>>` instead of `std::type_index` and `std::any`. Components may be move-only, and the registry is move-only.
- **Generational entities** (step 11): an index plus a generation, `Entity::NONE`, stale handles detected by `isAlive`.
- **Systems inside the registry** (bootstrap style, `addSystem<Cs...>`), grouped in **stages**, with **resources** for what systems need beyond components, and a **command buffer** applied after each system (step 12).

Left for Part 2: running systems in parallel, paged `_sparse` arrays, groups, component events.

The full specification is in [`modules/engine.md`](../modules/engine.md) § ECS.

## Consequences

- **`SimContext` and `SystemPipeline` disappear.** What systems needed from the context (the random generator, rules, event outbox, NetId allocator, player roster, `dt`) becomes registry resources.
- **Killing during a system** goes through `registry.commands()`, applied after each system, so a collision can kill both entities and a weapon can spawn a missile from inside a view loop.
- **Stored entity handles are safe**: a dead target is detected instead of silently designating a new entity. Network messages still name entities by `NetId`, never by entity index or type id.
- **The ECS becomes the largest engine module of Part 1.** To keep the game team unblocked, the public headers (classes and signatures) are published first, and the game codes against them while the internals are finished.
- **Benchmarks are part of the module** (`ecs_benchmarks`, Google Benchmark), with the journey's targets: lookups in a few nanoseconds, views within 10% of a hand-written join.
