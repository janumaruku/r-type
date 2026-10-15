/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LoggerTests.cpp
*/

#include <gtest/gtest.h>

#include <regex>

#include "Logger.hpp"

TEST(Logger, Trace)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.trace() << "This is a trace message";
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_TRACE);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is a trace message");
}

TEST(Logger, Debug)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.debug() << "This is a debug message";
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_DEBUG);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is a debug message");
}

TEST(Logger, Info)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.info() << "This is an info message";
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is an info message");
}

TEST(Logger, Warn)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.warn() << "This is a warning message";
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_WARN);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is a warning message");
}

TEST(Logger, Error)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.error() << "This is an error message";
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_ERROR);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is an error message");
}

TEST(Logger, WritesFormattedOutputToStderr)
{
    testing::internal::CaptureStderr();
    {
        engine::core::log::Logger logger("Test");
        logger.setThreadName("MainThread");
        logger.info() << "This is an info message";
    }

    const std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(std::regex_match(output,
        std::regex(R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\] \[INFO\] \[MainThread\] \[Test\] This is an info message\n)")));
}

TEST(Logger, OmitsEmptyContextFromOutput)
{
    testing::internal::CaptureStderr();
    {
        engine::core::log::Logger logger;
        logger.setThreadName("TestThread");
        logger.error() << "This is an error message";
    }

    const std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(std::regex_match(output,
        std::regex(R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\] \[ERR\] \[TestThread\] This is an error message\n)")));
}

TEST(Logger, SetThreadName)
{
    engine::core::log::Logger logger("Test");
    logger.setThreadName("MainThread");
    auto logLine = logger.info() << "This is an info message";
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is an info message");
}

TEST(Logger, FormatString)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.info("This is a formatted message: {}", 42);
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "This is a formatted message: 42");
}

TEST(Logger, FormatMultipleArgs)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.info("Formatted message: {}, {}, {}", 1, 2.5, "three");
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "Formatted message: 1, 2.5, three");
}

TEST(Logger, FormatWithDifferentTypes)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.info("Int: {}, Float: {}, String: {}", 10, 3.14, "Hello");
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "Int: 10, Float: 3.14, String: Hello");
}

TEST(Logger, FormatWithSpecifier)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.info("Padded value: {:04}", 42);
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "Padded value: 0042");
}

TEST(Logger, FormatWithMultipleFormatSpecifiers)
{
    engine::core::log::Logger logger("Test");
    auto logLine = logger.info("Hex: {:x}, Float: {:.2f}", 255, 3.14159);
    EXPECT_EQ(logLine.level, engine::core::log::Level::EG_INFO);
    EXPECT_EQ(logLine.context, "Test");
    EXPECT_TRUE(logLine.active);
    EXPECT_EQ(logLine.buffer.str(), "Hex: ff, Float: 3.14");
}