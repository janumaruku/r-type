/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Builders
*/

#ifndef R_TYPE_BUILDERS_HPP
#define R_TYPE_BUILDERS_HPP

#include "rtype/game/Types.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"

namespace rtype::game {
NetId makePlayerShip(engine::ecs::Registry &registry,
    engine::ecs::Entity entity,
    std::uint8_t playerId);

NetId makeBydosSlave(engine::ecs::Registry &registry,
    engine::ecs::Entity entity,
    engine::core::Vec2f position, MovePattern pattern);

NetId makePlayerMissile(engine::ecs::Registry &registry,
    engine::ecs::Entity entity,
    NetId owner, std::uint8_t ownerPlayerId, engine::core::Vec2f position);

NetId makeEnemyMissile(engine::ecs::Registry &registry,
    engine::ecs::Entity entity,
    NetId owner, engine::core::Vec2f position, engine::core::Vec2f direction);
}

#endif //R_TYPE_BUILDERS_HPP
