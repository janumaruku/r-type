# render_demo assets

| File | Origin | Licence |
| --- | --- | --- |
| `sprites.png` | Made for this project by `../tools/make_sprites.py` (Pillow). Five 32 × 32 cells, left to right: ship, ship with engine flame, enemy, bullet, floor tile. The ship faces right | Same as the repository |
| `DejaVuSansMono.ttf` | DejaVu Sans Mono 2.37, copied unmodified from the Ubuntu package `fonts-dejavu-core` ([dejavu-fonts.github.io](https://dejavu-fonts.github.io/)) | Bitstream Vera licence, DejaVu changes in the public domain: see `DejaVuSansMono-LICENSE.txt`, which must ship with the font |
| `keybindings.json` | Written for this project. The demo's default bindings; edit it to test rebinding | Same as the repository |

To change a sprite, edit its `draw_*` function in `make_sprites.py`, run `python3 apps/render_demo/tools/make_sprites.py` from the repository root, and commit the new `sprites.png` with the script.
