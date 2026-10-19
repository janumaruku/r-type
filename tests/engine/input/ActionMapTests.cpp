/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ActionMapTests
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "engine/input/ActionMap.hpp"

using engine::input::ActionMap;
using engine::input::InputEvent;
using engine::input::InputState;
using engine::input::Key;
using engine::input::NamedAction;

// The test's own Action enum, with toString and fromString in its namespace:
// ActionMap must find them by argument-dependent lookup.
namespace game {
enum class Action { MOVE_UP, MOVE_DOWN, FIRE, QUIT };

constexpr std::array<std::string_view, 4> NAMES{"MOVE_UP", "MOVE_DOWN", "FIRE",
    "QUIT"};

std::string_view toString(Action action)
{
    return NAMES[static_cast<std::size_t>(action)];
}

bool fromString(std::string_view name, Action &action)
{
    const auto it = std::ranges::find(NAMES, name);

    if (it == NAMES.end())
        return false;
    action = static_cast<Action>(it - NAMES.begin());
    return true;
}
} // namespace game

namespace other {
enum class Unnamed { A };
} // namespace other

using game::Action;

static_assert(NamedAction<Action>);
static_assert(!NamedAction<other::Unnamed>);
static_assert(!NamedAction<int>);

namespace {
std::vector<Key> sorted(std::vector<Key> keys)
{
    std::ranges::sort(keys);
    return keys;
}

InputEvent pressed(Key key)
{
    InputEvent event;
    event.type = InputEvent::Type::KEY_PRESSED;
    event.key = key;
    return event;
}

ActionMap<Action> defaults()
{
    ActionMap<Action> map;
    map.bind(Action::MOVE_UP, Key::UP);
    map.bind(Action::MOVE_UP, Key::KEY_W);
    map.bind(Action::FIRE, Key::SPACE);
    return map;
}

class ActionMapFile : public testing::Test {
protected:
    void SetUp() override
    {
        const auto *info = testing::UnitTest::GetInstance()->current_test_info();
        _dir = std::filesystem::temp_directory_path() / "rtype-actionmap-tests"
            / info->name();
        std::filesystem::remove_all(_dir);
        std::filesystem::create_directories(_dir);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(_dir);
    }

    [[nodiscard]] std::filesystem::path file() const
    {
        return _dir / "keybindings.cfg";
    }

    void write(const std::string &text) const
    {
        std::ofstream(file()) << text;
    }

    std::filesystem::path _dir;
};
} // namespace

TEST(ActionMap, AnyBoundKeyHeldMakesTheActionActive)
{
    const auto map = defaults();
    InputState state;

    state.apply(pressed(Key::KEY_W));

    EXPECT_TRUE(map.isActive(Action::MOVE_UP, state));
    EXPECT_FALSE(map.isActive(Action::MOVE_DOWN, state));
}

TEST(ActionMap, TriggeredOnlyOnTheFirstFrameActiveWhileHeld)
{
    const auto map = defaults();
    InputState state;

    state.apply(pressed(Key::SPACE));
    EXPECT_TRUE(map.wasTriggered(Action::FIRE, state));
    EXPECT_TRUE(map.isActive(Action::FIRE, state));

    state.beginFrame();
    EXPECT_FALSE(map.wasTriggered(Action::FIRE, state));
    EXPECT_TRUE(map.isActive(Action::FIRE, state));
}

TEST(ActionMap, BindingAPairTwiceStoresItOnce)
{
    ActionMap<Action> map;

    map.bind(Action::FIRE, Key::SPACE);
    map.bind(Action::FIRE, Key::SPACE);
    map.unbind(Action::FIRE, Key::SPACE);

    EXPECT_TRUE(map.keysFor(Action::FIRE).empty());
}

TEST(ActionMap, UnbindRemovesOnlyThatKey)
{
    auto map = defaults();

    map.unbind(Action::MOVE_UP, Key::KEY_W);

    EXPECT_EQ(map.keysFor(Action::MOVE_UP), std::vector<Key>{Key::UP});
}

TEST(ActionMap, AKeyCanBelongToSeveralActions)
{
    ActionMap<Action> map;
    map.bind(Action::FIRE, Key::ENTER);
    map.bind(Action::QUIT, Key::ENTER);
    InputState state;

    state.apply(pressed(Key::ENTER));

    EXPECT_TRUE(map.isActive(Action::FIRE, state));
    EXPECT_TRUE(map.isActive(Action::QUIT, state));
}

