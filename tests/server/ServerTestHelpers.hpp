#ifndef SERVER_TEST_HELPERS_HPP
#define SERVER_TEST_HELPERS_HPP

#include <asio.hpp>
#include <chrono>
#include <span>
#include <thread>
#include <vector>

#include "rtype/game/Rules.hpp"
#include "rtype/protocol/Decode.tpp"
#include "rtype/protocol/Header.hpp"
#include "ServerNetworkAdapter.hpp"

namespace server_test {

using namespace std::chrono_literals;
using rtype::server::TimePoint;

/** @brief Default rules, but the game starts as soon as one player joins. */
inline rtype::game::Rules soloRules()
{
    rtype::game::Rules rules;
    rules.playersToStart = 1;
    return rules;
}

inline asio::ip::udp::endpoint endpointFor(unsigned index)
{
    return {asio::ip::address_v4::loopback(),
        static_cast<unsigned short>(20000U + index)};
}

template <typename Message>
net::Datagram datagram(const asio::ip::udp::endpoint &from,
    rtype::protocol::MessageType type, const Message &message,
    std::uint32_t sequence = 0)
{
    const auto packet = rtype::protocol::makePacket(type, sequence, message);
    return {
        std::vector<std::uint8_t>(packet.bytes().begin(), packet.bytes().end()),
        from};
}

template <typename Message>
net::Datagram datagramWithVersion(const asio::ip::udp::endpoint &from,
    rtype::protocol::MessageType type, const Message &message,
    std::uint8_t version)
{
    net::Packet packet;
    packet << rtype::protocol::Header{.version = version, .type = type}
           << message;
    return {
        std::vector<std::uint8_t>(packet.bytes().begin(), packet.bytes().end()),
        from};
}

inline std::vector<net::Datagram> receive(asio::ip::udp::socket &socket,
    std::size_t count, std::chrono::milliseconds timeout = 1s)
{
    socket.non_blocking(true);
    std::vector<net::Datagram> result;
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::array<std::uint8_t, 1200> bytes{};
    while (result.size() < count &&
        std::chrono::steady_clock::now() < deadline) {
        asio::ip::udp::endpoint from;
        asio::error_code error;
        const auto size =
            socket.receive_from(asio::buffer(bytes), from, 0, error);
        if (!error) {
            result.push_back({std::vector<std::uint8_t>(bytes.begin(),
                                  bytes.begin() + size),
                from});
        } else if (error != asio::error::would_block) {
            break;
        } else {
            std::this_thread::sleep_for(1ms);
        }
    }
    return result;
}

inline rtype::protocol::Header readHeader(const net::Datagram &datagram)
{
    net::Packet packet(datagram.bytes);
    rtype::protocol::Header header;

    packet >> header;
    return header;
}

template <typename Message>
Message readMessage(const net::Datagram &datagram)
{
    net::Packet packet(datagram.bytes);
    (void)packet.read<rtype::protocol::Header>();
    return rtype::protocol::decode<Message>(packet);
}

struct Fixture {
    Fixture():
        socket(0), world(soloRules(), 1),
        adapter(socket, world, sessions),
        client(io, {asio::ip::address_v4::loopback(), 0})
    {
        socket.start();
    }

    ~Fixture()
    {
        socket.stop();
    }

    void send(const net::Datagram &datagram)
    {
        adapter.handle(datagram, now);
    }

    net::UdpSocket socket;
    rtype::game::GameWorld world;
    rtype::server::SessionManager sessions;
    rtype::server::ServerNetworkAdapter adapter;
    asio::io_context io;
    asio::ip::udp::socket client;
    TimePoint now{};
};

} // namespace server_test

#endif
