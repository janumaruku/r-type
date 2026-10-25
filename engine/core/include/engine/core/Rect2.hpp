/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Rect2.hpp
*/

#ifndef RECT2_HPP
#define RECT2_HPP

#include <cstdint>
#include <initializer_list>
#include "engine/core/Vec2.hpp"

namespace engine::core {

template<typename T>
class Rect2 {
public:
    Rect2() = default;
    Rect2(const Vec2<T> &origin, const T &w, const T &h) noexcept;

    [[nodiscard]] const Vec2<T> &origin() const noexcept;
    [[nodiscard]] const T &width() const noexcept;
    [[nodiscard]] const T &height() const noexcept;

    [[nodiscard]] bool contains(const Vec2<T> &point) const noexcept;
    [[nodiscard]] bool intersects(const Rect2<T> &other) const noexcept;

    [[nodiscard]] Vec2<T> center() const;
    [[nodiscard]] static Rect2 fromCenter(const Vec2<T> &center,
        const Vec2<T> &halfsize);

private:
    Vec2<T> _origin;
    T _width;
    T _height;
};

using FloatRect = Rect2<float>;
using IntRect = Rect2<std::int32_t>;
using UintRect = Rect2<std::uint32_t>;

}

#endif
#include "engine/core/Rect2.tpp"
