/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Rules
*/

#ifndef R_TYPE_RULES_HPP
#define R_TYPE_RULES_HPP

#include <array>

#include "Types.hpp"
#include "engine/core/Vec2.hpp"

namespace rtype::game {
struct Rules {
    engine::core::Vec2f playfield{1280.0F, 720.0F};
    std::uint8_t maxPlayers = 4;
    float playerSpeed = 320.0F;
    std::array<engine::core::Vec2f, 4> playerSpawn{
        {{120, 144}, {120, 288}, {120, 432}, {120, 576}},};
    std::uint8_t playerLives = 3;
    float respawnDelay = 2.0F;
    float respawnInvulnerability = 2.0F;
    float playerFireCooldown = 0.20F;
    float playerMissileSpeed = 900.0F;
    float playerMissileLifetime = 2.0F;
    FloatRange enemySpeed{.min = 160.0F, .max = 240.0F};
    FloatRange sineAmplitude{.min = 40.0F, .max = 120.0F};
    FloatRange sineFrequency{.min = 0.5F, .max = 1.2F};
    float sineChance = 0.5F;
    FloatRange enemyFireInterval{.min = 1.5F, .max = 3.0F};
    float enemyMissileSpeed = 420.0F;
    float enemyMissileLifetime = 4.0F;
    float spawnIntervalStart = 1.6F;
    float spawnIntervalStep = 0.05F;
    float spawnIntervalStepEvery = 10.0F;
    float spawnIntervalMin = 0.5F;
    float enemySpawnX = 1312.0F;
    FloatRange enemySpawnY{.min = 40.0F, .max = 680.0F};
    std::uint8_t maxEnemies = 30;
    std::uint32_t killScore = 100;
    float gameOverResetDelay = 5.0F;
    float despawnMargin = 64.0F;

    engine::core::Vec2f playerHalfSize{16.0F, 7.0F};
    engine::core::Vec2f enemyHalfSize{10.5F, 12.0F};
    engine::core::Vec2f playerMissileHalfSize{6.0F, 2.0F};
    engine::core::Vec2f enemyMissileHalfSize{3.5F, 3.0F};
    engine::core::Vec2f playerMuzzleOffset{16.0F, 0.0F};
    engine::core::Vec2f enemyMuzzleOffset{-10.5F, 0.0F};
};
}

#endif //R_TYPE_RULES_HPP
