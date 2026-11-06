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
#include <limits>

#include <nlohmann/json.hpp>

namespace rtype::client {

namespace {
using Json = nlohmann::ordered_json;

constexpr std::size_t MAX_DEFS = 65535;
constexpr std::size_t MAX_STATES = 256;
constexpr std::string_view IDLE = "idle";

constexpr std::array KEYS{
    SpriteKey::PLAYER_SHIP, SpriteKey::BYDOS_SLAVE, SpriteKey::PLAYER_MISSILE,
    SpriteKey::ENEMY_MISSILE, SpriteKey::EXPLOSION,
};

/*
** Every (kind, variant) the server can send, plus the client-only
** explosion. A catalog without one of them can't draw the game.
*/
constexpr std::array<std::pair<SpriteKey, std::uint8_t>, 9> REQUIRED{{
    {SpriteKey::PLAYER_SHIP, 0}, {SpriteKey::PLAYER_SHIP, 1},
    {SpriteKey::PLAYER_SHIP, 2}, {SpriteKey::PLAYER_SHIP, 3},
    {SpriteKey::BYDOS_SLAVE, 0}, {SpriteKey::BYDOS_SLAVE, 1},
    {SpriteKey::PLAYER_MISSILE, 0}, {SpriteKey::ENEMY_MISSILE, 0},
    {SpriteKey::EXPLOSION, 0},
}};

[[noreturn]] void fail(const std::string &where, std::string_view problem)
{
    throw CatalogError(std::format("{}: {}", where, problem));
}

[[nodiscard]] bool sameLetter(char left, char right) noexcept
{
    return std::toupper(static_cast<unsigned char>(left))
        == std::toupper(static_cast<unsigned char>(right));
}

[[nodiscard]] const Json &field(const Json &entry, std::string_view name,
    const std::string &where)
{
    const auto it = entry.find(name);

    if (it == entry.end())
        fail(std::format("{}.{}", where, name), "missing");
    return *it;
}

[[nodiscard]] SpriteKey readKind(const Json &entry, const std::string &where)
{
    const auto &kind = field(entry, "kind", where);
    std::optional<SpriteKey> key;

    if (kind.is_string())
        key = spriteKeyFromString(kind.get<std::string>());
    if (!key)
        fail(where + ".kind", std::format("unknown sprite kind {}",
            kind.dump()));
    return *key;
}

/*
** nullopt for "*", which stands for every variant of the kind.
*/
[[nodiscard]] std::optional<std::uint8_t> readVariant(const Json &entry,
    const std::string &where)
{
    const auto &variant = field(entry, "variant", where);

    if (variant.is_string() && variant.get<std::string>() == "*")
        return std::nullopt;
    if (!variant.is_number_integer() || variant.get<long long>() < 0
        || variant.get<long long>() > 255)
        fail(where + ".variant", std::format(
            "expected a whole number from 0 to 255 or \"*\", got {}",
            variant.dump()));
    return static_cast<std::uint8_t>(variant.get<long long>());
}

[[nodiscard]] std::string readSheet(const Json &entry,
    const std::string &where)
{
    const auto &sheet = field(entry, "sheet", where);

    if (!sheet.is_string() || sheet.get<std::string>().empty())
        fail(where + ".sheet", "expected a file name");
    return sheet.get<std::string>();
}

[[nodiscard]] float readFrameTime(const Json &entry, const std::string &where)
{
    const auto &frameTime = field(entry, "frameTime", where);

    if (!frameTime.is_number() || !(frameTime.get<double>() >= 0.0))
        fail(where + ".frameTime", std::format(
            "expected a number of seconds >= 0, got {}", frameTime.dump()));
    return frameTime.get<float>();
}

[[nodiscard]] bool readLoop(const Json &entry, const std::string &where)
{
    const auto it = entry.find("loop");

    if (it == entry.end())
        return true;
    if (!it->is_boolean())
        fail(where + ".loop", std::format("expected true or false, got {}",
            it->dump()));
    return it->get<bool>();
}

[[nodiscard]] bool isWholeNumber(const Json &value, long long min) noexcept
{
    return value.is_number_integer() && value.get<long long>() >= min
        && value.get<long long>() <= std::numeric_limits<std::int32_t>::max();
}

[[nodiscard]] engine::core::IntRect readRect(const Json &rect,
    const std::string &where)
{
    if (!rect.is_array() || rect.size() != 4 || !isWholeNumber(rect[0], 0)
        || !isWholeNumber(rect[1], 0) || !isWholeNumber(rect[2], 1)
        || !isWholeNumber(rect[3], 1))
        fail(where, "expected [x, y, w, h], whole numbers with w and h > 0");
    return {{rect[0].get<std::int32_t>(), rect[1].get<std::int32_t>()},
        rect[2].get<std::int32_t>(), rect[3].get<std::int32_t>()};
}

[[nodiscard]] AnimationState readState(const std::string &name,
    const Json &frames, const std::string &where)
{
    AnimationState state{.name = name, .frames = {}};

    if (!frames.is_array() || frames.empty())
        fail(where, "expected a non-empty array of [x, y, w, h]");
    for (std::size_t index = 0; index < frames.size(); ++index)
        state.frames.push_back(readRect(frames[index],
            std::format("{}[{}]", where, index)));
    return state;
}

/*
** idle comes first, so state index 0 is idle everywhere. The other states
** follow in file order, which ordered_json keeps.
*/
[[nodiscard]] std::vector<AnimationState> readStates(const Json &entry,
    const std::string &where)
{
    const auto &states = field(entry, "states", where);
    std::vector<AnimationState> result;

    if (!states.is_object() || states.empty())
        fail(where + ".states", "expected an object with at least one state");
    if (states.size() > MAX_STATES)
        fail(where + ".states", std::format("more than {} states",
            MAX_STATES));
    if (!states.contains(IDLE))
        fail(where + ".states", "an idle state is required");
    result.push_back(readState(std::string(IDLE), states.at(IDLE),
        where + ".states.idle"));
    for (const auto &[name, frames] : states.items()) {
        if (name.empty())
            fail(where + ".states", "a state name can't be empty");
        if (name != IDLE)
            result.push_back(readState(name, frames,
                std::format("{}.states.{}", where, name)));
    }
    return result;
}

void checkInside(const SpriteDef &def, const engine::core::Vec2u &sheetSize,
    const std::string &where)
{
    for (const auto &state : def.states)
        for (std::size_t index = 0; index < state.frames.size(); ++index) {
            const auto &frame = state.frames[index];
            const auto right = static_cast<long long>(frame.origin().x())
                + frame.width();
            const auto bottom = static_cast<long long>(frame.origin().y())
                + frame.height();

            if (right > sheetSize.x() || bottom > sheetSize.y())
                fail(std::format("{}.states.{}[{}]", where, state.name, index),
                    std::format("outside the {} x {} sheet", sheetSize.x(),
                        sheetSize.y()));
        }
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
    Json json;

    if (!stream.is_open())
        throw CatalogError(std::format("{}: cannot open the sprite catalog",
            file.string()));
    try {
        json = Json::parse(stream);
    } catch (const nlohmann::json::parse_error &error) {
        throw CatalogError(std::format("{}: {}", file.string(), error.what()));
    }
    loadJson(json, file.parent_path(), textures);
}

void SpriteCatalog::loadJson(const nlohmann::ordered_json &json,
    const std::filesystem::path &sheetsDir,
    engine::render::TextureCache &textures)
{
    if (!json.is_object() || !json.contains("sprites")
        || !json.at("sprites").is_array())
        fail("sprites", "expected an object with a \"sprites\" array");
    SpriteCatalog loaded;
    const auto &entries = json.at("sprites");

    if (entries.size() > MAX_DEFS)
        fail("sprites", std::format("more than {} entries", MAX_DEFS));
    for (std::size_t index = 0; index < entries.size(); ++index)
        loaded.add(entries.at(index), std::format("sprites[{}]", index),
            sheetsDir, textures);
    for (const auto &[key, variant] : REQUIRED)
        if (!loaded.contains(key, variant))
            fail("sprites", std::format("no entry for {} variant {}",
                toString(key), variant));
    *this = std::move(loaded);
}

void SpriteCatalog::add(const nlohmann::ordered_json &entry,
    const std::string &where, const std::filesystem::path &sheetsDir,
    engine::render::TextureCache &textures)
{
    if (!entry.is_object())
        fail(where, "expected an object");
    if (entry.contains("frames"))
        fail(where + ".frames", "replaced by states, see client.md");
    const SpriteKey key = readKind(entry, where);
    const std::optional<std::uint8_t> variant = readVariant(entry, where);
    SpriteDef def;

    def.frameTime = readFrameTime(entry, where);
    def.loop = readLoop(entry, where);
    def.states = readStates(entry, where);
    def.texture = textures.load(sheetsDir / readSheet(entry, where));
    checkInside(def, textures.size(def.texture), where);
    const SpriteDefId id{static_cast<std::uint16_t>(_defs.size())};

    if (variant) {
        if (!_exact.emplace(std::pair{key, *variant}, id).second)
            fail(where, std::format("{} variant {} is already defined",
                toString(key), *variant));
    } else if (!_anyVariant.emplace(key, id).second) {
        fail(where, std::format("{} \"*\" is already defined",
            toString(key)));
    }
    _defs.push_back(std::move(def));
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
