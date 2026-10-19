/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ErrorTests.cpp
*/

#include <gtest/gtest.h>
#include "Errors.hpp"

TEST(Error, EngineError)
{
    EXPECT_THROW(throw engine::core::EngineError("Engine error"), engine::core::EngineError);
}

TEST(Error, RessourceError)
{
    EXPECT_THROW(throw engine::core::ResourceError("Ressource error"), engine::core::ResourceError);
}

TEST(Error, ConfigError)
{
    EXPECT_THROW(throw engine::core::ConfigError("Config error"), engine::core::ConfigError);
}

TEST(Error, NetworkError)
{
    EXPECT_THROW(throw engine::core::NetworkError("Network error"), engine::core::NetworkError);
}