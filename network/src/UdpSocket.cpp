/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocket.cpp
*/

#include "network/UdpSocket.hpp"

#include <asio/post.hpp>
#include <cassert>
#include <chrono>
#include <format>
#include <utility>

#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"

namespace net {

UdpSocket::UdpSocket(std::uint16_t port):
    _port(port), _socket(_ioContext)
{
}

UdpSocket::~UdpSocket()
{
    stop();
}

void UdpSocket::start()
{
    assert(!_socket.is_open());

    asio::error_code error;
    _socket.open(asio::ip::udp::v4(), error);
    if (error)
        throw engine::core::NetworkError(std::format(
            "Failed to open UDP socket on port {}: {}", _port,
            error.message()));

    _socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _port), error);
    if (error) {
        _socket.close();
        throw engine::core::NetworkError(std::format(
            "Failed to bind UDP socket on port {}: {}", _port,
            error.message()));
    }

    _socket.set_option(asio::socket_base::receive_buffer_size(
        SYSTEM_RECEIVE_BUFFER_BYTES), error);
    if (error)
        _logger.warn("Cannot enlarge the UDP receive buffer: {}",
            error.message());

    asio::socket_base::receive_buffer_size bufferSize;
    _socket.get_option(bufferSize, error);
    _logger.info("UDP socket bound on port {}, system receive buffer {} bytes",
        localEndpoint().port(), error ? 0 : bufferSize.value());

    _ioContext.restart();
    _workGuard.emplace(_ioContext.get_executor());
    _receiveErrors = 0;
    _running = true;
    _receiving = true;
    receiveNext();
    _thread = std::thread([this] {
        _logger.setThreadName("net-io");
        _ioContext.run();
    });
}

void UdpSocket::receiveNext()
{
    if (!_socket.is_open())
        return;
    try {
        _socket.async_receive_from(asio::buffer(_receiveBuffer), _sender,
            [this](const asio::error_code &error, std::size_t bytesReceived) {
                try {
                    handleReceive(error, bytesReceived);
                } catch (const std::exception &exception) {
                    countReceiveError();
                    _logger.error(
                        "Exception in UDP receive handler: {}",
                        exception.what());
                    if (_receiveErrors < MAX_RECEIVE_ERRORS)
                        rearmReceive();
                    else
                        giveUpReceiving();
                } catch (...) {
                    countReceiveError();
                    _logger.error("Unknown exception in UDP receive handler");
                    if (_receiveErrors < MAX_RECEIVE_ERRORS)
                        rearmReceive();
                    else
                        giveUpReceiving();
                }
            });
    } catch (const std::exception &exception) {
        _logger.error(
            "Exception while starting UDP receive: {}", exception.what());
        retryReceiveLater();
    } catch (...) {
        _logger.error("Unknown exception while starting UDP receive");
        retryReceiveLater();
    }
}

void UdpSocket::countReceiveError()
{
    ++_receiveErrors;
    _counters.receiveErrors.fetch_add(1, std::memory_order_relaxed);
}

void UdpSocket::retryReceiveLater()
{
    countReceiveError();
    if (_receiveErrors >= MAX_RECEIVE_ERRORS) {
        giveUpReceiving();
        return;
    }
    try {
        asio::post(_ioContext, [this] {
            rearmReceive();
        });
    } catch (...) {
        _logger.error("Cannot schedule a new UDP receive");
        giveUpReceiving();
    }
}

void UdpSocket::giveUpReceiving()
{
    _receiving = false;
    const auto [received, droppedEmpty, droppedOversized, droppedQueueFull,
        receiveErrors, sent, sendErrors] = stats();
    _logger.error("UDP receive loop stopped after repeated errors; "
        "the socket no longer receives (received {}, dropped {} empty, "
        "{} oversized, {} on a full queue, {} receive errors, sent {}, "
        "{} send errors)", received, droppedEmpty,
        droppedOversized, droppedQueueFull, receiveErrors,
        sent, sendErrors);
}

void UdpSocket::handleReceive(const asio::error_code &error,
    std::size_t bytesReceived)
{
    if (error == asio::error::operation_aborted)
        return;

    #ifdef _WIN32
    if (error == asio::error::connection_refused ||
        error == asio::error::connection_reset) {
        _receiveErrors = 0;
        rearmReceive();
        return;
    }
    #endif

    if (error == asio::error::message_size) {
        _receiveErrors = 0;
        dropOversized();
        rearmReceive();
        return;
    }

    if (error) {
        handleReceiveError(error);
        return;
    }

    _receiveErrors = 0;
    if (bytesReceived == 0) {
        _counters.droppedEmpty.fetch_add(1, std::memory_order_relaxed);
        rearmReceive();
        return;
    }
    if (bytesReceived > Packet::MAX_SIZE) {
        dropOversized();
        rearmReceive();
        return;
    }

    if (!enqueueDatagram(bytesReceived)) {
        const std::uint64_t count = _queueFullLog.record(
            std::chrono::steady_clock::now());
        if (count != 0)
            _logger.warn("UDP queue full: {} datagram(s) dropped since the "
                "last warning", count);
    }
    rearmReceive();
}

