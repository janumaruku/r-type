//
// Created by janumaruku on 11/5/26.
//

#include "GameScene.hpp"

#include <chrono>
#include <memory>
#include <utility>

#include "ClientStages.hpp"
#include "InterpolationSystem.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/render/RenderQueue.hpp"

namespace rtype::client {
namespace {
constexpr auto SERVER_TIMEOUT = std::chrono::seconds(5);

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

/*
** TODO(RTP-128): spawns the starfield and starts the looping stage1.ogg
** music.
*/
void GameScene::onEnter([[maybe_unused]] FrameContext &context)
{
}

/*
** The 5 steps of client.md § Frame loop: quit, receive, send the inputs at
** 60 Hz, check the connection, then run the UPDATE stage.
** TODO(RTP-127): _prediction.record(_inputSequence, buttons, context.now)
** after each sendInput.
** TODO(RTP-126): the local ship's step after each sendInput (subtask 2).
*/
SceneRequest GameScene::update(FrameContext &context, float dt)
{
    if (context.actions.wasTriggered(Action::QUIT, context.input)) {
        _leftByChoice = true;
        return SceneRequest::quit();
    }
    receive(context.now);
    advanceRenderClock(dt);
    const int steps = _inputStep.advance(dt);

    for (int step = 0; step < steps; ++step)
        _adapter.sendInput(++_inputSequence,
            sampleButtons(context.actions, context.input));
    if (_adapter.leaveCause() != LeaveCause::NO_CAUSE)
        return leaveFor(_adapter.leaveCause());
    if (context.now - _link->lastHeard > SERVER_TIMEOUT)
        return SceneRequest::change(std::make_unique<MessageScene>(
            "Connection lost", "The server stopped answering", true));
    _world.registry().getResource<engine::ecs::DeltaTime>().seconds = dt;
    _world.registry().runStage(ClientStage::UPDATE);
    return SceneRequest::stay();
}

void GameScene::receive(TimePoint now)
{
    for (const auto &datagram: _link->socket.receiveAll())
        _adapter.handle(datagram, now);
    _events.dispatch();
}

/*
** A newer server tick restarts the time since it, otherwise that time grows
** by dt, so renderTick() keeps moving between two snapshots.
*/
void GameScene::advanceRenderClock(float dt)
{
    auto &clock = _world.registry().getResource<RenderClock>();
    const std::uint32_t latest = _world.latestServerTick();

    if (latest > clock.latestServerTick) {
        clock.latestServerTick = latest;
        clock.sinceLatestTick = 0.0F;
    } else {
        clock.sinceLatestTick += dt;
    }
}
}
