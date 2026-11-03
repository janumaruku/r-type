#include <gtest/gtest.h>

#include "Goodbyes.hpp"
#include "rtype/protocol/Messages.hpp"

namespace {

TEST(GoodbyesTests, SendsQueuedPacketForThreeTicks)
{
    rtype::server::Goodbyes goodbyes;
    const auto endpoint = asio::ip::udp::endpoint(
        asio::ip::address_v4::loopback(), 12345);
    const auto packet = rtype::protocol::makePacket(
        rtype::protocol::MessageType::KICK, 7,
        rtype::protocol::Kick{
            .reason = rtype::protocol::KickReason::PROTOCOL_ERROR});
    goodbyes.add(endpoint, packet);

    const auto first = goodbyes.takeDue();
    const auto second = goodbyes.takeDue();
    const auto third = goodbyes.takeDue();
    const auto fourth = goodbyes.takeDue();

    ASSERT_EQ(first.size(), 1U);
    ASSERT_EQ(second.size(), 1U);
    ASSERT_EQ(third.size(), 1U);
    EXPECT_TRUE(fourth.empty());
    EXPECT_EQ(first.front().first, endpoint);
    EXPECT_TRUE(std::ranges::equal(first.front().second.bytes(),
        packet.bytes()));
}

}
