#include "ServerApp.hpp"

#include <chrono>
#include <filesystem>
#include <thread>
#include <utility>

#include "engine/core/Logger.hpp"

namespace rtype::server {

namespace {
constexpr std::chrono::milliseconds SEND_TIMEOUT{200};

/*
** --rules names a file that exists, checked by ServerConfig::parse. Without
** it, config/rules.json is read when it exists, and the built-in defaults
** are used otherwise. A file the loader refuses throws ConfigError, which
** main turns into exit 84.
*/
game::Rules loadRules(const ServerConfig &config)
{
    engine::core::log::Logger logger("ServerApp");
    const std::filesystem::path path =
        config.rulesPath.value_or(DEFAULT_RULES_PATH);

    if (!config.rulesPath && !std::filesystem::exists(path)) {
        logger.info("{}: not found, using the built-in rules", path.string());
        return {};
    }
    game::Rules rules = game::Rules::load(path);

    logger.info("rules loaded from {}", path.string());
    return rules;
}
}

ServerApp::ServerApp(ServerConfig config,
    const std::atomic<bool> &stopRequested):
    _config(std::move(config)), _socket(_config.port),
    _world(loadRules(_config), _config.seed), _adapter(_socket, _world, _sessions),
    _ticker(TICK_PERIOD), _stopRequested(stopRequested)
{
    engine::core::log::setLevel(_config.logLevel);
    _logger.info("server seed {}", _config.seed);
    _socket.start();
}

std::uint16_t ServerApp::port() const noexcept
{
    return _socket.localEndpoint().port();
}

int ServerApp::run()
{
    while (!_stopRequested) {
        tick();
        _ticker.waitNext();
    }
    const auto before = _socket.stats();
    const auto queued = _adapter.broadcastShutdown();

    waitForSends(before.sent + before.sendErrors + queued);
    _socket.stop();
    return 0;
}

void ServerApp::waitForSends(std::uint64_t target)
{
    const auto deadline = Clock::now() + SEND_TIMEOUT;

    while (Clock::now() < deadline) {
        const auto stats = _socket.stats();

        if (stats.sent + stats.sendErrors >= target)
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    _logger.warn("Shutdown: not every SHUTDOWN left within {} ms",
        SEND_TIMEOUT.count());
}

void ServerApp::tick()
{
    const auto now = Clock::now();
    for (const auto &datagram: _socket.receiveAll())
        _adapter.handle(datagram, now);
    for (const auto &endpoint: _sessions.timedOut(now))
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
    const auto socket   = _socket.stats();
    const auto &adapter = _adapter.stats();
    const auto offences = adapter.totalBadMagic + adapter.totalBadSize +
        adapter.totalUnknownType + adapter.totalWrongDirection +
        adapter.totalTruncated + adapter.totalTrailingBytes +
        adapter.totalOutOfRange;
    _logger.info("players {}, entities {}, overruns {}, in {}, out {}, "
                 "dropped {}/{}/{}, offences {}, kicks {}",
        _sessions.size(), entities.size(), _ticker.overruns(), socket.received,
        socket.sent, socket.droppedEmpty, socket.droppedOversized,
        socket.droppedQueueFull, offences, adapter.totalKicked);
}

} // namespace rtype::server
