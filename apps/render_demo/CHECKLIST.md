# render_demo manual checklist

CI has no screen, so the window, the GPU and the keyboard are checked by hand. Copy this file into the PR description, tick every row on **Linux and Windows**, and attach one screenshot per platform.

Run it from any folder: the demo finds its assets through `AssetLocator`.

```sh
cmake --build --preset linux-debug --target render_demo
./build/linux-debug/apps/render_demo/render_demo
```

On Windows the binary is `build\windows-clangcl-debug\apps\render_demo\Debug\render_demo.exe`.

| Check | How | Linux | Windows |
| --- | --- | --- | --- |
| Letterbox | Resize the window very wide, very tall and tiny: the playfield keeps its 16:9 ratio with black bars, never stretched, never upside down | [ ] | [ ] |
| Minimise | Minimise and restore: no crash (Windows reports a size of 0) | [ ] | [ ] |
| Layers | Left panel: the "layer 300" text is over the red enemy, which is over the blue panel. Right: the green rectangle is over the red one (same layer, pushed last) | [ ] | [ ] |
| Movement | Arrows and WASD both move the ship; it shows its flame while moving and faces left when going left | [ ] | [ ] |
| Rebinding | In `apps/render_demo/assets/keybindings.json`, set FIRE's keys to `["ENTER"]` and restart: ENTER fires, SPACE doesn't. Then put an unknown key name in: the log warns about that entry and the other bindings still work. Restore the file afterwards | [ ] | [ ] |
| Quick tap | A very quick tap on fire still adds one to "shots fired" and launches a bullet | [ ] | [ ] |
| Focus | Hold an arrow and switch to another window: the ship stops, and the log shows `FOCUS_LOST`, then `FOCUS_GAINED` when you come back | [ ] | [ ] |
| Escape | Escape quits through the `QUIT` action (the log ends with `quit after N shots`) | [ ] | [ ] |
| Text | The first two HUD lines use DejaVu Sans Mono, the next ones Raylib's built-in font | [ ] | [ ] |
| Caches | The log shows `sprites.png loaded twice: same id 0`. `render_demo --missing-asset` logs `Texture file does not exist: …missing.png` and exits with 84, without a crash | [ ] | [ ] |
| Log | Raylib's own messages appear with the `[raylib]` context, in our format | [ ] | [ ] |
| Shutdown | Closing the window: every `Unloaded …` line from Raylib comes before `Window closed successfully`, and there's no warning after it | [ ] | [ ] |
