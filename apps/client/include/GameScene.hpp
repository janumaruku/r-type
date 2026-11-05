/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameScene
*/

#ifndef R_TYPE_GAME_SCENE_HPP
#define R_TYPE_GAME_SCENE_HPP

#include "ClientNetworkAdapter.hpp"
#include "Scenes.hpp"
#include "ServerLink.hpp"
#include "engine/runtime/FixedTimestep.hpp"
#include "rtype/protocol/Messages.hpp"

namespace rtype::client {
class GameScene: public IScene {
public:
    GameScene(std::unique_ptr<ServerLink> link, protocol::Welcome welcome,
        const SpriteCatalog &sprites);

    // the 4 IScene functions
private:
    [[nodiscard]] SceneRequest leaveFor(LeaveCause cause) const;

    std::unique_ptr<ServerLink> _link;
    std::uint8_t _playerId;
    GameEventBus _events;
    ClientWorld _world;
    ClientNetworkAdapter _adapter;
    // PredictionState _prediction;
    // engine::runtime::FixedTimestep _inputStep{1.0 / 60.0};
    std::uint32_t _inputSequence = 0;
    game::Rules _rules;
    bool _leftByChoice = false;
    // story 4 adds the reactors and the HUD here, after _events
};
}

#endif //R_TYPE_GAME_SCENE_HPP
