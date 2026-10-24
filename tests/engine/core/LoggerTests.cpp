/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LoggerTests.cpp
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <format>
#include <iostream>
#include <numbers>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

#include "engine/core/Logger.hpp"

using engine::core::log::Level;
using engine::core::log::Logger;
using engine::core::log::isEnabled;
using engine::core::log::parseLevel;
using engine::core::log::setColors;
using engine::core::log::setLevel;
using engine::core::log::setOutput;

namespace {
struct Counted {
    int value;
};

int formatCalls = 0; // NOLINT

const std::regex LINE_FORMAT( // NOLINT
    R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\] \[(TRACE|DEBUG|INFO|WARN|ERR)\] \[[^\]]+\]( \[[^\]]+\])? .*\n)");

/*
** The level and the output are process-wide, so every test starts from INFO,
** plain stderr output without colours, and gets them back when it ends.
** CaptureStderr only swaps the file descriptor, so colours must be forced
** off here for the stderr tests to match when run in a terminal.
*/
class LoggerTest: public testing::Test {
protected:
    void SetUp() override
    {
        _saved = engine::core::log::level();
        setLevel(Level::EG_INFO);
        setOutput(std::cerr);
        setColors(false);
    }

    void TearDown() override
    {
        setLevel(_saved);
        setOutput(std::cerr);
    }

    Level _saved = Level::EG_INFO;
    std::ostringstream _output;
};
} // namespace

template <>
struct std::formatter<Counted>: std::formatter<int> {
    auto format(const Counted &counted, std::format_context &context) const
    {
        ++formatCalls;
        return std::formatter<int>::format(counted.value, context);
    }
};

TEST_F(LoggerTest, Trace)
{
    setLevel(Level::EG_TRACE);
    Logger logger("Test");
    const auto logLine = logger.trace() << "This is a trace message";
    EXPECT_EQ(logLine.level, Level::EG_TRACE);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is a trace message");
}

TEST_F(LoggerTest, Debug)
{
    setLevel(Level::EG_DEBUG);
    Logger logger("Test");
    const auto logLine = logger.debug() << "This is a debug message";
    EXPECT_EQ(logLine.level, Level::EG_DEBUG);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is a debug message");
}

TEST_F(LoggerTest, Info)
{
    Logger logger("Test");
    const auto logLine = logger.info() << "This is an info message";
    EXPECT_EQ(logLine.level, Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is an info message");
}

TEST_F(LoggerTest, Warn)
{
    Logger logger("Test");
    auto logLine = logger.warn() << "This is a warning message";
    EXPECT_EQ(logLine.level, Level::EG_WARN);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is a warning message");
}

TEST_F(LoggerTest, Error)
{
    Logger logger("Test");
    const auto logLine = logger.error() << "This is an error message";
    EXPECT_EQ(logLine.level, Level::EG_ERROR);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is an error message");
}

TEST_F(LoggerTest, WritesFormattedOutputToStderr)
{
    testing::internal::CaptureStderr();
    {
        Logger logger("Test");
        logger.setThreadName("MainThread");
        logger.info() << "This is an info message";
    }

    const std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(std::regex_match(output,
        std::regex(
            R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\] \[INFO\] \[MainThread\] \[Test\] This is an info message\n)"
        )));
}

TEST_F(LoggerTest, OmitsEmptyContextFromOutput)
{
    testing::internal::CaptureStderr();
    {
        Logger logger;
        logger.setThreadName("TestThread");
        logger.error() << "This is an error message";
    }

    const std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(std::regex_match(output,
        std::regex(
            R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\] \[ERR\] \[TestThread\] This is an error message\n)"
        )));
}

