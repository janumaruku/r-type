/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientNetworkAdapter.hpp
*/

#ifndef CLIENT_NETWORK_ADAPTER_HPP
#define CLIENT_NETWORK_ADAPTER_HPP

#include "ClientWorld.hpp"
#include "GameCommandFactory.hpp"
#include "GameEventBus.hpp"
#include "SnapshotAssembler.hpp"
#include "EventFilter.hpp"
#include "ServerLink.hpp"

namespace rtype::client {

class ClientNetworkAdapter {
public:
    ClientNetworkAdapter(ServerLink &link, ClientWorld &world, GameEventBus &events);

    void handle(const net::Datagram &datagram, TimePoint now);
    void sendInput(std::uint32_t sequence, std::uint8_t buttons);
    void sendDisconnect();

    [[nodiscard]] ClientWorld &world() noexcept;
    [[nodiscard]] SnapshotAssembler &assembler() noexcept;
    [[nodiscard]] GameEventBus &events() noexcept;
    [[nodiscard]] EventFilter &seenEvents() noexcept;
    void leave(LeaveCause cause) noexcept;
    [[nodiscard]] LeaveCause leaveCause() const noexcept;

private:
    void registerCommands() noexcept;

    ServerLink &_link;
    ClientWorld &_world;
    GameEventBus &_events;
    SnapshotAssembler _assembler;
    EventFilter _seenEvents;
    GameCommandFactory _commands;
    ClientLogLimiter _logs;
    LeaveCause _leave = LeaveCause::NONE;
};

}
#endif /* !CLIENT_NETWORK_ADAPTER_HPP_ */