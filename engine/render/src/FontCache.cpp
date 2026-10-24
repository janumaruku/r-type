/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FontCache.cpp
*/

#include "FontCache.hpp"

#include <raylib.h>

#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include "Errors.hpp"
#include "ResourcePath.hpp"

namespace engine::render {

namespace {
constexpr int BASE_SIZE = 64;
}

struct FontCache::Impl {
    std::vector<Font> fonts;                                 // index == FontId::value
    std::unordered_map<std::filesystem::path, FontId> pathToId;   // key: cacheKey(path)
};

FontCache::FontCache() : _impl(std::make_unique<Impl>()) {}

FontCache::~FontCache()
{
    for (const Font &font : _impl->fonts)
        UnloadFont(font);
}

/*
** When Raylib can't decode a font it returns its default font, which passes
** IsFontValid: comparing the texture with the default one is the only way to
** tell, and that font must never be stored, or the destructor would unload it.
** Fonts are rasterised once at BASE_SIZE with a bilinear filter, so text
** stays readable when TextCmd::characterSize scales it.
*/
FontId FontCache::load(const std::filesystem::path &path)
{
    const std::filesystem::path key = cacheKey(path);

    if (const auto it = _impl->pathToId.find(key); it != _impl->pathToId.end())
        return it->second;

    if (!IsWindowReady())
        throw core::EngineError("FontCache::load called before the window was created: " + key.string());
    if (!std::filesystem::is_regular_file(key))
        throw core::ResourceError("Font file does not exist: " + key.string());
    if (_impl->fonts.size() >= FontId::DEFAULT)
        throw core::ResourceError("Too many fonts (max 65535): " + key.string());

    const Font font = LoadFontEx(key.string().c_str(), BASE_SIZE, nullptr, 0);
    if (font.texture.id == GetFontDefault().texture.id)
        throw core::ResourceError("Failed to decode font: " + key.string());
    if (!IsFontValid(font)) {
        UnloadFont(font);
        throw core::ResourceError("Failed to decode font: " + key.string());
    }
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    const FontId id{static_cast<std::uint16_t>(_impl->fonts.size())};
    _impl->fonts.push_back(font);
    _impl->pathToId.emplace(key, id);
    return id;
}

}
