/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerCommands.hpp
*/

#ifndef SERVER_COMMANDS_HPP
#define SERVER_COMMANDS_HPP

#include <cstdint>
#include <memory>

#include "engine/core/FactoryTemplate.hpp"
#include "engine/core/ICommand.hpp"
#include "network/Packet.hpp"
#include "RateLimits.hpp"

namespace rtype::protocol {
enum class MessageType : std::uint8_t;
}

namespace rtype::server {

struct ClientSession;
class ServerNetworkAdapter;

struct IncomingMessage {
    asio::ip::udp::endpoint from;
    ClientSession *session;
    net::Packet &packet;
    TimePoint receivedAt;
};

using IServerCommand =
    designPattern::ICommand<ServerNetworkAdapter &, IncomingMessage &>;
using ServerCommandFactory =
    designPattern::FactoryTemplate<IServerCommand, protocol::MessageType>;

class HelloCommand: public IServerCommand {
public:
    static std::unique_ptr<IServerCommand> create();

    bool execute(ServerNetworkAdapter &adapter,
        IncomingMessage &message) override;

    bool operator()(ServerNetworkAdapter &adapter,
        IncomingMessage &message) override;
};

class InputCommand: public IServerCommand {
public:
    static std::unique_ptr<IServerCommand> create();

    bool execute(ServerNetworkAdapter &adapter,
        IncomingMessage &message) override;

    bool operator()(ServerNetworkAdapter &adapter,
        IncomingMessage &message) override;
};

class DisconnectCommand: public IServerCommand {
public:
    static std::unique_ptr<IServerCommand> create();

    bool execute(ServerNetworkAdapter &adapter,
        IncomingMessage &message) override;

    bool operator()(ServerNetworkAdapter &adapter,
        IncomingMessage &message) override;
};
} // namespace rtype::server
#endif
