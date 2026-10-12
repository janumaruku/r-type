/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Rect2.tpp
*/

#ifndef RECT2_TPP
#define RECT2_TPP

#include "Rect2.hpp"

namespace engine::core {

template<typename T>
Rect2<T>::Rect2(const Vec2<T> &origin, const T &w, const T &h) noexcept :
_origin(origin.x(), origin.y()), _width(w), _height(h)
{}

template<typename T>
const Vec2<T> &Rect2<T>::origin() const noexcept
{
    return _origin;
}

template<typename T>
const T &Rect2<T>::width() const noexcept
{
    return _width;
}

template<typename T>
const T &Rect2<T>::height() const noexcept
{
    return _height;
}


template<typename T>
bool Rect2<T>::contains(const Vec2<T> &point) const noexcept
{
    T px = point.x();
    T py = point.y();
    return (_origin.x() <= px && px < _origin.x() + _width) &&
    (_origin.y() <= py && py <= _origin.y() + _height);
}

template<typename T>
bool Rect2<T>::intersects(const Rect2<T>& other) const noexcept
{
    return _origin.x() < other._origin.x() + other._width &&
           _origin.x() + _width > other._origin.x() &&
           _origin.y() < other._origin.y() + other._height &&
           _origin.y() + _height > other._origin.y();
}

template <typename T>
Vec2<T> Rect2<T>::center() const
{
    return Vec2<T>{_origin.x() + (_width / T{2}), _origin.y() + (_height / T{2})};
}

template <typename T>
Rect2<T> Rect2<T>::fromCenter(const Vec2<T> &center, const Vec2<T> &halfsize)
{
    return Rect2<T>{Vec2<T>{center.x() - halfsize.x(), center.y() - halfsize.y()}, halfsize.x() * T{2}, halfsize.y() * T{2}};
}

}

#endif