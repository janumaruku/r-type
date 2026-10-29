/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerConfig.cpp
*/

#include "ServerConfig.hpp"

#include <algorithm>
#include <charconv>
#include <CLI/CLI.hpp>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <system_error>
#include <type_traits>
#include <limits>

#include "engine/core/Logger.hpp"

namespace rtype::server {
int parse(int argc, char *const argv[], ServerConfig &serverConfig)
{
    CLI::App app{"R-type game server", "rtype-server"};

    app.add_option("-p,--port", serverConfig.port,
           "Server listening on port (default 4242)")
        ->check(CLI::Range(0, 65535))
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
    ;

    app.add_option_function<std::string>(
    "--seed",
    [&serverConfig](const std::string& value) {
        std::uint32_t seed{};
        const auto [ptr, ec] = std::from_chars(
            value.data(), value.data() + value.size(), seed);

        if (value.empty() || ec != std::errc{} ||
            ptr != value.data() + value.size())
            throw CLI::ValidationError("--seed", "Invalid seed");

        serverConfig.seed = seed;
    },
    "Seed for random number generator")$
    ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    app.add_option_function<std::string>(
           "--log-level",
           [&serverConfig](const std::string &name) {
               serverConfig.logLevel = *engine::core::log::parseLevel(name);
           },
           "Logging level")
        ->check(CLI::IsMember({"debug", "trace", "warn", "info", "error"}))
        ->default_str("info")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    try {
        app.parse(argc, argv);
    } catch (const CLI::Error &e) {
        if (e.get_exit_code() == 0) {
            std::cout << app.help() << std::endl;
            return 0;
        }
        std::cout << e.what() << "\n\n" << app.help() << std::endl;
        return 84;
    }
    return 0;
}
} // namespace rtype::server
