/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RulesLoadTests
*/

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

#include "engine/core/Errors.hpp"
#include "rtype/game/Rules.hpp"

using engine::core::ConfigError;
using engine::core::Vec2f;
using nlohmann::json;
using rtype::game::FloatRange;
using rtype::game::Rules;

namespace {

/* Expects fromJson to throw a ConfigError whose message contains `part`. */
void expectRefused(const json &rules, std::string_view part)
{
    try {
        (void)Rules::fromJson(rules);
        ADD_FAILURE() << "no ConfigError for " << rules.dump();
    } catch (const ConfigError &error) {
        EXPECT_NE(std::string_view{error.what()}.find(part),
            std::string_view::npos)
            << error.what();
    }
}

/* A file named after the current test, removed at the end of it. */
class RulesFile: public testing::Test {
protected:
    void TearDown() override
    {
        std::filesystem::remove(_path);
    }

    const std::filesystem::path &write(const std::string &content)
    {
        std::ofstream(_path) << content;
        return _path;
    }

    std::filesystem::path _path = std::filesystem::temp_directory_path() /
        ("rtype_rules_" +
            std::string(testing::UnitTest::GetInstance()
                    ->current_test_info()
                    ->name()) +
            ".json");
};

} // namespace

TEST(RulesLoadTests, AnEmptyObjectGivesTheDefaults)
{
    EXPECT_EQ(Rules::fromJson(json::object()), Rules{});
}

TEST(RulesLoadTests, TheRepositoryFileHoldsEveryRuleAtItsDefault)
{
    const std::filesystem::path path =
        std::filesystem::path(RTYPE_CONFIG_DIR) / "rules.json";
    std::ifstream file(path);
    const json content = json::parse(file);

    EXPECT_EQ(content.size(), 35U) << "a rule is missing from rules.json";
    EXPECT_EQ(Rules::load(path), Rules{});
}

TEST(RulesLoadTests, EachKindOfValueIsRead)
{
    const Rules rules = Rules::fromJson({
        {"playerSpeed", 400},
        {"playerLives", 5},
        {"playerHalfSize", {20, 9}},
        {"enemySpeed", {{"min", 100}, {"max", 300}}},
        {"playerSpawn", {{10, 20}, {10, 40}, {10, 60}, {10, 80}}},
        {"killScore", 4000000000LL},
    });

    EXPECT_FLOAT_EQ(rules.playerSpeed, 400.0F);
    EXPECT_EQ(rules.playerLives, 5);
    EXPECT_EQ(rules.playerHalfSize, Vec2f(20.0F, 9.0F));
    EXPECT_EQ(rules.enemySpeed, (FloatRange{.min = 100.0F, .max = 300.0F}));
    EXPECT_EQ(rules.playerSpawn[3], Vec2f(10.0F, 80.0F));
    EXPECT_EQ(rules.killScore, 4000000000U);
}

TEST(RulesLoadTests, AMissingKeyKeepsItsDefault)
{
    const Rules rules = Rules::fromJson({{"playerSpeed", 400}});

    EXPECT_EQ(rules.playerLives, Rules{}.playerLives);
    EXPECT_EQ(rules.enemySpeed, Rules{}.enemySpeed);
}

TEST(RulesLoadTests, AnUnknownKeyIsRefused)
{
    expectRefused({{"playerSped", 400}}, "playerSped: unknown rule");
}

TEST(RulesLoadTests, ThePlayfieldIsFixed)
{
    expectRefused({{"playfield", {1920, 1080}}}, "playfield: unknown rule");
}

TEST(RulesLoadTests, TheTopLevelMustBeAnObject)
{
    expectRefused(json::array({1, 2}), "JSON object");
}

TEST(RulesLoadTests, AWrongTypeIsRefusedNamingTheKey)
{
    expectRefused({{"playerSpeed", "fast"}}, "playerSpeed: must be a number");
    expectRefused({{"playerLives", 2.5}}, "playerLives: must be a whole");
    expectRefused({{"playerHalfSize", {16}}}, "playerHalfSize: must be [x, y]");
    expectRefused({{"enemySpeed", {100, 200}}}, "enemySpeed: must be {");
    expectRefused({{"playerSpawn", {{1, 2}}}}, "playerSpawn: must be an array");
}

TEST(RulesLoadTests, AnOutOfRangeValueIsRefusedNamingTheKey)
{
    expectRefused({{"playerSpeed", 0}}, "playerSpeed: must be a number > 0");
    expectRefused({{"respawnDelay", -1}}, "respawnDelay: must be a number >=");
    expectRefused({{"sineChance", 1.5}}, "sineChance: must be a number in");
    expectRefused({{"maxPlayers", 5}}, "maxPlayers: must be a whole number in");
    expectRefused({{"playerLives", 0}}, "playerLives");
    expectRefused({{"playerLives", 100}}, "playerLives");
    expectRefused({{"playerHealth", 256}}, "playerHealth");
    expectRefused({{"enemyMissileDamage", 0}}, "enemyMissileDamage");
    expectRefused({{"enemyCollisionDamage", -20}}, "enemyCollisionDamage");
    expectRefused({{"maxEnemies", 256}}, "maxEnemies");
    expectRefused({{"killScore", -1}}, "killScore");
    expectRefused({{"playerHalfSize", {16, 0}}}, "playerHalfSize");
    expectRefused({{"playerSpeed", 1e300}}, "playerSpeed: must be a finite");
}

TEST(RulesLoadTests, ARangeWithMinAboveMaxIsRefused)
{
    expectRefused({{"enemySpeed", {{"min", 300}, {"max", 100}}}},
        "enemySpeed: min must not be above max");
    expectRefused({{"enemyFireInterval", {{"min", 0}, {"max", 1}}}},
        "enemyFireInterval: min must be > 0");
}

TEST(RulesLoadTests, RulesThatDisagreeAreRefused)
{
    expectRefused({{"spawnIntervalMin", 2.0}}, "spawnIntervalMin");
    expectRefused({{"playerSpawn", {{120, 144}, {120, 288}, {120, 432},
                                       {1300, 576}}}},
        "playerSpawn: point 3");
    expectRefused({{"enemySpawnY", {{"min", 40}, {"max", 800}}}},
        "enemySpawnY");
}

TEST_F(RulesFile, LoadReadsTheFile)
{
    const Rules rules = Rules::load(write(R"({"playerSpeed": 500})"));

    EXPECT_FLOAT_EQ(rules.playerSpeed, 500.0F);
}

TEST_F(RulesFile, LoadNamesTheFileAndTheKey)
{
    try {
        (void)Rules::load(write(R"({"playerSpeed": -5})"));
        ADD_FAILURE() << "no ConfigError";
    } catch (const ConfigError &error) {
        const std::string message = error.what();

        EXPECT_TRUE(message.starts_with(_path.string() + ": playerSpeed"))
            << message;
    }
}

TEST_F(RulesFile, InvalidJsonIsRefused)
{
    EXPECT_THROW((void)Rules::load(write("{\"playerSpeed\": ")), ConfigError);
}

TEST(RulesLoadTests, AMissingFileIsRefused)
{
    EXPECT_THROW((void)Rules::load("no/such/rules.json"), ConfigError);
}
