/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PaintCommands
*/

#ifndef R_TYPE_PAINT_COMMANDS_HPP
#define R_TYPE_PAINT_COMMANDS_HPP

#include <variant>
#include <vector>

#include "kollection/core/Dimensions.hpp"
#include "kollection/core/Styles.hpp"

/**
 * @namespace kollection::paint
 * @brief The drawing vocabulary between the framework and the backends.
 *
 * Render objects never draw directly. They describe what to draw as a list
 * of small commands, and the backend turns that list into real drawing
 * calls (Raylib, terminal or SDL). The framework only depends on this
 * namespace, never on a backend.
 */
namespace kollection::paint {

/**
 * @brief Fills a rectangle with a solid color.
 *
 * In the Raylib backend this becomes a filled @c RectCmd.
 */
struct FillRect {
    core::Rect rect; ///< The area to fill, in logical units.
    core::Color color; ///< The fill color.
};

/**
 * @brief Draws the outline of a rectangle, without filling it.
 *
 * In the Raylib backend this becomes a @c RectCmd with an outline only.
 */
struct StrokeRect {
    core::Rect rect; ///< The rectangle whose border is drawn, in logical units.
    core::Color color; ///< The border color.
    double width = 0.0; ///< The border thickness, in logical units.
};

/**
 * @brief Draws one run of text in a single style.
 *
 * In the Raylib backend this becomes a @c TextCmd.
 */
struct DrawText {
    core::Offset at; ///< The top-left corner of the text, in logical units.
    core::TextRun run; ///< The text and the style to draw it with.
};

/**
 * @brief Any single drawing command.
 *
 * Each command only carries its own fields, and the backend picks the
 * drawing call with @c std::visit:
 * @li @ref FillRect: a filled rectangle.
 * @li @ref StrokeRect: a rectangle outline.
 * @li @ref DrawText: a run of text.
 */
using PaintCommand = std::variant<FillRect, StrokeRect, DrawText>;

/**
 * @brief The commands of one frame, in drawing order.
 *
 * Commands are drawn first to last, so a later command is drawn on top of
 * an earlier one. A parent paints itself before its children, so children
 * appear above it.
 */
using PaintList = std::vector<PaintCommand>;

/**
 * @brief Adds drawing commands to a @ref PaintList.
 *
 * The pipeline creates one context per frame and passes it to the
 * @c paint() method of every render object. Each call appends exactly one
 * command at the end of the list.
 *
 * @warning The context keeps a reference to the list, so the list must
 * outlive it.
 *
 * @par Example
 * @code{.cpp}
 * PaintList list;
 * PaintContext context{list};
 * context.fillRect({0, 0, 200, 40}, core::Color::BLUE);
 * context.drawText({8, 8}, {.text = "Play"});
 * // list now holds a FillRect followed by a DrawText.
 * @endcode
 */
class PaintContext {
public:
    /**
     * @brief Creates a context that appends to @p list.
     *
     * @param list The list to append the commands to. It is not cleared.
     */
    explicit PaintContext(PaintList &list);

    /**
     * @brief Appends a @ref FillRect.
     *
     * @param rect The area to fill, in logical units.
     * @param color The fill color.
     */
    void fillRect(const core::Rect &rect, const core::Color &color);

    /**
     * @brief Appends a @ref StrokeRect.
     *
     * @param rect The rectangle whose border is drawn, in logical units.
     * @param color The border color.
     * @param width The border thickness, in logical units.
     */
    void strokeRect(const core::Rect &rect, const core::Color &color,
        double width);

    /**
     * @brief Appends a @ref DrawText.
     *
     * @param at The top-left corner of the text, in logical units.
     * @param run The text and the style to draw it with. It is copied.
     */
    void drawText(const core::Offset &at, const core::TextRun &run);

private:
    PaintList &_list; ///< The list the commands are appended to.
};
}

#endif //R_TYPE_PAINT_COMMANDS_HPP
