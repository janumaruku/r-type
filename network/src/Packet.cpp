#include "network/Packet.hpp"

#include <algorithm>

namespace net {

Packet::Packet(std::span<const std::uint8_t> bytes)
    : _buffer(bytes.begin(), bytes.end())
{
    if (bytes.size() > MAX_SIZE) {
        throw PacketError("packet exceeds MAX_SIZE");
    }
}

void Packet::writeRaw(const void* data, std::size_t size)
{
    if (size > MAX_SIZE - _buffer.size()) {
        throw PacketError("packet would exceed MAX_SIZE");
    }
    const auto* bytes = static_cast<const std::uint8_t*>(data);
    _buffer.insert(_buffer.end(), bytes, bytes + size);
}

void Packet::readRaw(void* data, std::size_t size)
{
    if (size > remaining()) {
        throw PacketError("not enough bytes in packet");
    }
    std::memcpy(data, _buffer.data() + _offset, size);
    _offset += size;
}

}