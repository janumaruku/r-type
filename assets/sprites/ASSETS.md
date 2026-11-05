# Sprite assets

The sprite sheets and the starting frame rectangles come from [ManuelR-T/R-Type](https://github.com/ManuelR-T/R-Type) (commit `4b535e3`, 2024-11-12), folder `assets/typesheets/`, under the MIT licence below. Only its assets and data are reused, never its code.

| File | Origin | Licence |
| --- | --- | --- |
| `player_ships.gif` | `r-typesheet42.gif`, copied unmodified. A 166 × 86 grid of 33 × 17 cells: one row per player colour (cyan, magenta, green, red, blue), one column per tilt state (`top`, `up`, `idle`, `down`, `bottom`) | MIT, ManuelR-T/R-Type |
| `bydos_slave.gif` | `r-typesheet5.gif`, copied unmodified. The Bydos slave turning, 16 frames | MIT, ManuelR-T/R-Type |
| `player_shots.gif` | `r-typesheet1.gif`, copied unmodified. Charge effects, beams and the player missile | MIT, ManuelR-T/R-Type |
| `enemy_shots_and_explosions.png` | `r-typesheet43.png`, copied unmodified. ManuelR-T/R-Type's edited 424-px version of the original sheet: the enemy ball missile and the explosions. Every rectangle in `catalog.json` was measured on it | MIT, ManuelR-T/R-Type |
| `catalog.json` | Written for this project. The ship grid, the state names and the starting rectangles come from ManuelR-T/R-Type's sprite data (`missileBall.json`, `explosion.json`); each entry's `source` field names the original sheet and frames | Same as the repository |

The sheets were renamed after what they hold. The `source` fields in `catalog.json` keep the original names, so they can be traced back to the reference repository.

To change a rectangle, edit `catalog.json` and check it in game. Sprites are drawn at scale 1, so each entity's largest visible frame is its hitbox in `Rules`: re-measuring a frame means changing `Rules` too ([client.md](../../docs/modules/client.md#presentation)).

## Licence of the sheets

```text
Copyright (c) 2024 R-TYPE

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
