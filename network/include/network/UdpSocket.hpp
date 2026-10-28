/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocket.hpp
*/

#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <array>
#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <thread>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <vector>

#include "engine/core/Logger.hpp"
#include "network/Packet.hpp"

namespace net {

struct Datagram {
    std::vector<std::uint8_t> bytes;
    asio::ip::udp::endpoint from;
};

/*
** @struct SocketStats
** @brief What a @ref UdpSocket received, dropped and sent, per cause.
**
** Counts since the socket was constructed, never reset, not even by
** @ref UdpSocket::stop() and @ref UdpSocket::start().
** @ref UdpSocket::stats() reads the 7 values one by one, not as a single
** atomic snapshot, which is enough for statistics.
*/
struct SocketStats {
    std::uint64_t received = 0;
    std::uint64_t droppedEmpty = 0;
    std::uint64_t droppedOversized = 0;
    std::uint64_t droppedQueueFull = 0;
    std::uint64_t receiveErrors = 0;
    std::uint64_t sent = 0;
    std::uint64_t sendErrors = 0;
};

class UdpSocket {
public:
    static constexpr std::size_t MAX_PACKET_SIZE = 1200;
    static constexpr std::size_t MAX_QUEUED_DATAGRAMS = 4096;

    explicit UdpSocket(std::uint16_t port = 0);
    ~UdpSocket();

    UdpSocket() = delete;
    UdpSocket(const UdpSocket &) = delete;
    UdpSocket(UdpSocket &&) = delete;

    UdpSocket &operator=(const UdpSocket &) = delete;
    UdpSocket &operator=(UdpSocket &&) = delete;

    void start();
    void stop();

    void sendTo(const asio::ip::udp::endpoint &endpoint, const Packet &packet);
    [[nodiscard]] std::vector<Datagram> receiveAll(std::size_t max = 1024);

    [[nodiscard]] asio::ip::udp::endpoint localEndpoint() const;
    [[nodiscard]] SocketStats stats() const;
    [[nodiscard]] std::uint64_t dropped() const;
    [[nodiscard]] bool isReceiving() const noexcept;

private:
    static constexpr std::size_t MAX_RECEIVE_ERRORS = 3;
    static constexpr int SYSTEM_RECEIVE_BUFFER_BYTES = 4 * 1024 * 1024;

    engine::core::log::Logger _logger{"UDP-socket"};

    std::atomic<bool> _running{false};
    std::atomic<bool> _receiving{false};
    std::size_t _receiveErrors = 0;

    /*
    ** @struct Counters
    ** @brief The atomic counters behind @ref SocketStats.
    **
    ** Written only on the IO thread and read from any thread, so relaxed
    ** atomics are enough.
    */
    struct Counters {
        std::atomic<std::uint64_t> received{0};
        std::atomic<std::uint64_t> droppedEmpty{0};
        std::atomic<std::uint64_t> droppedOversized{0};
        std::atomic<std::uint64_t> droppedQueueFull{0};
        std::atomic<std::uint64_t> receiveErrors{0};
        std::atomic<std::uint64_t> sent{0};
        std::atomic<std::uint64_t> sendErrors{0};
    };

    /*
    ** @struct LogLimiter
    ** @brief Turns a stream of events into at most one log line per second.
    **
    ** Used only on the IO thread. @ref record counts one event and returns
    ** how many to log now, or 0 if the last line is less than a second old.
    ** Events after the last line are logged only when a new event comes.
    */
    struct LogLimiter {
        std::chrono::steady_clock::time_point last{};
        std::uint64_t pending = 0;

        [[nodiscard]] std::uint64_t record(
            std::chrono::steady_clock::time_point now);
    };

    Counters _counters;
    LogLimiter _queueFullLog;
    LogLimiter _oversizedLog;
    LogLimiter _sendErrorLog;

    void countReceiveError();
    void receiveNext();
    void handleReceive(const asio::error_code &error,
        std::size_t bytesReceived);
    void handleReceiveError(const asio::error_code &error);
    [[nodiscard]] bool enqueueDatagram(std::size_t bytesReceived);
    void dropOversized();
    void rearmReceive();
    void retryReceiveLater();
    void giveUpReceiving();

    std::uint16_t _port = 0;
    asio::io_context _ioContext;
    /*
    ** @brief Keeps @ref _ioContext's run() from returning when no operation
    ** is pending, as after @ref giveUpReceiving(), so that @ref sendTo()
    ** still works.
    **
    ** An empty optional before @ref start() and after @ref stop(), since a
    ** work guard can't be re-armed.
    */
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>>
        _workGuard;
    asio::ip::udp::socket _socket;

    std::thread _thread;
    std::array<std::uint8_t, 1500> _receiveBuffer;

    asio::ip::udp::endpoint _sender;
    std::mutex _mutex;
    std::deque<Datagram> _incoming;
};

} // namespace net

#endif
