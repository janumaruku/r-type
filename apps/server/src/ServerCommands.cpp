/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerCommands.cpp
*/


#include <memory>

#include "ServerCommands.hpp"
#include "ServerNetworkAdapter.hpp"
#include "Conversions.hpp"
#include "rtype/protocol/Decode.tpp"
#include "rtype/protocol/Messages.hpp"

namespace rtype::server {

bool HelloCommand::execute(ServerNetworkAdapter &adapter,
    IncomingMessage &message)
{
    const auto hello = protocol::decode<protocol::Hello>(message.packet);
    if (hello.protocolVersion != protocol::Header::VERSION) {
        adapter.reject(message.from, protocol::RejectReason::VERSION_MISMATCH,
            message.receivedAt);
        return true;
    }
    if (message.session != nullptr) {
        adapter.sendTo(*message.session, protocol::MessageType::WELCOME,
            protocol::Welcome{.playerId = message.session->playerId,
                .serverTick = adapter.world().tick()});
        return true;
    }

    const auto playerId = adapter.world().reserveSlot();
    if (!playerId.has_value()) {
        adapter.reject(message.from, protocol::RejectReason::SERVER_FULL,
            message.receivedAt);
        return true;
    }

    const auto name = protocol::sanitizeName(hello.playerName, *playerId);
    auto &session = adapter.sessions().add(message.from, *playerId, name,
        message.receivedAt);
    adapter.sendTo(session, protocol::MessageType::WELCOME,
        protocol::Welcome{.playerId = *playerId,
            .serverTick = adapter.world().tick()});
    adapter.world().apply(game::JoinPlayer{.playerId=*playerId, .name=name});
    return true;
}

bool HelloCommand::operator()(ServerNetworkAdapter &adapter,
    IncomingMessage &message)
{
    return execute(adapter, message);
}

std::unique_ptr<IServerCommand> HelloCommand::create()
{
    return std::make_unique<HelloCommand>();
}


bool InputCommand::execute(ServerNetworkAdapter &adapter,
    IncomingMessage &message)
{
    if (message.session == nullptr)
        return false;
    const auto input = protocol::decode<protocol::Input>(message.packet);
    if (input.inputSequence > message.session->lastProcessedInput)
        message.session->lastProcessedInput = input.inputSequence;
    adapter.world().apply(game::SetInput{message.session->playerId,
        input.inputSequence, toGame(input.buttons)});
    return true;
}

bool InputCommand::operator()(ServerNetworkAdapter &adapter,
    IncomingMessage &message)
{
    return execute(adapter, message);
}

std::unique_ptr<IServerCommand> InputCommand::create()
{
    return std::make_unique<InputCommand>();
}

bool DisconnectCommand::execute(ServerNetworkAdapter &adapter,
    IncomingMessage &message)
{
    if (message.session == nullptr)
        return false;
    (void)protocol::decode<protocol::Disconnect>(message.packet);
    adapter.removeClient(message.from, game::LeaveReason::QUIT);
    return true;
}

bool DisconnectCommand::operator()(ServerNetworkAdapter &adapter,
    IncomingMessage &message)
{
    return execute(adapter, message);
}

std::unique_ptr<IServerCommand> DisconnectCommand::create()
{
    return std::make_unique<DisconnectCommand>();
}

} // namespace rtype::server
