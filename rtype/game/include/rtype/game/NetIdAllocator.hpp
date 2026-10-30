/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** NetIdAllocator
*/

#ifndef R_TYPE_NET_ID_ALLOCATOR_HPP
#define R_TYPE_NET_ID_ALLOCATOR_HPP

#include <cstddef>
#include <optional>
#include <unordered_map>

#include "engine/ecs/Entity.hpp"
#include "rtype/game/Types.hpp"

namespace engine::ecs {
class Registry;
}

namespace rtype::game {
class NetIdAllocator {
public:
    [[nodiscard]] NetId allocate(engine::ecs::Entity entity);

    [[nodiscard]] std::optional<engine::ecs::Entity> find(NetId netId,
        const engine::ecs::Registry &registry) const;

    void releaseDead(const engine::ecs::Registry &registry);

    [[nodiscard]] std::size_t size() const noexcept;

private:
    NetId _next = 1;
    std::unordered_map<NetId, engine::ecs::Entity> _byNetId;
};
}

#endif //R_TYPE_NET_ID_ALLOCATOR_HPP
