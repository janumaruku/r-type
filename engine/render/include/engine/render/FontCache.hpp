/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FontCache.hpp
*/

#ifndef FONT_CACHE_HPP
#define FONT_CACHE_HPP

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>

#include "DrawCommand.hpp"

namespace engine::render {

namespace detail {
struct CacheAccess;
}

class FontCache {
public:
    FontCache();

    FontCache(const FontCache &) = delete;

    FontCache(FontCache &&) = delete;

    ~FontCache();

    FontCache &operator=(const FontCache &) = delete;

    FontCache &operator=(FontCache &&) = delete;

    FontId load(const std::filesystem::path &path);

    /*
    ** The size text takes when a TextCmd draws it with this font and
    ** characterSize, in logical pixels. Throws EngineError before the window
    ** exists, since Raylib only loads its default font in InitWindow.
    */
    [[nodiscard]] core::Vec2f measure(FontId font, std::string_view text,
        std::uint8_t characterSize) const;

private:
    friend struct detail::CacheAccess;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif
