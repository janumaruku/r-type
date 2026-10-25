/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** TextureCache.hpp
*/

#ifndef TEXTURE_CACHE_HPP
#define TEXTURE_CACHE_HPP

#include <filesystem>
#include <memory>

#include "DrawCommand.hpp"
#include "engine/core/Vec2.hpp"

namespace engine::render {

namespace detail {
struct CacheAccess;
}

class TextureCache {
public:
    TextureCache();

    TextureCache(const TextureCache &) = delete;

    TextureCache(TextureCache &&) = delete;

    ~TextureCache();

    TextureCache &operator=(const TextureCache &) = delete;

    TextureCache &operator=(TextureCache &&) = delete;

    TextureId load(const std::filesystem::path &path);

    [[nodiscard]] core::Vec2u size(TextureId id) const;

private:
    friend struct detail::CacheAccess;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif
