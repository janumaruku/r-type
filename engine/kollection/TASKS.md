# Kollection — task list

The step-by-step plan for building Kollection, the declarative UI framework of the second game. The design is in [`docs/architecture/kollection.md`](../../docs/architecture/kollection.md); the § numbers below point into it.

## Conventions

- The library lives in `engine/kollection`, but its namespace is `kollection` (alias `koll`), as in the spec.
- Files are PascalCase like the rest of the repo (`BoxConstraints.hpp`), included as `#include "kollection/core/BoxConstraints.hpp"`.
- Two targets:
  - `engine_kollection`: `core`, `paint`, `framework`, `widget`. Depends on nothing but the standard library, so it always builds and is fully unit-tested.
  - `engine_kollection_raylib`: `backend::raylib`. Links `engine_render` and `engine_input`, built only with `RTYPE_BUILD_CLIENT`.
- Unit tests go in `tests/engine/kollection/`. The visual checks use the demo, `r-type_kollection_demo`.
- Tasks marked **⚙ engine** add what Kollection needs to `engine_render` or `engine_input`.
- Every phase ends with a ✅ check: tests passing and, from Phase 2 on, something new on screen.

---

## Phase 0 — Scaffolding

- [x] **T0. Targets and the empty demo**
  - `engine/kollection/CMakeLists.txt` with `engine_kollection`. CMake won't generate a static library without sources, so `core::Color` (from T1) comes first, as its first `.cpp`. `engine_kollection_raylib` is created in T7, with its first source.
  - `add_subdirectory(kollection)` in `engine/CMakeLists.txt`.
  - `tests/engine/kollection/CMakeLists.txt` with `rtype_add_test`, plus `add_subdirectory(kollection)` in `tests/engine/CMakeLists.txt`.
  - `apps/kollection_demo/` → `r-type_kollection_demo` written to the repo root, built under `RTYPE_BUILD_EXAMPLES` like `ecs_example`. At first it opens a `Window` with a `RaylibRenderer` and clears the screen every frame.
  - ✅ The demo opens a dark window, and the empty test executable runs.

## Phase 1 — Core value types (§3.4, §4.6.1, §5.1)

- [x] **T1.** `Size`, `Offset`, `Rect`, `EdgeInsets`, `Alignment`, `WidgetKey` (`Color` was done in T0).
  
  - Tests: `Rect::contains` on its edges, `Alignment::inscribe` for the corners and the centre.

- [x] **T2.** `TextStyle` (every field optional), `TextRun`, `TextStyle::merge`.
  
  - Tests: fields set on the right side win; unset fields keep the left value.

- [x] **T3.** `BoxConstraints`: `tight`, `loose`, `expand`, `constrain`, `smallest`, `isTight`, `hasBoundedWidth/Height`, `deflate`, `loosen`, `enforce`.
  
  - Test it heavily, since most layout bugs start here. For example: `deflate` never goes below 0, and `expand().enforce(loose(100×50))` is tight 100×50.

- [x] **T4.** `KeyCode`, `KeyEvent`, `PointerEvent`, `ResizeEvent`, `Event`. No tests needed.

- ✅ All core tests pass.

## Phase 2 — Paint, then first pixels (§2.2, §4.9, §5.2)

- [ ] **T5.** `paint::` commands (the variant), `PaintList`, `PaintContext`.
  
  - Test: the context records the right commands, in order.

- [ ] **T6. ⚙ engine: text measurement.** Add `FontCache::measure(FontId, std::string_view, size) → Vec2f`, built on `MeasureTextEx`.
  
  - Its font size and spacing must match what `RaylibRenderer` uses to draw a `TextCmd`, or text is measured at one size and drawn at another.

- [ ] **T7.** `framework::Surface` and `framework::TextMeasurer`, then `backend::raylib::RaylibSurface`, which creates the `engine_kollection_raylib` target (`RaylibSurface.cpp` is its first source):
  
  - `sizeInUnits()` returns the logical size;
  - `measure()` goes through `FontCache`;
  - `submit()` turns `FillRect` / `StrokeRect` / `DrawText` into `RectCmd` / `TextCmd` on a UI layer, and stores them;
  - `appendTo(RenderQueue&)` adds the stored commands to a frame's queue.

- ✅ **Demo:** a hand-written `PaintList` (a blue rect, an outline, some text) on screen, drawn every frame.

## Phase 3 — Render tree without widgets (§4.6, §4.10)

- [ ] **T8.** `ParentData`, `RenderObject`, `RenderProxyBox`, `RenderBoxWithChildren`, and the first `Pipeline` with only the render side: `setRootRenderObject`, `requestLayout`, `requestPaint`, `textMeasurer`, `drawFrame` (layout + paint + submit).
  
  - Tests, with a fake render object that counts `performLayout` calls: the memo skips a clean subtree; `markNeedsLayout` bubbles up to the root.

