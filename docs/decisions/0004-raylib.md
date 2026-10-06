# ADR 0004: Raylib for rendering, input and audio

- **Status:** accepted
- **Date:** 2026-10-05

## Context

The subject allows SFML, SDL or Raylib, and forbids full game engines (T-09). The first draft of the spec used SFML 3. The team chose Raylib instead, and the existing CI image (`janumaruku/epitest-with-raylib`) was already built for it.

## Decision

- Use **Raylib 5.5**, fetched and built from source by CPM.cmake, for the client's window, drawing, keyboard and audio.
- Only `engine_render` and `engine_audio` use it, privately, behind a pimpl. The rest of the engine, the game and the server never see a Raylib type.

## Consequences

- **The engine API doesn't change.** `IRenderer`, `DrawCommand`, `InputEvent` and `IAudio` were already library-neutral, so only the backends were renamed (`RaylibRenderer`, `RaylibAudio`).
- **Input is polled, not queued.** Raylib answers "is this key pressed this frame?" instead of delivering events. `Window::pollEvents()` turns those answers into the same `InputEvent`s as before ([engine: input](../modules/engine.md#input-engine_input)).
- **Raylib closes the window on Escape by default.** `Window` turns that off with `SetExitKey(KEY_NULL)`, so Escape stays a rebindable key.
- **Music needs a per-frame call.** `IAudio` gained `update()`, which calls `UpdateMusicStream`.
- **One global window.** Raylib supports a single window and OpenGL context, and textures can only exist while it is open. The client creates the window before loading any texture, and destroys it last.
- **Name clashes with `windows.h`.** On Windows, Asio includes `windows.h`, which declares `Rectangle`, `CloseWindow`, `ShowCursor`, `DrawText`, `PlaySound` and more, the same names Raylib uses. A file that includes both doesn't compile. The dependency rules keep `raylib.h` and Asio in separate files ([overview, rule 6](../architecture/overview.md#dependency-rules)).
- **Raylib is a C library.** Its functions are global and its structs are plain C. The pimpl classes are where they get wrapped in C++ ownership (constructors load, destructors unload).
