/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerNetworkAdapter.hpp
*/

#ifndef SERVER_NETWORK_ADAPTER_HPP
#define SERVER_NETWORK_ADAPTER_HPP

#include "EventRepeater.hpp"
#include "Goodbyes.hpp"
#include "SnapshotBuilder.hpp"
#include "network/UdpSocket.hpp"
#include "rtype/game/GameWorld.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/ProtocolError.hpp"
#include "ServerCommands.hpp"
#include "Session.hpp"

namespace rtype::server {
struct IncomingMessage;

struct AdapterStats {
    std::uint32_t totalDatagramsHandled = 0;
    std::uint32_t toatBadSize           = 0;
    std::uint32_t totalBadMagic         = 0;
    std::uint32_t totalUnknownType      = 0;
    std::uint32_t totalWrongDirection   = 0;
    std::uint32_t totalTruncated        = 0;
    std::uint32_t totalTrailingBytes    = 0;
    std::uint32_t totalOutOfRange       = 0;
    std::uint32_t totalKicked           = 0;
};

class ServerNetworkAdapter {
public:
    static constexpr std::uint32_t FIRST_SEQUENCE = 1;

    explicit ServerNetworkAdapter(::net::UdpSocket &socket,
        game::GameWorld &world, SessionManager &sessions,
        ServerCommandFactory *commands = nullptr);

    void handle(const ::net::Datagram &datagram, TimePoint now);
    void removeClient(const asio::ip::udp::endpoint &endpoint,
        game::LeaveReason reason);
    void kick(const asio::ip::udp::endpoint &endpoint,
        protocol::KickReason reason);
    void reject(const asio::ip::udp::endpoint &endpoint,
        protocol::RejectReason reason, TimePoint now);
    /**
     * @brief Sends `message` to a client with a session, numbered with the
     * session's next sequence: the first datagram is 1 (RFC § 4).
     */
    void sendTo(ClientSession &session, protocol::MessageType type,
        const auto &message)
    {
        ++session.sendSequence;
        _socket.sendTo(session.endpoint,
            protocol::makePacket(type, session.sendSequence, message));
    }

    /**
     * @brief Sends `message` to an address with no session (REJECT). It's the
     * only datagram that address gets, so it's sequence 1 (RFC § 4).
     */
    void sendUnsessioned(const asio::ip::udp::endpoint &endpoint,
        protocol::MessageType type, const auto &message)
    {
        _socket.sendTo(endpoint,
            protocol::makePacket(type, FIRST_SEQUENCE, message));
    }

    [[nodiscard]] game::GameWorld &world() noexcept;
    [[nodiscard]] SessionManager &sessions() noexcept;
    [[nodiscard]] const AdapterStats &stats() const noexcept;

    void publish(const std::vector<game::GameEvent> &events);
    void flushEvents();
    void sendSnapshots();
    void flushGoodbyes();
    void broadcastShutdown();

private:
    void registerCommands() noexcept;
    [[nodiscard]] bool allowDatagram(ClientSession *session,
        const asio::ip::udp::endpoint &from, TimePoint now);
    void processDatagram(const ::net::Datagram &datagram,
        ClientSession *session, TimePoint now);
    void handlePacketError(const ::net::Datagram &datagram,
        ClientSession *session, const net::PacketError &error, TimePoint now);
    void handleUnexpectedError(ClientSession *session,
        const std::exception &error);
    void countOffence(ClientSession &session, protocol::DecodeError code,
        TimePoint now);

    net::UdpSocket &_socket;
    game::GameWorld &_world;
    SessionManager &_sessions;
    ServerCommandFactory _defaultCommands;
    ServerCommandFactory *_commands;
    RejectLimiter _rejects;
    std::map<asio::ip::udp::endpoint,
        std::map<protocol::DecodeError, TimePoint>>
        _lastProtocolLogs;
    AdapterStats _stats;
    engine::core::log::Logger _logger{"Server"};
    EventRepeater _repeater;
    Goodbyes _goodbyes;
    SnapshotBuilder _snapshotBuilder;
};

} // namespace rtype::server
#endif
