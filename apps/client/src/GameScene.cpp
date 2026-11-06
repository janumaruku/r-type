/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameScene
*/

#include "GameScene.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include "ClientStages.hpp"
#include "InterpolationSystem.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/render/RenderQueue.hpp"
#include "rtype/game/Motion.hpp"

namespace rtype::client {
namespace {
constexpr auto SERVER_TIMEOUT = std::chrono::seconds(5);

[[nodiscard]] SceneRequest showMessage(std::string title, std::string detail)
{
    return SceneRequest::change(std::make_unique<MessageScene>(
        std::move(title), std::move(detail), true));
}

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

    for (int step = 0; step < steps; ++step) {
        const std::uint8_t buttons = sampleButtons(context.actions,
            context.input);

        _adapter.sendInput(++_inputSequence, buttons);
        stepLocalShip(buttons);
    }
    if (_adapter.leaveCause() != LeaveCause::NO_CAUSE) {
        _serverGone = true;
        return leaveFor(_adapter.leaveCause());
    }
    if (context.now - _link->lastHeard > SERVER_TIMEOUT) {
        _serverGone = true;
        return showMessage("Connection lost", "The server stopped answering");
    }
    _world.registry().getResource<engine::ecs::DeltaTime>().seconds = dt;
    _world.registry().runStage(ClientStage::UPDATE);
    return SceneRequest::stay();
}

void GameScene::draw(engine::render::RenderQueue &queue,
    [[maybe_unused]] const FrameContext &context)
{
    auto &stageQueue = _world.registry().getResource<
        engine::render::RenderQueue>();

    std::swap(stageQueue, queue);
    _world.registry().runStage(ClientStage::RENDER);
    std::swap(stageQueue, queue);
}

/*
** Says goodbye unless the server already forgot us: after QUIT
** (_leftByChoice) or when the window closes (ClientApp's change(nullptr)),
** but not after SHUTDOWN, KICK or a timeout, which set _serverGone.
** TODO(RTP-128): stops the music.
*/
void GameScene::onExit([[maybe_unused]] FrameContext &context)
{
    if (!_serverGone)
        _adapter.sendDisconnect();
}

SceneRequest GameScene::leaveFor(LeaveCause cause) const
{
    switch (cause) {
    case LeaveCause::SHUTDOWN:
        return showMessage("Server closed", "The server stopped");
    case LeaveCause::KICKED_PROTOCOL:
        return showMessage("Kicked", "Too many invalid messages");
    case LeaveCause::KICKED_RATE:
        return showMessage("Kicked", "Too many messages");
    case LeaveCause::NO_CAUSE:
        break;
    }
    return SceneRequest::stay();
}

void GameScene::stepLocalShip(std::uint8_t buttons)
{
    const auto ship = _world.localShip();

    if (!ship)
        return;
    auto *transform = _world.registry().getComponent<game::Transform>(*ship);

    if (transform != nullptr)
        transform->position = game::movePlayer(transform->position,
            game::InputButtons{.bits = buttons},
            static_cast<float>(_inputStep.step()), _rules);
}

void GameScene::receive(TimePoint now)
{
    for (const auto &datagram: _link->socket.receiveAll())
        _adapter.handle(datagram, now);
    _events.dispatch();
}

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
