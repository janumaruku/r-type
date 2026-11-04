/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** WidgetKey
*/

#ifndef R_TYPE_WIDGET_KEY_HPP
#define R_TYPE_WIDGET_KEY_HPP

#include <cstdint>
#include <string>
#include <variant>

namespace kollection::core {

/**
 * @brief An optional name that tells sibling widgets of the same type apart.
 *
 * When a parent rebuilds, the framework matches each new child widget with
 * an existing element: the element is reused only if the widget's type and
 * key are the same. Without keys, siblings are matched by position, so
 * removing or moving one child hands the state of the next one to the wrong
 * widget. A key per child keeps each state with its own child.
 *
 * Most widgets need no key. A key only has to be unique among its siblings.
 *
 * @ref value holds one of three alternatives:
 * @li @c std::monostate: no key. This is the default, from @c WidgetKey{}.
 * @li @c std::int64_t: a numeric key, such as an id from the game's data.
 * @li @c std::string: a named key, such as @c "play" for a menu button.
 *
 * @par Example
 * @code{.cpp}
 * WidgetKey none;
 * WidgetKey byId{std::int64_t{42}};
 * WidgetKey byName{std::string{"play"}};
 * none == WidgetKey{};                         // true: no key equals no key
 * byId == WidgetKey{std::string{"42"}};        // false: different types
 * @endcode
 */
struct WidgetKey {
    std::variant<std::monostate, std::int64_t, std::string> value; ///< The key, or @c std::monostate for none.

    /**
     * @brief Compares two keys.
     *
     * @param other The key to compare with.
     * @return @c true if both hold the same alternative with the same value.
     * Two empty keys are equal, so unkeyed widgets are matched by type only.
     */
    bool operator==(const WidgetKey &other) const = default;
};
}

#endif //R_TYPE_WIDGET_KEY_HPP
