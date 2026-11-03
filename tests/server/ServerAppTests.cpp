#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "ServerApp.hpp"
#include "engine/core/Errors.hpp"

namespace {

using namespace std::chrono_literals;

TEST(ServerAppTests, StopsCleanly)
{
    rtype::server::ServerConfig config;
    config.port = 0;
    config.seed = 1;
    rtype::server::ServerApp app(config);
    int result = -1;
    std::thread server([&] { result = app.run(); });

    std::this_thread::sleep_for(100ms);
    app.requestStop();
    server.join();

    EXPECT_EQ(result, 0);
}

TEST(ServerAppTests, RejectsBusyPort)
{
    rtype::server::ServerConfig firstConfig;
    firstConfig.port = 0;
    rtype::server::ServerApp first(firstConfig);

    rtype::server::ServerConfig secondConfig;
    secondConfig.port = first.port();
    EXPECT_THROW({
        rtype::server::ServerApp second(secondConfig);
    }, engine::core::NetworkError);
}

}
