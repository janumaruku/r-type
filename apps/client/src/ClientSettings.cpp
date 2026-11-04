/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientSettings.hpp
*/

#include "client/src/ClientSettings.hpp"

#include <cstring>

#include "client/src/ClientConfig.hpp"
#include <fstream>
#include <iostream>

#include "engine/core/Logger.hpp"

namespace rtype::client {
ClientSettings ClientSettings::load(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path)) {
        // call log (file not found)
        // TODO log errors on exceptions or failed branches
        engine::core::log::Logger logger("ClientSettings");
        logger.error("failed to find file: {}", std::strerror(errno));
        return ClientSettings{};
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        throw ConfigError("Failed to open file: " + path.string());
    }
    nlohmann::json json;
    auto parsed = nlohmann::json::parse(file, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object()) {
        engine::core::log::Logger logger("ClientSettings");
        logger.error("json is invalid: {}", std::strerror(errno));
        return ClientSettings{};
    }
    return fromJson(parsed);
}

ClientSettings ClientSettings::fromJson(const nlohmann::json &json)
{
    ClientSettings settings;

    if (!json.is_object())
        return settings;
    if (json.contains("masterVolume")) {
        if (json["masterVolume"].is_number()) {
            float val = json["masterVolume"].get<float>();
            if (!std::isnan(val))
                settings.masterVolume = std::clamp(val, 0.0f, 1.0f);
        } else {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("wrong type for masterVolume: {}", std::strerror(errno));
        }
    }
    if (json.contains("musicVolume")) {
        if (json["musicVolume"].is_number()) {
            float val = json["musicVolume"].get<float>();
            if (!std::isnan(val))
                settings.musicVolume = std::clamp(val, 0.0f, 1.0f);
        } else {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("wrong type for musicVolume: {}", std::strerror(errno));
        }
    }
    if (json.contains("sfxVolume")) {
        if (json["sfxVolume"].is_number()) {
            float val = json["sfxVolume"].get<float>();
            if (!std::isnan(val))
                settings.sfxVolume = std::clamp(val, 0.0f, 1.0f);
        } else {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("wrong type for sfxVolume: {}", std::strerror(errno));
        }
    }

    if (json.contains("muted")) {
        if (json["muted"].is_boolean()) {
            settings.muted = json["muted"].get<bool>();
        } else {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("wrong type for muted (must be bool): {}", std::strerror(errno));
        }
    }

    if (json.contains("hudFontSize")) {
        if (json["hudFontSize"].is_number_unsigned()) {
            unsigned size = json["hudFontSize"].get<unsigned>();
            if (size >= 8 && size <= 96) {
                settings.hudFontSize = size;
            }
        } else {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("hudFrontSize is or wrong type or is out of range: {}", std::strerror(errno));
        }
    }

    if (json.contains("highContrast")) {
        if (json["highContrast"].is_boolean()) {
            settings.highContrast = json["highContrast"].get<bool>();
        } else {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("wrong type for highContrast (must be bool): {}", std::strerror(errno));
        }
    }

    if (json.contains("reduceFlashes")) {
        if (json["reduceFlashes"].is_boolean()) {
            settings.reduceFlashes = json["reduceFlashes"].get<bool>();
        } else {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("wrong type for reduceFlashes (must be bool): {}", std::strerror(errno));
        }
    }

    if (json.contains("fullscreen")) {
        if (json["fullscreen"].is_boolean()) {
            settings.fullscreen = json["fullscreen"].get<bool>();
        } else {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("wrong type for fullscreen (must be bool): {}", std::strerror(errno));
        }
    }

    for (auto it = json.begin(); it != json.end(); ++it) {
        const auto &key = it.key();
        if (key != "masterVolume" && key != "musicVolume" && key != "sfxVolume"
            &&
            key != "muted" && key != "hudFontSize" && key != "highContrast" &&
            key != "reduceFlashes" && key != "fullscreen") {
            engine::core::log::Logger logger("ClientSettings");
            logger.error("key is wrong: {}", std::strerror(errno));
        }
    }

    return settings;
}

}
