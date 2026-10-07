# ADR 0002: State-based snapshots instead of acknowledgements and resends

- **Status:** accepted
- **Date:** 2026-10-05

## Context

UDP may lose, duplicate or reorder datagrams. The first draft of the protocol sent separate ENTITY_SPAWN and ENTITY_DESTROY messages. Those had to be reliable: a lost DESTROY would leave a ghost entity on a client forever. That required sequence numbers, ack bitfields, resend timers, ordered delivery and an out-of-order buffer per client. It was the most complex and bug-prone part of the design, and there are several of us writing code.

## Decision

- Every SNAPSHOT describes the **whole** game: every replicated entity (NetId, kind, variant, position, velocity) and every player slot. A snapshot that doesn't fit in one datagram is split into numbered chunks.
- The client creates the entities it hasn't seen, updates the ones it knows, and deletes those missing from a complete snapshot.
- Nothing is acknowledged or resent. The few messages that matter beyond one snapshot are made robust in simpler ways:
    - HELLO is repeated every 0.5 s until WELCOME arrives, and every HELLO gets a WELCOME.
    - Important EVENTs (a player killed, joined or left, game over) are sent on three consecutive ticks with the same `eventId`, and clients ignore repeats.
    - DISCONNECT and SHUTDOWN are sent three times.
- Events only explain *why* things happened (sounds, explosions, banners). The state itself always comes from snapshots.

The full rules are in the [RFC](../protocol/RFC-rtype.md).

## Consequences

- A lost datagram is repaired by the next snapshot, 33 ms later. No ghost entities, no stuck state, no resend logic.
- A player joining mid-game needs no special catch-up: their first snapshot is complete.
- Snapshots are larger than position-only updates: about 70 KB/s per client at the planned peak, which is fine on any home connection or LAN.
- A lost one-time event (a single shot sound) is simply not played. That is acceptable.
- If Part 2 needs reliable delivery for something new, it can be added then, for that message only.
