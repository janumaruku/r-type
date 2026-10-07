#include "network/UdpSocket.hpp"

#include <algorithm>

namespace net {

UdpSocket::UdpSocket(std::uint16_t port)
    : _work(asio::make_work_guard(_io)), _socket(_io), _port(port)
{
}

UdpSocket::~UdpSocket()
{
    stop();
}

void UdpSocket::start()
{
    if (_started) {
        return;
    }

    asio::error_code error;
    _socket.open(asio::ip::udp::v4(), error);
    if (error) {
        throw NetworkError("unable to open UDP socket: " + error.message());
    }
    _socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _port), error);
    if (error) {
        throw NetworkError("unable to bind UDP socket: " + error.message());
    }

    _started = true;
    receiveNext();
    _thread = std::thread([this] { _io.run(); });
}

void UdpSocket::stop()
{
    if (!_started) {
        return;
    }

    asio::post(_io, [this] {
        asio::error_code error;
        _socket.close(error);
    });
    _work.reset();
    _io.stop();
    if (_thread.joinable()) {
        _thread.join();
    }
    _started = false;
}

void UdpSocket::sendTo(const asio::ip::udp::endpoint& to, const Packet& packet)
{
    auto bytes = std::make_shared<std::vector<std::uint8_t>>(
        packet.bytes().begin(), packet.bytes().end());
    asio::post(_io, [this, to, bytes] {
        _socket.async_send_to(
            asio::buffer(*bytes), to,
            [bytes](const asio::error_code&, std::size_t) {});
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

asio::ip::udp::endpoint UdpSocket::localEndpoint() const
{
    asio::error_code error;
    return _socket.local_endpoint(error);
}

std::uint64_t UdpSocket::dropped() const noexcept
{
    return _dropped.load();
}

void UdpSocket::receiveNext()
{
    _socket.async_receive_from(
        asio::buffer(_receiveBuffer), _sender,
        [this](const asio::error_code& error, std::size_t size) {
            if (!error && size > 0 && size <= Packet::MAX_SIZE) {
                std::scoped_lock lock(_mutex);
                if (_incoming.size() < 4096) {
                    _incoming.push_back({_sender,
                                         std::vector<std::uint8_t>(
                                             _receiveBuffer.begin(),
                                             _receiveBuffer.begin() + size)});
                } else {
                    ++_dropped;
                }
            } else if (!error ||
                       (error != asio::error::operation_aborted &&
                        error != asio::error::connection_refused &&
                        error != asio::error::connection_reset)) {
                ++_dropped;
            }

            if (_socket.is_open()) {
                receiveNext();
            }
        });
}

}