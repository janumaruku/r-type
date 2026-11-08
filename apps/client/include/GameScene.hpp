/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameScene
*/

#ifndef R_TYPE_GAME_SCENE_HPP
#define R_TYPE_GAME_SCENE_HPP

#include "AudioReactor.hpp"
#include "ClientNetworkAdapter.hpp"
#include "EffectsReactor.hpp"
#include "PredictionState.hpp"
#include "Scenes.hpp"
#include "ServerLink.hpp"
#include "SoundBank.hpp"
#include "engine/runtime/FixedTimestep.hpp"
#include "rtype/protocol/Messages.hpp"

namespace rtype::client {
class GameScene: public IScene {
public:
    GameScene(std::unique_ptr<ServerLink> link, protocol::Welcome welcome,
        const FrameContext &context);

    void onEnter(FrameContext &context) override;

    void onExit(FrameContext &context) override;

    SceneRequest update(FrameContext &context, float dt) override;

    void draw(engine::render::RenderQueue &queue,
        const FrameContext &context) override;

    [[nodiscard]] std::uint8_t playerId() const noexcept;
private:
    [[nodiscard]] SceneRequest leaveFor(LeaveCause cause) const;
    void receive(TimePoint now);
    void advanceRenderClock(float dt);
    void stepLocalShip(std::uint8_t buttons);
    void reconcileLocalShip(TimePoint now);
    void setHudResources(const FrameContext &context);
    void updateHudState(const FrameContext &context);
    void drawWaiting(engine::render::RenderQueue &queue,
        const FrameContext &context) const;

    std::unique_ptr<ServerLink> _link;
    std::uint8_t _playerId;
    GameEventBus _events;
    ClientWorld _world;
    ClientNetworkAdapter _adapter;
    SoundBank _sounds;
    AudioReactor _audioReactor;
    EffectsReactor _effectsReactor;
    PredictionState _prediction;
    engine::runtime::FixedTimestep _inputStep{1.0 / 60.0, 15};
    std::uint32_t _inputSequence = 0;
    game::Rules _rules;
    std::uint8_t _playersToStart;
    bool _leftByChoice = false;
    bool _serverGone = false;
    std::uint32_t _reconciledTick = 0;
};
}

#endif //R_TYPE_GAME_SCENE_HPP
