/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Core module interface
*/

#ifndef VEC2_HPP
#define VEC2_HPP

#include <cstdint>
#include <initializer_list>
#include <type_traits>

namespace engine::core {

template<typename T>
class Vec2 {
public:
    Vec2() = default;
    Vec2(const T &x, const T &y) noexcept;

    [[nodiscard]] const T &x() const noexcept;
    [[nodiscard]] const T &y() const noexcept;

    [[nodiscard]] T length() const noexcept;
    [[nodiscard]] T lengthSquared() const noexcept;
    [[nodiscard]] Vec2<T> normalized() const noexcept;
    
    [[nodiscard]] Vec2<T>operator+(const Vec2 &other) const noexcept;
    [[nodiscard]] Vec2<T>operator-(const Vec2 &other) const noexcept;

    [[nodiscard]] Vec2<T>operator*(const T &scalar) const noexcept;
    [[nodiscard]] Vec2<T>operator/(const T &scalar) const noexcept;

    void operator+=(const Vec2 &other) noexcept;
    void operator-=(const Vec2 &other) noexcept;

    [[nodiscard]] bool operator==(const Vec2 &other) const noexcept;
    [[nodiscard]] bool operator!=(const Vec2 &other) const noexcept;

private:
    T _x;
    T _y;
};


using Vec2f = Vec2<float>;
using Vec2i = Vec2<std::int32_t>;
using Vec2u = Vec2<std::uint32_t>;

}

#endif

#include "engine/core/Vec2.tpp"
