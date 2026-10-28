/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerConfigTests.cpp
*/

#include "ServerConfig.hpp"

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

using rtype::server::ConfigError;
using rtype::server::HelpRequested;
using rtype::server::ServerConfig;
using engine::core::log::Level;

ServerConfig parseArgs(std::initializer_list<std::string_view> args)
{
    const std::vector<std::string_view> views(args);
    return ServerConfig::parse(views);
}

// Expects a ConfigError whose message names `option`.
void expectConfigError(
    std::initializer_list<std::string_view> args,
    std::string_view option)
{
    try {
        parseArgs(args);
        ADD_FAILURE() << "expected ConfigError";
    } catch (const ConfigError& error) {
        const std::string message = error.what();
        EXPECT_NE(message.find(option), std::string::npos) << message;
    }
}

TEST(ServerConfigTests, NoArgumentsUsesDefaults)
{
    const ServerConfig config = parseArgs({});

    EXPECT_EQ(config.port, 4242);
    EXPECT_FALSE(config.seed.has_value());
    EXPECT_EQ(config.logLevel, Level::EG_INFO);
}

TEST(ServerConfigTests, ParsesAllOptions)
{
    const ServerConfig config = parseArgs(
        {"-p", "5000", "--seed", "7", "--log-level", "debug"});

    EXPECT_EQ(config.port, 5000);
    ASSERT_TRUE(config.seed.has_value());
    EXPECT_EQ(*config.seed, 7U);
    EXPECT_EQ(config.logLevel, Level::EG_DEBUG);
}

TEST(ServerConfigTests, AcceptsLongPortForm)
{
    EXPECT_EQ(parseArgs({"--port", "4243"}).port, 4243);
}

TEST(ServerConfigTests, AcceptsPortBoundaries)
{
    EXPECT_EQ(parseArgs({"--port", "1"}).port, 1);
    EXPECT_EQ(parseArgs({"--port", "65535"}).port, 65535);
}

TEST(ServerConfigTests, AcceptsSeedBoundaries)
{
    const ServerConfig low = parseArgs({"--seed", "0"});
    const ServerConfig high = parseArgs({"--seed", "4294967295"});

    ASSERT_TRUE(low.seed.has_value());
    ASSERT_TRUE(high.seed.has_value());
    EXPECT_EQ(*low.seed, 0U);
    EXPECT_EQ(*high.seed, 4294967295U);
}

TEST(ServerConfigTests, LastOccurrenceWins)
{
    EXPECT_EQ(parseArgs({"-p", "1", "-p", "2"}).port, 2);

    const ServerConfig config = parseArgs({"--seed", "1", "--seed", "2"});
    ASSERT_TRUE(config.seed.has_value());
    EXPECT_EQ(*config.seed, 2U);
}

class PortRejected : public testing::TestWithParam<std::string> {};

TEST_P(PortRejected, ThrowsNamingThePort)
{
    expectConfigError(
        {"--port", std::string_view{GetParam()}},
        "--port");
}

INSTANTIATE_TEST_SUITE_P(
    ServerConfigTests,
    PortRejected,
    testing::Values(
        std::string{"0"},
        std::string{"65536"},
        std::string{"42x"},
        std::string{""},
        std::string{"-1"}));

class SeedRejected : public testing::TestWithParam<std::string> {};

TEST_P(SeedRejected, ThrowsNamingTheSeed)
{
    expectConfigError(
        {"--seed", std::string_view{GetParam()}},
        "--seed");
}

INSTANTIATE_TEST_SUITE_P(
    ServerConfigTests,
    SeedRejected,
    testing::Values(
        std::string{"4294967296"},
        std::string{"-1"},
        std::string{"+5"},
        std::string{"42x"},
        std::string{""}));

TEST(ServerConfigTests, RejectsUnknownLogLevel)
{
    expectConfigError({"--log-level", "loud"}, "--log-level");
}

TEST(ServerConfigTests, RejectsUnknownOption)
{
    expectConfigError({"--frobnicate"}, "--frobnicate");
}

TEST(ServerConfigTests, RejectsMissingValue)
{
    expectConfigError({"-p"}, "-p");
    expectConfigError({"--port"}, "--port");
    expectConfigError({"--seed"}, "--seed");
    expectConfigError({"--log-level"}, "--log-level");
}

TEST(ServerConfigTests, HelpFlagsThrowHelpRequested)
{
    EXPECT_THROW(parseArgs({"-h"}), HelpRequested);
    EXPECT_THROW(parseArgs({"--help"}), HelpRequested);
}

TEST(ServerConfigTests, HelpWinsAfterOtherOptions)
{
    EXPECT_THROW(parseArgs({"-p", "5000", "--help"}), HelpRequested);
}

TEST(ServerConfigTests, HelpWinsOverEarlierErrors)
{
    EXPECT_THROW(parseArgs({"--frobnicate", "-h"}), HelpRequested);
}

TEST(ServerConfigTests, UsageNamesProgramAndOptions)
{
    const std::string usage = ServerConfig::usage("r-type_server");

    EXPECT_NE(usage.find("r-type_server"), std::string::npos);

    for (const std::string_view option :
         {"--port", "--seed", "--log-level", "--help"}) {
        EXPECT_NE(usage.find(option), std::string::npos) << option;
    }
}