TEST(ActionMap, AnUnboundActionHasNoKeysAndIsNeverActive)
{
    const ActionMap<Action> map;
    InputState state;
    state.apply(pressed(Key::SPACE));

    EXPECT_TRUE(map.keysFor(Action::FIRE).empty());
    EXPECT_FALSE(map.isActive(Action::FIRE, state));
    EXPECT_FALSE(map.wasTriggered(Action::FIRE, state));
}

TEST_F(ActionMapFile, SaveThenLoadGivesTheSameBindings)
{
    const auto saved = defaults();
    ASSERT_TRUE(saved.save(file()));

    ActionMap<Action> loaded;
    ASSERT_TRUE(loaded.load(file()));

    EXPECT_EQ(sorted(loaded.keysFor(Action::MOVE_UP)),
        sorted({Key::UP, Key::KEY_W}));
    EXPECT_EQ(loaded.keysFor(Action::FIRE), std::vector<Key>{Key::SPACE});
    EXPECT_TRUE(loaded.keysFor(Action::QUIT).empty());
}

TEST_F(ActionMapFile, SaveCreatesTheParentFolder)
{
    const auto map = defaults();
    const auto nested = _dir / "a" / "b" / "keybindings.cfg";

    ASSERT_TRUE(map.save(nested));

    EXPECT_TRUE(std::filesystem::exists(nested));
}

TEST_F(ActionMapFile, SaveReturnsFalseWhenTheFileCantBeWritten)
{
    std::ofstream(_dir / "blocker") << "a file, not a folder";
    const auto map = defaults();

    EXPECT_FALSE(map.save(_dir / "blocker" / "keybindings.cfg"));
}

TEST_F(ActionMapFile, LoadingAMissingFileKeepsTheDefaults)
{
    auto map = defaults();

    EXPECT_FALSE(map.load(file()));

    EXPECT_EQ(map.keysFor(Action::FIRE), std::vector<Key>{Key::SPACE});
}

TEST_F(ActionMapFile, AnActionInTheFileReplacesItsKeysOthersKeepTheirs)
{
    write(R"({"config": [{"name": "FIRE", "keys": ["LCTRL"]}]})");
    auto map = defaults();

    ASSERT_TRUE(map.load(file()));

    EXPECT_EQ(map.keysFor(Action::FIRE), std::vector<Key>{Key::LCTRL});
    EXPECT_EQ(sorted(map.keysFor(Action::MOVE_UP)),
        sorted({Key::UP, Key::KEY_W}));
}

TEST_F(ActionMapFile, UnknownActionsAndKeysAreSkipped)
{
    write(R"({"config": [
        {"name": "JUMP", "keys": ["UP"]},
        {"name": "FIRE", "keys": ["F13"]},
        {"name": "MOVE_DOWN", "keys": ["DOWN", "W"]}
    ]})");
    auto map = defaults();

    ASSERT_TRUE(map.load(file()));

    EXPECT_EQ(map.keysFor(Action::FIRE), std::vector<Key>{Key::SPACE});
    EXPECT_EQ(map.keysFor(Action::MOVE_DOWN), std::vector<Key>{Key::DOWN});
}

TEST_F(ActionMapFile, KeyNamesAcceptAnyCase)
{
    write(R"({"config": [{"name": "MOVE_UP", "keys": ["key_w", "Up"]}]})");
    ActionMap<Action> map;

    ASSERT_TRUE(map.load(file()));

    EXPECT_EQ(sorted(map.keysFor(Action::MOVE_UP)),
        sorted({Key::UP, Key::KEY_W}));
}

TEST_F(ActionMapFile, InvalidJsonChangesNothing)
{
    write("{ not json");
    auto map = defaults();

    EXPECT_FALSE(map.load(file()));

    EXPECT_EQ(map.keysFor(Action::FIRE), std::vector<Key>{Key::SPACE});
}

TEST_F(ActionMapFile, AWronglyTypedFieldRejectsTheWholeFile)
{
    write(R"({"config": [
        {"name": "FIRE", "keys": ["LCTRL"]},
        {"name": "MOVE_UP", "keys": [42]}
    ]})");
    auto map = defaults();

    EXPECT_FALSE(map.load(file()));

    EXPECT_EQ(map.keysFor(Action::FIRE), std::vector<Key>{Key::SPACE});
}

TEST_F(ActionMapFile, AFileWithoutConfigIsRejected)
{
    write(R"({"bindings": []})");
    auto map = defaults();

    EXPECT_FALSE(map.load(file()));

    EXPECT_EQ(map.keysFor(Action::FIRE), std::vector<Key>{Key::SPACE});
}
