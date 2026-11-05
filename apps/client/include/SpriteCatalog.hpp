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
    #include <nlohmann/json_fwd.hpp>
    #include <optional>

    #include "../../../engine/render/include/engine/render/DrawCommand.hpp"
#include "../../../engine/render/include/engine/render/TextureCache.hpp"
namespace rtype::client {
enum class SpriteKey : std::uint8_t {
    PLAYER_SHIP    = 1,
    BYDOS_SLAVE    = 2,
    PLAYER_MISSILE = 3,
    ENEMY_MISSILE  = 4,
    EXPLOSION      = 100,
};

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
    bool loop       = true;

    [[nodiscard]] std::optional<std::uint8_t> stateIndex(std::string_view name)
        const noexcept;
};

class SpriteCatalog {
public:
    void load(const std::filesystem::path &file,
        engine::render::TextureCache &textures);
    void loadJson(const nlohmann::json &json,
        const std::filesystem::path &sheetsDir,
        engine::render::TextureCache &textures);
    [[nodiscard]] SpriteDefId find(SpriteKey key, std::uint8_t variant) const;
    [[nodiscard]] const SpriteDef &get(SpriteDefId id) const;
    [[nodiscard]] bool contains(SpriteKey key,
        std::uint8_t variant) const noexcept;

private:
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
#endif /* !SPRITE_CATALOG_HPP_ */
