/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** CacheAccess.hpp
*/

#ifndef CACHE_ACCESS_HPP
#define CACHE_ACCESS_HPP

#include <raylib.h>

#include "DrawCommand.hpp"
#include "FontCache.hpp"
#include "TextureCache.hpp"

namespace engine::render::detail {

/*
** The renderer's only way to the Raylib objects behind the ids: the public
** cache headers stay free of raylib.h. Each function is defined next to its
** cache's Impl. An id the cache didn't return is a programming error.
*/
struct CacheAccess {
    [[nodiscard]] static Texture2D texture(const TextureCache &cache,
        TextureId id);

    /*
    ** A default FontId gives Raylib's built-in font.
    */
    [[nodiscard]] static Font font(const FontCache &cache, FontId id);
};

}

#endif /* !CACHE_ACCESS_HPP */
