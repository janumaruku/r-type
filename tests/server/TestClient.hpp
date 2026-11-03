#ifndef SERVER_TEST_CLIENT_HPP
#define SERVER_TEST_CLIENT_HPP

#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <thread>
#include <type_traits>
#include <vector>

#include "network/UdpSocket.hpp"
#include "rtype/protocol/Decode.tpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace server_test {

class TestClient {
public:
    explicit TestClient(std::uint16_t serverPort):
        _socket(0),
        _server({asio::ip::address_v4::loopback(), serverPort})
    {
        _socket.start();
    }

    ~TestClient()
    {
        _socket.stop();
    }

    TestClient(const TestClient &) = delete;
    TestClient &operator=(const TestClient &) = delete;

    template <typename Message>
    void send(rtype::protocol::MessageType type, const Message &message)
    {
        _socket.sendTo(_server,
            rtype::protocol::makePacket(type, _sequence++, message));
    }

    void sendRaw(std::span<const std::uint8_t> bytes)
    {
        net::Packet packet;
        packet.writeRaw(bytes.data(), bytes.size());
        _socket.sendTo(_server, packet);
    }

    template <typename Message>
    [[nodiscard]] std::optional<Message> waitFor(
        rtype::protocol::MessageType type,
        std::chrono::milliseconds timeout = std::chrono::seconds(2))
    {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            for (const auto &datagram : _socket.receiveAll()) {
                auto result = consume<Message>(datagram, type);
                if (result)
                    return result;
            }
            std::this_thread::yield();
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<rtype::protocol::Event> eventsSoFar() const
    {
        pump();
        return _events;
    }

    [[nodiscard]] std::optional<rtype::protocol::Snapshot>
    lastSnapshot() const
    {
        pump();
        if (_snapshots.empty())
            return std::nullopt;
        return _snapshots.back();
    }

private:
    template <typename Message>
    std::optional<Message> consume(const net::Datagram &datagram,
        rtype::protocol::MessageType type) const
    {
        try {
            net::Packet packet(datagram.bytes);
            const auto header = packet.read<rtype::protocol::Header>();
            if (header.type == rtype::protocol::MessageType::SNAPSHOT) {
                const auto snapshot =
                    rtype::protocol::decode<rtype::protocol::Snapshot>(packet);
                _snapshots.push_back(snapshot);
                if constexpr (std::is_same_v<Message,
                    rtype::protocol::Snapshot>)
                    return type == header.type ? std::optional{snapshot} :
                        std::nullopt;
            } else if (header.type == rtype::protocol::MessageType::EVENT) {
                const auto event =
                    rtype::protocol::decode<rtype::protocol::Event>(packet);
                _events.push_back(event);
                if constexpr (std::is_same_v<Message, rtype::protocol::Event>)
                    return type == header.type ? std::optional{event} :
                        std::nullopt;
            }
            if (header.type != type)
                return std::nullopt;
            return rtype::protocol::decode<Message>(packet);
        } catch (const net::PacketError &) {
            return std::nullopt;
        }
    }

    void pump() const
    {
        for (const auto &datagram : _socket.receiveAll()) {
            (void)consume<rtype::protocol::Snapshot>(
                datagram, rtype::protocol::MessageType::SNAPSHOT);
        }
    }

    mutable net::UdpSocket _socket;
    asio::ip::udp::endpoint _server;
    std::uint32_t _sequence = 0;
    mutable std::vector<rtype::protocol::Snapshot> _snapshots;
    mutable std::vector<rtype::protocol::Event> _events;
};

}

#endif
