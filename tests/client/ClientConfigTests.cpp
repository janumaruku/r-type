/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientConfigTests.cpp
*/

#include <gtest/gtest.h>

#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ClientConfig.hpp"

namespace rtype::client {
namespace {

ClientConfig parseArgs(std::vector<std::string_view> args)
{
    return ClientConfig::parse(args);
}

std::string configErrorFor(std::vector<std::string_view> args)
{
    try {
        (void)ClientConfig::parse(args);
    } catch (const ConfigError &error) {
        return error.what();
    }
    return "";
}

class ScopedEnv {
public:
    ScopedEnv(const char *name, const char *value): _name(name)
    {
        if (const char *saved = std::getenv(name))
            _saved = saved;
        set(value);
    }

    ~ScopedEnv()
    {
        set(_saved ? _saved->c_str() : nullptr);
    }

    ScopedEnv(const ScopedEnv &) = delete;
    ScopedEnv &operator=(const ScopedEnv &) = delete;

private:
    void set(const char *value) const
    {
#ifdef _WIN32
        _putenv_s(_name, value ? value : "");
#else
        if (value)
            setenv(_name, value, 1);
        else
            unsetenv(_name);
#endif
    }

    const char *_name;
    std::optional<std::string> _saved;
};
}

TEST(ClientConfigTests, NoArgumentGivesTheDefaults)
{
    const ClientConfig config = parseArgs({});

    EXPECT_EQ(config.host, "127.0.0.1");
    EXPECT_EQ(config.port, 4242);
    EXPECT_EQ(config.name, ClientConfig::defaultName());
    EXPECT_FALSE(config.fullscreen);
    EXPECT_EQ(config.logLevel, engine::core::log::Level::EG_INFO);
}

TEST(ClientConfigTests, ShortOptionsAreAllApplied)
{
    const ClientConfig config = parseArgs(
        {"-h", "10.0.0.2", "-p", "5000", "-n", "Alice", "--fullscreen", "--log-level", "warn"});

    EXPECT_EQ(config.host, "10.0.0.2");
    EXPECT_EQ(config.port, 5000);
    EXPECT_EQ(config.name, "Alice");
    EXPECT_TRUE(config.fullscreen);
    EXPECT_EQ(config.logLevel, engine::core::log::Level::EG_WARN);
}

TEST(ClientConfigTests, LongOptionsAreAllApplied)
{
    const ClientConfig config = parseArgs(
        {"--host", "10.0.0.3", "--port", "6000", "--name", "Bob", "--log-level", "debug"});

    EXPECT_EQ(config.host, "10.0.0.3");
    EXPECT_EQ(config.port, 6000);
    EXPECT_EQ(config.name, "Bob");
    EXPECT_EQ(config.logLevel, engine::core::log::Level::EG_DEBUG);
}

TEST(ClientConfigTests, LastOccurrenceOfAnOptionWins)
{
    const ClientConfig config = parseArgs({"-p", "5000", "-h", "a", "-p", "6000", "-h", "b"});

    EXPECT_EQ(config.port, 6000);
    EXPECT_EQ(config.host, "b");
}

TEST(ClientConfigTests, HelpThrowsHelpRequested)
{
    EXPECT_THROW((void)parseArgs({"--help"}), HelpRequested);
    EXPECT_THROW((void)parseArgs({"-p", "5000", "--help"}), HelpRequested);
}

TEST(ClientConfigTests, ShortHIsTheHostNotHelp)
{
    EXPECT_EQ(parseArgs({"-h", "example.org"}).host, "example.org");
}

TEST(ClientConfigTests, HostWithoutValueNamesTheOption)
{
    EXPECT_NE(configErrorFor({"-h"}).find("host"), std::string::npos);
}

TEST(ClientConfigTests, PortOutOfRangeNamesTheOption)
{
    EXPECT_NE(configErrorFor({"-p", "70000"}).find("port"), std::string::npos);
    EXPECT_NE(configErrorFor({"-p", "0"}).find("port"), std::string::npos);
}

TEST(ClientConfigTests, PortThatIsNotAWholeNumberNamesTheOption)
{
    EXPECT_NE(configErrorFor({"-p", "4x"}).find("port"), std::string::npos);
    EXPECT_NE(configErrorFor({"-p", "+5"}).find("port"), std::string::npos);
    EXPECT_NE(configErrorFor({"-p", "0x10"}).find("port"), std::string::npos);
}

TEST(ClientConfigTests, UnknownOptionNamesTheOption)
{
    EXPECT_NE(configErrorFor({"--frobnicate"}).find("frobnicate"), std::string::npos);
}

TEST(ClientConfigTests, EmptyHostIsRejected)
{
    EXPECT_THROW((void)parseArgs({"-h", ""}), ConfigError);
}

TEST(ClientConfigTests, UnknownLogLevelNamesTheOption)
{
    EXPECT_NE(configErrorFor({"--log-level", "loud"}).find("log-level"), std::string::npos);
}

TEST(ClientConfigTests, LongNameIsCutTo16Bytes)
{
    EXPECT_EQ(parseArgs({"-n", "ABCDEFGHIJKLMNOPQRST"}).name, "ABCDEFGHIJKLMNOP");
}

TEST(ClientConfigTests, UnprintableBytesInTheNameBecomeQuestionMarks)
{
    EXPECT_EQ(parseArgs({"-n", "Al\x01" "ce"}).name, "Al?ce");
}

TEST(ClientConfigTests, DefaultNameIsEmptyWithoutUserVariables)
{
    const ScopedEnv user("USER", nullptr);
    const ScopedEnv username("USERNAME", nullptr);

    EXPECT_EQ(ClientConfig::defaultName(), "");
}

TEST(ClientConfigTests, EmptyNameStaysEmptyForTheServerToPick)
{
    EXPECT_EQ(parseArgs({"-n", ""}).name, "");
}

TEST(ClientConfigTests, DefaultNameIsCutTo16Bytes)
{
    const ScopedEnv user("USER", "ABCDEFGHIJKLMNOPQRST");
    const ScopedEnv username("USERNAME", "ABCDEFGHIJKLMNOPQRST");

    EXPECT_EQ(ClientConfig::defaultName(), "ABCDEFGHIJKLMNOP");
}

TEST(ClientConfigTests, DefaultNameIsMadePrintable)
{
    const ScopedEnv user("USER", "Al\x01" "ce");
    const ScopedEnv username("USERNAME", "Al\x01" "ce");

    EXPECT_EQ(ClientConfig::defaultName(), "Al?ce");
}
}
