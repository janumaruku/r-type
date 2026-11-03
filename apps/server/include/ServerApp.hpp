#ifndef SERVER_APP_HPP
#define SERVER_APP_HPP

#include <atomic>
#include <cstdint>

#include "ServerConfig.hpp"
#include "ServerNetworkAdapter.hpp"
#include "engine/runtime/FixedRateTicker.hpp"

namespace rtype::server {

class ServerApp {
public:
    explicit ServerApp(ServerConfig config);
    ~ServerApp();

    ServerApp(const ServerApp &) = delete;
    ServerApp &operator=(const ServerApp &) = delete;

    [[nodiscard]] std::uint16_t port() const noexcept;
    [[nodiscard]] int run();
    void requestStop() noexcept;

private:
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
    std::atomic<bool> _running{true};
    std::uint32_t _ticks = 0;
    engine::core::log::Logger _logger{"ServerApp"};
};

}

#endif
