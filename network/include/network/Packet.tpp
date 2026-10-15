/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Packet.tpp
*/

#ifndef PACKET_TPP
#define PACKET_TPP

#include "network/Packet.hpp"

namespace net {

template <Serializable T>
Packet &Packet::write(const T &value)
{
    if constexpr (Primitive<T>)
        writeRaw(&value, sizeof(T));
    else
        serialize(*this, value);
    return *this;
}

template <Serializable T>
[[nodiscard]] T Packet::read()
{
    T value{};

    if constexpr (Primitive<T>)
        readRaw(&value, sizeof(T));
    else
        deserialize(*this, value);
    return value;
}

template <Serializable T>
Packet &Packet::operator<<(const T &value)
{
    return { write(value) };
}

template <Serializable T>
Packet &Packet::operator>>(T &value)
{
    value = read<T>();
    return *this;
}

} // namespace net

#endif
