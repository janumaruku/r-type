/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocket.cpp
*/

#include "UdpSocket.hpp"

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
{}

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
        throw engine::core::
            NetworkError(std::format("Failed to open UDP socket on port {}: {}",
                _port, error.message()));

    _socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _port), error);
    if (error) {
        _socket.close();
        throw engine::core::
            NetworkError(std::format("Failed to bind UDP socket on port {}: {}",
                _port, error.message()));
    }

    _running = true;
    receiveNext();
    _thread = std::thread([this]() { _ioContext.run(); });
}

void UdpSocket::receiveNext()
{
    _socket.async_receive_from(asio::buffer(_receiveBuffer), _sender,
        [this](const asio::error_code &error, std::size_t bytesReceived) {
            if (error == asio::error::operation_aborted)
                return;
            if (error) {
                receiveNext();
                return;
            }
            if (bytesReceived == 0) {
                receiveNext();
                return;
            }
            if (bytesReceived > Packet::MAX_SIZE) {
                ++_dropped;
                engine::core::log::Logger("UdpSocket")
                    .debug("Dropped UDP datagram from {}: size {} exceeds "
                           "maximum {}",
                        _sender.address().to_string(), bytesReceived,
                        Packet::MAX_SIZE);
                receiveNext();
                return;
            }

            std::scoped_lock lock(_mutex);
            if (_incoming.size() >= 4096) {
                ++_dropped;
            } else {
                _incoming.push_back({
                    .bytes = std::vector<std::uint8_t>(_receiveBuffer.begin(),
                        _receiveBuffer.begin() + bytesReceived),
                    .from  = _sender,
                });
            }
            receiveNext();
        });
}

void UdpSocket::stop()
{
    _running = false;
    _workGuard.reset();
    _ioContext.stop();
    if (_thread.joinable())
        _thread.join();
}

void UdpSocket::sendTo(const asio::ip::udp::endpoint &endpoint,
    const Packet &packet)
{
    if (!_running) {
        engine::core::log::Logger("UdpSocket")
            .debug("tried to send a packet while the socket is not running");
        return;
    }
    const auto packetBytes = packet.bytes();
    std::vector<std::uint8_t> bytes(packetBytes.begin(), packetBytes.end());
    std::shared_ptr<std::vector<std::uint8_t>> sharedBytes =
        std::make_shared<std::vector<std::uint8_t>>(std::move(bytes));
    asio::post(_workGuard.get_executor(),
        [this, endpoint, sharedBytes = std::move(sharedBytes)]() {
            _socket.async_send_to(asio::buffer(*sharedBytes), endpoint,
                [endpoint, sharedBytes = sharedBytes](
                    const asio::error_code &error, std::size_t) {
                    if (!error)
                        return;
                    engine::core::log::Logger("UdpSocket").debug(
                        "Failed to send UDP datagram to {}:{}: {}",
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
