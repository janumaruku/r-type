/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SpriteCatalog
*/

#include "SpriteCatalog.hpp"

#include <format>
#include <ostream>
#include <stdexcept>

namespace rtype::client {

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
