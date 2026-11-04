/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Dimensions
*/

#ifndef R_TYPE_DIMENSIONS_HPP
#define R_TYPE_DIMENSIONS_HPP

#include <limits>

/**
 * @brief Value types shared by every Kollection layer.
 *
 * Geometry, colours, text styles, widget keys, constraints and input events.
 * Nothing here depends on another Kollection layer or on a backend, so the
 * widget, framework, paint and backend layers can all use these types.
 *
 * Every length is in logical units: one terminal cell for the terminal
 * backend, one logical pixel for the others. The origin is the top-left
 * corner, x grows to the right and y grows downwards.
 */
namespace kollection::core {

/**
 * @brief An unbounded length, for constraints with no maximum.
 */
inline constexpr double INF = std::numeric_limits<double>::infinity();

/**
 * @brief The width and height of a box, in logical units.
 *
 * A size has no position: @ref Offset gives one, and @ref Rect combines
 * both.
 */
struct Size {
    double width = 0;  ///< Horizontal extent, never negative after layout.
    double height = 0; ///< Vertical extent, never negative after layout.

    /**
     * @brief Compares both dimensions exactly.
     *
     * @param other The size to compare with.
     * @return @c true if the widths and the heights are equal.
     */
    bool operator==(const Size &other) const = default;
};

/**
 * @brief A position or a displacement, in logical units.
 *
 * Used both for a point (a pointer position, a paint origin) and for the
 * distance between two points (a child's offset inside its parent).
 */
struct Offset {
    double x = 0; ///< Horizontal component, positive to the right.
    double y = 0; ///< Vertical component, positive downwards.

    /**
     * @brief Compares both components exactly.
     *
     * @param other The offset to compare with.
     * @return @c true if both components are equal.
     */
    bool operator==(const Offset &other) const = default;

    /**
     * @brief Adds two offsets component by component.
     *
     * Moves a point by a displacement, or chains two displacements: a
     * child's origin is its parent's origin plus the child's offset.
     *
     * @param other The offset to add.
     * @return The sum of this offset and @p other.
     *
     * @par Example
     * @code{.cpp}
     * Offset parentOrigin{10, 20};
     * Offset childOffset{1, 1};
     * Offset childOrigin = parentOrigin + childOffset; // {11, 21}
     * @endcode
     */
    Offset operator+(Offset other) const;

    /**
     * @brief Subtracts two offsets component by component.
     *
     * Turns a point into coordinates local to another origin, as hit-testing
     * does when it moves from a parent into a child.
     *
     * @param other The offset to subtract.
     * @return This offset minus @p other.
     */
    Offset operator-(Offset other) const;
};

/**
 * @brief An axis-aligned rectangle, from its top-left corner.
 *
 * The left and top edges are inside the rectangle, the right and bottom
 * edges are outside (see @ref contains).
 */
struct Rect {
    double x = 0;      ///< Left edge.
    double y = 0;      ///< Top edge.
    double width = 0;  ///< Horizontal extent, expected to be positive or zero.
    double height = 0; ///< Vertical extent, expected to be positive or zero.

    /**
     * @brief Checks whether a point lies inside the rectangle.
     *
     * The left and top edges are inside and the right and bottom edges are
     * outside, so two rectangles that touch never both contain a point. A
     * rectangle with a zero width or height contains nothing.
     *
     * @param point The point, in the same space as the rectangle.
     * @retval true  @p point is inside.
     * @retval false @p point is outside, or on the right or bottom edge.
     *
     * @par Example
     * @code{.cpp}
     * Rect box{.x = 0, .y = 0, .width = 10, .height = 3};
     * box.contains({0, 0});  // true: top-left corner
     * box.contains({10, 1}); // false: on the right edge
     * @endcode
     */
    [[nodiscard]] bool contains(Offset point) const;
};

/**
 * @brief Space on each side of a box, in logical units.
 *
 * Used for padding (space inside a box, around its child) and margins
 * (space outside it).
 *
 * @see horizontal, vertical
 */
struct EdgeInsets {
    double left = 0;   ///< Space on the left side.
    double top = 0;    ///< Space above.
    double right = 0;  ///< Space on the right side.
    double bottom = 0; ///< Space below.

    /**
     * @brief Builds insets with the same value on all four sides.
     *
     * @param inset The space on each side.
     * @return Insets of @p inset on every side.
     */
    static EdgeInsets all(double inset);

    /**
     * @brief Builds insets with one value for both horizontal sides and one
     * for both vertical sides.
     *
     * @param horizontalInset The space on the left and on the right.
     * @param verticalInset The space above and below.
     * @return The insets.
     *
     * @par Example
     * @code{.cpp}
     * EdgeInsets button = EdgeInsets::symmetric(16, 8);
     * // left = right = 16, top = bottom = 8
     * @endcode
     */
    static EdgeInsets symmetric(double horizontalInset, double verticalInset);

    /**
     * @brief Returns the total horizontal space.
     *
     * @return @ref left + @ref right, the width a padded box adds to its
     * child.
     */
    [[nodiscard]] double horizontal() const;

    /**
     * @brief Returns the total vertical space.
     *
     * @return @ref top + @ref bottom, the height a padded box adds to its
     * child.
     */
    [[nodiscard]] double vertical() const;

    /**
     * @brief Compares the four sides exactly.
     *
     * @param other The insets to compare with.
     * @return @c true if every side is equal.
     */
    bool operator==(const EdgeInsets &other) const = default;
};

/**
 * @brief Where a child sits inside a larger parent.
 *
 * Each axis goes from -1 to 1: -1 is the start (left or top), 0 the centre
 * and 1 the end (right or bottom). Values outside that range place the child
 * partly outside the parent.
 *
 * Common positions:
 * @li top-left: {-1, -1}, see @ref topLeft
 * @li centre: {0, 0}, see @ref center
 * @li bottom-right: {1, 1}
 *
 * @see inscribe
 */
struct Alignment {
    double x = 0; ///< Horizontal position: -1 left, 0 centre, 1 right.
    double y = 0; ///< Vertical position: -1 top, 0 centre, 1 bottom.

    /**
     * @brief Returns the alignment that centres a child on both axes.
     *
     * @return The alignment {0, 0}.
     */
    static constexpr Alignment center()
    {
        return {
            .x = 0,
            .y = 0,
        };
    }

    /**
     * @brief Returns the alignment that puts a child in the top-left corner.
     *
     * @return The alignment {-1, -1}.
     */
    static constexpr Alignment topLeft()
    {
        return {
            .x = -1,
            .y = -1,
        };
    }

    /**
     * @brief Computes the offset that places a child inside a parent with
     * this alignment.
     *
     * The free space on each axis (parent minus child) is shared according to
     * the alignment: none of it before the child at -1, half at 0, all of it
     * at 1. A child larger than its parent gets a negative offset and
     * overflows on both sides when centred.
     *
     * @param child The size of the box to place.
     * @param parent The size of the box it is placed in.
     * @return The child's top-left corner, relative to the parent's.
     *
     * @par Example
     * @code{.cpp}
     * Alignment::center().inscribe({4, 2}, {10, 6}); // {3, 2}
     * Alignment::topLeft().inscribe({4, 2}, {10, 6}); // {0, 0}
     * @endcode
     */
    [[nodiscard]] Offset inscribe(Size child, Size parent) const;

    /**
     * @brief Compares both axes exactly.
     *
     * @param other The alignment to compare with.
     * @return @c true if both axes are equal.
     */
    bool operator==(const Alignment &other) const = default;
};
}

#endif //R_TYPE_DIMENSIONS_HPP
