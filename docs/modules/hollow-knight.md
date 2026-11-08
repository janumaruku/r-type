# Hollow Knight engine demo

`hollow_knight` is a standalone, single-player application that demonstrates a second game built on the repository's engine. It contains a title screen and one playable cave room, with no dependency on the R-Type rules or its client scenes.

Build and run it from the repository root:

```sh
cmake --preset linux-debug
cmake --build --preset linux-debug --target hollow_knight
./hollow_knight
```

It is built when `RTYPE_BUILD_CLIENT` is enabled. The Knight, three enemy types, Nail slash and cave decorations use Hollow Knight PNG artwork through the shared `TextureCache` and `SpriteCmd` renderer. Source and copyright notes are in [`assets/hollow_knight/ASSETS.md`](../../assets/hollow_knight/ASSETS.md). The cave background is composed from layered sprite and rectangle commands. Ambient spores and action particles are animated from room time and actor state. A flying Crossroads Guard appears in each wave.

The app uses the shared `engine::audio::IAudio` backend and loops an optional
`assets/hollow_knight/greenpath.ogg`, `.mp3` or `.flac` file. The original OST
is not bundled; its source and file placement are documented alongside the
assets.

## Controls

| Action | Default keys |
| --- | --- |
| Move | A / D or left / right |
| Jump | Space |
| Nail attack | J or left mouse button |
| Directional Nail attack | Hold up or down and press J or click left mouse button |
| Dash | Left Shift |
| Focus to heal | Hold F while grounded and still; costs 33 Soul |
| Rest at the bench | E near the bench |
| Confirm / menu | Enter, W / S or up / down |
| Pause / back | Escape |

The room has four stepped platforms, five health masks, a Soul vessel, per-enemy collision boxes, knockback, temporary damage invulnerability, pogo bounce on a downward Nail hit, a resting bench and checkpoint respawn. Nail attacks play a synthesized swing; confirmed hits play an impact and emit a brief burst of bright sparks. Clearing a wave starts the next after a short breather. Waves add enemies up to a cap of eight and raise enemy health and speed; the wave counter itself has no gameplay cap. The bench checkpoints the current wave. Defeating enemies gives Soul; each nail hit can add up to 11 points. Resting restores masks and resets the current wave. Death returns the Knight to the last bench with one mask and no Soul.

## ECS and protocol reuse

`World` owns an `engine::ecs::Registry` with app-specific `Player`, `Enemy`, `Body` and `Transform` components. The update stage runs the player controller, enemy behaviour, platform physics and combat systems. `FixedTimestep`, `InputState`, `ActionMap`, `RenderQueue` and `RaylibRenderer` come from the shared engine.

The room checkpoint is serialized into a bounded `net::Packet` using the shared `rtype::protocol::Snapshot` payload codec. The protocol adds `GENERIC_PLAYER` and `GENERIC_ENEMY` entity kinds for other games; the R-Type server still emits only its original four kinds. In this app, the generic player's `variant` stores health and facing, and a generic enemy's `variant` stores health. Checkpoint bytes stay local to this single-player demo. The app does not open a UDP socket or claim multiplayer support.

The demo keeps Hollow Knight's gameplay components in the app instead of adding them to `rtype_game`, which remains the authoritative R-Type simulation.
