/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ConnectingScene
*/

#include "ConnectingScene.hpp"

namespace rtype::client {
ConnectingScene::ConnectingScene()
{
    registerCommands();
}

void ConnectingScene::registerCommands() noexcept
{
    // _commands.registerCreator<WelcomeCommand>(protocol::MessageType::WELCOME);
    // _commands.registerCreator<RejectCommand>(protocol::MessageType::REJECT);
}
}
