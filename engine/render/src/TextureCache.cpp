#include "TextureCache.hpp"

#include <raylib.h>

#include <cstdint>
#include <filesystem>
#include <limits>
#include <unordered_map>
#include <vector>

#include "Errors.hpp"

namespace engine::render {

namespace {
constexpr std::size_t MAX_TEXTURES = std::numeric_limits<std::uint16_t>::max();   // 65 535
}

struct TextureCache::Impl {
    std::vector<Texture2D> textures;                                 // index == TextureId::value
    std::unordered_map<std::filesystem::path, TextureId> pathToId;   // key: normalized path
};

TextureCache::TextureCache() : _impl(std::make_unique<Impl>()) {}

TextureCache::~TextureCache()
{
    if (!_impl)
        return;
    for (const Texture2D &texture : _impl->textures)
        UnloadTexture(texture);
}

TextureId TextureCache::load(const std::filesystem::path &path)
{
    const std::filesystem::path key = path.lexically_normal();

    if (const auto it = _impl->pathToId.find(key); it != _impl->pathToId.end())
        return it->second;

    if (!std::filesystem::is_regular_file(key))
        throw core::ResourceError("Texture file does not exist: " + key.string());
    if (_impl->textures.size() >= MAX_TEXTURES)
        throw core::ResourceError("Too many textures (max 65535): " + key.string());

    // No SetTextureFilter: Raylib's default is point filtering, which pixel art needs.
    const Texture2D texture = LoadTexture(key.string().c_str());
    if (!IsTextureValid(texture))
        throw core::ResourceError("Failed to decode texture: " + key.string());

    const TextureId id{static_cast<std::uint16_t>(_impl->textures.size())};
    _impl->textures.push_back(texture);
    _impl->pathToId.emplace(key, id);
    return id;
}

core::Vec2u TextureCache::size(TextureId id) const
{
    const Texture2D &texture = _impl->textures.at(id.value);
    return {static_cast<unsigned>(texture.width), static_cast<unsigned>(texture.height)};
}

} // namespace engine::render