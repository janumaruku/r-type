#include <gtest/gtest.h>
#include "network/Packet.hpp"

TEST(Network, TestSetup) {
    net::Packet packet;
    packet << std::uint32_t{42};
    EXPECT_EQ(packet.read<std::uint32_t>(), 42u);
}
