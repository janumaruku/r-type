/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientSettings.hpp
*/

#ifndef R_TYPE_CLIENTSETTINGS_HPP
#define R_TYPE_CLIENTSETTINGS_HPP

#include <filesystem>

#include <nlohmann/json.hpp>

namespace rtype::client {
struct ClientSettings {
    float masterVolume = 1.0F;
    float musicVolume = 0.6F;
    float sfxVolume = 0.8F;
    bool muted = false;
    unsigned hudFontSize = 20;
    bool highContrast = false;
    bool reduceFlashes = false;
    bool fullscreen = false;

    [[nodiscard]] static ClientSettings load(const std::filesystem::path &path);
    [[nodiscard]] static ClientSettings fromJson(const nlohmann::json &json);
};
}

#endif //R_TYPE_CLIENTSETTINGS_HPP
