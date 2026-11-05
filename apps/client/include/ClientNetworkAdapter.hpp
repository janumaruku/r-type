/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientNetworkAdapter.hpp
*/

#ifndef CLIENT_NETWORK_ADAPTER_HPP
#define CLIENT_NETWORK_ADAPTER_HPP

#include <cstdint>

#include "ClientDispatch.hpp"
#include "ClientWorld.hpp"
#include "engine/core/FactoryTemplate.hpp"
#include "GameEventBus.hpp"
#include "ServerLink.hpp"
#include "SnapshotAssembler.hpp"

namespace rtype::client {

enum class LeaveCause : std::uint8_t {
    NO_CAUSE,
    SHUTDOWN,
    KICKED_PROTOCOL,
    KICKED_RATE,
};

class ClientNetworkAdapter;

using IGameCommand =
    designPattern::ICommand<ClientNetworkAdapter &, net::Packet &>;
using GameCommandFactory =
    designPattern::FactoryTemplate<IGameCommand, protocol::MessageType>;

class ClientNetworkAdapter {
public:
    ClientNetworkAdapter(ServerLink &link, ClientWorld &world,
        GameEventBus &events);

    void handle(const net::Datagram &datagram, TimePoint now);
    void sendInput(std::uint32_t sequence, std::uint8_t buttons);
    void sendDisconnect();

    [[nodiscard]] ClientWorld &world() const noexcept;
    [[nodiscard]] SnapshotAssembler &assembler() noexcept;
    [[nodiscard]] GameEventBus &events() const noexcept;
    [[nodiscard]] EventFilter &seenEvents() noexcept;
    void leave(LeaveCause cause) noexcept;
    [[nodiscard]] LeaveCause &leaveCause() noexcept;

private:
    void registerCommands() noexcept;

    ServerLink &_link;
    ClientWorld &_world;
    GameEventBus &_events;
    SnapshotAssembler _assembler;
    EventFilter _seenEvents;
    GameCommandFactory _commands;
    ClientLogLimiter _logs;
    LeaveCause _leave = LeaveCause::NO_CAUSE;
};

} // namespace rtype::client
#endif /* !CLIENT_NETWORK_ADAPTER_HPP_ */
