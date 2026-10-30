/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MotionSystems
*/

#include "rtype/game/MotionSystems.hpp"

#include <algorithm>

#include "engine/ecs/Resources.hpp"
#include "rtype/game/Motion.hpp"
#include "rtype/game/Rules.hpp"

namespace rtype::game {

void playerInputSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<PlayerShip> &,
    engine::ecs::SparseSet<Velocity> &,
    engine::ecs::SparseSet<Weapon> &)
{
    const auto &rules = registry.getResource<Rules>();
    const auto &delta = registry.getResource<engine::ecs::DeltaTime>();

    for (auto &&[ship, velocity, weapon]:
         registry.view<PlayerShip, Velocity, Weapon>()) {
        velocity.value = inputDirection(ship.buttons) * rules.playerSpeed;
        weapon.triggered = ship.buttons.held(InputButtons::FIRE);
        ship.invulnerableFor =
            std::max(0.0F, ship.invulnerableFor - delta.seconds);
    }
}
}
