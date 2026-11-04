/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** BoxConstraints
*/

#ifndef R_TYPE_BOX_CONSTRAINTS_HPP
#define R_TYPE_BOX_CONSTRAINTS_HPP

#include "Dimensions.hpp"

namespace kollection::core {

/**
 * @brief The range of sizes a parent allows its child, on each axis.
 *
 * Layout works in three steps: a parent passes constraints down to its
 * child, the child picks a size inside them and returns it, then the parent
 * places the child. The child never chooses a size outside the range; it
 * uses @ref constrain to clamp the size it would like.
 *
 * Three common shapes:
 * @li tight, when the minimum equals the maximum: the child must be exactly
 * that size (see @ref tight and @ref isTight);
 * @li loose, when the minimum is 0: the child may be anything up to the
 * maximum (see @ref loose and @ref loosen);
 * @li unbounded, when the maximum is @ref INF, so the child may be as big as
 * it wants on that axis, as along the main axis of a column.
 *
 * @invariant @ref minWidth <= @ref maxWidth and @ref minHeight <=
 * @ref maxHeight. Every function keeps it, and @ref constrain relies on it.
 * The only exception is @ref expand, which must go through @ref enforce
 * before it is used.
 *
 * @par Example
 * @code{.cpp}
 * BoxConstraints screen = BoxConstraints::loose({1280, 720});
 * BoxConstraints inner = screen.deflate(EdgeInsets::all(16));
 * Size text = inner.constrain({2000, 24}); // {1248, 24}
 * @endcode
 */
struct BoxConstraints {
    double minWidth = 0;    ///< Smallest width allowed.
    double maxWidth = INF;  ///< Largest width allowed, @ref INF for no limit.
    double minHeight = 0;   ///< Smallest height allowed.
    double maxHeight = INF; ///< Largest height allowed, @ref INF for no limit.

    /**
     * @brief Builds constraints that allow exactly one size.
     *
     * @param size The only size allowed.
     * @return Constraints whose minimum and maximum both equal @p size.
     *
     * @par Example
     * @code{.cpp}
     * BoxConstraints::tight({100, 50}); // width 100..100, height 50..50
     * @endcode
     */
    static BoxConstraints tight(Size size);

    /**
     * @brief Builds constraints that allow any size up to a maximum.
     *
     * The pipeline gives these to the root, so the UI takes the size it
     * needs, up to the size of the surface.
     *
     * @param size The largest size allowed.
     * @return Constraints from 0 to @p size on each axis.
     *
     * @par Example
     * @code{.cpp}
     * BoxConstraints::loose({1280, 720}); // width 0..1280, height 0..720
     * @endcode
     */
    static BoxConstraints loose(Size size);

    /**
     * @brief Builds constraints that mean "as big as the parent allows".
     *
     * Every bound is @ref INF, which no size can satisfy on its own. They
     * only make sense once clamped into the parent's constraints with
     * @ref enforce, which turns them into the parent's maximum.
     *
     * @return Constraints with every bound at @ref INF.
     *
     * @warning Never pass the result to @ref constrain or to a child
     * directly: call @ref enforce first.
     *
     * @par Example
     * @code{.cpp}
     * BoxConstraints parent = BoxConstraints::loose({100, 50});
     * BoxConstraints::expand().enforce(parent); // tight 100 x 50
     * @endcode
     */
    static BoxConstraints expand();

    /**
     * @brief Clamps a size into these constraints.
     *
     * Each axis is clamped on its own, between its minimum and maximum. A
     * render object ends its layout with this, so the size it would like
     * becomes a size its parent accepts.
     *
     * @param size The size wished for.
     * @return The closest size these constraints allow.
     *
     * @pre The invariant holds: never call it on @ref expand directly.
     *
     * @par Example
     * @code{.cpp}
     * BoxConstraints box{.maxWidth = 100, .maxHeight = 50};
     * box.constrain({140, 20}); // {100, 20}
     * @endcode
     */
    [[nodiscard]] Size constrain(Size size) const;

    /**
     * @brief Returns the smallest size these constraints allow.
     *
     * A box with nothing to show takes this size.
     *
     * @return {@ref minWidth, @ref minHeight}.
     */
    [[nodiscard]] Size smallest() const;

    /**
     * @brief Checks whether only one size is allowed.
     *
     * When the constraints are tight, the child's size is known before it is
     * laid out.
     *
     * @retval true  The minimum equals the maximum on both axes.
     * @retval false At least one axis leaves a choice.
     */
    [[nodiscard]] bool isTight() const;

    /**
     * @brief Checks whether the width has a finite maximum.
     *
     * A box that wants to fill the available width must check this first:
     * filling an unbounded width would give it an infinite size.
     *
     * @retval true  @ref maxWidth is finite.
     * @retval false @ref maxWidth is @ref INF.
     */
    [[nodiscard]] bool hasBoundedWidth() const;

    /**
     * @brief Checks whether the height has a finite maximum.
     *
     * @retval true  @ref maxHeight is finite.
     * @retval false @ref maxHeight is @ref INF.
     *
     * @see hasBoundedWidth
     */
    [[nodiscard]] bool hasBoundedHeight() const;

    /**
     * @brief Returns the same maxima, with the minima removed.
     *
     * Lets a child be smaller than its parent: a box that aligns its child
     * may be forced to a size, but its child doesn't have to fill it.
     *
     * @return Constraints from 0 to the same maxima.
     *
     * @par Example
     * @code{.cpp}
     * BoxConstraints::tight({400, 300}).loosen(); // 0..400 x 0..300
     * @endcode
     */
    [[nodiscard]] BoxConstraints loosen() const;

    /**
     * @brief Returns the constraints left for a child once padding is taken
     * out.
     *
     * @ref EdgeInsets::horizontal is subtracted from both width bounds and
     * @ref EdgeInsets::vertical from both height bounds. No bound goes below
     * 0, the minima never exceed the new maxima, and @ref INF stays
     * @ref INF.
     *
     * @param insets The padding around the child.
     * @return The constraints for the padded child.
     *
     * @par Example
     * @code{.cpp}
     * BoxConstraints::loose({20, 5}).deflate(EdgeInsets::all(1));
     * // 0..18 x 0..3
     * BoxConstraints::tight({20, 20}).deflate(EdgeInsets::all(15));
     * // 0..0 x 0..0: the padding is larger than the box
     * @endcode
     */
    [[nodiscard]] BoxConstraints deflate(const EdgeInsets &insets) const;

    /**
     * @brief Clamps these constraints into a parent's constraints.
     *
     * Each bound is clamped into the parent's range for its axis, so the
     * result always satisfies @p parent, whatever these constraints asked
     * for. A box with a size of its own uses this: it asks for its size, and
     * its parent has the last word.
     *
     * @param parent The constraints the result must fit in.
     * @return These constraints, narrowed to @p parent.
     *
     * @par Example
     * @code{.cpp}
     * BoxConstraints parent{.maxWidth = 150, .maxHeight = 150};
     * BoxConstraints::tight({200, 200}).enforce(parent); // tight 150 x 150
     * BoxConstraints::expand().enforce(parent);          // tight 150 x 150
     * @endcode
     */
    [[nodiscard]] BoxConstraints enforce(const BoxConstraints &parent) const;

    /**
     * @brief Compares the four bounds exactly.
     *
     * @param other The constraints to compare with.
     * @return @c true if every bound is equal.
     */
    bool operator==(const BoxConstraints &other) const = default;
};
}

#endif //R_TYPE_BOX_CONSTRAINTS_HPP
