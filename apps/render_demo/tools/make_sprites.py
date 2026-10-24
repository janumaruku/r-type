#!/usr/bin/env python3
"""Generate render_demo's sprite sheet: assets/sprites.png.

Five 32x32 cells, left to right:
  0 ship, 1 ship with its engine flame, 2 enemy, 3 bullet, 4 floor tile.

The ship faces right, so a negative source width in a SpriteCmd shows it
facing left. Run it again after changing a cell:
  python3 apps/render_demo/tools/make_sprites.py
Needs Pillow (pip install pillow).
"""

from pathlib import Path

from PIL import Image, ImageDraw

CELL = 32
CELLS = 5
OUTPUT = Path(__file__).resolve().parent.parent / "assets" / "sprites.png"

HULL = (60, 210, 255, 255)
COCKPIT = (230, 250, 255, 255)
WING = (30, 120, 200, 255)
FLAME = (255, 170, 40, 255)
FLAME_CORE = (255, 240, 150, 255)
ENEMY = (240, 70, 80, 255)
ENEMY_EYE = (255, 230, 90, 255)
BULLET = (255, 225, 80, 255)
TILE = (70, 80, 110, 255)
TILE_EDGE = (110, 125, 160, 255)


def cell_box(index):
    return index * CELL, 0


def draw_ship(draw, ox, oy, flame):
    draw.polygon([(ox + 6, oy + 8), (ox + 28, oy + 16), (ox + 6, oy + 24)],
                 fill=HULL)
    draw.polygon([(ox + 8, oy + 4), (ox + 16, oy + 12), (ox + 8, oy + 12)],
                 fill=WING)
    draw.polygon([(ox + 8, oy + 28), (ox + 16, oy + 20), (ox + 8, oy + 20)],
                 fill=WING)
    draw.rectangle([ox + 16, oy + 14, ox + 20, oy + 18], fill=COCKPIT)
    if flame:
        draw.polygon([(ox + 6, oy + 12), (ox + 0, oy + 16), (ox + 6, oy + 20)],
                     fill=FLAME)
        draw.rectangle([ox + 3, oy + 15, ox + 6, oy + 17], fill=FLAME_CORE)


def draw_enemy(draw, ox, oy):
    draw.ellipse([ox + 4, oy + 6, ox + 27, oy + 25], fill=ENEMY)
    draw.rectangle([ox + 8, oy + 13, ox + 13, oy + 18], fill=ENEMY_EYE)


def draw_bullet(draw, ox, oy):
    draw.ellipse([ox + 10, oy + 13, ox + 21, oy + 18], fill=BULLET)


def draw_tile(draw, ox, oy):
    draw.rectangle([ox, oy, ox + CELL - 1, oy + CELL - 1], fill=TILE_EDGE)
    draw.rectangle([ox + 2, oy + 2, ox + CELL - 3, oy + CELL - 3], fill=TILE)


def main():
    sheet = Image.new("RGBA", (CELL * CELLS, CELL), (0, 0, 0, 0))
    draw = ImageDraw.Draw(sheet)

    draw_ship(draw, *cell_box(0), flame=False)
    draw_ship(draw, *cell_box(1), flame=True)
    draw_enemy(draw, *cell_box(2))
    draw_bullet(draw, *cell_box(3))
    draw_tile(draw, *cell_box(4))
    sheet.save(OUTPUT)
    print(f"wrote {OUTPUT}")


if __name__ == "__main__":
    main()
