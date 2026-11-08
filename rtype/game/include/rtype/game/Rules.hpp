/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Rules
*/

#ifndef R_TYPE_RULES_HPP
#define R_TYPE_RULES_HPP

#include <array>
#include <filesystem>
#include <nlohmann/json_fwd.hpp>

#include "engine/core/Vec2.hpp"
#include "Types.hpp"

namespace rtype::game {
struct Rules {
    engine::core::Vec2f playfield{1280.0F, 720.0F};
    std::uint8_t maxPlayers = 4;
    std::uint8_t playersToStart = 4;
    float playerSpeed       = 320.0F;
    std::array<engine::core::Vec2f, 4> playerSpawn{
        {{120, 144}, {120, 288}, {120, 432}, {120, 576}},
    };
    std::uint8_t playerLives     = 3;
    std::int16_t playerHealth    = 100;
    float respawnDelay           = 2.0F;
    float respawnInvulnerability = 2.0F;
    float playerFireCooldown     = 0.20F;
    float playerMissileSpeed     = 900.0F;
    float playerMissileLifetime  = 2.0F;
    FloatRange enemySpeed{.min = 60.0F, .max = 100.0F};
    FloatRange sineAmplitude{.min = 15.0F, .max = 40.0F};
    FloatRange sineFrequency{.min = 0.2F, .max = 0.5F};
    float sineChance = 0.5F;
    FloatRange enemyFireInterval{.min = 2.5F, .max = 5.0F};
    float enemyMissileSpeed      = 240.0F;
    float enemyMissileLifetime   = 6.0F;
    std::int16_t enemyMissileDamage   = 5;
    std::int16_t enemyCollisionDamage = 20;
    float spawnIntervalStart     = 1.6F;
    float spawnIntervalStep      = 0.05F;
    float spawnIntervalStepEvery = 10.0F;
    float spawnIntervalMin       = 0.5F;
    float enemySpawnX            = 1312.0F;
    FloatRange enemySpawnY{.min = 40.0F, .max = 680.0F};
    std::uint8_t maxEnemies  = 30;
    std::uint32_t killScore  = 100;
    float gameOverResetDelay = 5.0F;
    float despawnMargin      = 64.0F;

    engine::core::Vec2f playerHalfSize{16.0F, 7.0F};
    engine::core::Vec2f enemyHalfSize{10.5F, 12.0F};
    engine::core::Vec2f playerMissileHalfSize{6.0F, 2.0F};
    engine::core::Vec2f enemyMissileHalfSize{3.5F, 3.0F};
    engine::core::Vec2f playerMuzzleOffset{16.0F, 0.0F};
    engine::core::Vec2f enemyMuzzleOffset{-10.5F, 0.0F};

    bool operator==(const Rules &) const = default;

    /**
     * @brief Reads `fromJson` from a file such as `config/rules.json`.
     * @throw engine::core::ConfigError if the file can't be opened, isn't
     * valid JSON or `fromJson` refuses it. what() starts with the path.
     */
    [[nodiscard]] static Rules load(const std::filesystem::path &path);

    /**
     * @brief The defaults, overridden by each key of `json`.
     *
     * The keys are the field names, except `playfield`, which is fixed:
     * every client draws the same playfield. Every key is optional. A vector is
     * `[x, y]`, a range `{"min": a, "max": b}` and `playerSpawn` an array
     * of 4 vectors. `game.md` § Rules and tuning values lists the limits.
     * @throw engine::core::ConfigError naming the key, on an unknown key, a
     * wrong type or an out-of-range value, or when two rules disagree
     * (`spawnIntervalMin` above `spawnIntervalStart`, a spawn point outside
     * the playfield).
     */
    [[nodiscard]] static Rules fromJson(const nlohmann::json &json);
};
} // namespace rtype::game

#endif // R_TYPE_RULES_HPP
