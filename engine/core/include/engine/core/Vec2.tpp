/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Vec2.cpp
*/


#ifndef VEC2_TPP
#define VEC2_TPP

#include <cmath>
#include "engine/core/Vec2.hpp"

namespace engine::core {

template<typename T>
Vec2<T>::Vec2(const T &x, const T &y) noexcept : _x(x), _y(y) {}

template<typename T>
const T &Vec2<T>::x() const noexcept
{
    return _x;
}

template<typename T>
const T &Vec2<T>::y() const noexcept
{
    return _y;
}

template<typename T>
T Vec2<T>::length() const noexcept
{
    return std::sqrt((_x * _x) + (_y * _y));
}

template<typename T>
Vec2<T> Vec2<T>::normalized() const noexcept
{
    if (*this == Vec2<T>{0, 0})
        return *this;
    T length = this->length();

    return Vec2<T>(_x / length, _y / length);
}


template<typename T>
Vec2<T> Vec2<T>::operator+(const Vec2<T> &other) const noexcept
{
    return {_x + other.x(), _y + other.y()};
}

template<typename T>
Vec2<T> Vec2<T>::operator-(const Vec2<T> &other) const noexcept
{
    return {_x - other.x(), _y - other.y()};
}

template<typename T>
Vec2<T> Vec2<T>::operator*(const T &scalar) const noexcept
{
    return {_x * scalar, _y * scalar};
}

template<typename T>
Vec2<T> Vec2<T>::operator/(const T &scalar) const noexcept
{
    return {_x / scalar, _y / scalar};
}

template<typename T>
void Vec2<T>::operator+=(const Vec2<T> &other) noexcept
{
    _x += other.x();
    _y += other.y();
}

template<typename T>
void Vec2<T>::operator-=(const Vec2<T> &other) noexcept
{
    _x -= other.x();
    _y -= other.y();
}

template<typename T>
bool Vec2<T>::operator==(const Vec2<T> &other) const noexcept
{
    return _x == other.x() && _y == other.y();
}

template<typename T>
bool Vec2<T>::operator!=(const Vec2<T> &other) const noexcept
{
    return !(*this == other);
}

}
#endif