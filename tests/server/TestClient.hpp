#ifndef SERVER_TEST_CLIENT_HPP
#define SERVER_TEST_CLIENT_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <span>
#include <thread>
#include <utility>
#include <vector>

#include "network/UdpSocket.hpp"
#include "rtype/protocol/Decode.tpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace server_test {

/**
 * @brief A loopback client for the integration tests.
 *
 * Every datagram waits in arrival order until a @ref waitFor reaches it, so
 * no message is lost because it came in the same batch as another one.
 * SNAPSHOT and EVENT are also recorded as they arrive, for @ref lastSnapshot
 * and @ref eventsSoFar. Any other message a @ref waitFor skips is kept in
 * @ref unexpected, and a datagram without a valid header is counted in
 * @ref malformed, so a stray KICK or REJECT can be asserted on.
 */
class TestClient {
public:
    explicit TestClient(std::uint16_t serverPort):
        _socket(0), _server({asio::ip::address_v4::loopback(), serverPort})
    {
        _socket.start();
    }

    ~TestClient()
    {
        _socket.stop();
    }

    TestClient(const TestClient &)            = delete;
    TestClient &operator=(const TestClient &) = delete;

    /**
     * @brief Sends one message. Sequence numbers start at 1 (RFC § 4).
     */
    template <typename Message>
    void send(rtype::protocol::MessageType type, const Message &message)
    {
        ++_sequence;
        _socket.sendTo(_server,
            rtype::protocol::makePacket(type, _sequence, message));
    }

    void sendRaw(std::span<const std::uint8_t> bytes)
    {
        net::Packet packet;
        packet.writeRaw(bytes.data(), bytes.size());
        _socket.sendTo(_server, packet);
    }

    /**
     * @brief Returns the oldest message of `type` not read yet, waiting at
     * most `timeout` for one to arrive.
     *
     * The messages before it are consumed: skipped SNAPSHOT and EVENT stay
     * in their history, any other type goes to @ref unexpected. The ones
     * after it stay queued for the next call.
     * @return std::nullopt on timeout.
     */
    template <typename Message>
    [[nodiscard]] std::optional<Message> waitFor(rtype::protocol::MessageType
                                                     type,
        std::chrono::milliseconds timeout = std::chrono::seconds(2))
    {
        const auto deadline = std::chrono::steady_clock::now() + timeout;

        while (true) {
            pump();
            while (!_inbox.empty()) {
                Incoming incoming = std::move(_inbox.front());
                _inbox.pop_front();
                if (incoming.header.type == type)
                    return rtype::protocol::decode<Message>(incoming.body);
                if (!isRecorded(incoming.header.type))
                    _unexpected.push_back(incoming.header.type);
            }
            if (std::chrono::steady_clock::now() >= deadline)
                return std::nullopt;
            std::this_thread::sleep_for(POLL_INTERVAL);
        }
    }

    [[nodiscard]] std::vector<rtype::protocol::Event> eventsSoFar() const
    {
        pump();
        return _events;
    }

    [[nodiscard]] std::optional<rtype::protocol::Snapshot> lastSnapshot() const
    {
        pump();
        if (_snapshots.empty())
            return std::nullopt;
        return _snapshots.back();
    }

    /**
     * @brief The types a @ref waitFor skipped, other than SNAPSHOT and
     * EVENT, in arrival order.
     */
    [[nodiscard]] const std::vector<rtype::protocol::MessageType> &unexpected()
        const noexcept
    {
        return _unexpected;
    }

    /**
     * @brief How many datagrams had no readable header, or were a SNAPSHOT
     * or EVENT that did not decode.
     */
    [[nodiscard]] std::size_t malformed() const noexcept
    {
        return _malformed;
    }

private:
    static constexpr std::chrono::milliseconds POLL_INTERVAL{1};

    struct Incoming {
        rtype::protocol::Header header;
        net::Packet body;
    };

    [[nodiscard]] static bool isRecorded(rtype::protocol::MessageType type)
    {
        return type == rtype::protocol::MessageType::SNAPSHOT ||
            type == rtype::protocol::MessageType::EVENT;
    }

    void pump() const
    {
        for (const auto &datagram: _socket.receiveAll()) {
            net::Packet packet(datagram.bytes);
            rtype::protocol::Header header{};

            try {
                packet >> header;
                record(header.type, packet);
            } catch (const net::PacketError &) {
                ++_malformed;
                continue;
            }
            _inbox.push_back({header, std::move(packet)});
        }
    }

    /* ** Takes the body by value: decoding moves the read offset, and the
    ** inbox keeps its own copy for waitFor. */
    void record(rtype::protocol::MessageType type, net::Packet body) const
    {
        if (type == rtype::protocol::MessageType::SNAPSHOT)
            _snapshots.push_back(rtype::protocol::decode<
                rtype::protocol::Snapshot>(body));
        else if (type == rtype::protocol::MessageType::EVENT)
            _events.push_back(rtype::protocol::decode<
                rtype::protocol::Event>(body));
    }

    mutable net::UdpSocket _socket;
    asio::ip::udp::endpoint _server;
    std::uint32_t _sequence = 0;
    mutable std::deque<Incoming> _inbox;
    mutable std::vector<rtype::protocol::Snapshot> _snapshots;
    mutable std::vector<rtype::protocol::Event> _events;
    std::vector<rtype::protocol::MessageType> _unexpected;
    mutable std::size_t _malformed = 0;
};

} // namespace server_test

#endif
