/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Styles
*/

#ifndef R_TYPE_STYLES_HPP
#define R_TYPE_STYLES_HPP

#include <cstdint>
#include <optional>
#include <string>

namespace kollection::core {

/**
 * @brief An 8-bit RGBA colour, opaque black by default.
 *
 * Colours are always truecolor inside Kollection; a backend with fewer
 * colours (a 256- or 16-colour terminal) picks the closest one itself.
 *
 * Transparent is a real colour, never a way to say "not set": a field that
 * can be unset is a @c std::optional<Color>, as in @ref TextStyle.
 *
 * @par Example
 * @code{.cpp}
 * Color red{.r = 255};                    // g, b = 0 and a = 255
 * Color glass{.r = 255, .g = 255, .b = 255, .a = 64};
 * @endcode
 */
struct Color {
    std::uint8_t r = 0;   ///< Red channel.
    std::uint8_t g = 0;   ///< Green channel.
    std::uint8_t b = 0;   ///< Blue channel.
    std::uint8_t a = 255; ///< Opacity: 0 is fully transparent, 255 opaque.

    /**
     * @brief Compares the four channels.
     *
     * @param other The colour to compare with.
     * @return @c true if every channel is equal.
     */
    bool operator==(const Color &other) const = default;

    static const Color BLACK;       ///< Opaque black, {0, 0, 0, 255}.
    static const Color WHITE;       ///< Opaque white, {255, 255, 255, 255}.
    static const Color BLUE;        ///< Opaque blue, {0, 0, 255, 255}.
    static const Color TRANSPARENT; ///< Fully transparent, {0, 0, 0, 0}.
};

constexpr Color Color::BLACK = {.r = 0, .g = 0, .b = 0};
constexpr Color Color::WHITE = {.r = 255, .g = 255, .b = 255};
constexpr Color Color::BLUE = {.r = 0, .g = 0, .b = 255};
constexpr Color Color::TRANSPARENT = {.r = 0, .g = 0, .b = 0, .a = 0};

/**
 * @brief How a run of text looks.
 *
 * Every field is optional: an unset field means "inherit it", so styles can
 * be layered with @ref merge, from the ambient default style down to the
 * style of one piece of text. A field still unset after every merge uses
 * the backend's default.
 */
struct TextStyle {
    std::optional<Color> fg;        ///< Text colour.
    std::optional<Color> bg;        ///< Colour behind the text.
    std::optional<double> size;     ///< Font size in logical pixels; ignored by the terminal backend.
    std::optional<bool> bold;       ///< Bold weight.
    std::optional<bool> underline;  ///< Underlined text.
    std::optional<int> font;        ///< Backend font id; unset means the backend's default font.

    /**
     * @brief Compares every field, including whether it is set.
     *
     * @param other The style to compare with.
     * @return @c true if each field is set in both styles with the same
     * value, or unset in both.
     */
    bool operator==(const TextStyle &other) const = default;

    /**
     * @brief Returns this style overridden by every field set in
     * @p overrides.
     *
     * A field set in @p overrides wins. A field unset in @p overrides keeps
     * this style's value, set or not. Neither style is modified.
     *
     * @param overrides The more specific style.
     * @return The merged style.
     *
     * @par Example
     * @code{.cpp}
     * TextStyle base{.fg = Color::WHITE, .size = 20};
     * TextStyle title = base.merge({.size = 32, .bold = true});
     * // title: fg WHITE, size 32, bold true
     * @endcode
     */
    [[nodiscard]] TextStyle merge(const TextStyle &overrides) const;
};

/**
 * @brief A piece of text and the style it is drawn with.
 *
 * The unit a backend measures and draws: one string, one style. The style
 * is the effective one, already merged with the ambient default.
 */
struct TextRun {
    std::string text; ///< The text, in UTF-8.
    TextStyle style;  ///< How @ref text is drawn.

    /**
     * @brief Compares the text and the style.
     *
     * @param other The run to compare with.
     * @return @c true if both the text and the style are equal.
     */
    bool operator==(const TextRun &other) const = default;
};
}

#endif //R_TYPE_STYLES_HPP
