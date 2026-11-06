/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PredictionSystem
*/

#include "PredictionSystem.hpp"

#include "ClientComponents.hpp"
#include "PredictionState.hpp"
#include "rtype/game/Components.hpp"

namespace rtype::client {
namespace {
constexpr float SETTLED_DISTANCE = 0.5F;
}

void predictionSystem(engine::ecs::Registry &registry)
{
    auto &correction = registry.getResource<Correction>();
    const auto ships = registry.getComponents<LocalPlayer>().entities();

    if (!correction.target || ships.empty())
        return;
    auto *transform = registry.getComponent<game::Transform>(ships.front());

    if (transform == nullptr)
        return;
    transform->position = PredictionState::smooth(transform->position,
        *correction.target);
    if ((*correction.target - transform->position).length()
        < SETTLED_DISTANCE) {
        transform->position = *correction.target;
        correction.target.reset();
    }
}
}
