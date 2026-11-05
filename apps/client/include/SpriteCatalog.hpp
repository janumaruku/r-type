/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SpriteCatalog.hpp
*/

#ifndef R_TYPE_SPRITE_CATALOG_HPP
#define R_TYPE_SPRITE_CATALOG_HPP

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "engine/core/Rect2.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/render/TextureCache.hpp"

namespace rtype::client {
enum class SpriteKey : std::uint8_t {
    PLAYER_SHIP = 1,
    BYDOS_SLAVE = 2,
    PLAYER_MISSILE = 3,
    ENEMY_MISSILE = 4,
    EXPLOSION = 100,
};

[[nodiscard]] std::string_view toString(SpriteKey key) noexcept;

std::ostream &operator<<(std::ostream &stream, const SpriteKey &key);

struct SpriteDefId {
    std::uint16_t value{};

    bool operator==(const SpriteDefId &) const = default;
};

struct AnimationState {
    std::string name;
    std::vector<engine::core::IntRect> frames;
};

struct SpriteDef {
    engine::render::TextureId texture;
    std::vector<AnimationState> states;
    float frameTime = 0.0F;
    bool loop = true;

    [[nodiscard]] std::optional<std::uint8_t> stateIndex(std::string_view name)
    const noexcept;
};

class SpriteCatalog {
public:
    void load(const std::filesystem::path &file,
        engine::render::TextureCache &textures);

    void loadJson(const nlohmann::ordered_json &json,
        const std::filesystem::path &sheetsDir,
        engine::render::TextureCache &textures);

    [[nodiscard]] SpriteDefId find(SpriteKey key, std::uint8_t variant) const;

    [[nodiscard]] const SpriteDef &get(SpriteDefId id) const;

    [[nodiscard]] bool contains(SpriteKey key,
        std::uint8_t variant) const noexcept;

private:
    void add(const nlohmann::ordered_json &entry, const std::string &where,
        const std::filesystem::path &sheetsDir,
        engine::render::TextureCache &textures);

    std::vector<SpriteDef> _defs;
    std::map<std::pair<SpriteKey, std::uint8_t>, SpriteDefId> _exact;
    std::map<SpriteKey, SpriteDefId> _anyVariant;
};

[[nodiscard]] std::optional<SpriteKey> spriteKeyFromString(std::string_view
    name) noexcept;

class CatalogError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
} // namespace rtype::client
#endif // R_TYPE_SPRITE_CATALOG_HPP
