/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Rules
*/

#include "rtype/game/Rules.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <limits>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

#include "engine/core/Errors.hpp"

namespace rtype::game {
namespace {

using engine::core::ConfigError;
using engine::core::Vec2f;
using Json = nlohmann::json;

enum class Sign : std::uint8_t {
    ANY,
    NON_NEGATIVE,
    POSITIVE,
};

[[noreturn]] void fail(std::string_view key, std::string_view expected,
    const Json &value)
{
    throw ConfigError(std::format("{}: {}, got {}", key, expected,
        value.dump()));
}

/*
** A finite number that still fits a float: 1e400 parses as a double but
** would become infinity.
*/
float number(std::string_view key, const Json &value, Sign sign)
{
    if (!value.is_number())
        fail(key, "must be a number", value);
    const auto parsed = value.get<double>();

    if (!std::isfinite(parsed) ||
        std::abs(parsed) > std::numeric_limits<float>::max())
        fail(key, "must be a finite number", value);
    if (sign == Sign::POSITIVE && parsed <= 0.0)
        fail(key, "must be a number > 0", value);
    if (sign == Sign::NON_NEGATIVE && parsed < 0.0)
        fail(key, "must be a number >= 0", value);
    return static_cast<float>(parsed);
}

float probability(std::string_view key, const Json &value)
{
    const float parsed = number(key, value, Sign::NON_NEGATIVE);

    if (parsed > 1.0F)
        fail(key, "must be a number in [0, 1]", value);
    return parsed;
}

template <class Integer>
Integer whole(std::string_view key, const Json &value, std::int64_t min,
    std::int64_t max)
{
    if (!value.is_number_integer())
        fail(key, "must be a whole number", value);
    const auto parsed = value.get<std::int64_t>();

    if (parsed < min || parsed > max)
        fail(key, std::format("must be a whole number in [{}, {}]", min, max),
            value);
    return static_cast<Integer>(parsed);
}

Vec2f vector(std::string_view key, const Json &value, Sign sign)
{
    if (!value.is_array() || value.size() != 2)
        fail(key, "must be [x, y]", value);
    return {number(key, value[0], sign), number(key, value[1], sign)};
}

FloatRange range(std::string_view key, const Json &value)
{
    if (!value.is_object() || value.size() != 2 || !value.contains("min") ||
        !value.contains("max"))
        fail(key, R"(must be {"min": a, "max": b})", value);
    const FloatRange parsed{
        .min = number(key, value["min"], Sign::NON_NEGATIVE),
        .max = number(key, value["max"], Sign::NON_NEGATIVE)};

    if (parsed.min > parsed.max)
        fail(key, "min must not be above max", value);
    return parsed;
}

std::array<Vec2f, 4> spawnPoints(std::string_view key, const Json &value)
{
    if (!value.is_array() || value.size() != 4)
        fail(key, "must be an array of 4 [x, y]", value);
    std::array<Vec2f, 4> points{};

    for (std::size_t slot = 0; slot < points.size(); ++slot)
        points[slot] = vector(key, value[slot], Sign::NON_NEGATIVE);
    return points;
}

using Reader = void (*)(Rules &, std::string_view, const Json &);

struct Rule {
    std::string_view key;
    Reader read;
};

constexpr std::int64_t U8_MAX  = std::numeric_limits<std::uint8_t>::max();
constexpr std::int64_t MAX_LIVES = 99;
constexpr std::int64_t U32_MAX = std::numeric_limits<std::uint32_t>::max();

/*
** One entry per field of Rules, in the struct's order, except playfield:
** it is fixed, since every client draws the same 1280 x 720 playfield,
** letterboxed to its window. maxPlayers stops at 4: the roster,
** playerSpawn and the protocol all have 4 slots. playerLives stops at 99
** and playerHealth at 255, the most a SNAPSHOT's PlayerInfo carries.
*/
constexpr std::array RULES{
    Rule{"maxPlayers",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.maxPlayers = whole<std::uint8_t>(key, value, 1, 4);
        }},
    Rule{"playersToStart",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playersToStart = whole<std::uint8_t>(key, value, 1, 4);
        }},
    Rule{"playerSpeed",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerSpeed = number(key, value, Sign::POSITIVE);
        }},
    Rule{"playerHealth",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerHealth = whole<std::int16_t>(key, value, 1, U8_MAX);
        }},
    Rule{"playerSpawn",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerSpawn = spawnPoints(key, value);
        }},
    Rule{"playerLives",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerLives = whole<std::uint8_t>(key, value, 1, MAX_LIVES);
        }},
    Rule{"respawnDelay",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.respawnDelay = number(key, value, Sign::NON_NEGATIVE);
        }},
    Rule{"respawnInvulnerability",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.respawnInvulnerability =
                number(key, value, Sign::NON_NEGATIVE);
        }},
    Rule{"playerFireCooldown",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerFireCooldown = number(key, value, Sign::POSITIVE);
        }},
    Rule{"playerMissileSpeed",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerMissileSpeed = number(key, value, Sign::POSITIVE);
        }},
    Rule{"playerMissileLifetime",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerMissileLifetime = number(key, value, Sign::POSITIVE);
        }},
    Rule{"enemySpeed",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemySpeed = range(key, value);
        }},
    Rule{"sineAmplitude",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.sineAmplitude = range(key, value);
        }},
    Rule{"sineFrequency",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.sineFrequency = range(key, value);
        }},
    Rule{"sineChance",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.sineChance = probability(key, value);
        }},
    Rule{"enemyFireInterval",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemyFireInterval = range(key, value);
            if (rules.enemyFireInterval.min <= 0.0F)
                fail(key, "min must be > 0", value);
        }},
    Rule{"enemyMissileSpeed",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemyMissileSpeed = number(key, value, Sign::POSITIVE);
        }},
    Rule{"enemyMissileLifetime",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemyMissileLifetime = number(key, value, Sign::POSITIVE);
        }},
    Rule{"enemyMissileDamage",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemyMissileDamage =
                whole<std::int16_t>(key, value, 1, U8_MAX);
        }},
    Rule{"enemyCollisionDamage",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemyCollisionDamage =
                whole<std::int16_t>(key, value, 1, U8_MAX);
        }},
    Rule{"spawnIntervalStart",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.spawnIntervalStart = number(key, value, Sign::POSITIVE);
        }},
    Rule{"spawnIntervalStep",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.spawnIntervalStep = number(key, value, Sign::NON_NEGATIVE);
        }},
    Rule{"spawnIntervalStepEvery",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.spawnIntervalStepEvery = number(key, value, Sign::POSITIVE);
        }},
    Rule{"spawnIntervalMin",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.spawnIntervalMin = number(key, value, Sign::POSITIVE);
        }},
    Rule{"enemySpawnX",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemySpawnX = number(key, value, Sign::ANY);
        }},
    Rule{"enemySpawnY",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemySpawnY = range(key, value);
        }},
    Rule{"maxEnemies",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.maxEnemies = whole<std::uint8_t>(key, value, 0, U8_MAX);
        }},
    Rule{"killScore",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.killScore = whole<std::uint32_t>(key, value, 0, U32_MAX);
        }},
    Rule{"gameOverResetDelay",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.gameOverResetDelay = number(key, value, Sign::NON_NEGATIVE);
        }},
    Rule{"despawnMargin",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.despawnMargin = number(key, value, Sign::NON_NEGATIVE);
        }},
    Rule{"playerHalfSize",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerHalfSize = vector(key, value, Sign::POSITIVE);
        }},
    Rule{"enemyHalfSize",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemyHalfSize = vector(key, value, Sign::POSITIVE);
        }},
    Rule{"playerMissileHalfSize",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerMissileHalfSize = vector(key, value, Sign::POSITIVE);
        }},
    Rule{"enemyMissileHalfSize",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemyMissileHalfSize = vector(key, value, Sign::POSITIVE);
        }},
    Rule{"playerMuzzleOffset",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.playerMuzzleOffset = vector(key, value, Sign::ANY);
        }},
    Rule{"enemyMuzzleOffset",
        [](Rules &rules, std::string_view key, const Json &value) {
            rules.enemyMuzzleOffset = vector(key, value, Sign::ANY);
        }},
};

