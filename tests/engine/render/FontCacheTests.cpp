/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FontCacheTests
*/

#include <gtest/gtest.h>

#include "engine/core/Errors.hpp"
#include "engine/render/FontCache.hpp"

using engine::core::EngineError;
using engine::render::FontCache;
using engine::render::FontId;

TEST(FontCacheTests, MeasuringBeforeTheWindowExistsThrows)
{
    const FontCache fonts;

    EXPECT_THROW(static_cast<void>(fonts.measure(FontId{}, "Play", 20)),
        EngineError);
}
