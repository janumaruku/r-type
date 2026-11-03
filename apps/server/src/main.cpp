#include <utility>
#include <csignal>

#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"
#include "ServerApp.hpp"
#include "ServerConfig.hpp"

namespace {
rtype::server::ServerApp *g_app = nullptr;

void stopServer(int)
{
    if (g_app != nullptr)
        g_app->requestStop();
}
}

int main(int argc, char *argv[])
{
    rtype::server::ServerConfig config;
    const int result = parse(argc, argv, config);
    if (result != 0)
        return result;
    try {
        rtype::server::ServerApp app(config);
        g_app = &app;
        std::signal(SIGINT, stopServer);
        std::signal(SIGTERM, stopServer);
        const int exitCode = app.run();
        g_app = nullptr;
        return exitCode;
    } catch (const engine::core::NetworkError &error) {
        engine::core::log::Logger("Server").error(error.what());
        return rtype::server::EPITECH_EXIT_FAIL;
    } catch (const std::exception &error) {
        engine::core::log::Logger("Server").error(error.what());
        return rtype::server::EPITECH_EXIT_FAIL;
    }
}
