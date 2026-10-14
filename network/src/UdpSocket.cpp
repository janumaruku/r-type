/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocket.cpp
*/

#include "UdpSocket.hpp"

#include <cassert>
#include <format>

#include "Errors.hpp"

namespace net {

UdpSocket::UdpSocket(std::uint16_t port):
    _port(port), _workGuard(asio::make_work_guard(_ioContext)),
    _socket(_ioContext)
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

    receiveAll();

    _thread = std::thread([this]() { _ioContext.run(); });
}

void UdpSocket::stop()
{
    _workGuard.reset();
    _ioContext.stop();
    if (_thread.joinable())
        _thread.join();
}

void UdpSocket::sendTo(const asio::ip::udp::endpoint &endpoint,
    const Packet &packet)
{
    auto bytes = packet.bytes();
    _socket.async_send_to(asio::buffer(bytes.data(), bytes.size()), endpoint,
        [](const asio::error_code &error, std::size_t bytesSent) {
            if (error)
                throw PacketError(std::format("Failed to send {} bytes: {}",
                    bytesSent, error.message()));
        });
}

void UdpSocket::receiveAll()
{
    _socket.async_receive_from(asio::buffer(_receiveBuffer), _sender,
        [this](const asio::error_code &error, std::size_t bytesReceived) {
            if (!error && bytesReceived > 0) {
                std::vector<std::uint8_t> data(_receiveBuffer.begin(),
                    _receiveBuffer.begin() + bytesReceived);
                _incoming.push_back({.data = std::move(data), .from = _sender});
            }
            receiveAll();
        });
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
