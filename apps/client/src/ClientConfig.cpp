/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientConfig.hpp
*/

#include "client/src/ClientConfig.hpp"
#include <algorithm>
#include <charconv>
#include <CLI/CLI.hpp>
#include <cstdlib>



namespace rtype::client
{
        ClientConfig ClientConfig::parse(std::span<const std::string_view> args)
        {
            ClientConfig config;
            config.name = defaultName();
            CLI::App app{"R-Type Client", "rtype-client"};

            app.set_help_flag("--help", "Display help information");

            app.add_option("-h,--host", config.host, "Server host")
                ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

            app.add_option_function<std::string>("-p,--port", [&config](const std::string &portStr) {
                uint16_t parsedPort = 0;
                const auto [ptr, ec] = std::from_chars(portStr.data(), portStr.data() + portStr.size(), parsedPort);
                if (ec != std::errc{} || ptr != portStr.data() + portStr.size() || parsedPort == 0) {
                    throw CLI::ValidationError("--port", "Invalid port number: " + portStr);
                }
                config.port = parsedPort;
            }, "Server port")
            ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

            app.add_option_function<std::string>("-n,--name", [&config](const std::string &nameStr) {
                std::string n = nameStr;
                if (n.length() > 16) {
                    n = n.substr(0, 16);
                }
                config.name = n;
            }, "Player name")
            ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

            app.add_flag("--fullscreen", config.fullscreen, "Enable fullscreen");

            app.add_option_function<std::string>("--log-level", [&config](const std::string &levelStr) {
                auto levelOpt = engine::core::log::parseLevel(levelStr);
                if (!levelOpt.has_value()) {
                    throw CLI::ValidationError("--log-level", "Unknown log level: " + levelStr);
                }
                config.logLevel = *levelOpt;
            }, "Logging level")
            ->check(CLI::IsMember({"debug", "trace", "warn", "info", "error"}))
            ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
            std::vector<std::string> argList;
            argList.reserve(args.size());
            for (auto sv : args)
                argList.emplace_back(sv);
            try {
                app.parse(argList);
            } catch (const CLI::CallForHelp &) {
                throw HelpRequested{};
            } catch (const CLI::ParseError &e) {
                throw ConfigError(e.what());
            }
            if (config.host.empty())
                throw ConfigError("Empty host provided");
            return config;
        }

        // TODO make it compatible with windows too
        std::string ClientConfig::defaultName()
        {
            #ifdef _WIN32
                char *user = nullptr;
                std::size_t userSize = 0;
                _dupenv_s(&user, &userSize, "USER");
                if (user == nullptr)
                    _dupenv_s(&user, &userSize, "USERNAME");
                if (user == nullptr)
                    return "";
                std::string username(user);
                std::free(user);
                if (username.length() > 16)
                    username = username.substr(0, 16);
                return username;
            #else
                const char *user = std::getenv("USER");

                if (!user)
                    user = std::getenv("USERNAME");
                if (!user)
                    return "";
                std::string username(user);
                if (username.length() > 16)
                    username = username.substr(0, 16);
                return username;
            #endif
        }

} // namespace rtype::client
