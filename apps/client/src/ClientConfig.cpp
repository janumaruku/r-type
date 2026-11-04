/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientConfig.cpp
*/

#include "ClientConfig.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <CLI/CLI.hpp>
#include <cstdlib>
#include <optional>

#include "rtype/protocol/Messages.hpp"

namespace rtype::client {
namespace {
constexpr std::string_view PROGRAM = "r-type-client";

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

/*
** An empty name stays empty, so the server picks "Player N" (RFC § 6.1).
** Any other name is cut to 16 bytes and its unprintable bytes become '?',
** so what the client sends always passes the server's checks.
*/
std::string printableName(std::string_view name)
{
    return name.empty() ? std::string{} : protocol::sanitizeName(name, 0);
}

std::optional<std::string> readEnvironment(const char *name)
{
#ifdef _WIN32
    char *value = nullptr;
    std::size_t size = 0;

    if (_dupenv_s(&value, &size, name) != 0 || value == nullptr)
        return std::nullopt;
    std::string result(value);
    std::free(value);
    return result;
#else
    const char *value = std::getenv(name);

    if (value == nullptr)
        return std::nullopt;
    return std::string(value);
#endif
}
}

std::unique_ptr<CLI::App> makeApp(ClientConfig &config,
    std::string_view program)
{
    auto app = std::make_unique<CLI::App>("R-Type game client",
        std::string(program));

    app->set_help_flag("--help", "Prints usage and exits with 0");

    app->add_option("-h,--host", config.host, "Server host")
       ->capture_default_str()
       ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    app->add_option("-p,--port", config.port,
           "UDP port to bind on to communicate with the server")
       ->type_name("PORT")
       ->check(WHOLE_NUMBER)
       ->check(CLI::Range(1, 65535))
       ->capture_default_str()
       ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    app->add_option_function<std::string>("-n,--name",
           [&config](const std::string &nameStr) {
               config.name = printableName(nameStr);
           }, "Player name")
       ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    app->add_flag("--fullscreen", config.fullscreen, "Enable fullscreen");

    app->add_option_function<std::string>("--log-level",
           [&config](const std::string &name) {
               config.logLevel = engine::core::log::parseLevel(name).value();
           }, "Minimum level printed")
       ->check(CLI::IsMember({"trace", "debug", "info", "warn", "error"}))
       ->default_str("info")
       ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    return app;
}

ClientConfig ClientConfig::parse(std::span<const std::string_view> args)
{
    ClientConfig config;
    config.name = defaultName();
    auto app = makeApp(config, PROGRAM);
    std::vector<std::string> argList(args.rbegin(), args.rend());
    
    try {
        app->parse(argList);
    } catch (const CLI::CallForHelp &) {
        throw HelpRequested{};
    } catch (const CLI::ParseError &e) {
        throw ConfigError(e.what());
    }
    if (config.host.empty())
        throw ConfigError("Empty host provided");
    return config;
}

std::string ClientConfig::defaultName()
{
#ifdef _WIN32
    constexpr std::array<const char *, 2> variables{"USERNAME", "USER"};
#else
    constexpr std::array<const char *, 2> variables{"USER", "USERNAME"};
#endif

    for (const char *variable : variables)
        if (const auto user = readEnvironment(variable))
            return printableName(*user);
    return "";
}

} // namespace rtype::client
