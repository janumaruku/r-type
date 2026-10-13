#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace net {

class Packet;

class PacketError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template <typename T>
concept Primitive = (std::is_arithmetic_v<T> || std::is_enum_v<T>) &&
                    !std::is_same_v<T, bool>;

template <typename T>
concept Serializable =
    Primitive<T> || requires(Packet& packet, const T& input, T& output) {
        serialize(packet, input);
        deserialize(packet, output);
    };

class Packet {
public:
    static constexpr std::size_t MAX_SIZE = 1200;

    Packet() = default;
    explicit Packet(std::span<const std::uint8_t> bytes);

    template <Serializable T>
    Packet& write(const T& value)
    {
        if constexpr (Primitive<T>) {
            writeRaw(&value, sizeof(T));
        } else {
            serialize(*this, value);
        }
        return *this;
    }

    template <Serializable T>
    [[nodiscard]] T read()
    {
        T value{};
        if constexpr (Primitive<T>) {
            readRaw(&value, sizeof(T));
        } else {
            deserialize(*this, value);
        }
        return value;
    }

    template <Serializable T>
    Packet& operator<<(const T& value)
    {
        return write(value);
    }

    template <Serializable T>
    Packet& operator>>(T& value)
    {
        value = read<T>();
        return *this;
    }

    void writeRaw(const void* data, std::size_t size);
    void readRaw(void* data, std::size_t size);

    [[nodiscard]] std::size_t size() const noexcept { return _buffer.size(); }
    [[nodiscard]] std::size_t remaining() const noexcept
    {
        return _buffer.size() - _offset;
    }
    [[nodiscard]] bool fullyRead() const noexcept
    {
        return _offset == _buffer.size();
    }
    [[nodiscard]] std::span<const std::uint8_t> bytes() const noexcept
    {
        return _buffer;
    }

private:
    std::vector<std::uint8_t> _buffer;
    std::size_t _offset = 0;
};

inline void serialize(Packet& packet, const std::string& value)
{
    if (value.size() > std::numeric_limits<std::uint8_t>::max()) {
        throw PacketError("string is too large");
    }
    packet << static_cast<std::uint8_t>(value.size());
    packet.writeRaw(value.data(), value.size());
}

inline void deserialize(Packet& packet, std::string& value)
{
    const auto size = packet.read<std::uint8_t>();
    if (size > packet.remaining()) {
        throw PacketError("string exceeds packet bounds");
    }
    value.resize(size);
    packet.readRaw(value.data(), size);
}

template <Serializable T>
void serialize(Packet& packet, const std::vector<T>& values)
{
    if (values.size() > std::numeric_limits<std::uint16_t>::max()) {
        throw PacketError("vector is too large");
    }
    packet << static_cast<std::uint16_t>(values.size());
    for (const auto& value : values) {
        packet << value;
    }
}

template <Serializable T>
void deserialize(Packet& packet, std::vector<T>& values)
{
    const auto count = packet.read<std::uint16_t>();
    if (count > packet.remaining()) {
        throw PacketError("vector count exceeds packet bounds");
    }
    values.clear();
    values.reserve(count);
    for (std::uint16_t index = 0; index < count; ++index) {
        values.push_back(packet.read<T>());
    }
}

template <Serializable T, std::size_t Size>
void serialize(Packet& packet, const std::array<T, Size>& values)
{
    for (const auto& value : values) {
        packet << value;
    }
}

template <Serializable T, std::size_t Size>
void deserialize(Packet& packet, std::array<T, Size>& values)
{
    for (auto& value : values) {
        value = packet.read<T>();
    }
}

static_assert(std::endian::native == std::endian::little,
              "the wire format is little-endian");
static_assert(std::numeric_limits<float>::is_iec559,
              "floats must be IEEE-754");

}