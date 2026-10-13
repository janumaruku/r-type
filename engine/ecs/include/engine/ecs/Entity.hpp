/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** entity
*/

#ifndef R_TYPE_ENTITY_HPP
#define R_TYPE_ENTITY_HPP
#include <cstdint>
#include <type_traits>

namespace engine::ecs {

class Entity {
public:
    [[nodiscard]] std::uint32_t index() const noexcept;

    [[nodiscard]] std::uint32_t generation() const noexcept;

    bool operator==(const Entity &rhs) const noexcept;

    static const Entity NONE;

private:
    std::uint32_t _index;
    std::uint32_t _generation;

    Entity(std::uint32_t index, std::uint32_t generation) noexcept;
};
} // namespace engine::ecs

static_assert(sizeof(engine::ecs::Entity) == 8);
static_assert(std::is_trivially_copyable_v<engine::ecs::Entity>);

#endif // R_TYPE_ENTITY_HPP
