/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** main
*/

#include <algorithm>
#include <exception>
#include <iostream>
#include <string_view>
#include <vector>

#include "ClientApp.hpp"
#include "ClientConfig.hpp"
#include "ClientSettings.hpp"
#include "engine/core/Logger.hpp"

namespace {
constexpr std::string_view PROGRAM = "r-type_client";
constexpr int EPITECH_EXIT_FAIL = 84;
constexpr const char *SETTINGS_FILE = "config/client.json";
}

int main(int argc, char *argv[])
{
    using rtype::client::ClientConfig;
    const std::vector<std::string_view> args(argv + std::min(argc, 1),
        argv + argc);
    ClientConfig config;

    try {
        config = ClientConfig::parse(args);
    } catch (const rtype::client::HelpRequested &) {
        std::cout << ClientConfig::usage(PROGRAM);
        return 0;
    } catch (const rtype::client::ConfigError &error) {
        std::cerr << error.what() << "\n\n" << ClientConfig::usage(PROGRAM);
        return EPITECH_EXIT_FAIL;
    }
    engine::core::log::setLevel(config.logLevel);
    try {
        auto settings = rtype::client::ClientSettings::load(SETTINGS_FILE);

        settings.fullscreen = settings.fullscreen || config.fullscreen;
        rtype::client::ClientApp app(config, settings);
        return app.run();
    } catch (const std::exception &error) {
        engine::core::log::Logger("client").error(error.what());
        return EPITECH_EXIT_FAIL;
    }
}
