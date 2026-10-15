/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Packet.cpp
*/

#include "../include/network/Packet.hpp"
#include <cstddef>
#include <cstring>

namespace net {

Packet::Packet(std::span<const std::uint8_t> bytes)
{
    if (bytes.size() > MAX_SIZE)
        throw PacketError("Packet::writeRaw: packet would exceed MAX_SIZE");
    _buffer.assign(bytes.begin(), bytes.end());
    _offset = 0;
}

void Packet::writeRaw(const void* data, std::size_t size)
{
    if (size > MAX_SIZE - _buffer.size())
        throw PacketError("Packet::writeRaw: packet would exceed MAX_SIZE");
    const auto *bytes = static_cast<const std::uint8_t*>(data);
    _buffer.insert(_buffer.end(), bytes, bytes + size);
}

void Packet::readRaw(void *data, std::size_t size)
{
    if (size > remaining())
        throw PacketError("Packet::readRaw: not enough bytes in internal buffer");
    if (data == nullptr)
        throw PacketError("Packet::readRaw: cannot read from null");

    std::memcpy(data, _buffer.data() + _offset, size);
    _offset += size;      
}

[[nodiscard]] std::size_t Packet::size() const noexcept
{
    return _buffer.size();
}

[[nodiscard]] std::size_t Packet::remaining() const noexcept
{
    return _buffer.size() - _offset;
}

[[nodiscard]] bool Packet::fullyRead() const noexcept
{
    return _offset == _buffer.size();
}

[[nodiscard]] std::span<const std::uint8_t> Packet::bytes() const noexcept
{
    return {_buffer.data(), _buffer.size()};
}

}