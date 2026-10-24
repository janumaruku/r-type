/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AssetLocator.cpp
*/

#include <cstdlib>
#include <vector>
#include <filesystem>
#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include "engine/core/AssetLocator.hpp"

namespace engine::core {

namespace {

std::filesystem::path executableDirectory()
{
#ifdef _WIN32
    std::wstring buffer(256, L'\0');
    DWORD length = 0;
    do {
        buffer.resize(buffer.size() * 2);
        length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    } while (length == buffer.size());
    if (length == 0)
        return {};
    buffer.resize(length);
    return std::filesystem::path(buffer).parent_path();
#else
    std::error_code error;
    const auto executable = std::filesystem::read_symlink("/proc/self/exe", error);
    if (error)
        return {};
    return executable.parent_path();
#endif
}

std::optional<std::filesystem::path> environmentAssetsDirectory()
{
#ifdef _WIN32
    char *environmentPath = nullptr;
    std::size_t environmentPathSize = 0;
    _dupenv_s(&environmentPath, &environmentPathSize, "RTYPE_ASSETS_DIR");
    if (environmentPath == nullptr)
        return std::nullopt;

    std::filesystem::path path(environmentPath);
    std::free(environmentPath);
    return path;
#else
    const char *const environmentPath = std::getenv("RTYPE_ASSETS_DIR");
    if (environmentPath == nullptr)
        return std::nullopt;
    return std::filesystem::path(environmentPath);
#endif
}

}

AssetLocator::AssetLocator(std::optional<std::filesystem::path> overrideRoot)
{
    std::vector<std::filesystem::path> candidates;
    if (overrideRoot.has_value())
        candidates.push_back(*overrideRoot);

    if (const auto environmentRoot = environmentAssetsDirectory();
        environmentRoot.has_value()) {
        candidates.push_back(*environmentRoot);
    }

    const auto executableRoot = executableDirectory();
    if (!executableRoot.empty())
        candidates.push_back(executableRoot / "assets");
    candidates.push_back(std::filesystem::current_path() / "assets");

    for (const auto &candidate : candidates) {
        if (std::filesystem::is_directory(candidate)) {
            _root = candidate;
            log::Logger("assets").info("Assets directory resolved to: {}", _root.string());
            return;
        }
    }

    std::string message = "Assets directory not found. Tried:";
    for (const auto &candidate : candidates)
        message += " " + candidate.string();
    throw ResourceError(message);
}

[[nodiscard]] std::filesystem::path AssetLocator::resolve(const std::string &relative) const
{
    if (!std::filesystem::exists(_root / relative) || std::filesystem::is_empty(_root / relative)) {
        throw ResourceError(std::format("asset not found: <{}>", relative));
    }
    return _root / relative;
}

const std::filesystem::path &AssetLocator::root() const noexcept
{
    return _root;
}

}