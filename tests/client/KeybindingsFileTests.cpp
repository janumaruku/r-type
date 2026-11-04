/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** KeybindingsFileTests.cpp
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <set>
#include <string>

#include <nlohmann/json.hpp>

#include "engine/input/Key.hpp"

namespace rtype::client {
namespace {

/*
** TODO(RTP-121): use fromString(name, action) once the client implements it,
** instead of this copy of the Action names.
*/
const std::set<std::string> ACTION_NAMES{
    "MOVE_UP", "MOVE_DOWN", "MOVE_LEFT", "MOVE_RIGHT", "FIRE", "TOGGLE_HUD_HELP", "QUIT", "CONFIRM",
};

nlohmann::json loadDefaultKeybindings()
{
    std::ifstream file(std::filesystem::path(RTYPE_CONFIG_DIR) / "keybindings.json");

    return nlohmann::json::parse(file, nullptr, false);
}
}

TEST(KeybindingsFileTests, DefaultFileHasTheConfigArrayActionMapReads)
{
    const nlohmann::json json = loadDefaultKeybindings();

    ASSERT_TRUE(json.is_object());
    ASSERT_TRUE(json.contains("config"));
    EXPECT_TRUE(json.at("config").is_array());
    EXPECT_FALSE(json.at("config").empty());
}

TEST(KeybindingsFileTests, EveryEntryNamesAnAction)
{
    const nlohmann::json json = loadDefaultKeybindings();

    ASSERT_TRUE(json.contains("config"));
    for (const auto &entry : json.at("config"))
        EXPECT_TRUE(ACTION_NAMES.contains(entry.at("name").get<std::string>()))
            << entry.at("name") << " is not an Action";
}

TEST(KeybindingsFileTests, EveryKeyIsKnownToTheEngine)
{
    const nlohmann::json json = loadDefaultKeybindings();

    ASSERT_TRUE(json.contains("config"));
    for (const auto &entry : json.at("config")) {
        EXPECT_FALSE(entry.at("keys").empty()) << entry.at("name") << " has no key";
        for (const auto &key : entry.at("keys"))
            EXPECT_TRUE(engine::input::keyFromString(key.get<std::string>()).has_value())
                << key << " is not a key name";
    }
}

TEST(KeybindingsFileTests, MovementUsesTheArrowsAndFireUsesSpace)
{
    const nlohmann::json json = loadDefaultKeybindings();
    std::set<std::string> boundKeys;

    ASSERT_TRUE(json.contains("config"));
    for (const auto &entry : json.at("config"))
        for (const auto &key : entry.at("keys"))
            boundKeys.insert(key.get<std::string>());
    for (const char *required : {"UP", "DOWN", "LEFT", "RIGHT", "SPACE"})
        EXPECT_TRUE(boundKeys.contains(required)) << required << " is not bound";
}
}
