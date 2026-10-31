/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** NetIdCleanupTests
*/

#include <gtest/gtest.h>

#include <vector>

#include "GameSystemsFixture.hpp"

using engine::ecs::Entity;
using rtype::game::Health;
using rtype::game::NetId;
using rtype::game::NetIdAllocator;

class NetIdCleanup: public game_tests::GameSystems {
protected:
    void SetUp() override
    {
        GameSystems::SetUp();
        addHealthSystem();
        addNetIdCleanupSystem();
    }

    NetIdAllocator &allocator()
    {
        return _registry.getResource<NetIdAllocator>();
    }
};

TEST_F(NetIdCleanup, NetIdsOfEntitiesKilledDuringTheTickAreReleased)
{
    std::vector<Entity> spawned;

    for (int i = 0; i < 5; ++i)
        spawned.push_back(spawnBydos({600.0F, 100.0F * static_cast<float>(i + 1)}));

    const NetId first = netIdOf(spawned[0]);
    const NetId third = netIdOf(spawned[2]);

    ASSERT_EQ(allocator().size(), 5U);
    component<Health>(spawned[0]).current = 0;
    component<Health>(spawned[2]).current = 0;
    tick();
    EXPECT_EQ(allocator().size(), 3U);
    EXPECT_FALSE(find(first).has_value());
    EXPECT_FALSE(find(third).has_value());
    EXPECT_EQ(entityOf(netIdOf(spawned[1])), spawned[1]);
}

TEST_F(NetIdCleanup, NothingKilledReleasesNothing)
{
    for (int i = 0; i < 3; ++i)
        (void)spawnBydos({600.0F, 100.0F * static_cast<float>(i + 1)});
    tick();
    EXPECT_EQ(allocator().size(), 3U);
}
