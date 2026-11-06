#include "ClientWorld.hpp"

#include <gtest/gtest.h>

#include "SnapshotAssembler.hpp"

namespace rtype::client {
namespace {

TEST(ClientWorld, KeepsHighestProcessedInputSequence)
{
    ClientWorld world{SpriteCatalog{}};
    world.setLastProcessedInput(10);
    world.setLastProcessedInput(7);
    EXPECT_EQ(world.lastProcessedInput(), 10U);
}

TEST(ClientWorld, StoresSpriteCatalogAsRegistryResource)
{
    ClientWorld world{SpriteCatalog{}};
    EXPECT_NO_THROW((void)world.registry().getResource<SpriteCatalog>());
}

} // namespace
} // namespace rtype::client
