/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientNetworkAdapter.cpp
*/

#include "ClientNetworkAdapter.hpp"

#include "GameCommands.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/MessageType.hpp"
#include "ServerLink.tpp"
#include "SnapshotAssembler.hpp"

namespace rtype::client {

ClientNetworkAdapter::ClientNetworkAdapter(ServerLink &link, ClientWorld &world,
    GameEventBus &events): _link(link), _world(world), _events(events)
{
    registerCommands();
}

void ClientNetworkAdapter::registerCommands() noexcept
{
    _commands.registerCreator<
        SnapshotCommand>(rtype::protocol::MessageType::SNAPSHOT);
    _commands
        .registerCreator<EventCommand>(rtype::protocol::MessageType::EVENT);
    _commands.registerCreator<
        ShutdownCommand>(rtype::protocol::MessageType::SHUTDOWN);
    _commands.registerCreator<KickCommand>(rtype::protocol::MessageType::KICK);
}

void ClientNetworkAdapter::handle(const net::Datagram &datagram, TimePoint now)
{
    auto message = checkFromServer(datagram, _link, _logs, now);
    if (!message.has_value())
        return;
    auto &packet = message->packet;
    try {
        if (!_commands.hasKey(message->header.type)) {
            throw net::PacketError("Unknown message type");
        }
        auto command =
            _commands.create(message->header.type)->execute(*this, packet);
        if (!command) {
            throw net::PacketError("Command execution failed");
        }
        _link.lastHeard = now;
    } catch (const net::PacketError &e) {
        (void)_logs.shouldLog(protocol::DecodeError::UNKNOWN_TYPE, now);
    }
}

void ClientNetworkAdapter::sendInput(std::uint32_t sequence,
    std::uint8_t buttons)
{
    _link.send(protocol::MessageType::INPUT,
        protocol::Input{.inputSequence = sequence, .buttons = buttons});
}

void ClientNetworkAdapter::sendDisconnect()
{
    for (std::uint32_t i = 0; i < 3; ++i) {
        _link.send(protocol::MessageType::DISCONNECT, protocol::Disconnect{});
        std::chrono::milliseconds timeout(20);
        std::this_thread::sleep_for(timeout);
    }
}

ClientWorld &ClientNetworkAdapter::world() const noexcept
{
    return _world;
}

SnapshotAssembler &ClientNetworkAdapter::assembler() noexcept
{
    return _assembler;
}

GameEventBus &ClientNetworkAdapter::events() const noexcept
{
    return _events;
}

EventFilter &ClientNetworkAdapter::seenEvents() noexcept
{
    return _seenEvents;
}

void ClientNetworkAdapter::leave(LeaveCause cause) noexcept
{
    if (_leaveCause == LeaveCause::NO_CAUSE)
        _leaveCause = cause;
}

LeaveCause &ClientNetworkAdapter::leaveCause() noexcept
{
    return _leaveCause;
}

} // namespace rtype::client
