/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ActionMap.hpp
*/

#pragma once

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "engine/core/Logger.hpp"
#include "engine/input/InputState.hpp"
#include "engine/input/Key.hpp"
#include "engine/input/json.hpp"

namespace engine::input {

// The client writes toString and fromString next to its Action enum, and they
// are found by argument-dependent lookup.
template <class Action>
concept NamedAction = std::is_enum_v<Action>
    && requires(Action action, std::string_view name) {
           { toString(action) } -> std::convertible_to<std::string_view>;
           { fromString(name, action) } -> std::same_as<bool>;
       };

template <NamedAction Action>
class ActionMap {
public:
    void bind(Action action, Key key)
    {
        _bindings[action].insert(key);
    }

    void unbind(Action action, Key key)
    {
        const auto it = _bindings.find(action);

        if (it == _bindings.end())
            return;
        it->second.erase(key);
        if (it->second.empty())
            _bindings.erase(it);
    }

    [[nodiscard]] std::vector<Key> keysFor(Action action) const
    {
        const auto it = _bindings.find(action);

        if (it == _bindings.end())
            return {};
        return {it->second.begin(), it->second.end()};
    }

    [[nodiscard]] bool isActive(Action action, const InputState &state) const
    {
        return anyKey(action, [&state](Key key) { return state.isDown(key); });
    }

    [[nodiscard]] bool wasTriggered(Action action,
        const InputState &state) const
    {
        return anyKey(action,
            [&state](Key key) { return state.wasPressed(key); });
    }

    bool load(const std::filesystem::path &path)
    {
        std::ifstream file(path);

        if (!file.is_open())
            return false;
        core::log::Logger logger("input");
        Bindings loaded;
        try {
            const auto data = nlohmann::json::parse(file);
            const auto &entries = data.at("config");
            for (std::size_t index = 0; index < entries.size(); ++index) {
                const auto &entry = entries.at(index);
                Action action{};
                const auto actionName = entry.at("name").get<std::string>();
                if (!fromString(actionName, action)) {
                    logger.warn("{}: entry {}: unknown action \"{}\", skipped",
                        path.string(), index, actionName);
                    continue;
                }
                for (const auto &name : entry.at("keys")) {
                    const auto keyName = name.get<std::string>();
                    if (const auto key = keyFromString(keyName))
                        loaded[action].insert(*key);
                    else
                        logger.warn("{}: entry {}: unknown key \"{}\", skipped",
                            path.string(), index, keyName);
                }
            }
        } catch (const nlohmann::json::exception &error) {
            logger.warn("{}: {}, keeping the current bindings", path.string(),
                std::string{error.what()});
            return false;
        }
        for (auto &[action, keys] : loaded)
            _bindings[action] = std::move(keys);
        return true;
    }

    [[nodiscard]] bool save(const std::filesystem::path &path) const
    {
        nlohmann::json config = nlohmann::json::array();
        for (const auto &[action, keys] : _bindings) {
            nlohmann::json names = nlohmann::json::array();
            for (const Key key : keys)
                names.push_back(toString(key));
            config.push_back({{"name", toString(action)}, {"keys", names}});
        }

        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        std::ofstream file(path);
        file << nlohmann::json{{"config", config}}.dump(2) << '\n';
        if (!file) {
            core::log::Logger("input").warn("{}: could not write the bindings",
                path.string());
            return false;
        }
        return true;
    }

private:
    using Bindings = std::unordered_map<Action, std::unordered_set<Key>>;

    template <class Predicate>
    [[nodiscard]] bool anyKey(Action action, Predicate predicate) const
    {
        const auto it = _bindings.find(action);

        return it != _bindings.end()
            && std::ranges::any_of(it->second, predicate);
    }

    Bindings _bindings;
};

} // namespace engine::input
