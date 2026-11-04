#include <atomic>
#include <exception>
#include <stdexcept>
#include <utility>
#include <csignal>

#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"
#include "ServerApp.hpp"
#include "ServerConfig.hpp"

namespace {

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

    StopOnSignal(const StopOnSignal &) = delete;
    StopOnSignal &operator=(const StopOnSignal &) = delete;
};
}

int main(int argc, char *argv[])
{
    rtype::server::ServerConfig config;
    if (const auto result = parse(argc, argv, config))
        return result.value();
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
