# Accessibility

The subject requires the game and its documentation to be accessible for motor, audio/visual and cognitive disabilities (requirement A-01). These are the features Part 1 commits to.

| Category | Features in Part 1 |
| --- | --- |
| Motor | Every action can be rebound (`config/keybindings.cfg`). Fire is hold-to-fire, so there's no button mashing. Arrows and WASD both work by default. Nothing depends on precise timing beyond normal play. |
| Visual | Players are told apart by colour **and** a `P1`–`P4` label. A high-contrast mode adds outlines to enemies and enemy missiles. HUD font size is configurable. A reduced-flash option dims explosions. |
| Audio | Nothing important is conveyed by sound alone: joins, departures and deaths also show as HUD banners. Master, music and SFX volumes are separate, with a mute toggle. |
| Cognitive | One screen, one goal. A controls overlay is toggled with F1. The HUD uses plain words ("P3 disconnected") rather than codes. A game over restarts automatically, with nothing to navigate. |

## Settings

All settings live in `config/client.cfg` ([handbook](../handbook/development.md#configuration-files-client-only)):

| Key | Default | Effect |
| --- | --- | --- |
| `masterVolume`, `musicVolume`, `sfxVolume` | 1.0, 0.6, 0.8 | 0 to 1 |
| `muted` | false | Silences everything |
| `hudFontSize` | 20 | In pixels at 720p |
| `highContrast` | false | Outlines enemies and enemy missiles |
| `reduceFlashes` | false | Dims explosions |
| `fullscreen` | false | |

## Documentation

The documentation itself is plain Markdown published as a static site with search and a navigation outline. Every diagram comes with tables or text that explain it, so the content doesn't depend on seeing the picture.
