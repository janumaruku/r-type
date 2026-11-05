/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameScene.cpp
*/

#include "GameScene.hpp"

namespace rtype::client {

        GameScene::GameScene(std::unique_ptr<ServerLink> link, protocol::Welcome welcome,
            const SpriteCatalog &sprites):
            // _link(),
            // _playerId(),
            // _events(),
            // _world(),
            // _adapter(),
            // _prediction(),
            // _inputStep(1.0 / 60.0),
            // _inputSequence(0),
            // _rules(),
            // _leftByChoice(false)
        {
        
        }
        // the 4 IScene functions




        void onEnter()
        {
        
        }

        void onExit()
        {
        
        }

        SceneRequest update(FrameContext& context, float dt)
        {
        
        }

        void draw(engine::render::RenderQueue&)
        {
            
        }



        SceneRequest GameScene::leaveFor(LeaveCause cause) const
        {
            if (cause == LeaveCause::SHUTDOWN) {
                return SceneRequest::change(std::make_unique<MessageScene>("Server closed", "The server stopped", true));
            } else if (cause == LeaveCause::KICKED_PROTOCOL) {
                return SceneRequest::change(std::make_unique<MessageScene>("Kicked", "Too many invalid messages", true));
            } else if (cause == LeaveCause::KICKED_RATE) {
                return SceneRequest::change(std::make_unique<MessageScene>("Kicked", "Too many messages", true));
            } else {
                return SceneRequest::change(std::make_unique<MessageScene>("Disconnected", "Connection closed", true));
            }
        }

} // namespace rtype::client
