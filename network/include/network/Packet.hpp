/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Packet.hpp
*/

#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>
#include <stdexcept>
#include <bit>

#include "network/Serializers.hpp"

namespace net {

class PacketError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Packet {
public:
    static constexpr std::size_t MAX_SIZE = 1200;

    Packet() = default;
    explicit Packet(std::span<const std::uint8_t> bytes);

    template <Serializable T>
    Packet &write(const T &value);

    template <Serializable T>
    [[nodiscard]] T read();

    template <Serializable T>
    Packet &operator<<(const T &value);

    template <Serializable T>
    Packet &operator>>(T &value);

    void writeRaw(const void *data, std::size_t size);
    void readRaw(void *data, std::size_t size);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::size_t remaining() const noexcept;
    [[nodiscard]] bool fullyRead() const noexcept;
    [[nodiscard]] std::span<const std::uint8_t> bytes() const noexcept;

private:
    std::vector<std::uint8_t> _buffer;
    std::size_t _offset = 0;
};

static_assert(std::endian::native == std::endian::little,
    "the wire format is little-endian");
static_assert(std::numeric_limits<float>::is_iec559, "floats must be IEEE-754");

}
#endif

#include "network/Packet.tpp"
#include "network/Serializers.tpp"
