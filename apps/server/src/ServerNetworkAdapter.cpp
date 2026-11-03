/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerNetworkAdapter.cpp
*/

#include "ServerNetworkAdapter.hpp"

#include <cassert>
#include <string>

#include "Conversions.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/MessageType.hpp"
#include "rtype/protocol/ProtocolError.hpp"
#include "ServerCommands.hpp"

namespace rtype::server {
using protocol::classify;

namespace {

void incrementErrorStat(AdapterStats &stats, protocol::DecodeError code)
{
    switch (code) {
    case protocol::DecodeError::BAD_SIZE: ++stats.toatBadSize; break;
    case protocol::DecodeError::BAD_MAGIC: ++stats.totalBadMagic; break;
    case protocol::DecodeError::UNKNOWN_TYPE: ++stats.totalUnknownType; break;
    case protocol::DecodeError::WRONG_DIRECTION: ++stats.totalWrongDirection; break;
    case protocol::DecodeError::TRUNCATED: ++stats.totalTruncated; break;
    case protocol::DecodeError::TRAILING_BYTES: ++stats.totalTrailingBytes; break;
    case protocol::DecodeError::OUT_OF_RANGE: ++stats.totalOutOfRange; break;
    }
}

bool isQuiet(protocol::DecodeError code) noexcept
{
    return code == protocol::DecodeError::BAD_SIZE ||
        code == protocol::DecodeError::BAD_MAGIC;
}

}

ServerNetworkAdapter::ServerNetworkAdapter(::net::UdpSocket &socket,
    game::GameWorld &world, SessionManager &sessions,
    ServerCommandFactory *commands):
    _socket(socket), _world(world), _sessions(sessions),
    _commands(commands != nullptr ? commands : &_defaultCommands)
{
    _logger.setThreadName("main");
    if (commands == nullptr)
        registerCommands();
}

void ServerNetworkAdapter::registerCommands() noexcept
{
    _commands->registerCreator<HelloCommand>(
        rtype::protocol::MessageType::HELLO);
    _commands->registerCreator<InputCommand>(
        rtype::protocol::MessageType::INPUT);
    _commands->registerCreator<
        DisconnectCommand>(rtype::protocol::MessageType::DISCONNECT);
}

void ServerNetworkAdapter::handle(const ::net::Datagram &datagram,
    TimePoint now)
{
    ++_stats.totalDatagramsHandled;
    auto *const session = _sessions.find(datagram.from);
    if (!allowDatagram(session, datagram.from, now))
        return;

    try {
        processDatagram(datagram, session, now);
    } catch (const net::PacketError &error) {
        handlePacketError(datagram, session, error, now);
    } catch (const std::exception &error) {
        handleUnexpectedError(session, error);
    }
}

bool ServerNetworkAdapter::allowDatagram(ClientSession *session,
    const asio::ip::udp::endpoint &from, TimePoint now)
{
    if (session == nullptr || session->datagrams.allow(now))
        return true;
    if (session->datagrams.shouldKick(now))
        kick(from, protocol::KickReason::RATE_LIMITED);
    return false;
}

/*
** A command can add the session (HELLO) or remove it (DISCONNECT), and
** removing it frees `session`. So once the command has run, the session is
** looked up again by address instead of going through the old pointer.
*/
void ServerNetworkAdapter::processDatagram(const ::net::Datagram &datagram,
    ClientSession *session, TimePoint now)
{
    protocol::checkDatagramSize(datagram.bytes.size());
    net::Packet packet(datagram.bytes);
    protocol::Header header;
    packet >> header;

    if (header.version != protocol::Header::VERSION) {
        if (header.type == protocol::MessageType::HELLO && session == nullptr)
            reject(datagram.from, protocol::RejectReason::VERSION_MISMATCH,
                now);
        return;
    }
    if (session == nullptr && header.type != protocol::MessageType::HELLO)
        return;

    protocol::checkType(header.type, protocol::Direction::CLIENT_TO_SERVER);
    assert(_commands->hasKey(header.type));
    IncomingMessage message{
        .from = datagram.from,
        .session = session,
        .packet = packet,
        .receivedAt = now,
    };
    const bool accepted =
        _commands->create(header.type)->execute(*this, message);
    auto *const current = _sessions.find(datagram.from);

    if (current == nullptr)
        return;
    if (!accepted) {
        countOffence(*current, protocol::DecodeError::OUT_OF_RANGE, now);
        return;
    }
    current->lastHeard = now;
}

void ServerNetworkAdapter::handlePacketError(
    const ::net::Datagram &datagram, ClientSession *session,
    const net::PacketError &error, TimePoint now)
{
    const auto code = classify(error);
    incrementErrorStat(_stats, code);

    auto &lastLogged = _lastProtocolLogs[datagram.from][code];
    if (lastLogged == TimePoint{} ||
        now - lastLogged >= std::chrono::seconds(1)) {
        lastLogged = now;
        if (session == nullptr && isQuiet(code))
            _logger.debug(error.what());
        else
            _logger.info(error.what());
    }
    if (session != nullptr)
        countOffence(*session, code, now);
}

void ServerNetworkAdapter::handleUnexpectedError(ClientSession *session,
    const std::exception &error)
{
    _logger.error(error.what());
    if (session != nullptr)
        kick(session->endpoint, protocol::KickReason::PROTOCOL_ERROR);
}

void ServerNetworkAdapter::countOffence(ClientSession &session,
    protocol::DecodeError code, TimePoint now)
{
    if (!session.offences.record(now)) {
        kick(session.endpoint, protocol::KickReason::PROTOCOL_ERROR);
        return;
    }
    (void)code;
}

void ServerNetworkAdapter::removeClient(const asio::ip::udp::endpoint &endpoint,
    game::LeaveReason reason)
{
    auto *const session = _sessions.find(endpoint);
    if (session == nullptr)
        return;
    _world.apply(game::LeavePlayer{.playerId=session->playerId, .reason=reason});
    _sessions.remove(endpoint);
}

void ServerNetworkAdapter::kick(const asio::ip::udp::endpoint &endpoint,
    protocol::KickReason reason)
{
    auto *const session = _sessions.find(endpoint);
    if (session == nullptr)
        return;
    const protocol::Kick message{.reason = reason};
    for (int copy = 0; copy != 3; ++copy)
        sendTo(*session, protocol::MessageType::KICK, message);
    ++_stats.totalKicked;
    _logger.warn("Kicking client " + endpoint.address().to_string() + ":" +
        std::to_string(endpoint.port()) + " (" +
        std::to_string(static_cast<unsigned>(reason)) + ")");
    removeClient(endpoint, game::LeaveReason::KICKED);
}

void ServerNetworkAdapter::reject(const asio::ip::udp::endpoint &endpoint,
    protocol::RejectReason reason, TimePoint now)
{
    if (!_rejects.allow(endpoint, now))
        return;
    sendUnsessioned(endpoint, protocol::MessageType::REJECT,
        protocol::Reject{.reason = reason});
}

[[nodiscard]] game::GameWorld &ServerNetworkAdapter::world() noexcept
{
    return _world;
}

[[nodiscard]] SessionManager &ServerNetworkAdapter::sessions() noexcept
{
    return _sessions;
}

[[nodiscard]] const AdapterStats &ServerNetworkAdapter::stats() const noexcept
{
    return _stats;
}

void ServerNetworkAdapter::publish(const std::vector<game::GameEvent> &events)
{
    for (const auto &event : events) {
        const auto outgoing = toProtocol(event);
        _repeater.add(outgoing.event, outgoing.copies);
    }
}

void ServerNetworkAdapter::flushEvents()
{
    const auto events = _repeater.takeDue();
    for (auto &[endpoint, session] : _sessions.all()) {
        (void)endpoint;
        for (const auto &event : events)
            sendTo(session, protocol::MessageType::EVENT, event);
    }
}

void ServerNetworkAdapter::sendSnapshots()
{
    for (auto &[endpoint, session] : _sessions.all()) {
        (void)endpoint;
        const auto &chunks = _snapshotBuilder.build(_world,
            session.lastProcessedInput);
        for (const auto &chunk : chunks)
            sendTo(session, protocol::MessageType::SNAPSHOT, chunk);
    }
}

void ServerNetworkAdapter::flushGoodbyes()
{
    for (auto &[endpoint, packet] : _goodbyes.takeDue())
        _socket.sendTo(endpoint, packet);
}

void ServerNetworkAdapter::broadcastShutdown()
{
    const protocol::Shutdown shutdown;
    for (auto &[endpoint, session] : _sessions.all()) {
        (void)endpoint;
        for (int copy = 0; copy != 3; ++copy)
            sendTo(session, protocol::MessageType::SHUTDOWN, shutdown);
    }
}
} // namespace rtype::server
