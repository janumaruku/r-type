/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameCommands.hpp
*/

#ifndef GAME_COMMANDS_HPP
#define GAME_COMMANDS_HPP

#include "ClientNetworkAdapter.hpp"

namespace rtype::client {

class SnapshotCommand: public IGameCommand {
public:
    bool execute(ClientNetworkAdapter &adapter, net::Packet &packet) override;
    bool operator()(ClientNetworkAdapter &adapter,
        net::Packet &packet) override;
    static std::unique_ptr<IGameCommand> create();
};

class EventCommand: public IGameCommand {
public:
    bool execute(ClientNetworkAdapter &adapter, net::Packet &packet) override;
    bool operator()(ClientNetworkAdapter &adapter,
        net::Packet &packet) override;
    static std::unique_ptr<IGameCommand> create();
};
class ShutdownCommand: public IGameCommand {
public:
    bool execute(ClientNetworkAdapter &adapter, net::Packet &packet) override;
    bool operator()(ClientNetworkAdapter &adapter,
        net::Packet &packet) override;
    static std::unique_ptr<IGameCommand> create();
};
class KickCommand: public IGameCommand {
public:
    bool execute(ClientNetworkAdapter &adapter, net::Packet &packet) override;
    bool operator()(ClientNetworkAdapter &adapter,
        net::Packet &packet) override;
    static std::unique_ptr<IGameCommand> create();
};

} // namespace rtype::client
#endif /* !GAME_COMMANDS_HPP_ */
