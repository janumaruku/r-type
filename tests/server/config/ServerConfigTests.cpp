/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerConfigTests.cpp
*/

#include "ServerConfig.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using rtype::server::ServerConfig;
using engine::core::log::Level;

namespace {

struct ParseResult {
    int exitCode;
    ServerConfig config;
    std::string output;
};

class CoutRedirect {
public:
    explicit CoutRedirect(std::ostream& output)
        : previous_(std::cout.rdbuf(output.rdbuf()))
    {
    }

    ~CoutRedirect()
    {
        std::cout.rdbuf(previous_);
    }

    CoutRedirect(const CoutRedirect&) = delete;
    CoutRedirect& operator=(const CoutRedirect&) = delete;

private:
    std::streambuf* previous_;
};

ParseResult parseArgs(std::initializer_list<std::string_view> args)
{
    std::vector<std::string> storage;
    storage.reserve(args.size() + 1);
    storage.emplace_back("rtype-server");

    for (const std::string_view arg : args) {
        storage.emplace_back(arg);
    }

    std::vector<char*> argv;
    argv.reserve(storage.size());

    for (std::string& arg : storage) {
        argv.push_back(arg.data());
    }

    ServerConfig config{};
    std::ostringstream output;
    int exitCode = 0;

    {
        CoutRedirect redirect(output);
        exitCode = rtype::server::parse(
            static_cast<int>(argv.size()),
            argv.data(),
            config);
    }

    return {exitCode, config, output.str()};
}

void expectConfigError(
    std::initializer_list<std::string_view> args,
    std::string_view option)
{
    const ParseResult result = parseArgs(args);

    EXPECT_EQ(result.exitCode, 84) << result.output;
    EXPECT_NE(result.output.find(option), std::string::npos)
        << result.output;
}

} // namespace

TEST(ServerConfigTests, NoArgumentsUsesDefaults)
{
    const ParseResult result = parseArgs({});

    ASSERT_EQ(result.exitCode, 0);
    EXPECT_EQ(result.config.port, 4242);
    EXPECT_FALSE(result.config.seed.has_value());
    EXPECT_EQ(result.config.logLevel, Level::EG_INFO);
}

TEST(ServerConfigTests, ParsesAllOptions)
{
    const ParseResult result = parseArgs(
        {"-p", "5000", "--seed", "7", "--log-level", "debug"});

    ASSERT_EQ(result.exitCode, 0);
    EXPECT_EQ(result.config.port, 5000);
    ASSERT_TRUE(result.config.seed.has_value());
    EXPECT_EQ(*result.config.seed, 7U);
    EXPECT_EQ(result.config.logLevel, Level::EG_DEBUG);
}

TEST(ServerConfigTests, AcceptsLongPortForm)
{
    const ParseResult result = parseArgs({"--port", "4243"});

    ASSERT_EQ(result.exitCode, 0);
    EXPECT_EQ(result.config.port, 4243);
}

TEST(ServerConfigTests, AcceptsPortBoundaries)
{
    const ParseResult zero = parseArgs({"--port", "0"});
    const ParseResult low = parseArgs({"--port", "1"});
    const ParseResult high = parseArgs({"--port", "65535"});

    ASSERT_EQ(zero.exitCode, 0);
    ASSERT_EQ(low.exitCode, 0);
    ASSERT_EQ(high.exitCode, 0);

    EXPECT_EQ(zero.config.port, 0);
    EXPECT_EQ(low.config.port, 1);
    EXPECT_EQ(high.config.port, 65535);
}

TEST(ServerConfigTests, AcceptsSeedBoundaries)
{
    const ParseResult low = parseArgs({"--seed", "0"});
    const ParseResult high = parseArgs({"--seed", "4294967295"});

    ASSERT_EQ(low.exitCode, 0);
    ASSERT_EQ(high.exitCode, 0);

    ASSERT_TRUE(low.config.seed.has_value());
    ASSERT_TRUE(high.config.seed.has_value());

    EXPECT_EQ(*low.config.seed, 0U);
    EXPECT_EQ(
        *high.config.seed,
        std::numeric_limits<std::uint32_t>::max());
}

TEST(ServerConfigTests, LastOccurrenceWins)
{
    const ParseResult port = parseArgs({"-p", "1", "-p", "2"});

    ASSERT_EQ(port.exitCode, 0);
    EXPECT_EQ(port.config.port, 2);

    const ParseResult seed = parseArgs(
        {"--seed", "1", "--seed", "2"});

    ASSERT_EQ(seed.exitCode, 0);
    ASSERT_TRUE(seed.config.seed.has_value());
    EXPECT_EQ(*seed.config.seed, 2U);

    const ParseResult logLevel = parseArgs(
        {"--log-level", "debug", "--log-level", "info"});

    ASSERT_EQ(logLevel.exitCode, 0);
    EXPECT_EQ(logLevel.config.logLevel, Level::EG_INFO);
}

class PortRejected : public testing::TestWithParam<std::string> {};

TEST_P(PortRejected, ReturnsErrorNamingThePort)
{
    expectConfigError(
        {"--port", std::string_view{GetParam()}},
        "--port");
}

INSTANTIATE_TEST_SUITE_P(
    ServerConfigTests,
    PortRejected,
    testing::Values(
        std::string{"65536"},
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

TEST(ServerConfigTests, HelpFlagsReturnSuccessAndPrintUsage)
{
    for (const std::string_view flag : {"-h", "--help"}) {
        const ParseResult result = parseArgs({flag});

        EXPECT_EQ(result.exitCode, 0) << result.output;
        EXPECT_NE(result.output.find("--port"), std::string::npos);
        EXPECT_NE(result.output.find("--seed"), std::string::npos);
        EXPECT_NE(result.output.find("--log-level"), std::string::npos);
    }
}

TEST(ServerConfigTests, HelpWorksAfterValidOptions)
{
    const ParseResult result = parseArgs(
        {"-p", "5000", "--help"});

    EXPECT_EQ(result.exitCode, 0) << result.output;
    EXPECT_NE(result.output.find("--help"), std::string::npos);
}

TEST(ServerConfigTests, UsageNamesProgramAndOptions)
{
    const ParseResult result = parseArgs({"--help"});

    ASSERT_EQ(result.exitCode, 0);

    EXPECT_NE(result.output.find("rtype-server"), std::string::npos);

    for (const std::string_view option :
         {"--port", "--seed", "--log-level", "--help"}) {
        EXPECT_NE(result.output.find(option), std::string::npos)
            << option;
    }
}