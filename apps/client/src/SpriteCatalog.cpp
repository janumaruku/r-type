#include "SpriteCatalog.hpp"

#include <stdexcept>

namespace rtype::client {

SpriteDefId SpriteCatalog::find(SpriteKey key, std::uint8_t variant) const
{
    if (const auto it = _exact.find({key, variant}); it != _exact.end())
        return it->second;
    if (const auto it = _anyVariant.find(key); it != _anyVariant.end())
        return it->second;
    throw CatalogError("sprite key is not present in catalog");
}

const SpriteDef &SpriteCatalog::get(SpriteDefId id) const
{
    if (id.value >= _defs.size())
        throw CatalogError("sprite definition id is out of range");
    return _defs[id.value];
}

bool SpriteCatalog::contains(SpriteKey key, std::uint8_t variant) const noexcept
{
    return _exact.contains({key, variant}) || _anyVariant.contains(key);
}

} // namespace rtype::client
