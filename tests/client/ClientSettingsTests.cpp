/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientSettingsTests.cpp
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <set>
#include <string>

#include "ClientSettings.hpp"

namespace rtype::client {
namespace {

void expectDefaults(const ClientSettings &settings)
{
    const ClientSettings defaults;

    EXPECT_FLOAT_EQ(settings.masterVolume, defaults.masterVolume);
    EXPECT_FLOAT_EQ(settings.musicVolume, defaults.musicVolume);
    EXPECT_FLOAT_EQ(settings.sfxVolume, defaults.sfxVolume);
    EXPECT_EQ(settings.muted, defaults.muted);
    EXPECT_EQ(settings.hudFontSize, defaults.hudFontSize);
    EXPECT_EQ(settings.highContrast, defaults.highContrast);
    EXPECT_EQ(settings.reduceFlashes, defaults.reduceFlashes);
    EXPECT_EQ(settings.fullscreen, defaults.fullscreen);
}

class ClientSettingsFileTests : public ::testing::Test {
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

    std::filesystem::path _path = std::filesystem::temp_directory_path()
        / ("rtype_client_settings_"
            + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name())
            + ".json");
};
}

TEST(ClientSettingsTests, EmptyObjectGivesTheDefaults)
{
    expectDefaults(ClientSettings::fromJson(nlohmann::json::object()));
}

TEST(ClientSettingsTests, EveryValidKeyIsApplied)
{
    const ClientSettings settings = ClientSettings::fromJson({
        {"masterVolume", 0.5},
        {"musicVolume", 0.25},
        {"sfxVolume", 0.0},
        {"muted", true},
        {"hudFontSize", 32},
        {"highContrast", true},
        {"reduceFlashes", true},
        {"fullscreen", true},
    });

    EXPECT_FLOAT_EQ(settings.masterVolume, 0.5F);
    EXPECT_FLOAT_EQ(settings.musicVolume, 0.25F);
    EXPECT_FLOAT_EQ(settings.sfxVolume, 0.0F);
    EXPECT_TRUE(settings.muted);
    EXPECT_EQ(settings.hudFontSize, 32U);
    EXPECT_TRUE(settings.highContrast);
    EXPECT_TRUE(settings.reduceFlashes);
    EXPECT_TRUE(settings.fullscreen);
}

TEST(ClientSettingsTests, MissingKeysKeepTheirDefaults)
{
    const ClientSettings settings = ClientSettings::fromJson({{"muted", true}});

    EXPECT_TRUE(settings.muted);
    EXPECT_FLOAT_EQ(settings.masterVolume, ClientSettings{}.masterVolume);
    EXPECT_EQ(settings.hudFontSize, ClientSettings{}.hudFontSize);
}

TEST(ClientSettingsTests, VolumeAboveOneIsClamped)
{
    EXPECT_FLOAT_EQ(ClientSettings::fromJson({{"sfxVolume", 2}}).sfxVolume, 1.0F);
}

TEST(ClientSettingsTests, VolumeBelowZeroIsClamped)
{
    EXPECT_FLOAT_EQ(ClientSettings::fromJson({{"musicVolume", -1}}).musicVolume, 0.0F);
}

TEST(ClientSettingsTests, VolumeOfTheWrongTypeKeepsItsDefault)
{
    EXPECT_FLOAT_EQ(ClientSettings::fromJson({{"masterVolume", "loud"}}).masterVolume,
        ClientSettings{}.masterVolume);
}

TEST(ClientSettingsTests, BooleanOfTheWrongTypeKeepsItsDefault)
{
    EXPECT_FALSE(ClientSettings::fromJson({{"muted", "yes"}}).muted);
    EXPECT_FALSE(ClientSettings::fromJson({{"fullscreen", 1}}).fullscreen);
}

TEST(ClientSettingsTests, HudFontSizeOutOfRangeKeepsItsDefault)
{
    EXPECT_EQ(ClientSettings::fromJson({{"hudFontSize", 500}}).hudFontSize, 20U);
    EXPECT_EQ(ClientSettings::fromJson({{"hudFontSize", 7}}).hudFontSize, 20U);
    EXPECT_EQ(ClientSettings::fromJson({{"hudFontSize", -10}}).hudFontSize, 20U);
}

TEST(ClientSettingsTests, HudFontSizeBoundsAreAccepted)
{
    EXPECT_EQ(ClientSettings::fromJson({{"hudFontSize", 8}}).hudFontSize, 8U);
    EXPECT_EQ(ClientSettings::fromJson({{"hudFontSize", 96}}).hudFontSize, 96U);
}

TEST(ClientSettingsTests, HudFontSizeThatIsNotAWholeNumberKeepsItsDefault)
{
    EXPECT_EQ(ClientSettings::fromJson({{"hudFontSize", 20.5}}).hudFontSize, 20U);
    EXPECT_EQ(ClientSettings::fromJson({{"hudFontSize", "big"}}).hudFontSize, 20U);
}

TEST(ClientSettingsTests, UnknownKeyIsIgnored)
{
    expectDefaults(ClientSettings::fromJson({{"colour", "blue"}}));
}

TEST_F(ClientSettingsFileTests, MissingFileGivesTheDefaults)
{
    expectDefaults(ClientSettings::load(_path));
}

TEST_F(ClientSettingsFileTests, ValidFileIsRead)
{
    const ClientSettings settings = ClientSettings::load(write(R"({"muted": true, "hudFontSize": 40})"));

    EXPECT_TRUE(settings.muted);
    EXPECT_EQ(settings.hudFontSize, 40U);
}

TEST_F(ClientSettingsFileTests, InvalidJsonGivesTheDefaults)
{
    expectDefaults(ClientSettings::load(write(R"({ "muted": tru)")));
}

TEST_F(ClientSettingsFileTests, TopLevelThatIsNotAnObjectGivesTheDefaults)
{
    expectDefaults(ClientSettings::load(write("[1, 2]")));
}

TEST(ClientSettingsTests, DefaultConfigFileHoldsTheDefaults)
{
    const std::filesystem::path path = std::filesystem::path(RTYPE_CONFIG_DIR) / "client.json";

    ASSERT_TRUE(std::filesystem::exists(path));
    expectDefaults(ClientSettings::load(path));
}

TEST(ClientSettingsTests, DefaultConfigFileHoldsExactlyTheSettingKeys)
{
    std::ifstream file(std::filesystem::path(RTYPE_CONFIG_DIR) / "client.json");
    const nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
    std::set<std::string> keys;

    ASSERT_TRUE(json.is_object());
    for (const auto &[key, value] : json.items())
        keys.insert(key);
    EXPECT_EQ(keys, (std::set<std::string>{"masterVolume", "musicVolume", "sfxVolume", "muted",
        "hudFontSize", "highContrast", "reduceFlashes", "fullscreen"}));
}
}
