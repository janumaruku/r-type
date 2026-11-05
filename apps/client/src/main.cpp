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
        engine::core::log::Logger("client").error("{}", error.what());
        return EPITECH_EXIT_FAIL;
    }
}

/**
*Validation was cut on purpose. After your change, these all load without error: a variant above 255 (it silently wraps to another variant), a negative frameTime, an old frames field, an empty state or state name, a rectangle outside its sheet, the same key twice, and a catalog missing a server key. The ticket lists every one as a CatalogError. I’d keep two of them: the variant range, so a typo can’t silently draw another player’s colour, and the final check for required keys. Without that one, a missing ship colour fails mid-game at the first find, which is what the ticket’s “at start-up, never mid-game” rule is about.
Error messages don’t name the entry. The ticket wants sprites[2].states.up[1]: …. Defining JSON_DIAGNOSTICS=1 on client_logic gets most of that back for free.
The states after idle are stored alphabetically, not in file order, because nlohmann::json sorts object keys.
 */
