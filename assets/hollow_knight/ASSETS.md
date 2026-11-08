# Hollow Knight demo assets

These assets use Hollow Knight artwork from the sources listed below. The
character and bestiary images were copied from the public resource archive
[`zheyuanzhou/Hollow-Knight-Resources`](https://github.com/zheyuanzhou/Hollow-Knight-Resources);
the Knight frames and Nail slash were cropped from the Unity atlas, and the HUD
mask was isolated from the supplied gameplay reference.

| File | Original file | Source |
| --- | --- | --- |
| `knight.png` | `Characters_0017_sprite.png` | [`HK_Character Sprites`](https://github.com/zheyuanzhou/Hollow-Knight-Resources/tree/master/HK_Character%20Sprites) |
| `knight_frame_0.png`–`knight_frame_2.png` | side-facing Knight frames `#561`–`#563` | Cropped from the [`The Knight main sprites` atlas](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/The%20Knight/The%20Knight%20main%20sprites%20-%20atlas0%20%2300000357.png) using its Unity rects; cycled by the in-game animation |
| `crawler.png` | `bestiary_crawler_f.png` | `Bestiary_Sprites.zip` in the [resource archive](https://github.com/zheyuanzhou/Hollow-Knight-Resources) |
| `infected_husk.png` | `bestiary_infected_husk1_f.png` | `Bestiary_Sprites.zip` in the [resource archive](https://github.com/zheyuanzhou/Hollow-Knight-Resources) |
| `crossroads_guard.png` | `bestiary_crossroads-guard_f.png` | `Bestiary_Sprites.zip` in the [resource archive](https://github.com/zheyuanzhou/Hollow-Knight-Resources) |
| `nail_slash.png` | atlas slice `_449` | [`The Knight main sprites - atlas0`](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/The%20Knight/The%20Knight%20main%20sprites%20-%20atlas0%20%2300000357.png), cropped from Unity rect `(x=1900, y=2072, 213 × 105)`; the renderer crops transparent padding to `(3, 3, 201 × 86)` to align the Nail hitbox with the visible slash |
| `mask.png` | Health-mask HUD icon | Cropped from the Hollow Knight gameplay reference image supplied with this demo request; surrounding scene pixels were made transparent |
| `nail_swing.wav` | Nail swing sound | Procedurally synthesized for this demo; not sampled from the original game |
| `nail_hit.wav` | Nail impact sound | Procedurally synthesized for this demo; not sampled from the original game |
| `ambient.ogg` | `apps/render_demo/assets/music.ogg` | Project-generated 8 s chiptune loop; copied as a fallback for this room |
| `platform_ledge.png` | `_0001_abyss_plat3.png` | [Abyss environment sprites](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/Architecture%20%26%20Environment/Area%20specfic%20architecture/Abyss) |
| `platform_stone.png` | `_0002_abyss_plat2.png` | [Abyss environment sprites](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/Architecture%20%26%20Environment/Area%20specfic%20architecture/Abyss) |
| `blue_grass_small.png` | `abyss_0002_blue_grass_02.png` | [Abyss Lifeblood room sprites](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/Architecture%20%26%20Environment/Area%20specfic%20architecture/Abyss/Lifeblood%20room) |
| `blue_grass_tall.png` | `abyss_0003_blue_grass_01.png` | [Abyss Lifeblood room sprites](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/Architecture%20%26%20Environment/Area%20specfic%20architecture/Abyss/Lifeblood%20room) |
| `root_platform.png` | `abyss_0010_root_floor_02.png` | [Abyss Lifeblood room sprites](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/Architecture%20%26%20Environment/Area%20specfic%20architecture/Abyss/Lifeblood%20room) |
| `ceiling_thorns.png` | `abyss_spike_bg_roof.png` | [Abyss environment sprites](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/Architecture%20%26%20Environment/Area%20specfic%20architecture/Abyss) |
| `blue_vine.png` | `abyss_vine_packed_01.png` | [Abyss environment sprites](https://git.coon.myds.me/cooney/Knight_Hollow/src/commit/ef8c35530201844dff948477d6004d256fc8876f/Assets/Art/Sprites/Architecture%20%26%20Environment/Area%20specfic%20architecture/Abyss) |

Hollow Knight and its artwork are © Team Cherry. These assets are included only
for the repository's local educational game-engine demo; no ownership or
separate licence is claimed for them. See the upstream archive for its
provenance.

## Optional soundtrack

The app looks for `greenpath.ogg`, `greenpath.mp3` or `greenpath.flac` in this
directory and loops the first available file through the engine audio backend.
If none is present, it loops `ambient.ogg`, the project's generated chiptune
placeholder, so the room is not silent. The original soundtrack is not bundled.
The Greenpath track can be purchased and downloaded from Christopher Larkin's
[official Bandcamp track page](https://christopherlarkin.bandcamp.com/track/greenpath).
