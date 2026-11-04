/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Event
*/

#ifndef R_TYPE_EVENT_HPP
#define R_TYPE_EVENT_HPP

#include <variant>

#include "Dimensions.hpp"

namespace kollection::core {

/**
 * @brief The keys Kollection understands.
 *
 * The list is kept small on purpose: it is what focus navigation, buttons
 * and text input need. Each backend maps its own keys to these values and
 * drops the keys it cannot map.
 *
 * Every printable character is reported as @ref Char, with the character
 * itself in @ref KeyEvent::codepoint, so no letter or digit needs its own
 * value here.
 */
enum class KeyCode {
    Up,        ///< Arrow up. Moves focus to the previous widget if nobody handles it.
    Down,      ///< Arrow down. Moves focus to the next widget if nobody handles it.
    Left,      ///< Arrow left.
    Right,     ///< Arrow right.
    Enter,     ///< Enter or Return. Presses the focused button.
    Escape,    ///< Escape.
    Tab,       ///< Tab. Moves focus to the next widget, or the previous one with Shift.
    Backspace, ///< Backspace. Deletes the character before the cursor in text input.
    Char,      ///< A printable character, stored in @ref KeyEvent::codepoint.
};

/**
 * @brief A key was pressed, released or held down.
 *
 * Key events go to the focused widget first, then to each enclosing
 * @c Focus widget until one of them handles it.
 *
 * @par Example
 * @code{.cpp}
 * KeyEvent shiftTab{.key = KeyCode::Tab, .shift = true};
 * KeyEvent letter{.key = KeyCode::Char, .codepoint = U'a'};
 * KeyEvent release{.key = KeyCode::Enter, .phase = KeyEvent::Phase::Up};
 * @endcode
 */
struct KeyEvent {
    /**
     * @brief What happened to the key.
     */
    enum class Phase {
        Down,   ///< The key was just pressed.
        Up,     ///< The key was just released.
        Repeat, ///< The key is held down and the system sent a repeat.
    };

    KeyCode key = KeyCode::Char; ///< The key. Defaults to @ref KeyCode::Char.
    Phase phase = Phase::Down;   ///< Pressed, released or repeated. Defaults to @ref Phase::Down.
    char32_t codepoint = 0;      ///< The Unicode character. Only valid when @ref key is @ref KeyCode::Char, 0 otherwise.
    bool ctrl = false;           ///< @c true if a Ctrl key was held.
    bool alt = false;            ///< @c true if an Alt key was held.
    bool shift = false;          ///< @c true if a Shift key was held.
};

/**
 * @brief The mouse moved, or one of its buttons was pressed or released.
 *
 * The position is in the same logical units as the layout, so it can be
 * compared directly with the rectangles of the render tree during hit
 * testing.
 *
 * @par Example
 * @code{.cpp}
 * PointerEvent click{.pos = {120, 40}, .phase = PointerEvent::Phase::Down};
 * PointerEvent move{.pos = {125, 42}, .phase = PointerEvent::Phase::Move};
 * @endcode
 */
struct PointerEvent {
    /**
     * @brief What the pointer did.
     */
    enum class Phase {
        Down, ///< A button was just pressed.
        Up,   ///< A button was just released.
        Move, ///< The pointer moved. @ref button is not meaningful.
    };

    Offset pos;                ///< The pointer position, in logical units from the top-left corner.
    Phase phase = Phase::Down; ///< Pressed, released or moved. Defaults to @ref Phase::Down.

    /**
     * @brief The button that changed state, for @ref Phase::Down and @ref Phase::Up.
     *
     * Uses the same numbering as Raylib and the engine input:
     * @li 0: left button. This is the default.
     * @li 1: right button.
     * @li 2: middle button.
     */
    int button = 0;
};

/**
 * @brief The drawing area changed size.
 *
 * The pipeline uses the new size as the tight constraints of the root, so
 * the whole tree is laid out again. Every backend reports a resize this
 * way, whether it comes from a game window, a terminal or an SDL window.
 */
struct ResizeEvent {
    Size newSize; ///< The new size of the drawing area, in logical units.
};

/**
 * @brief Any input event Kollection handles.
 *
 * Backends turn their native events into one of these alternatives and
 * hand it to the pipeline, which dispatches it by type:
 * @li @ref KeyEvent: sent to the focused widget.
 * @li @ref PointerEvent: sent to the widgets under the pointer.
 * @li @ref ResizeEvent: changes the root constraints.
 *
 * @par Example
 * @code{.cpp}
 * Event event = KeyEvent{.key = KeyCode::Enter};
 * if (const auto *key = std::get_if<KeyEvent>(&event))
 *     handleKey(*key);
 * @endcode
 */
using Event = std::variant<KeyEvent, PointerEvent, ResizeEvent>;
}

#endif //R_TYPE_EVENT_HPP