void UdpSocket::dropOversized()
{
    _counters.droppedOversized.fetch_add(1, std::memory_order_relaxed);
    const std::uint64_t count = _oversizedLog.record(
        std::chrono::steady_clock::now());
    if (count != 0)
        _logger.debug("{} UDP datagram(s) larger than {} bytes dropped since "
            "the last message", count, Packet::MAX_SIZE);
}

void UdpSocket::handleReceiveError(const asio::error_code &error)
{
    countReceiveError();
    _logger.warn("UDP receive failed: {}", error.message());
    if (_receiveErrors >= MAX_RECEIVE_ERRORS) {
        giveUpReceiving();
        return;
    }
    rearmReceive();
}

/*
** @brief Queues the datagram, or counts it as dropped if the queue is full.
** @return false when it was dropped. The caller logs it, so that nothing is
** logged while @ref _mutex is held.
*/
bool UdpSocket::enqueueDatagram(std::size_t bytesReceived)
{
    std::scoped_lock lock(_mutex);
    if (_incoming.size() >= MAX_QUEUED_DATAGRAMS) {
        _counters.droppedQueueFull.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    _incoming.push_back({
        .bytes = std::vector(_receiveBuffer.begin(),
            _receiveBuffer.begin() + bytesReceived),
        .from = _sender,
    });
    _counters.received.fetch_add(1, std::memory_order_relaxed);
    return true;
}

void UdpSocket::rearmReceive()
{
    if (_socket.is_open())
        receiveNext();
}

void UdpSocket::stop()
{
    _running = false;
    _receiving = false;
    _workGuard.reset();
    _ioContext.stop();
    _socket.close();

    if (_thread.joinable()) {
        _thread.join();
        _logger.info("UDP socket stopped");
    }
}

void UdpSocket::sendTo(const asio::ip::udp::endpoint &endpoint,
    const Packet &packet)
{
    if (!_running) {
        _logger.debug("tried to send a packet while the socket is not running");
        return;
    }

    const auto packetBytes = packet.bytes();
    std::vector bytes(packetBytes.begin(), packetBytes.end());
    auto sharedBytes = std::make_shared<std::vector<std::uint8_t>>(
        std::move(bytes));
    asio::post(_ioContext.get_executor(),
        [this, endpoint, sharedBytes = std::move(sharedBytes)] {
            _socket.async_send_to(asio::buffer(*sharedBytes), endpoint,
                [this, endpoint,
                    sharedBytes = sharedBytes](const asio::error_code &error,
                std::size_t) {
                    if (!error) {
                        _counters.sent.fetch_add(1, std::memory_order_relaxed);
                        return;
                    }
                    if (error == asio::error::operation_aborted)
                        return;
                    _counters.sendErrors.fetch_add(1,
                        std::memory_order_relaxed);
                    const std::uint64_t count = _sendErrorLog.record(
                        std::chrono::steady_clock::now());
                    if (count != 0)
                        _logger.debug("{} UDP send(s) failed since the last "
                            "message, the last one to {}:{}: {}", count,
                            endpoint.address().to_string(), endpoint.port(),
                            error.message());
                });
        });
}

std::vector<Datagram> UdpSocket::receiveAll(std::size_t max)
{
    std::vector<Datagram> result;
    std::scoped_lock lock(_mutex);
    const auto count = std::min(max, _incoming.size());
    result.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        result.push_back(std::move(_incoming.front()));
        _incoming.pop_front();
    }
    return result;
}

[[nodiscard]] asio::ip::udp::endpoint UdpSocket::localEndpoint() const
{
    return _socket.local_endpoint();
}

[[nodiscard]] SocketStats UdpSocket::stats() const
{
    return {
        .received = _counters.received.load(std::memory_order_relaxed),
        .droppedEmpty = _counters.droppedEmpty.load(std::memory_order_relaxed),
        .droppedOversized =
        _counters.droppedOversized.load(std::memory_order_relaxed),
        .droppedQueueFull =
        _counters.droppedQueueFull.load(std::memory_order_relaxed),
        .receiveErrors =
        _counters.receiveErrors.load(std::memory_order_relaxed),
        .sent = _counters.sent.load(std::memory_order_relaxed),
        .sendErrors = _counters.sendErrors.load(std::memory_order_relaxed),
    };
}

[[nodiscard]] std::uint64_t UdpSocket::dropped() const
{
    const SocketStats total = stats();

    return total.droppedEmpty + total.droppedOversized
        + total.droppedQueueFull;
}

std::uint64_t UdpSocket::LogLimiter::record(
    std::chrono::steady_clock::time_point now)
{
    ++pending;
    if (last != std::chrono::steady_clock::time_point{}
        && now - last < std::chrono::seconds(1))
        return 0;
    last = now;
    return std::exchange(pending, 0);
}

[[nodiscard]] bool UdpSocket::isReceiving() const noexcept
{
    return _receiving.load();
}

} // namespace net
