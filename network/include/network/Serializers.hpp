/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Serializers.hpp
*/

#ifndef SERIALIZERS_HPP
#define SERIALIZERS_HPP

#include <array>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

namespace net {

class Packet;

template <typename T>
concept Primitive =
    (std::is_arithmetic_v<T> || std::is_enum_v<T>) && !std::is_same_v<T, bool>;

void serialize(Packet &packet, const std::string &value);

void deserialize(Packet &packet, std::string &value);

// A type's serialize/deserialize go in the same namespace as the type, in the
// same header, right after it: they are found by argument-dependent lookup, and
// a Serializable<T> check made before they are declared stays false for T.
// Primitive types, enums included, never reach serialize/deserialize, so enum
// values are validated in the deserialize of the message that contains them.
// See docs/modules/network.md, "Making your own type serializable".
template <typename T>
concept Serializable =
    Primitive<T> || requires(Packet &packet, const T &in, T &out) {
        serialize(packet, in);
        deserialize(packet, out);
    };

template <typename T>
    requires Serializable<T>
void serialize(Packet &packet, const std::vector<T> &values);

template <typename T>
    requires Serializable<T>
void deserialize(Packet &packet, std::vector<T> &values);

template <typename T, std::size_t N>
    requires Serializable<T>
void serialize(Packet &packet, const std::array<T, N> &values);

template <typename T, std::size_t N>
    requires Serializable<T>
void deserialize(Packet &packet, std::array<T, N> &values);

} // namespace net
#endif
