/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientSettings.cpp
*/

#include "ClientSettings.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <string_view>

#include "ClientConfig.hpp"
#include "engine/core/Logger.hpp"

namespace rtype::client {
namespace {
constexpr std::array<std::string_view, 8> KEYS{
    "masterVolume",
    "musicVolume",
    "sfxVolume",
    "muted",
    "hudFontSize",
    "highContrast",
    "reduceFlashes",
    "fullscreen",
};

constexpr long long MIN_HUD_FONT_SIZE = 8;
constexpr long long MAX_HUD_FONT_SIZE = 96;

engine::core::log::Logger &logger()
{
    static engine::core::log::Logger instance("settings");

    return instance;
}

/*
** A number is clamped to [0, 1], with a warning when it had to be. Anything
** else, NaN included, keeps the default.
*/
void readVolume(const nlohmann::json &json, std::string_view key, float &field)
{
    const auto it = json.find(key);

    if (it == json.end())
        return;
    if (!it->is_number() || std::isnan(it->get<double>())) {
        logger().warn("{}: must be a number, keeping {}", key, field);
        return;
    }
    const auto value   = it->get<double>();
    const auto clamped = std::clamp(value, 0.0, 1.0);
    if (clamped != value)
        logger().warn("{}: {} is out of [0, 1], using {}", key, value, clamped);
    field = static_cast<float>(clamped);
}

void readBool(const nlohmann::json &json, std::string_view key, bool &field)
{
    const auto it = json.find(key);

    if (it == json.end())
        return;
    if (!it->is_boolean()) {
        logger().warn("{}: must be true or false, keeping {}", key, field);
        return;
    }
    field = it->get<bool>();
}

void readFontSize(const nlohmann::json &json, std::string_view key,
    unsigned &field)
{
    const auto it = json.find(key);

    if (it == json.end())
        return;
    if (!it->is_number_integer()) {
        logger().warn("{}: must be a whole number, keeping {}", key, field);
        return;
    }
    const auto value = it->get<long long>();
    if (value < MIN_HUD_FONT_SIZE || value > MAX_HUD_FONT_SIZE) {
        logger().warn("{}: {} is out of [{}, {}], keeping {}", key, value,
            MIN_HUD_FONT_SIZE, MAX_HUD_FONT_SIZE, field);
        return;
    }
    field = static_cast<unsigned>(value);
}
} // namespace

ClientSettings ClientSettings::load(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path)) {
        logger().info("{}: not found, using the default settings",
            path.string());
        return {};
    }
    std::ifstream file(path);
    if (!file.is_open())
        throw ConfigError("cannot open " + path.string());
    const auto json = nlohmann::json::parse(file, nullptr, false);
    if (json.is_discarded()) {
        logger().warn("{}: not valid JSON, using the default settings",
            path.string());
        return {};
    }
    if (!json.is_object()) {
        logger().warn("{}: the top level is not an object, using the "
                      "default settings",
            path.string());
        return {};
    }
    return fromJson(json);
}

ClientSettings ClientSettings::fromJson(const nlohmann::json &json)
{
    ClientSettings settings;

    if (!json.is_object()) {
        logger().warn("settings must be a JSON object, using the defaults");
        return settings;
    }
    readVolume(json, "masterVolume", settings.masterVolume);
    readVolume(json, "musicVolume", settings.musicVolume);
    readVolume(json, "sfxVolume", settings.sfxVolume);
    readBool(json, "muted", settings.muted);
    readFontSize(json, "hudFontSize", settings.hudFontSize);
    readBool(json, "highContrast", settings.highContrast);
    readBool(json, "reduceFlashes", settings.reduceFlashes);
    readBool(json, "fullscreen", settings.fullscreen);
    for (const auto &[key, value]: json.items())
        if (std::ranges::find(KEYS, key) == KEYS.end())
            logger().warn("{}: unknown setting, ignored", key);
    return settings;
}
} // namespace rtype::client
