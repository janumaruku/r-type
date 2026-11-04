/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerConfigTests.cpp
*/

#include "ServerConfig.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

using rtype::server::ConfigError;
using rtype::server::HelpRequested;
using rtype::server::ServerConfig;
using engine::core::log::Level;

namespace {

ServerConfig parseArgs(std::initializer_list<std::string_view> args)
{
    const std::vector<std::string_view> arguments(args);

    return ServerConfig::parse(arguments);
}

void expectConfigError(
    std::initializer_list<std::string_view> args,
    std::string_view option)
{
    try {
        (void)parseArgs(args);
        ADD_FAILURE() << "no ConfigError for " << option;
    } catch (const ConfigError &error) {
        EXPECT_NE(std::string_view{error.what()}.find(option),
            std::string_view::npos) << error.what();
    }
}

} // namespace

TEST(ServerConfigTests, NoArgumentsIsAnErrorNamingTheSeed)
{
    expectConfigError({}, "--seed");
}

TEST(ServerConfigTests, OnlyTheSeedUsesDefaults)
{
    const ServerConfig config = parseArgs({"--seed", "3"});

    EXPECT_EQ(config.port, 4242);
    EXPECT_EQ(config.seed, 3U);
    EXPECT_EQ(config.logLevel, Level::EG_INFO);
}

TEST(ServerConfigTests, OptionsWithoutTheSeedAreAnError)
{
    expectConfigError({"-p", "5000", "--log-level", "debug"}, "--seed");
}

TEST(ServerConfigTests, ParsesAllOptions)
{
    const ServerConfig config = parseArgs(
        {"-p", "5000", "--seed", "7", "--log-level", "debug"});

    EXPECT_EQ(config.port, 5000);
    EXPECT_EQ(config.seed, 7U);
    EXPECT_EQ(config.logLevel, Level::EG_DEBUG);
}

TEST(ServerConfigTests, AcceptsLongPortForm)
{
    EXPECT_EQ(parseArgs({"--port", "4243", "--seed", "1"}).port, 4243);
}

TEST(ServerConfigTests, AcceptsPortBoundaries)
{
    EXPECT_EQ(parseArgs({"--port", "1", "--seed", "1"}).port, 1);
    EXPECT_EQ(parseArgs({"--port", "65535", "--seed", "1"}).port, 65535);
}

TEST(ServerConfigTests, AcceptsSeedBoundaries)
{
    const ServerConfig low = parseArgs({"--seed", "0"});
    const ServerConfig high = parseArgs({"--seed", "4294967295"});

    EXPECT_EQ(low.seed, 0U);
    EXPECT_EQ(
        high.seed,
        std::numeric_limits<std::uint32_t>::max());
}

TEST(ServerConfigTests, LastOccurrenceWins)
{
    EXPECT_EQ(parseArgs({"-p", "1", "-p", "2", "--seed", "1"}).port, 2);
    EXPECT_EQ(parseArgs({"--seed", "1", "--seed", "2"}).seed, 2U);
    EXPECT_EQ(parseArgs({"--log-level", "debug", "--log-level", "info",
        "--seed", "1"}).logLevel, Level::EG_INFO);
}

class PortRejected : public testing::TestWithParam<std::string> {};

TEST_P(PortRejected, ReturnsErrorNamingThePort)
{
    expectConfigError(
        {"--port", std::string_view{GetParam()}, "--seed", "1"},
        "--port");
}

INSTANTIATE_TEST_SUITE_P(
    ServerConfigTests,
    PortRejected,
    testing::Values(
        std::string{"0"},
        std::string{"65536"},
        std::string{"+5"},
        std::string{"42x"},
        std::string{""},
        std::string{"-1"}));

class SeedRejected : public testing::TestWithParam<std::string> {};

TEST_P(SeedRejected, ReturnsErrorNamingTheSeed)
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
    expectConfigError({"--log-level", "loud", "--seed", "1"}, "--log-level");
}

TEST(ServerConfigTests, RejectsUnknownOption)
{
    expectConfigError({"--frobnicate", "--seed", "1"}, "--frobnicate");
}

TEST(ServerConfigTests, RejectsMissingValue)
{
    expectConfigError({"--seed", "1", "-p"}, "-p");
    expectConfigError({"--seed", "1", "--port"}, "--port");
    expectConfigError({"--seed"}, "--seed");
    expectConfigError({"--seed", "1", "--log-level"}, "--log-level");
}

TEST(ServerConfigTests, HelpFlagsThrowHelpRequested)
{
    for (const std::string_view flag : {"-h", "--help"})
        EXPECT_THROW((void)parseArgs({flag}), HelpRequested) << flag;
}

TEST(ServerConfigTests, HelpWorksAfterValidOptions)
{
    EXPECT_THROW((void)parseArgs({"-p", "5000", "--help"}), HelpRequested);
}

TEST(ServerConfigTests, UsageNamesProgramAndOptions)
{
    const std::string usage = ServerConfig::usage("r-type_server");

    EXPECT_NE(usage.find("r-type_server"), std::string::npos);

    for (const std::string_view option :
         {"--port", "--seed", "--log-level", "--help"}) {
        EXPECT_NE(usage.find(option), std::string::npos)
            << option;
    }
}
