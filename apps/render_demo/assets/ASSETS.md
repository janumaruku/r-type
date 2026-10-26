# render_demo assets

| File | Origin | Licence |
| --- | --- | --- |
| `sprites.png` | Made for this project by `../tools/make_sprites.py` (Pillow). Five 32 × 32 cells, left to right: ship, ship with engine flame, enemy, bullet, floor tile. The ship faces right | Same as the repository |
| `DejaVuSansMono.ttf` | DejaVu Sans Mono 2.37, copied unmodified from the Ubuntu package `fonts-dejavu-core` ([dejavu-fonts.github.io](https://dejavu-fonts.github.io/)) | Bitstream Vera licence, DejaVu changes in the public domain: see `DejaVuSansMono-LICENSE.txt`, which must ship with the font |
| `keybindings.json` | Written for this project. The demo's default bindings; edit it to test rebinding | Same as the repository |
| `shot.wav` | Made for this project by `../tools/make_sounds.py` (NumPy, SoundFile). A 0.15 s descending square-wave chirp, 16-bit mono 44.1 kHz | Same as the repository |
| `explosion.wav` | Same script. A 0.7 s filtered noise burst that fades out | Same as the repository |
| `music.ogg` | Same script. An 8 s chiptune loop (A minor, F, C, G at 120 BPM), Ogg Vorbis mono 44.1 kHz; every note fades to silence, so the loop has no click | Same as the repository |

To change a sprite, edit its `draw_*` function in `make_sprites.py`, run `python3 apps/render_demo/tools/make_sprites.py` from the repository root, and commit the new `sprites.png` with the script.

To change a sound, edit its `make_*` function in `make_sounds.py`, run `python3 apps/render_demo/tools/make_sounds.py` from the repository root (after `pip install numpy soundfile`), and commit the new files with the script.

## Audio formats

- **Sound effects: WAV.** They are short and played often, and a WAV needs no decoding when it plays.
- **Music: OGG.** A track is long, so it is streamed, and OGG keeps the file small.
- **Never stream music from a WAV.** Raylib 5.5's `UnloadMusicStream` doesn't free the WAV decoder, so every WAV music leaks a little memory and ASan builds fail. Sound effects aren't affected.
