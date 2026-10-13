/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** entity
*/

#ifndef R_TYPE_ENTITY_HPP
#define R_TYPE_ENTITY_HPP
#include <cstdint>

namespace engine::ecs {

class Entity {
public:
    [[nodiscard]] std::uint32_t index() const noexcept;

    [[nodiscard]] std::uint32_t generation() const noexcept;

    bool operator==(const Entity &rhs) const noexcept;

private:
    std::uint32_t _index;
    std::uint32_t _generation;

    Entity(std::uint32_t index, std::uint32_t generation);
};

} // namespace engine::ecs

#endif // R_TYPE_ENTITY_HPP
