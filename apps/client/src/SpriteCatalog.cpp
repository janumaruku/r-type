/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SpriteCatalog
*/

#include "SpriteCatalog.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <format>
#include <fstream>
#include <ostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace rtype::client {

namespace {
constexpr std::string_view IDLE = "idle";

constexpr std::array KEYS{
    SpriteKey::PLAYER_SHIP, SpriteKey::BYDOS_SLAVE, SpriteKey::PLAYER_MISSILE,
    SpriteKey::ENEMY_MISSILE, SpriteKey::EXPLOSION,
};

[[nodiscard]] bool sameLetter(char left, char right) noexcept
{
    return std::toupper(static_cast<unsigned char>(left))
        == std::toupper(static_cast<unsigned char>(right));
}

/*
** nullopt for "*", which stands for every variant of the kind.
*/
[[nodiscard]] std::optional<std::uint8_t> readVariant(
    const nlohmann::json &variant)
{
    if (variant.is_string() && variant.get<std::string>() == "*")
        return std::nullopt;
    return variant.get<std::uint8_t>();
}

[[nodiscard]] AnimationState readState(const std::string &name,
    const nlohmann::json &frames)
{
    AnimationState state{.name = name, .frames = {}};

    for (const auto &frame : frames) {
        const auto rect = frame.get<std::array<std::int32_t, 4>>();

        state.frames.emplace_back(engine::core::Vec2<std::int32_t>{rect[0],
            rect[1]}, rect[2], rect[3]);
    }
    return state;
}

/*
** idle comes first, so state index 0 is idle everywhere.
*/
[[nodiscard]] std::vector<AnimationState> readStates(
    const nlohmann::json &states)
{
    std::vector<AnimationState> result;

    result.push_back(readState(std::string(IDLE), states.at(IDLE)));
    for (const auto &[name, frames] : states.items())
        if (name != IDLE)
            result.push_back(readState(name, frames));
    return result;
}
}

std::string_view toString(SpriteKey key) noexcept
{
    switch (key) {
    case SpriteKey::PLAYER_SHIP:
        return "PLAYER_SHIP";
    case SpriteKey::BYDOS_SLAVE:
        return "BYDOS_SLAVE";
    case SpriteKey::PLAYER_MISSILE:
        return "PLAYER_MISSILE";
    case SpriteKey::ENEMY_MISSILE:
        return "ENEMY_MISSILE";
    case SpriteKey::EXPLOSION:
        return "EXPLOSION";
    }

    return "UNKNOWN";
}

std::ostream &operator<<(std::ostream &stream, const SpriteKey &key)
{
    return stream << toString(key);
}

std::optional<std::uint8_t> SpriteDef::stateIndex(
    std::string_view name) const noexcept
{
    for (std::size_t index = 0; index < states.size(); ++index)
        if (states[index].name == name)
            return static_cast<std::uint8_t>(index);

    return std::nullopt;
}

void SpriteCatalog::load(const std::filesystem::path &file,
    engine::render::TextureCache &textures)
{
    std::ifstream stream(file, std::ios::binary);
    nlohmann::json json;

    if (!stream.is_open())
        throw CatalogError(std::format("{}: cannot open the sprite catalog",
            file.string()));
    try {
        json = nlohmann::json::parse(stream);
    } catch (const nlohmann::json::exception &error) {
        throw CatalogError(std::format("{}: {}", file.string(), error.what()));
    }
    loadJson(json, file.parent_path(), textures);
}

/*
** Builds the catalog aside and only replaces this one at the end, so an
** error leaves the catalog as it was.
*/
void SpriteCatalog::loadJson(const nlohmann::json &json,
    const std::filesystem::path &sheetsDir,
    engine::render::TextureCache &textures)
{
    SpriteCatalog loaded;

    try {
        for (const auto &entry : json.at("sprites"))
            loaded.add(entry, sheetsDir, textures);
    } catch (const nlohmann::json::exception &error) {
        throw CatalogError(error.what());
    }
    *this = std::move(loaded);
}

void SpriteCatalog::add(const nlohmann::json &entry,
    const std::filesystem::path &sheetsDir,
    engine::render::TextureCache &textures)
{
    const auto &kindName = entry.at("kind");
    const std::optional<SpriteKey> kind = spriteKeyFromString(
        kindName.get<std::string>());

    if (!kind)
        throw CatalogError(std::format("unknown sprite kind {}",
            kindName.dump()));
    const SpriteDefId id{static_cast<std::uint16_t>(_defs.size())};

    if (const auto variant = readVariant(entry.at("variant")))
        _exact[{*kind, *variant}] = id;
    else
        _anyVariant[*kind] = id;
    _defs.push_back({
        .texture = textures.load(sheetsDir
            / entry.at("sheet").get<std::string>()),
        .states = readStates(entry.at("states")),
        .frameTime = entry.at("frameTime").get<float>(),
        .loop = entry.value("loop", true),
    });
}

std::optional<SpriteKey> spriteKeyFromString(std::string_view name) noexcept
{
    for (const SpriteKey key : KEYS)
        if (std::ranges::equal(toString(key), name, sameLetter))
            return key;
    return std::nullopt;
}

SpriteDefId SpriteCatalog::find(SpriteKey key, std::uint8_t variant) const
{
    if (const auto it = _exact.find({key, variant}); it != _exact.end())
        return it->second;
    if (const auto it = _anyVariant.find(key); it != _anyVariant.end())
        return it->second;

    throw CatalogError(std::format("no sprite for kind {} variant {}",
        toString(key), variant));
}

const SpriteDef &SpriteCatalog::get(SpriteDefId id) const
{
    if (id.value >= _defs.size())
        throw CatalogError(std::format(
            "no sprite definition {} (the catalog has {})",
            id.value, _defs.size()));

    return _defs[id.value];
}

bool SpriteCatalog::contains(SpriteKey key, std::uint8_t variant) const noexcept
{
    return _exact.contains({key, variant}) || _anyVariant.contains(key);
}

} // namespace rtype::client
