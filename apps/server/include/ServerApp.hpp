#ifndef SERVER_APP_HPP
#define SERVER_APP_HPP

#include <atomic>
#include <chrono>
#include <cstdint>

#include "ServerConfig.hpp"
#include "ServerNetworkAdapter.hpp"
#include "engine/runtime/FixedRateTicker.hpp"

namespace rtype::server {

struct ServerAppTestAccess;

class ServerApp {
public:
    /**
     * @brief One tick every 1/60 s. Converted to nanoseconds before the
     * division, which would give 0 in seconds and 16 ms in milliseconds.
     */
    static constexpr std::chrono::nanoseconds TICK_PERIOD =
        std::chrono::nanoseconds(std::chrono::seconds(1)) / 60;

    /**
     * @brief Binds the port and starts the socket. run() returns once
     * `stopRequested` becomes true.
     *
     * `stopRequested` must outlive the app. main() sets it from its
     * SIGINT/SIGTERM handler, tests set it from their own thread.
     */
    ServerApp(ServerConfig config, const std::atomic<bool> &stopRequested);

    ServerApp(const ServerApp &) = delete;
    ServerApp &operator=(const ServerApp &) = delete;

    [[nodiscard]] std::uint16_t port() const noexcept;
    /**
     * @brief Ticks until the stop flag is set, sends SHUTDOWN, then stops
     * the socket so the port is free as soon as it returns.
     *
     * An app that never ran, or whose run() threw, is stopped by
     * UdpSocket's own destructor: ServerApp needs none.
     * @return 0 on a clean stop.
     */
    [[nodiscard]] int run();

private:
    friend struct ServerAppTestAccess;

    void tick();
    void logStats();

    /**
     * @brief Waits until the socket has sent (or failed to send) `target`
     * datagrams in total, or 200 ms have passed. UdpSocket::stop() stops
     * its IO context right away and may drop sends still queued.
     */
    void waitForSends(std::uint64_t target);

    ServerConfig _config;
    net::UdpSocket _socket;
    game::GameWorld _world;
    SessionManager _sessions;
    ServerNetworkAdapter _adapter;
    engine::runtime::FixedRateTicker _ticker;
    const std::atomic<bool> &_stopRequested;
    std::uint32_t _ticks = 0;
    engine::core::log::Logger _logger{"ServerApp"};
};

}

#endif
