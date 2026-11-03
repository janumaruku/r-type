/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ConnectingScene
*/

#include "ConnectingScene.hpp"

#include <memory>

#include "ServerLink.tpp"
#include "engine/core/Errors.hpp"
#include "rtype/protocol/Decode.hpp"

namespace rtype::client {
ConnectingScene::ConnectingScene()
{
    registerCommands();
}

void ConnectingScene::registerCommands() noexcept
{
    _commands.registerCreator<WelcomeCommand>(protocol::MessageType::WELCOME);
    _commands.registerCreator<RejectCommand>(protocol::MessageType::REJECT);
}

void ConnectingScene::onEnter(FrameContext &context)
{
    try {
        _link = std::make_unique<ServerLink>();
        _link->server = resolveServer(context.config.host,
            context.config.port);
        _link->socket.start();
        _link->send(protocol::MessageType::HELLO,
            protocol::Hello{.playerName = context.config.name});
    } catch (const engine::core::NetworkError &error) {
        _startError = error.what();
    }
}

void ConnectingScene::onWelcome(const protocol::Welcome &welcome)
{
    if (!_welcome)
        _welcome = welcome;
}

void ConnectingScene::onReject(protocol::RejectReason reason)
{
    _rejected = reason;
}

bool WelcomeCommand::execute(ConnectingScene &scene, net::Packet &packet)
{
    scene.onWelcome(protocol::decode<protocol::Welcome>(packet));
    return true;
}

bool WelcomeCommand::operator()(ConnectingScene &scene, net::Packet &packet)
{
    return execute(scene, packet);
}

std::unique_ptr<IConnectCommand> WelcomeCommand::create()
{
    return std::make_unique<WelcomeCommand>();
}

bool RejectCommand::execute(ConnectingScene &scene, net::Packet &packet)
{
    scene.onReject(protocol::decode<protocol::Reject>(packet).reason);
    return true;
}

bool RejectCommand::operator()(ConnectingScene &scene, net::Packet &packet)
{
    return execute(scene, packet);
}

std::unique_ptr<IConnectCommand> RejectCommand::create()
{
    return std::make_unique<RejectCommand>();
}
}
