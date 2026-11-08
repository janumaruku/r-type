/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RenderSystems.hpp
*/

#ifndef RENDER_SYSTEMS_HPP
#define RENDER_SYSTEMS_HPP

#include <string_view>

#include "ClientSettings.hpp"
#include "EffectsReactor.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/CommandBuffer.hpp"
#include "engine/ecs/IndexedView.hpp"
#include "PresentationComponents.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/protocol/Snapshot.hpp"
#include "SpriteCatalog.hpp"
#include "Starfield.hpp"

namespace rtype::client {

[[nodiscard]] std::int16_t layerFor(protocol::EntityKind kind) noexcept;

bool setAnimationState(Animation &animation, const SpriteDef &def,
    std::string_view name) noexcept;

void animationSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Animation> &animations,
    engine::ecs::SparseSet<Sprite> &sprites);

void shipTiltSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<ShipTilt> &tilts,
    const engine::ecs::SparseSet<game::Transform> &transforms,
    engine::ecs::SparseSet<Animation> &animations);

void renderSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<game::Transform> &transforms,
    const engine::ecs::SparseSet<Sprite> &sprites);

void effectSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<TransientEffect> &effects);

void spawnStarfield(engine::ecs::Registry &registry, engine::core::Rng &rng,
    engine::render::TextureId whitePixel);

void starfieldSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<game::Transform> &transforms,
    const engine::ecs::SparseSet<Star> &stars);
}

#endif /* !RENDER_SYSTEMS_HPP_ */
