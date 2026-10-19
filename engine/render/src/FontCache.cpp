/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FontCache.cpp
*/


#include <vector>
#include <unordered_map>
#include "FontCache.hpp"
#include "Errors.hpp"

namespace engine::render {

struct FontCache::Impl {
    std::vector<Font> fonts;                                 // index == FontId::value
    std::unordered_map<std::filesystem::path, FontId> pathToId;   // key: normalized path
};

FontCache::FontCache() : _impl(std::make_unique<Impl>()) {}

FontCache::~FontCache()
{
    if (!_impl)
        return;
    for (const Font &font : _impl->fonts)
        UnloadFont(font);
}

FontId FontCache::load(const std::filesystem::path &path)
{
    const std::filesystem::path key = path.lexically_normal();

    if (const auto it = _impl->pathToId.find(key); it != _impl->pathToId.end())
        return it->second;

    if (!std::filesystem::is_regular_file(key))
        throw core::ResourceError("Font file does not exist: " + key.string());

    const Font font = LoadFont(key.string().c_str());
    if (!IsFontValid(font))
        throw core::ResourceError("Failed to decode font: " + key.string());

    const FontId id{static_cast<std::uint32_t>(_impl->fonts.size())};
    _impl->fonts.push_back(font);
    _impl->pathToId.emplace(key, id);
    return id;
}
}