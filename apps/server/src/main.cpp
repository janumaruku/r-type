#include <algorithm>
#include <atomic>
#include <csignal>
#include <exception>
#include <iostream>
#include <string_view>
#include <vector>

#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"
#include "ServerApp.hpp"
#include "ServerConfig.hpp"

namespace {

constexpr std::string_view PROGRAM = "r-type_server";

std::atomic<bool> g_stopRequested{false};

static_assert(std::atomic<bool>::is_always_lock_free,
    "a signal handler may only touch lock-free atomics");

void stopServer(int)
{
    g_stopRequested = true;
}

/**
 * @brief Routes SIGINT and SIGTERM to @ref g_stopRequested while it lives,
 * and puts the default handlers back on every exit path, including
 * exceptions.
 *
 * Once it is gone, a signal ends the process again instead of setting a
 * flag nobody reads.
 */
class StopOnSignal {
public:
    StopOnSignal()
    {
        std::signal(SIGINT, stopServer);
        std::signal(SIGTERM, stopServer);
    }

    ~StopOnSignal()
    {
        std::signal(SIGINT, SIG_DFL);
        std::signal(SIGTERM, SIG_DFL);
    }

    StopOnSignal(const StopOnSignal &)            = delete;
    StopOnSignal &operator=(const StopOnSignal &) = delete;
};
} // namespace

int main(int argc, char *argv[])
{
    using rtype::server::ServerConfig;
    const std::vector<std::string_view> args(argv + std::min(argc, 1),
        argv + argc);
    ServerConfig config;

    try {
        config = ServerConfig::parse(args);
    } catch (const rtype::server::HelpRequested &) {
        std::cout << ServerConfig::usage(PROGRAM);
        return 0;
    } catch (const rtype::server::ConfigError &error) {
        std::cerr << error.what() << "\n\n" << ServerConfig::usage(PROGRAM);
        return rtype::server::EPITECH_EXIT_FAIL;
    }
    try {
        const StopOnSignal stopOnSignal;
        rtype::server::ServerApp app(config, g_stopRequested);
        return app.run();
    } catch (const engine::core::NetworkError &error) {
        engine::core::log::Logger("Server").error(error.what());
        return rtype::server::EPITECH_EXIT_FAIL;
    } catch (const std::exception &error) {
        engine::core::log::Logger("Server").error(error.what());
        return rtype::server::EPITECH_EXIT_FAIL;
    }
}
