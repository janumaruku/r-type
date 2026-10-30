/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RulesTests
*/

#include <gtest/gtest.h>

#include "rtype/game/Rules.hpp"

using rtype::game::FloatRange;
using rtype::game::Rules;
using engine::core::Vec2f;

namespace {

void expectVec(const Vec2f &actual, float x, float y)
{
    EXPECT_FLOAT_EQ(actual.x(), x);
    EXPECT_FLOAT_EQ(actual.y(), y);
}

void expectRange(const FloatRange &actual, float min, float max)
{
    EXPECT_FLOAT_EQ(actual.min, min);
    EXPECT_FLOAT_EQ(actual.max, max);
}

}

TEST(Rules, PlayerDefaultsMatchTheSpec)
{
    const Rules rules{};

    expectVec(rules.playfield, 1280.0F, 720.0F);
    EXPECT_EQ(rules.maxPlayers, 4);
    EXPECT_FLOAT_EQ(rules.playerSpeed, 320.0F);
    expectVec(rules.playerSpawn[0], 120.0F, 144.0F);
    expectVec(rules.playerSpawn[1], 120.0F, 288.0F);
    expectVec(rules.playerSpawn[2], 120.0F, 432.0F);
    expectVec(rules.playerSpawn[3], 120.0F, 576.0F);
    EXPECT_EQ(rules.playerLives, 3);
    EXPECT_FLOAT_EQ(rules.respawnDelay, 2.0F);
    EXPECT_FLOAT_EQ(rules.respawnInvulnerability, 2.0F);
    EXPECT_FLOAT_EQ(rules.playerFireCooldown, 0.20F);
    EXPECT_FLOAT_EQ(rules.playerMissileSpeed, 900.0F);
    EXPECT_FLOAT_EQ(rules.playerMissileLifetime, 2.0F);
}

TEST(Rules, EnemyDefaultsMatchTheSpec)
{
    const Rules rules{};

    expectRange(rules.enemySpeed, 160.0F, 240.0F);
    expectRange(rules.sineAmplitude, 40.0F, 120.0F);
    expectRange(rules.sineFrequency, 0.5F, 1.2F);
    EXPECT_FLOAT_EQ(rules.sineChance, 0.5F);
    expectRange(rules.enemyFireInterval, 1.5F, 3.0F);
    EXPECT_FLOAT_EQ(rules.enemyMissileSpeed, 420.0F);
    EXPECT_FLOAT_EQ(rules.enemyMissileLifetime, 4.0F);
}

TEST(Rules, SpawnAndGameDefaultsMatchTheSpec)
{
    const Rules rules{};

    EXPECT_FLOAT_EQ(rules.spawnIntervalStart, 1.6F);
    EXPECT_FLOAT_EQ(rules.spawnIntervalStep, 0.05F);
    EXPECT_FLOAT_EQ(rules.spawnIntervalStepEvery, 10.0F);
    EXPECT_FLOAT_EQ(rules.spawnIntervalMin, 0.5F);
    EXPECT_FLOAT_EQ(rules.enemySpawnX, 1312.0F);
    expectRange(rules.enemySpawnY, 40.0F, 680.0F);
    EXPECT_EQ(rules.maxEnemies, 30);
    EXPECT_EQ(rules.killScore, 100U);
    EXPECT_FLOAT_EQ(rules.gameOverResetDelay, 5.0F);
    EXPECT_FLOAT_EQ(rules.despawnMargin, 64.0F);
}

TEST(Rules, SizesMatchTheSpriteFrames)
{
    const Rules rules{};

    expectVec(rules.playerHalfSize, 16.0F, 7.0F);
    expectVec(rules.enemyHalfSize, 10.5F, 12.0F);
    expectVec(rules.playerMissileHalfSize, 6.0F, 2.0F);
    expectVec(rules.enemyMissileHalfSize, 3.5F, 3.0F);
}

TEST(Rules, MuzzlesAreAtTheNose)
{
    const Rules rules{};

    expectVec(rules.playerMuzzleOffset, 16.0F, 0.0F);
    expectVec(rules.enemyMuzzleOffset, -10.5F, 0.0F);
    EXPECT_FLOAT_EQ(rules.playerMuzzleOffset.x(), rules.playerHalfSize.x());
    EXPECT_FLOAT_EQ(rules.enemyMuzzleOffset.x(), -rules.enemyHalfSize.x());
}
