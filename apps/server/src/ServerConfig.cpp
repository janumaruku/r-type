/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerConfig.cpp
*/

#include "ServerConfig.hpp"

#include <CLI/CLI.hpp>
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "engine/core/Logger.hpp"

namespace rtype::server {

namespace {

constexpr std::string_view PROGRAM = "r-type_server";

/*
** CLI11 converts "+5", "0x10", "0b101", "1_000" or " 5" to a number before
** CLI::Range sees it. This validator runs on the raw text first, so only
** plain decimal digits reach the conversion.
*/
const CLI::Validator WHOLE_NUMBER(
    [](const std::string &value) {
        const bool digitsOnly = !value.empty()
            && std::ranges::all_of(value, [](unsigned char character) {
                return std::isdigit(character) != 0;
            });

        return digitsOnly
            ? std::string{}
            : "'" + value + "' is not a whole number";
    },
    "", "WHOLE_NUMBER");

std::unique_ptr<CLI::App> makeApp(ServerConfig &config,
    std::string_view program)
{
    auto app = std::make_unique<CLI::App>("R-Type game server",
        std::string(program));

    app->set_help_flag("-h,--help", "Prints usage and exits with 0");

    app->add_option("-p,--port", config.port,
           "UDP port to bind on every interface")
       ->type_name("PORT")
       ->check(WHOLE_NUMBER)
       ->check(CLI::Range(1, 65535))
       ->capture_default_str()
       ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    app->add_option("--seed", config.seed,
           "Seeds Rng, for reproducible runs and tests")
       ->type_name("N")
       ->required()
       ->check(WHOLE_NUMBER)
       ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    app->add_option_function<std::string>("--log-level",
           [&config](const std::string &name) {
               config.logLevel = engine::core::log::parseLevel(name).value();
           }, "Minimum level printed")
       ->check(CLI::IsMember({"trace", "debug", "info", "warn", "error"}))
       ->default_str("info")
       ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    return app;
}

}

/*
** CLI11 reads a vector in reverse order. Its CallForHelp becomes
** HelpRequested and any other ParseError a ConfigError, so main() alone
** decides what is printed and with which exit code.
*/
ServerConfig ServerConfig::parse(std::span<const std::string_view> args)
{
    ServerConfig config;
    const auto app = makeApp(config, PROGRAM);
    std::vector<std::string> reversed(args.rbegin(), args.rend());

    try {
        app->parse(reversed);
    } catch (const CLI::CallForHelp &) {
        throw HelpRequested{};
    } catch (const CLI::ParseError &error) {
        throw ConfigError(error.what());
    }
    return config;
}

std::string ServerConfig::usage(std::string_view program)
{
    ServerConfig unused;

    return makeApp(unused, program)->help();
}

}
