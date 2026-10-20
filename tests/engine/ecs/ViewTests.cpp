/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ViewTests
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

#include "engine/ecs/Registry.hpp"
#include "engine/ecs/View.hpp"

using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::SparseSet;
using engine::ecs::View;

namespace {
struct Tag {
    int id;
};

struct Speed {
    int value;
};

struct Mark {
    int value;
};

class ViewTest: public testing::Test {
protected:
    void SetUp() override
    {
        _registry.registerComponent<Tag>();
        _registry.registerComponent<Speed>();
        _registry.registerComponent<Mark>();
    }

    void spawnTagged(const int count)
    {
        for (int id = 0; id < count; ++id) {
            _entities.push_back(_registry.spawnEntity());
            _registry.addComponent(_entities.back(), Tag{id});
        }
    }

    template <class... Components>
    View<Components...> makeView()
    {
        return View<Components...>(
            _registry.getComponents<std::remove_const_t<Components>>()...);
    }

    template <class... Components>
    static std::vector<int> visitedIds(const View<Components...> &view)
    {
        std::vector<int> ids;

        for (auto &&components: view)
            ids.push_back(std::get<0>(components).id);
        std::ranges::sort(ids);
        return ids;
    }

    template <class... Components>
    static std::size_t countVisits(const View<Components...> &view)
    {
        return static_cast<std::size_t>(
            std::ranges::distance(view.begin(), view.end()));
    }

    Registry _registry;
    std::vector<Entity> _entities;
};
} // namespace

TEST_F(ViewTest, TwoSetJoinVisitsEntitiesInBothSetsOnce)
{
    spawnTagged(10);
    for (int id = 0; id < 10; id += 2)
        _registry.addComponent(_entities[id], Speed{id});

    EXPECT_EQ(visitedIds(makeView<Tag, Speed>()),
        (std::vector{0, 2, 4, 6, 8}));
}

TEST_F(ViewTest, ThreeSetJoinVisitsEntitiesInAllSetsOnce)
{
    spawnTagged(12);
    for (int id = 0; id < 12; id += 2)
        _registry.addComponent(_entities[id], Speed{id});
    for (int id = 0; id < 12; id += 3)
        _registry.addComponent(_entities[id], Mark{id});

    EXPECT_EQ(visitedIds(makeView<Tag, Speed, Mark>()), (std::vector{0, 6}));
}

TEST_F(ViewTest, ConstComponentYieldsAConstReference)
{
    using ViewType = View<Tag, const Speed>;

    static_assert(std::is_same_v<std::iter_reference_t<ViewType::iterator>,
        std::tuple<Tag &, const Speed &>>);
    static_assert(std::is_same_v<decltype(*std::declval<
        ViewType::iterator>()), std::tuple<Tag &, const Speed &>>);
}

TEST_F(ViewTest, WritesThroughTheViewAreVisibleInTheStorage)
{
    spawnTagged(4);
    for (const Entity &entity: _entities)
        _registry.addComponent(entity, Speed{1});

    for (auto &&[tag, speed]: makeView<const Tag, Speed>())
        speed.value = tag.id * 10;

    for (int id = 0; id < 4; ++id)
        EXPECT_EQ(_registry.getComponent<Speed>(_entities[id])->value, id * 10);
}

TEST_F(ViewTest, AnEmptySetGivesAnEmptyViewWhicheverSetItIs)
{
    spawnTagged(5);

    EXPECT_TRUE(visitedIds(makeView<Tag, Speed>()).empty());
    EXPECT_TRUE(visitedIds(makeView<Tag, const Speed>()).empty());

    for (int id = 0; id < 5; ++id)
        _registry.removeComponent<Tag>(_entities[id]);
    for (const Entity &entity: _entities)
        _registry.addComponent(entity, Speed{1});

    EXPECT_EQ(countVisits(makeView<Speed, Tag>()), 0U);
    EXPECT_EQ(countVisits(makeView<Tag, Speed>()), 0U);
}

TEST_F(ViewTest, KillingTheCurrentEntityVisitsEveryOtherEntityOnce)
{
    spawnTagged(10);
    for (const Entity &entity: _entities)
        _registry.addComponent(entity, Speed{0});

    std::vector<int> visited;
    for (auto &&[tag, speed]: makeView<Tag, Speed>()) {
        const int id = tag.id;

        visited.push_back(id);
        if (id % 3 == 0)
            _registry.killEntity(_entities[id]);
    }
    std::ranges::sort(visited);

    EXPECT_EQ(visited, (std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    EXPECT_EQ(visitedIds(makeView<Tag, Speed>()),
        (std::vector{1, 2, 4, 5, 7, 8}));
}

TEST_F(ViewTest, RemovingTheCurrentEntitysComponentVisitsEveryOtherOnce)
{
    spawnTagged(10);
    for (const Entity &entity: _entities)
        _registry.addComponent(entity, Speed{0});

    std::vector<int> visited;
    for (auto &&[tag, speed]: makeView<Tag, Speed>()) {
        const int id = tag.id;

        visited.push_back(id);
        if (id % 2 == 0)
            _registry.removeComponent<Speed>(_entities[id]);
    }
    std::ranges::sort(visited);

    EXPECT_EQ(visited, (std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    EXPECT_EQ(visitedIds(makeView<Tag, Speed>()), (std::vector{1, 3, 5, 7, 9}));
}

TEST_F(ViewTest, AddingComponentsToOtherEntitiesDuringTheLoopDoesntCrash)
{
    spawnTagged(10);
    for (int id = 0; id < 10; id += 2)
        _registry.addComponent(_entities[id], Speed{id});

    std::vector<int> visited;
    for (auto &&[tag, speed]: makeView<Tag, Speed>()) {
        const int id = tag.id;

        visited.push_back(id);
        _registry.addComponent(_entities[id + 1], Speed{id + 1});
        const Entity spawned = _registry.spawnEntity();
        _registry.addComponent(spawned, Tag{100 + id});
        _registry.addComponent(spawned, Speed{100 + id});
    }
    std::ranges::sort(visited);

    EXPECT_EQ(visited, (std::vector{0, 2, 4, 6, 8}));
}

TEST_F(ViewTest, ShrinkingTheDriverBelowTheCurrentPositionEndsTheLoop)
{
    spawnTagged(10);
    for (int id = 0; id < 6; ++id)
        _registry.addComponent(_entities[id], Speed{id});

    std::size_t visits = 0;
    for (auto &&components: makeView<Tag, Speed>()) {
        static_cast<void>(components);
        ++visits;
        for (int id = 0; id < 6; ++id)
            _registry.removeComponent<Speed>(_entities[id]);
    }

    EXPECT_EQ(visits, 1U);
}

TEST_F(ViewTest, AStaleHandleOnAReusedIndexIsNotYielded)
{
    SparseSet<Tag> tags;
    SparseSet<Speed> speeds;
    const Entity old = _registry.spawnEntity();
    _registry.killEntity(old);
    const Entity reused = _registry.spawnEntity();
    ASSERT_EQ(reused.index(), old.index());
    const Entity complete = _registry.spawnEntity();

    speeds.insertAt(old, Speed{0});
    tags.insertAt(reused, Tag{1});
    tags.insertAt(complete, Tag{2});
    speeds.insertAt(complete, Speed{2});

    EXPECT_EQ(visitedIds(View<Tag, Speed>(tags, speeds)), (std::vector{2}));
    EXPECT_EQ(countVisits(View<Speed, Tag>(speeds, tags)), 1U);
}
