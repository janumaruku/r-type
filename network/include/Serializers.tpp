/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Serializers.tpp
*/

#ifndef SERIALIZERS_TPP
#define SERIALIZERS_TPP

#include <cstdint>
#include <limits>
#include <string>

namespace net {

inline void serialize(Packet &packet, const std::string &in)
{
    if (in.size() > std::numeric_limits<std::uint8_t>::max())
        throw PacketError("string: size larger than uint8_t");
    packet.write<std::uint8_t>(static_cast<std::uint8_t>(in.size()));
    packet.writeRaw(in.c_str(), in.size());
}

inline void deserialize(Packet &packet, std::string &out)
{
    const auto count = packet.read<std::uint8_t>();
    if (count > packet.remaining())
        throw PacketError("string: count larger than the packet");
    out.resize(count);
    packet.readRaw(out.data(), count);
}

template <typename T>
    requires (!std::same_as<T, bool>)
void serialize(Packet &packet, const std::vector<T> &in)
{
    if (in.size() > std::numeric_limits<std::uint16_t>::max())
        throw PacketError("vector: size larger than uint16_t");
    packet.write<std::uint16_t>(static_cast<std::uint16_t>(in.size()));
    for (const auto& value : in)
        packet.write<T>(value);
}

template <typename T>
    requires (!std::same_as<T, bool>)
void deserialize(Packet &packet, std::vector<T> &out)
{
    const auto count = packet.read<std::uint16_t>();
    if (count > packet.remaining())
        throw PacketError("vector: count larger than the packet");
    out.clear();
    out.reserve(count);
    for (std::uint16_t i = 0; i < count; ++i)
        out.push_back(packet.read<T>());
}

template <typename T, std::size_t N>
    requires (!std::same_as<T, bool>)
void serialize(Packet &packet, const std::array<T, N> &in)
{
    for (const auto &value : in)
        packet.write<T>(value);
}

template <typename T, std::size_t N>
    requires (!std::same_as<T, bool>)
void deserialize(Packet &packet, std::array<T, N> &out)
{
    for (std::size_t i = 0; i < N; ++i)
        out[i] = packet.read<T>();
}

} // namespace net
#endif