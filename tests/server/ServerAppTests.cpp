#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

#include "ServerApp.hpp"
#include "engine/core/Errors.hpp"

namespace rtype::server {
/**
 * @brief Drives one ServerApp tick at a time, and reads the counters the
 * tests check.
 *
 * @ref stalledTick stands for a tick whose work takes `stall`: the delay
 * falls between tick() and waitNext(), as it would inside run().
 */
struct ServerAppTestAccess {
    static void tick(ServerApp &app)
    {
        app.tick();
        app._ticker.waitNext();
    }

    static void stalledTick(ServerApp &app, std::chrono::nanoseconds stall)
    {
        app.tick();
        std::this_thread::sleep_for(stall);
        app._ticker.waitNext();
    }

    [[nodiscard]] static std::uint64_t overruns(const ServerApp &app)
    {
        return app._ticker.overruns();
    }

    [[nodiscard]] static std::uint32_t ticks(const ServerApp &app)
    {
        return app._ticks;
    }

    [[nodiscard]] static net::SocketStats socketStats(const ServerApp &app)
    {
        return app._socket.stats();
    }
};
}

namespace {

using namespace std::chrono_literals;
using rtype::server::ServerApp;
using rtype::server::ServerAppTestAccess;
using Clock = std::chrono::steady_clock;

constexpr auto PERIOD = ServerApp::TICK_PERIOD;

rtype::server::ServerConfig testConfig()
{
    rtype::server::ServerConfig config;
    config.port = 0;
    config.seed = 1;
    return config;
}

TEST(ServerAppTests, StopsCleanly)
{
    std::atomic<bool> stopRequested{false};
    ServerApp app(testConfig(), stopRequested);
    int result = -1;
    std::thread server([&] { result = app.run(); });

    std::this_thread::sleep_for(100ms);
    const auto stopAt = Clock::now();
    stopRequested = true;
    server.join();

    EXPECT_LT(Clock::now() - stopAt, 500ms);
    EXPECT_EQ(result, 0);
}

TEST(ServerAppTests, RejectsBusyPort)
{
    const std::atomic<bool> stopRequested{false};
    ServerApp first(testConfig(), stopRequested);

    rtype::server::ServerConfig secondConfig;
    secondConfig.port = first.port();
    EXPECT_THROW({
        ServerApp second(secondConfig, stopRequested);
    }, engine::core::NetworkError);
}

/*
** The ticker drops a backlog over 250 ms, so a 300 ms stall must count one
** overrun. The 30 ticks after it must neither burst to catch up (at least
** 29 periods) nor stall again (no new overrun, well under a second).
*/
TEST(ServerAppTests, AStalledTickOverrunsOnceThenKeepsTheRate)
{
    constexpr int TICKS_AFTER = 30;
    const std::atomic<bool> stopRequested{false};
    ServerApp app(testConfig(), stopRequested);
    for (int warmUp = 0; warmUp != 5; ++warmUp)
        ServerAppTestAccess::tick(app);
    const auto before = ServerAppTestAccess::overruns(app);

    ServerAppTestAccess::stalledTick(app, 300ms);

    EXPECT_EQ(ServerAppTestAccess::overruns(app), before + 1);
    const auto start = Clock::now();
    for (int tick = 0; tick != TICKS_AFTER; ++tick)
        ServerAppTestAccess::tick(app);
    const auto elapsed = Clock::now() - start;
    EXPECT_GE(elapsed, (TICKS_AFTER - 1) * PERIOD);
    EXPECT_LT(elapsed, 1s);
    EXPECT_EQ(ServerAppTestAccess::overruns(app), before + 1);
}

/*
** "About 60 ticks" is checked against the time the server really ran, so a
** slow machine that oversleeps the 1 s doesn't fail it. Without a client
** there is no one to send a snapshot or an event to.
*/
TEST(ServerAppTests, OneSecondWithoutClientsTicksSixtyTimesAndSendsNothing)
{
    std::atomic<bool> stopRequested{false};
    ServerApp app(testConfig(), stopRequested);
    int result = -1;
    const auto start = Clock::now();
    std::thread server([&] { result = app.run(); });

    std::this_thread::sleep_for(1s);
    const auto ran = Clock::now() - start;
    stopRequested = true;
    server.join();

    const auto expected = static_cast<std::int64_t>(ran / PERIOD);
    const auto ticks = static_cast<std::int64_t>(
        ServerAppTestAccess::ticks(app));
    EXPECT_GE(ticks, expected - 3) << "ran " << ran / 1ms << " ms";
    EXPECT_LE(ticks, expected + 2) << "ran " << ran / 1ms << " ms";
    EXPECT_GE(ticks, 55);
    const auto stats = ServerAppTestAccess::socketStats(app);
    EXPECT_EQ(stats.sent, 0U);
    EXPECT_EQ(stats.sendErrors, 0U);
    EXPECT_EQ(result, 0);
}

}
