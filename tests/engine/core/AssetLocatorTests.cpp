/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AssetLocatorTests.cpp
*/

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "../../../engine/core/include/engine/core/AssetLocator.hpp"
#include "../../../engine/core/include/engine/core/Errors.hpp"

namespace {

class AssetsEnvironmentGuard {
public:
    explicit AssetsEnvironmentGuard(const std::filesystem::path &value)
    {
#ifdef _WIN32
        char *previousValue = nullptr;
        std::size_t previousSize = 0;
        _dupenv_s(&previousValue, &previousSize, "RTYPE_ASSETS_DIR");
        if (previousValue != nullptr) {
            _previous = previousValue;
            std::free(previousValue);
        }
        _putenv_s("RTYPE_ASSETS_DIR", value.string().c_str());
#else
        if (const char *previousValue = std::getenv("RTYPE_ASSETS_DIR");
            previousValue != nullptr)
            _previous = previousValue;
        setenv("RTYPE_ASSETS_DIR", value.c_str(), 1);
#endif
    }

    ~AssetsEnvironmentGuard()
    {
#ifdef _WIN32
        if (_previous.has_value())
            _putenv_s("RTYPE_ASSETS_DIR", _previous->c_str());
        else
            _putenv_s("RTYPE_ASSETS_DIR", "");
#else
        if (_previous.has_value())
            setenv("RTYPE_ASSETS_DIR", _previous->c_str(), 1);
        else
            unsetenv("RTYPE_ASSETS_DIR");
#endif
    }

private:
    std::optional<std::string> _previous;
};

class AssetLocatorTest : public testing::Test {
protected:
    void SetUp() override
    {
        std::filesystem::remove_all(_temporaryRoot);
        std::filesystem::create_directories(_temporaryRoot);
        _previousWorkingDirectory = std::filesystem::current_path();
        std::filesystem::current_path(_temporaryRoot);
    }

    void TearDown() override
    {
        std::filesystem::current_path(_previousWorkingDirectory);
        std::filesystem::remove_all(_temporaryRoot);
    }

    const std::filesystem::path _temporaryRoot =
        std::filesystem::temp_directory_path() / "rtype_asset_locator_tests";
    std::filesystem::path _previousWorkingDirectory;
};

}

TEST_F(AssetLocatorTest, ExistingOverrideWinsOverEnvironment)
{
    const auto overrideRoot = _temporaryRoot / "override";
    const auto environmentRoot = _temporaryRoot / "environment";
    std::filesystem::create_directory(overrideRoot);
    std::filesystem::create_directory(environmentRoot);
    AssetsEnvironmentGuard environment(environmentRoot);

    const engine::core::AssetLocator locator(overrideRoot);

    EXPECT_EQ(locator.root(), overrideRoot);
}

TEST_F(AssetLocatorTest, MissingOverrideFallsThroughToEnvironment)
{
    const auto missingOverride = _temporaryRoot / "missing-override";
    const auto environmentRoot = _temporaryRoot / "environment";
    std::filesystem::create_directory(environmentRoot);
    AssetsEnvironmentGuard environment(environmentRoot);

    const engine::core::AssetLocator locator(missingOverride);

    EXPECT_EQ(locator.root(), environmentRoot);
}

TEST_F(AssetLocatorTest, MissingCandidatesAreListedInException)
{
    const auto missingOverride = _temporaryRoot / "missing-override";
    const auto missingEnvironment = _temporaryRoot / "missing-environment";
    AssetsEnvironmentGuard environment(missingEnvironment);

    try {
        engine::core::AssetLocator locator(missingOverride);
        FAIL() << "Expected AssetLocator construction to fail";
    } catch (const engine::core::ResourceError &error) {
        const std::string message = error.what();
        EXPECT_NE(message.find(missingOverride.string()), std::string::npos);
        EXPECT_NE(message.find(missingEnvironment.string()), std::string::npos);
        EXPECT_NE(message.find((_temporaryRoot / "assets").string()), std::string::npos);
    }
}

TEST_F(AssetLocatorTest, ResolveReturnsExistingFile)
{
    const auto assetRoot = _temporaryRoot / "assets";
    const auto assetPath = assetRoot / "config" / "settings.json";
    std::filesystem::create_directories(assetPath.parent_path());
    std::ofstream(assetPath) << "{}";

    const engine::core::AssetLocator locator(assetRoot);

    EXPECT_EQ(locator.resolve("config/settings.json"), assetPath);
}

TEST_F(AssetLocatorTest, ResolveThrowsForMissingFile)
{
    const auto assetRoot = _temporaryRoot / "assets";
    std::filesystem::create_directory(assetRoot);
    const engine::core::AssetLocator locator(assetRoot);

    EXPECT_THROW((void)locator.resolve("missing.json"), engine::core::ResourceError);
}