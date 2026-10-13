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
#include <functional>

namespace engine::ecs {
class Registry;

namespace detail {
struct EntityAccess;
}

class Entity {
public:
    [[nodiscard]] constexpr std::uint32_t index() const noexcept
    {
        return _index;
    }

    [[nodiscard]] constexpr std::uint32_t generation() const noexcept
    {
        return _generation;
    }

    constexpr bool operator==(const Entity &rhs) const noexcept = default;

    static const Entity NONE;

private:
    std::uint32_t _index;
    std::uint32_t _generation;

    constexpr Entity(std::uint32_t index, std::uint32_t generation) noexcept:
        _index(index), _generation(generation)
    {
    }

    friend class Registry;
    friend struct detail::EntityAccess;
};

inline constexpr Entity Entity::NONE{0xFFFFFFFF, 0xFFFFFFFF};

namespace detail {
struct EntityAccess {
    [[nodiscard]] static constexpr Entity make(std::uint32_t index,
        std::uint32_t generation) noexcept
    {
        return Entity{index, generation};
    }
};
} // namespace detail
} // namespace engine::ecs

template <>
struct std::hash<engine::ecs::Entity> {
    std::size_t operator()(const engine::ecs::Entity &entity) const noexcept;
};

static_assert(sizeof(engine::ecs::Entity) == 8);
static_assert(std::is_trivially_copyable_v<engine::ecs::Entity>);

#endif // R_TYPE_ENTITY_HPP
