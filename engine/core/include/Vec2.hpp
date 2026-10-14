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

    const T &x() const noexcept;
    const T &y() const noexcept;

    T length() const noexcept;
    T lengthSquared() const noexcept;
    Vec2<T> normalized() const noexcept;
    
    Vec2<T>operator+(const Vec2 &other) const noexcept;
    Vec2<T>operator-(const Vec2 &other) const noexcept;

    Vec2<T>operator*(const T &scalar) const noexcept;
    Vec2<T>operator/(const T &scalar) const noexcept;

    void operator+=(const Vec2 &other) noexcept;
    void operator-=(const Vec2 &other) noexcept;

    bool operator==(const Vec2 &other) const noexcept;
    bool operator!=(const Vec2 &other) const noexcept;

private:
    T _x;
    T _y;
};


using Vec2f = Vec2<float>;
using Vec2i = Vec2<std::int32_t>;
using Vec2u = Vec2<std::uint32_t>;

}

#endif

#include "Vec2.tpp"