- [ ] **T9.** `RenderColoredBox`, `RenderPadding`, `RenderConstrainedBox`, `RenderAlign`, `RenderParagraph`. The tests use a `FakeMeasurer`.
  
  - Tests: the §7 numbers (a 10×3 box, child at (1,1)); `expand()` in an unbounded axis stays finite; `Align` centres its child.

- [ ] **T10.** `RenderFlex` (MVP: no flex factors).
  
  - Tests: a column of 3 children with spacing; each `mainAxisAlignment`; `crossAxisAlignment::stretch`.

- ✅ **Demo:** a column of padded, coloured boxes with text, built by hand from render objects.

## Phase 4 — Widgets and elements, stateless (§3.1–§3.5, §4.1–§4.4)

- [ ] **T11.** Widget base classes, `make<T>`, `Widget::canUpdate`. `Element`, `BuildContext`, `ComponentElement`, `StatelessElement`, and the three `RenderObjectElement` kinds with the attachment rules (§4.6.5). `Pipeline` takes a root widget. **First build only** (mount), no updates yet.

- [ ] **T12.** Reconciliation: `updateChild`, `Element::update`, the dirty list and `flushBuild` (index loop), `Pipeline::deactivate` and `finalizeTree`, multi-child update by index.
  
  - Tests: the same widget type keeps the same render object (compare the pointer); a different type replaces it; a removed child is unmounted only at the end of the pass.

- [ ] **T13.** The concrete widgets: `ColoredBox`, `Padding`, `ConstrainedBox` / `SizedBox`, `Align`, `Paragraph`, `Text` (explicit style for now), `row` / `column`, `Container`.
  
  - Test: what each `Container` config expands into.

- ✅ **Demo:** the Phase 3 screen, now written as widgets.

## Phase 5 — State (§3.6, §4.4.3)

- [ ] **T14.** `StateBase`, `State<W>`, `StatefulElement`, `setState`, `initState`, `didUpdateWidget`, `dispose`.
  
  - Tests: state survives a parent rebuild; `dispose` runs exactly once; `setState` after `dispose` does nothing; an element dirtied during `flushBuild` is rebuilt in the same pass.

- ✅ All state tests pass. There's nothing new to see until input is wired in Phase 6.

## Phase 6 — Pointer input (§4.7, §5.1, §5.2)

- [ ] **T15. ⚙ engine: mouse.**
  
  - Add `MOUSE_PRESSED`, `MOUSE_RELEASED` and `MOUSE_MOVED` to `InputEvent`, with a position and a button; emit them in `Window::pollEvents`.
  - Add a helper in `engine_render` that converts a window position to a logical one: the inverse of `letterbox`.
  - Test: the conversion, with letterbox bars on each side.

- [ ] **T16.** `HitTestResult`, `Listener` with `RenderListener`, `Pipeline::dispatchEvent` (pointer events and resize), `RaylibSurface::toEvent`.
  
  - Tests: the hit path is ordered deepest first; a click outside the tree hits nothing.

- ✅ **Demo:** the spec's `Counter` from §7. Clicking it increments the number.

## Phase 7 — Inherited widgets (§3.7, §4.5)

- [ ] **T17.** `InheritedWidget`, `InheritedElement`, `dependOnInheritedOfType` (ancestor walk), notifying dependents, and unsubscribing on deactivate. Then `DefaultTextStyle`, with `Text` using it.
  
  - Test: changing the style rebuilds only the elements that depend on it.

- ✅ **Demo:** a key that switches the default text colour.

## Phase 8 — Keyboard, focus and Button (§4.8)

- [ ] **T18. ⚙ engine: keys and gamepad.**
  
  - Add `TAB` and `BACKSPACE` to `Key` and `KEY_TABLE`, plus gamepad D-pad, A and B.
  - `RaylibSurface::toEvent` maps them to `KeyCode`s, and tracks `LSHIFT` so it can produce Shift+Tab.

- [ ] **T19.** `FocusNode`, `FocusManager`, the `Focus` widget, key dispatch going up through enclosing `Focus` widgets, and next/previous traversal.
  
  - Tests: traversal order and wrap-around; a handled key stops going up; focus moves on when the focused node is unmounted.

- [ ] **T20.** `Button`.

- ✅ **Demo:** a real menu (Play / Options / Quit). Arrows or the gamepad move focus, Enter activates, a mouse click works too, and Quit closes the window. **This completes the MVP.**

## Phase 9 — Wrap-up

- [ ] **T21.** Add a Kollection section to `docs/modules/engine.md`, and update `docs/architecture/kollection.md` wherever the code ended up different from the spec.

---

## After the MVP

The spec's §10 lists what comes next: `Expanded` with `FlexParentData`, keyed child lists, `Theme`, clip and translate in the Raylib adapter, then the terminal and SDL backends.
