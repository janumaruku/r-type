/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocket.cpp
*/

#include "network/UdpSocket.hpp"

#include <asio/post.hpp>
#include <cassert>
#include <format>
#include <utility>

#include "Errors.hpp"
#include "Logger.hpp"

namespace net {

UdpSocket::UdpSocket(std::uint16_t port):
    _port(port), _workGuard(asio::make_work_guard(_ioContext)),
    _socket(_ioContext), _dropped(0)
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

    _logger.info("UDP socket bound on port {}", localEndpoint().port());

    _running = true;
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
                    ++_receiveErrors;
                    _logger.error(
                        "Exception in UDP receive handler: {}",
                        exception.what());
                    if (_receiveErrors < MAX_RECEIVE_ERRORS)
                        rearmReceive();
                } catch (...) {
                    ++_receiveErrors;
                    _logger.error("Unknown exception in UDP receive handler");
                    if (_receiveErrors < MAX_RECEIVE_ERRORS)
                        rearmReceive();
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

void UdpSocket::retryReceiveLater()
{
    ++_receiveErrors;
    if (_receiveErrors >= MAX_RECEIVE_ERRORS) {
        _logger.error("Stopping UDP receive loop after repeated errors");
        return;
    }
    try {
        asio::post(_ioContext, [this] { rearmReceive(); });
    } catch (...) {
        _logger.error("Cannot schedule a new UDP receive");
    }
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
        ++_dropped;
        _receiveErrors = 0;
        _logger.debug("Dropped UDP datagram larger than the {}-byte receive buffer",
            _receiveBuffer.size());
        rearmReceive();
        return;
    }

    if (error) {
        handleReceiveError(error);
        return;
    }

    _receiveErrors = 0;
    if (bytesReceived == 0) {
        rearmReceive();
        return;
    }
    if (bytesReceived > Packet::MAX_SIZE) {
        ++_dropped;
        _logger.debug(
            "Dropped UDP datagram from {}: size {} exceeds maximum {}",
            _sender.address().to_string(), bytesReceived, Packet::MAX_SIZE);
        rearmReceive();
        return;
    }

    enqueueDatagram(bytesReceived);
    rearmReceive();
}

void UdpSocket::handleReceiveError(const asio::error_code &error)
{
    ++_receiveErrors;
    if (_receiveErrors >= MAX_RECEIVE_ERRORS) {
        _logger.error("Stopping UDP receive loop after repeated errors: {}",
            error.message());
        return;
    }

    _logger.warn("UDP receive failed: {}", error.message());
    rearmReceive();
}

void UdpSocket::enqueueDatagram(std::size_t bytesReceived)
{
    std::scoped_lock lock(_mutex);
    if (_incoming.size() >= MAX_QUEUED_DATAGRAMS) {
        ++_dropped;
        return;
    }

    _incoming.push_back({
        .bytes = std::vector(_receiveBuffer.begin(),
            _receiveBuffer.begin() + bytesReceived),
        .from = _sender,
    });
}

void UdpSocket::rearmReceive()
{
    if (_socket.is_open())
        receiveNext();
}

void UdpSocket::stop()
{
    _running = false;
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
    asio::post(_workGuard.get_executor(),
        [this, endpoint, sharedBytes = std::move(sharedBytes)] {
            _socket.async_send_to(asio::buffer(*sharedBytes), endpoint,
                [this, endpoint,
                    sharedBytes = sharedBytes](const asio::error_code &error,
                std::size_t) {
                    if (!error)
                        return;
                    _logger.debug("Failed to send UDP datagram to {}:{}: {}",
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

[[nodiscard]] std::uint64_t UdpSocket::dropped() const
{
    return _dropped.load();
}

} // namespace net
