#include "ServerApp.hpp"

#include <chrono>
#include <thread>
#include <utility>

#include "engine/core/Rng.hpp"
#include "engine/core/Logger.hpp"

namespace rtype::server {

namespace {
ServerConfig withSeed(ServerConfig config)
{
    if (!config.seed)
        config.seed = engine::core::Rng::randomSeed();
    return config;
}
}

ServerApp::ServerApp(ServerConfig config):
    _config(withSeed(std::move(config))),
    _socket(_config.port),
    _world(game::Rules{}, *_config.seed),
    _adapter(_socket, _world, _sessions),
    _ticker(std::chrono::milliseconds(1000 / 60))
{
    engine::core::log::setLevel(_config.logLevel);
    _logger.info("server seed {}", *_config.seed);
    _socket.start();
}

ServerApp::~ServerApp()
{
    _socket.stop();
}

std::uint16_t ServerApp::port() const noexcept
{
    return _socket.localEndpoint().port();
}

int ServerApp::run()
{
    while (_running) {
        tick();
        _ticker.waitNext();
    }
    _adapter.broadcastShutdown();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    _socket.stop();
    return 0;
}

void ServerApp::requestStop() noexcept
{
    _running = false;
}

void ServerApp::tick()
{
    const auto now = Clock::now();
    for (const auto &datagram : _socket.receiveAll())
        _adapter.handle(datagram, now);
    for (const auto &endpoint : _sessions.timedOut(now))
        _adapter.removeClient(endpoint, game::LeaveReason::TIMEOUT);
    _world.step(1.0F / 60.0F);
    _adapter.publish(_world.drainEvents());
    _adapter.flushEvents();
    _adapter.flushGoodbyes();
    ++_ticks;
    if (_ticks % 600 == 0)
        logStats();
    if (_ticks % 2 == 0)
        _adapter.sendSnapshots();
}

void ServerApp::logStats()
{
    std::vector<game::ReplicatedEntity> entities;
    _world.collectReplicated(entities);
    const auto socket = _socket.stats();
    const auto &adapter = _adapter.stats();
    const auto offences = adapter.totalBadMagic + adapter.toatBadSize +
        adapter.totalUnknownType + adapter.totalWrongDirection +
        adapter.totalTruncated + adapter.totalTrailingBytes +
        adapter.totalOutOfRange;
    _logger.info("players {}, entities {}, overruns {}, in {}, out {}, "
        "dropped {}/{}/{}, offences {}, kicks {}",
        _sessions.size(), entities.size(), _ticker.overruns(), socket.received,
        socket.sent, socket.droppedEmpty, socket.droppedOversized,
        socket.droppedQueueFull, offences, adapter.totalKicked);
}

}
