/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameScene
*/

#include "GameScene.hpp"

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include "ClientStages.hpp"
#include "Hud.hpp"
#include "InterpolationSystem.hpp"
#include "PredictionSystem.hpp"
#include "RenderSystems.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/render/RenderQueue.hpp"

namespace rtype::client {
namespace {
constexpr auto SERVER_TIMEOUT = std::chrono::seconds(5);

[[nodiscard]] SceneRequest showMessage(std::string title, std::string detail)
{
    return SceneRequest::change(std::make_unique<MessageScene>(
        std::move(title), std::move(detail), true));
}

/*
** Adds the systems in client.md's order: hudSystem runs last in RENDER, so
** the HUD draws over the sprites.
*/
void addSystems(engine::ecs::Registry &registry)
{
    registry.addSystem<game::Transform, const NetSamples>(ClientStage::UPDATE,
        "interpolation", interpolationSystem);
    registry.addSystem<>(ClientStage::UPDATE, "prediction", predictionSystem);
    registry.addSystem<ShipTilt, const game::Transform, Animation>(
        ClientStage::UPDATE, "shipTilt", shipTiltSystem);
    registry.addSystem<Animation, Sprite>(ClientStage::UPDATE, "animation",
        animationSystem);
    registry.addSystem<TransientEffect>(ClientStage::UPDATE, "effects",
        effectSystem);
    registry.addSystem<game::Transform, const Star>(ClientStage::UPDATE,
        "starfield", starfieldSystem);
    registry.addSystem<const game::Transform, const Sprite>(
        ClientStage::RENDER, "render", renderSystem);
    registry.addSystem<>(ClientStage::RENDER, "hud", hudSystem);
}

void setResources(engine::ecs::Registry &registry)
{
    registry.emplaceResource<engine::render::RenderQueue>();
    registry.emplaceResource<ClientSettings>();
    registry.emplaceResource<RenderClock>();
    registry.emplaceResource<Correction>();
    registry.emplaceResource<engine::ecs::DeltaTime>();
    registry.emplaceResource<engine::core::Rng>(
        engine::core::Rng::randomSeed());
}
}

/*
** The reactors are declared after _events and _world, so they are destroyed
** first and unsubscribe while the bus still exists. The HudReactor lives in
** the registry, inside _world, which is also destroyed before _events.
** The ActionMap is copied once: the bindings don't change during a game.
*/
GameScene::GameScene(std::unique_ptr<ServerLink> link,
    protocol::Welcome welcome, const FrameContext &context):
    _link(std::move(link)), _playerId(welcome.playerId),
    _world(context.sprites), _adapter(*_link, _world, _events),
    _sounds(context.sounds),
    _audioReactor(_events, context.audio, _sounds, _world),
    _effectsReactor(_events, _world.registry(), context.settings)
{
    _world.setLocalPlayer(_playerId);
    setResources(_world.registry());
    setHudResources(context);
    addSystems(_world.registry());
}

void GameScene::onEnter(FrameContext &context)
{
    auto &registry = _world.registry();

    spawnStarfield(registry, registry.getResource<engine::core::Rng>(),
        context.whitePixel);
    context.audio.playMusic(_sounds.stage1, true);
}

/*
** The steps of client.md § Frame loop: quit, receive, send the inputs at
** 60 Hz, check the connection, update the HUD state, then run the UPDATE
** stage.
*/
SceneRequest GameScene::update(FrameContext &context, float dt)
{
    if (context.actions.wasTriggered(Action::QUIT, context.input)) {
        _leftByChoice = true;
        return SceneRequest::quit();
    }
    receive(context.now);
    reconcileLocalShip(context.now);
    advanceRenderClock(dt);
    const int steps = _inputStep.advance(dt);

    for (int step = 0; step < steps; ++step) {
        const std::uint8_t buttons = sampleButtons(context.actions,
            context.input);

        _adapter.sendInput(++_inputSequence, buttons);
        _prediction.record(_inputSequence, buttons, context.now);
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
    updateHudState(context);
    _world.registry().getResource<engine::ecs::DeltaTime>().seconds = dt;
    _world.registry().runStage(ClientStage::UPDATE);
    return SceneRequest::stay();
}

/*
** The settings are copied every frame, so a change to highContrast shows
** at once.
*/
void GameScene::draw(engine::render::RenderQueue &queue,
    const FrameContext &context)
{
    auto &stageQueue = _world.registry().getResource<
        engine::render::RenderQueue>();

    _world.registry().setResource(context.settings);
    std::swap(stageQueue, queue);
    _world.registry().runStage(ClientStage::RENDER);
    std::swap(stageQueue, queue);
}

/*
** Says goodbye unless the server already forgot us: after QUIT
** (_leftByChoice) or when the window closes (ClientApp's change(nullptr)),
** but not after SHUTDOWN, KICK or a timeout, which set _serverGone.
** The music stops in every case.
*/
void GameScene::onExit(FrameContext &context)
{
    context.audio.stopMusic();
    if (!_serverGone)
        _adapter.sendDisconnect();
}

std::uint8_t GameScene::playerId() const noexcept
{
    return _playerId;
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
    auto &correction = _world.registry().getResource<Correction>();
    const auto ship = _world.localShip();

    if (correction.target)
        correction.target = _prediction.step(*correction.target, buttons,
            _rules);
    if (!ship)
        return;
    auto *transform = _world.registry().getComponent<game::Transform>(*ship);

    if (transform != nullptr)
        transform->position = _prediction.step(transform->position, buttons,
            _rules);
}

void GameScene::reconcileLocalShip(TimePoint now)
{
    const std::uint32_t complete = _adapter.assembler().lastCompleteTick();
    const auto ship = _world.localShip();

    if (complete <= _reconciledTick || !ship)
        return;
    _reconciledTick = complete;
    const auto *net = _world.registry().getComponent<NetSamples>(*ship);

    if (net == nullptr)
        return;
    const auto sample = std::ranges::find(net->ring, complete,
        &NetSample::tick);

    if (sample == net->ring.end())
        return;
    _prediction.notePing(_world.lastProcessedInput(), now);
    _world.registry().getResource<Correction>().target = _prediction.reconcile(
        sample->position, _world.lastProcessedInput(), _rules);
}

void GameScene::receive(TimePoint now)
{
    for (const auto &datagram: _link->socket.receiveAll())
        _adapter.handle(datagram, now);
    _events.dispatch();
}

void GameScene::setHudResources(const FrameContext &context)
{
    auto &registry = _world.registry();

    registry.emplaceResource<HudReactor>(_events, _world);
    registry.emplaceResource<HudState>();
    registry.setResource(context.actions);
    registry.setResource(static_cast<const ClientWorld *>(&_world));
}

/*
** F1 (TOGGLE_HUD_HELP) flips the help, and the ping is the average over
** the last second, or nothing without a recent sample.
*/
void GameScene::updateHudState(const FrameContext &context)
{
    auto &state = _world.registry().getResource<HudState>();

    if (context.actions.wasTriggered(Action::TOGGLE_HUD_HELP, context.input))
        state.showHelp = !state.showHelp;
    state.ping = _prediction.averagePing(context.now);
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