TEST_F(LoggerTest, SetThreadName)
{
    Logger logger("Test");
    logger.setThreadName("MainThread");
    const auto logLine = logger.info() << "This is an info message";
    EXPECT_EQ(logLine.level, Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is an info message");
}

TEST_F(LoggerTest, FormatString)
{
    Logger logger("Test");
    const auto logLine = logger.info("This is a formatted message: {}", 42);
    EXPECT_EQ(logLine.level, Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is a formatted message: 42");
}

TEST_F(LoggerTest, FormatMultipleArgs)
{
    Logger logger("Test");
    const auto logLine = logger.info("Formatted message: {}, {}, {}", 1, 2.5,
        "three");
    EXPECT_EQ(logLine.level, Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "Formatted message: 1, 2.5, three");
}

TEST_F(LoggerTest, FormatWithDifferentTypes)
{
    Logger logger("Test");
    const auto logLine = logger.info("Int: {}, Float: {}, String: {}", 10, 3.14,
        "Hello");
    EXPECT_EQ(logLine.level, Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "Int: 10, Float: 3.14, String: Hello");
}

TEST_F(LoggerTest, FormatWithSpecifier)
{
    Logger logger("Test");
    const auto logLine = logger.info("Padded value: {:04}", 42);
    EXPECT_EQ(logLine.level, Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "Padded value: 0042");
}

TEST_F(LoggerTest, FormatWithMultipleFormatSpecifiers)
{
    Logger logger("Test");
    const auto logLine = logger.info("Hex: {:x}, Float: {:.2f}", 255,
        std::numbers::pi);
    EXPECT_EQ(logLine.level, Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "Hex: ff, Float: 3.14");
}

TEST(LoggerDefaults, TheLevelStartsAtInfo)
{
    EXPECT_EQ(engine::core::log::level(), Level::EG_INFO);
}

TEST_F(LoggerTest, IsEnabledFollowsTheThreshold)
{
    setLevel(Level::EG_WARN);

    EXPECT_FALSE(isEnabled(Level::EG_INFO));
    EXPECT_TRUE(isEnabled(Level::EG_WARN));
    EXPECT_TRUE(isEnabled(Level::EG_ERROR));
    EXPECT_FALSE(isEnabled(Level::EG_OFF));
}

TEST_F(LoggerTest, AFilteredLineIsInactiveAndWritesNothing)
{
    setOutput(_output);
    {
        Logger logger("Test");
        const auto logLine = logger.debug() << "hidden";
        EXPECT_FALSE(logLine.active);
        EXPECT_TRUE(logLine.buffer.str().empty());
    }

    EXPECT_TRUE(_output.str().empty());
}

TEST_F(LoggerTest, OffSilencesEvenErrors)
{
    setOutput(_output);
    setLevel(Level::EG_OFF);

    Logger("Test").error() << "hidden";
    Logger("Test").error("hidden {}", 1);

    EXPECT_TRUE(_output.str().empty());
}

TEST_F(LoggerTest, AFilteredFormatCallDoesNotFormat)
{
    setOutput(_output);
    formatCalls = 0;
    Logger logger("Test");

    logger.debug("{}", Counted{1});
    EXPECT_EQ(formatCalls, 0);

    setLevel(Level::EG_DEBUG);
    logger.debug("{}", Counted{1});
    EXPECT_EQ(formatCalls, 1);
}

TEST_F(LoggerTest, ParseLevelAcceptsTheLowercaseNames)
{
    EXPECT_EQ(parseLevel("trace"), Level::EG_TRACE);
    EXPECT_EQ(parseLevel("debug"), Level::EG_DEBUG);
    EXPECT_EQ(parseLevel("info"), Level::EG_INFO);
    EXPECT_EQ(parseLevel("warn"), Level::EG_WARN);
    EXPECT_EQ(parseLevel("error"), Level::EG_ERROR);
    EXPECT_EQ(parseLevel("off"), Level::EG_OFF);
}

TEST_F(LoggerTest, ParseLevelRejectsAnythingElse)
{
    for (const char *name: {"INFO", "Info", "", "verbose", "err", "info "})
        EXPECT_FALSE(parseLevel(name).has_value()) << '"' << name << '"';
}

TEST_F(LoggerTest, SetOutputCapturesTheWholeLine)
{
    setOutput(_output);
    Logger logger("Test");
    logger.setThreadName("MainThread");

    logger.info("value {}", 42);

    EXPECT_TRUE(std::regex_match(_output.str(),
        std::regex(
            R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\] \[INFO\] \[MainThread\] \[Test\] value 42\n)"
        )));
}

TEST_F(LoggerTest, ARedirectedStreamGetsNoColours)
{
    setOutput(_output);

    Logger("Test").warn() << "careful";
    Logger("Test").error() << "failed";

    EXPECT_EQ(_output.str().find('\033'), std::string::npos);
}

TEST_F(LoggerTest, ColoursCoverThePrefixOnly)
{
    setOutput(_output);
    setColors(true);
    Logger logger("Test");
    logger.setThreadName("MainThread");

    logger.warn() << "careful";

    EXPECT_TRUE(_output.str().starts_with("\033[33m["));
    EXPECT_TRUE(
        _output.str().ends_with("[MainThread] [Test]\033[0m careful\n"));
}

TEST_F(LoggerTest, ColoursCoverTheWholeErrorLine)
{
    setOutput(_output);
    setColors(true);

    Logger("Test").error() << "failed";

    EXPECT_TRUE(_output.str().starts_with("\033[31m["));
    EXPECT_TRUE(_output.str().ends_with("[Test] failed\033[0m\n"));
}

TEST_F(LoggerTest, LinesFromTwoThreadsNeverMix)
{
    constexpr int LINES = 1000; // NOLINT
    setOutput(_output);
    auto writer = [](const char *name) {
        Logger logger(name);
        logger.setThreadName(name);
        for (int i = 0; i < LINES; ++i)
            logger.info("line {} from {}", i, name);
    };

    std::thread first(writer, "first");
    std::thread second(writer, "second");
    first.join();
    second.join();

    std::istringstream lines(_output.str());
    std::size_t count = 0;
    for (std::string line; std::getline(lines, line); ++count)
        EXPECT_TRUE(std::regex_match(line + '\n', LINE_FORMAT)) << line;
    EXPECT_EQ(count, 2U * LINES);
}