bool inside(const Vec2f &point, const Vec2f &playfield) noexcept
{
    return point.x() <= playfield.x() && point.y() <= playfield.y();
}

/*
** The checks that need two rules, run once every key is read, so the
** order of the keys in the file doesn't matter.
*/
void checkTogether(const Rules &rules)
{
    if (rules.playersToStart > rules.maxPlayers)
        throw ConfigError(std::format(
            "playersToStart: {} is above maxPlayers {}",
            rules.playersToStart, rules.maxPlayers));
    if (rules.spawnIntervalMin > rules.spawnIntervalStart)
        throw ConfigError(std::format(
            "spawnIntervalMin: {} is above spawnIntervalStart {}",
            rules.spawnIntervalMin, rules.spawnIntervalStart));
    for (std::size_t slot = 0; slot < rules.playerSpawn.size(); ++slot) {
        if (!inside(rules.playerSpawn[slot], rules.playfield))
            throw ConfigError(std::format(
                "playerSpawn: point {} is outside the playfield", slot));
    }
    if (rules.enemySpawnY.max > rules.playfield.y())
        throw ConfigError(std::format(
            "enemySpawnY: max {} is beyond the playfield height {}",
            rules.enemySpawnY.max, rules.playfield.y()));
}

} // namespace

Rules Rules::load(const std::filesystem::path &path)
{
    std::ifstream file(path);

    if (!file.is_open())
        throw ConfigError(path.string() + ": cannot open the rules");
    const auto json = Json::parse(file, nullptr, false);

    if (json.is_discarded())
        throw ConfigError(path.string() + ": not valid JSON");
    try {
        return fromJson(json);
    } catch (const ConfigError &error) {
        throw ConfigError(path.string() + ": " + error.what());
    }
}

Rules Rules::fromJson(const nlohmann::json &json)
{
    if (!json.is_object())
        throw ConfigError("the rules must be a JSON object");
    Rules rules;

    for (const auto &[key, value]: json.items()) {
        const auto *rule = std::ranges::find(RULES, key, &Rule::key);

        if (rule == RULES.end())
            throw ConfigError(key + ": unknown rule");
        rule->read(rules, rule->key, value);
    }
    checkTogether(rules);
    return rules;
}

} // namespace rtype::game
