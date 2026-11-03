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
