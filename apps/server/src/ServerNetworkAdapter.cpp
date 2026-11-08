/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerNetworkAdapter.cpp
*/

#include "ServerNetworkAdapter.hpp"

#include <cassert>
#include <chrono>
#include <string>
#include <thread>

#include "Conversions.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/MessageType.hpp"
#include "rtype/protocol/ProtocolError.hpp"
#include "ServerCommands.hpp"

namespace rtype::server {
using protocol::classify;

namespace {
constexpr std::chrono::seconds LOG_INTERVAL{1};
constexpr std::uint8_t SHUTDOWN_COPIES = 3;
constexpr std::chrono::milliseconds SHUTDOWN_INTERVAL{20};

void incrementErrorStat(AdapterStats &stats, protocol::DecodeError code)
{
    switch (code) {
    case protocol::DecodeError::BAD_SIZE:
        ++stats.totalBadSize;
        break;
    case protocol::DecodeError::BAD_MAGIC:
        ++stats.totalBadMagic;
        break;
    case protocol::DecodeError::UNKNOWN_TYPE:
        ++stats.totalUnknownType;
        break;
    case protocol::DecodeError::WRONG_DIRECTION:
        ++stats.totalWrongDirection;
        break;
    case protocol::DecodeError::TRUNCATED:
        ++stats.totalTruncated;
        break;
    case protocol::DecodeError::TRAILING_BYTES:
        ++stats.totalTrailingBytes;
        break;
    case protocol::DecodeError::OUT_OF_RANGE:
        ++stats.totalOutOfRange;
        break;
    }
}

bool isQuiet(protocol::DecodeError code) noexcept
{
    return code == protocol::DecodeError::BAD_SIZE ||
        code == protocol::DecodeError::BAD_MAGIC;
}

} // namespace

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
    _commands
        ->registerCreator<HelloCommand>(rtype::protocol::MessageType::HELLO);
    _commands
        ->registerCreator<InputCommand>(rtype::protocol::MessageType::INPUT);
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
        .from       = datagram.from,
        .session    = session,
        .packet     = packet,
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

void ServerNetworkAdapter::handlePacketError(const ::net::Datagram &datagram,
    ClientSession *session, const net::PacketError &error, TimePoint now)
{
    const auto code = classify(error);

    if (shouldLog(datagram.from, code, now)) {
        if (session == nullptr && isQuiet(code))
            _logger.debug(error.what());
        else
            _logger.info(error.what());
    }
    if (session != nullptr)
        countOffence(*session, code, now);
    else
        incrementErrorStat(_stats, code);
}

/*
** At most one line per second per address and code. Anyone can send from
** any address, so the table is bounded: when it's full, the entries older
** than a second (which would allow a line anyway) are removed, at most once
** a second so a flood of new addresses can't make every datagram scan it.
** If it's still full, the new address isn't logged: the stats still count it.
*/
bool ServerNetworkAdapter::shouldLog(const asio::ip::udp::endpoint &from,
    protocol::DecodeError code, TimePoint now)
{
    const auto key = std::pair{from, code};
    const auto it  = _lastProtocolLogs.find(key);

    if (it != _lastProtocolLogs.end()) {
        if (now - it->second < LOG_INTERVAL)
            return false;
        it->second = now;
        return true;
    }
    if (_lastProtocolLogs.size() >= MAX_LOG_ENTRIES) {
        if (_lastPrune && now - *_lastPrune < LOG_INTERVAL)
            return false;
        _lastPrune = now;
        std::erase_if(_lastProtocolLogs, [now](const auto &entry) {
            return now - entry.second >= LOG_INTERVAL;
        });
        if (_lastProtocolLogs.size() >= MAX_LOG_ENTRIES)
            return false;
    }
    _lastProtocolLogs.emplace(key, now);
    return true;
}

std::size_t ServerNetworkAdapter::loggedAddresses() const noexcept
{
    return _lastProtocolLogs.size();
}

void ServerNetworkAdapter::handleUnexpectedError(ClientSession *session,
    const std::exception &error)
{
    _logger.error(error.what());
    if (session != nullptr)
        kick(session->endpoint, protocol::KickReason::PROTOCOL_ERROR);
}

/*
** Every rejected datagram is counted once in the stats under its code: here
** when it comes from a session (a decode error, or a command that refused
** the message with OUT_OF_RANGE), in handlePacketError otherwise.
*/
void ServerNetworkAdapter::countOffence(ClientSession &session,
    protocol::DecodeError code, TimePoint now)
{
    incrementErrorStat(_stats, code);
    if (!session.offences.record(now))
        kick(session.endpoint, protocol::KickReason::PROTOCOL_ERROR);
}

void ServerNetworkAdapter::removeClient(const asio::ip::udp::endpoint &endpoint,
    game::LeaveReason reason)
{
    auto *const session = _sessions.find(endpoint);
    if (session == nullptr)
        return;
    _world.apply(game::LeavePlayer{
        .playerId = session->playerId, .reason = reason});
    _sessions.remove(endpoint);
}

void ServerNetworkAdapter::kick(const asio::ip::udp::endpoint &endpoint,
    protocol::KickReason reason)
{
    auto *const session = _sessions.find(endpoint);
    if (session == nullptr)
        return;
    ++session->sendSequence;
    _goodbyes.add(endpoint,
        protocol::makePacket(protocol::MessageType::KICK, session->sendSequence,
            protocol::Kick{.reason = reason}));
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
    for (const auto &event: events) {
        const auto outgoing = toProtocol(event);
        _repeater.add(outgoing.event, outgoing.copies);
    }
}

void ServerNetworkAdapter::flushEvents()
{
    const auto events = _repeater.takeDue();
    for (auto &[endpoint, session]: _sessions.all()) {
        (void)endpoint;
        for (const auto &event: events)
            sendTo(session, protocol::MessageType::EVENT, event);
    }
}

/*
** The snapshot is built once per snapshot tick, not once per client. Only
** lastProcessedInput differs between clients, so it's set on each chunk
** just before that client's copy is encoded.
*/
void ServerNetworkAdapter::sendSnapshots()
{
    auto &chunks = _snapshotBuilder.build(_world);

    for (auto &[endpoint, session]: _sessions.all()) {
        (void)endpoint;
        const auto *const slot = _world.roster().get(session.playerId);
        const std::uint32_t processed =
            slot != nullptr ? slot->lastInputSequence : 0;

        for (auto &chunk: chunks) {
            chunk.lastProcessedInput = processed;
            sendTo(session, protocol::MessageType::SNAPSHOT, chunk);
        }
    }
}

void ServerNetworkAdapter::flushGoodbyes()
{
    for (auto &[endpoint, packet]: _goodbyes.takeDue())
        _socket.sendTo(endpoint, packet);
}

std::size_t ServerNetworkAdapter::broadcastShutdown()
{
    const protocol::Shutdown shutdown;
    std::size_t sent = 0;

    for (std::uint8_t copy = 0; copy != SHUTDOWN_COPIES; ++copy) {
        if (copy != 0)
            std::this_thread::sleep_for(SHUTDOWN_INTERVAL);
        for (auto &[endpoint, session]: _sessions.all()) {
            (void)endpoint;
            sendTo(session, protocol::MessageType::SHUTDOWN, shutdown);
            ++sent;
        }
    }
    return sent;
}
} // namespace rtype::server
