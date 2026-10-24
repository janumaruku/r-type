/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FontCache.hpp
*/

#ifndef FONT_CACHE_HPP
#define FONT_CACHE_HPP

#include <filesystem>
#include <memory>

#include "DrawCommand.hpp"

namespace engine::render {

class FontCache {
public:
    FontCache();

    FontCache(const FontCache &) = delete;

    FontCache(FontCache &&) = delete;

    ~FontCache();

    FontCache &operator=(const FontCache &) = delete;

    FontCache &operator=(FontCache &&) = delete;

    FontId load(const std::filesystem::path &path);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif
