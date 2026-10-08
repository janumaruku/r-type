# How the modules work together

Four flows exercise every boundary in the architecture: joining a game, a key press reaching the screen, a Bydos being shot down, and a client crashing. In each diagram, "network" stands for `net::UdpSocket`, and only the adapters ever touch both protocol messages and game objects.

## Joining a game in progress

```mermaid
sequenceDiagram
    autonumber
    participant CS as Client ConnectingScene
    participant SA as ServerNetworkAdapter
    participant SM as SessionManager
    participant GW as GameWorld
    participant O as Other clients
    loop every 0.5 s until WELCOME
        CS->>SA: HELLO(v1, "Ana")
    end
    SA->>SA: decode header, factory creates HelloCommand
    SA->>GW: HelloCommand: reserveSlot() returns 2
    SA->>SM: add(endpoint, player 2, "Ana")
    SA->>CS: WELCOME(playerId 2)
    SA->>GW: apply(JoinPlayer 2)
    CS->>CS: switch to GameScene, keep the ServerLink
    Note over GW: next step(): the ship spawns, PlayerJoined is emitted
    SA->>O: EVENT PLAYER_JOINED(2) x3
    SA->>CS: SNAPSHOT: every entity, including ship 41, and the roster
    SA->>O: SNAPSHOT: ship 41 is new to them
    CS->>CS: create every entity it hasn't seen, find its ship in the roster
```

If WELCOME is lost, the client's next HELLO gets another one. If a snapshot is lost, the next one carries the same entities.

## From a key press to the screen

```mermaid
sequenceDiagram
    autonumber
    participant W as render::Window
    participant I as InputState + ActionMap
    participant GS as GameScene
    participant P as PredictionState
    participant SA as ServerNetworkAdapter
    participant GW as GameWorld
    participant R as RaylibRenderer
    W->>I: pollEvents() KEY_PRESSED(UP)
    GS->>I: isActive(MOVE_UP) at the 60 Hz step
    GS->>SA: INPUT(seq 812, UP)
    GS->>P: record(812, UP)
    GS->>GS: movePlayer() moves the local ship now
    SA->>GW: InputCommand: apply(SetInput player 2, 812, UP)
    GW->>GW: step(): playerInputSystem, movementSystem
    SA->>GS: SNAPSHOT(tick, lastProcessedInput 812) on an even tick
    GS->>P: reconcile(server position, 812)
    GS->>GS: interpolationSystem for remote entities
    GS->>R: renderSystem builds the RenderQueue, submit()
```

## A Bydos is shot down

```mermaid
sequenceDiagram
    autonumber
    participant GW as GameWorld systems
    participant SA as ServerNetworkAdapter
    participant C as All clients
    participant CW as ClientWorld + effects
    GW->>GW: collisionSystem: PLAYER_SHOT overlaps ENEMY
    GW->>GW: healthSystem: Bydos at 0 HP, kill commands for both, score +100
    GW->>SA: EnemyKilled(ship 41, bydos 63, position)
    GW->>GW: commands applied, then netIdCleanupSystem releases NetIds 63 and 77
    SA->>C: EVENT ENEMY_KILLED(41, 63, x, y)
    C->>CW: explosion effect at (x, y), explosion_small.wav
    SA->>C: next SNAPSHOT: 63 and 77 missing, roster score 1400
    C->>CW: tick complete, removeNotSeenSince(tick) deletes 63 and 77
    C->>CW: HUD shows score 1400
```

If the EVENT is lost, the Bydos still disappears with the next complete snapshot, just without the explosion.

## A client crashes

```mermaid
sequenceDiagram
    autonumber
    participant X as Crashed client
    participant SA as ServerNetworkAdapter
    participant SM as SessionManager
    participant GW as GameWorld
    participant O as Other clients
    X--xSA: datagrams stop (process killed)
    Note over SM: 5 s without a valid datagram
    SA->>SM: timedOut(now) returns the endpoint
    SA->>GW: apply(LeavePlayer 2, TIMEOUT)
    GW->>GW: destroy ship 41 and its missiles, free slot 2
    GW-->>SA: PlayerLeft(2, TIMEOUT)
    SA->>SM: remove(endpoint)
    SA->>O: EVENT PLAYER_LEFT(2, TIMEOUT) x3
    O->>O: HUD banner "P3 Ana lost connection"
    SA->>O: next SNAPSHOT: ship 41 gone, roster slot 2 EMPTY
    Note over GW,O: the tick loop never paused, and the others keep playing
```

## Who calls whom

| Caller | Callee | Through |
| --- | --- | --- |
| `ServerApp`, `ClientApp` | every module | Construction and wiring (composition roots) |
| `ServerNetworkAdapter` | `GameWorld` | `apply(PlayerCommand)`, `drainEvents()`, `collectReplicated()`, `roster()` |
| Both adapters | `rtype_protocol` | `Header` and message structs, read and written with `packet.read<T>()` and `packet << message` |
| Both adapters | `network` | `UdpSocket::sendTo`, `UdpSocket::receiveAll`, `net::Packet` |
| `GameWorld` systems | `engine_ecs` | `Registry` views, `commands()`, resources |
| Client `renderSystem` | `engine_render` | `RenderQueue::push`, `IRenderer::submit` |
| Client `GameScene` | `engine_input`, `engine_audio` | `ActionMap::isActive`, `IAudio::playSound` |
| Nothing | `GameWorld` → `network` or `render` | Forbidden by the [dependency rules](overview.md#dependency-rules) |
