#!/usr/bin/env python3
"""Generate render_demo's sounds into assets/.

  shot.wav       a short descending laser chirp
  explosion.wav  a noise burst that fades out
  music.ogg      an 8-second chiptune loop that ends where it starts

Effects are WAV (no decoding when played), the music OGG (small, streamed).
Run it again after changing a sound:
  python3 apps/render_demo/tools/make_sounds.py
Needs NumPy and SoundFile (pip install numpy soundfile).
"""

from pathlib import Path

import numpy as np
import soundfile

RATE = 44100
ASSETS = Path(__file__).resolve().parent.parent / "assets"
SEED = 2026

BPM = 120
BEAT = 60 / BPM
BARS = 4
ROOTS = [57, 53, 48, 55]   # A minor, F, C, G: one chord per bar
ARPEGGIO = [0, 3, 7, 12, 7, 3, 0, 7]


def seconds(duration):
    return np.arange(int(RATE * duration)) / RATE


def frequency(midi_note):
    return 440.0 * 2 ** ((midi_note - 69) / 12)


def fade(samples, attack, release):
    envelope = np.ones(len(samples))
    attack_len = int(RATE * attack)
    release_len = int(RATE * release)
    envelope[:attack_len] = np.linspace(0, 1, attack_len)
    envelope[-release_len:] = np.linspace(1, 0, release_len)
    return samples * envelope


def make_shot():
    t = seconds(0.15)
    sweep = np.linspace(1400, 300, len(t))
    phase = 2 * np.pi * np.cumsum(sweep) / RATE
    wave = np.sign(np.sin(phase)) * 0.35
    return fade(wave * np.exp(-t * 18), 0.002, 0.03)


def make_explosion(rng):
    t = seconds(0.7)
    noise = rng.uniform(-1, 1, len(t))
    rumble = np.convolve(noise, np.ones(24) / 24, mode="same")
    wave = (0.6 * rumble * 3 + 0.25 * noise) * np.exp(-t * 6)
    return fade(np.clip(wave, -1, 1) * 0.8, 0.002, 0.1)


def note(midi_note, duration, kind):
    t = seconds(duration)
    if kind == "square":
        wave = np.sign(np.sin(2 * np.pi * frequency(midi_note) * t)) * 0.12
    else:
        wave = 2 * np.abs(2 * ((frequency(midi_note) * t) % 1) - 1) - 1
        wave *= 0.25
    return fade(wave, 0.005, duration * 0.4)


def make_music():
    eighth = BEAT / 2
    lead = []
    bass = []
    for root in ROOTS:
        lead += [note(root + 12 + step, eighth, "square") for step in ARPEGGIO]
        bass += [note(root - 12, BEAT, "triangle") for _ in range(4)]
    mix = np.concatenate(lead) + np.concatenate(bass)
    return np.clip(mix, -1, 1)


def main():
    rng = np.random.default_rng(SEED)
    soundfile.write(ASSETS / "shot.wav", make_shot(), RATE, subtype="PCM_16")
    soundfile.write(ASSETS / "explosion.wav", make_explosion(rng), RATE,
                    subtype="PCM_16")
    soundfile.write(ASSETS / "music.ogg", make_music(), RATE,
                    format="OGG", subtype="VORBIS")


if __name__ == "__main__":
    main()
