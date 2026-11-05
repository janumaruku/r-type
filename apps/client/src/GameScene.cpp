//
// Created by janumaruku on 11/5/26.
//

#include "GameScene.hpp"

#include <utility>

#include "ClientStages.hpp"
#include "InterpolationSystem.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/render/RenderQueue.hpp"

namespace rtype::client {
namespace {
/*
** Adds the systems in client.md's order.
** TODO(RTP-127): predictionSystem after interpolationSystem.
** TODO(RTP-128): shipTiltSystem, animationSystem, effectSystem and
** starfieldSystem after it in UPDATE, then renderSystem and hudSystem in
** RENDER.
*/
void addSystems(engine::ecs::Registry &registry)
{
    registry.addSystem<game::Transform, const NetSamples>(ClientStage::UPDATE,
        "interpolation", interpolationSystem);
}

void setResources(engine::ecs::Registry &registry)
{
    registry.emplaceResource<engine::render::RenderQueue>();
    registry.emplaceResource<RenderClock>();
    registry.emplaceResource<engine::ecs::DeltaTime>();
    registry.emplaceResource<engine::core::Rng>(
        engine::core::Rng::randomSeed());
}
}

GameScene::GameScene(std::unique_ptr<ServerLink> link,
    protocol::Welcome welcome, const SpriteCatalog &sprites):
    _link(std::move(link)), _playerId(welcome.playerId),
    _world(sprites), _adapter(*_link, _world, _events)
{
    _world.setLocalPlayer(_playerId);
    setResources(_world.registry());
    addSystems(_world.registry());
}
}
