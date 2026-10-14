/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocketTests.cpp
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

#include "Errors.hpp"
#include "UdpSocket.hpp"

namespace {

asio::ip::udp::socket makeSender(asio::io_context& ioContext)
{
    return {ioContext,
        asio::ip::udp::endpoint(asio::ip::address_v4::loopback(), 0)};
}

void sendDatagram(asio::ip::udp::socket& sender,
    const asio::ip::udp::endpoint& destination,
    const std::vector<std::uint8_t>& bytes)
{
    sender.send_to(asio::buffer(bytes), destination);
}

std::vector<net::Datagram> waitForDatagrams(net::UdpSocket& receiver,
    std::size_t count)
{
    std::vector<net::Datagram> received;
    const auto deadline = std::chrono::steady_clock::now()
        + std::chrono::seconds(1);

    while (received.size() < count
        && std::chrono::steady_clock::now() < deadline) {
        auto batch = receiver.receiveAll(count - received.size());
        received.insert(received.end(),
            std::make_move_iterator(batch.begin()),
            std::make_move_iterator(batch.end()));
        if (received.size() < count)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return received;
}

} // namespace

TEST(UdpSocket, PortZeroBindsToAnAvailablePort)
{
    net::UdpSocket socket(0);

    socket.start();

    EXPECT_NE(socket.localEndpoint().port(), 0);
}

TEST(UdpSocket, BindingAnAlreadyUsedPortThrowsNetworkError)
{
    net::UdpSocket first(0);
    first.start();
    net::UdpSocket second(first.localEndpoint().port());

    EXPECT_THROW(second.start(), engine::core::NetworkError);
}

TEST(UdpSocket, StopIsIdempotent)
{
    net::UdpSocket socket(0);
    socket.start();

    EXPECT_NO_THROW(socket.stop());
    EXPECT_NO_THROW(socket.stop());
}

TEST(UdpSocket, DestructorStopsStartedSocket)
{
    EXPECT_NO_THROW({
        net::UdpSocket socket(0);
        socket.start();
    });
}

TEST(UdpSocket, ReceivesDatagramsInOrderWithSenderEndpoint)
{
    net::UdpSocket receiver(0);
    receiver.start();
    asio::io_context ioContext;
    auto sender = makeSender(ioContext);
    const auto destination = receiver.localEndpoint();

    sendDatagram(sender, destination, {1});
    sendDatagram(sender, destination, {2, 3});
    sendDatagram(sender, destination, {4, 5, 6});

    const auto received = waitForDatagrams(receiver, 3);

    ASSERT_EQ(received.size(), 3U);
    EXPECT_EQ(received[0].bytes, std::vector<std::uint8_t>{1});
    EXPECT_EQ(received[1].bytes, std::vector<std::uint8_t>({2, 3}));
    EXPECT_EQ(received[2].bytes, std::vector<std::uint8_t>({4, 5, 6}));
    EXPECT_EQ(received[0].from, sender.local_endpoint());
    EXPECT_EQ(received[1].from, sender.local_endpoint());
    EXPECT_EQ(received[2].from, sender.local_endpoint());
}

TEST(UdpSocket, ReceiveAllOnEmptyQueueReturnsImmediately)
{
    net::UdpSocket receiver(0);
    receiver.start();

    const auto begin = std::chrono::steady_clock::now();
    const auto received = receiver.receiveAll();
    const auto elapsed = std::chrono::steady_clock::now() - begin;

    EXPECT_TRUE(received.empty());
    EXPECT_LT(elapsed, std::chrono::milliseconds(100));
}

TEST(UdpSocket, ReceiveAllRespectsMaximumAndKeepsTheRestQueued)
{
    net::UdpSocket receiver(0);
    receiver.start();
    asio::io_context ioContext;
    auto sender = makeSender(ioContext);
    const auto destination = receiver.localEndpoint();

    sendDatagram(sender, destination, {1});
    sendDatagram(sender, destination, {2});
    sendDatagram(sender, destination, {3});
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    const auto first = receiver.receiveAll(2);
    const auto rest = receiver.receiveAll();

    ASSERT_EQ(first.size(), 2U);
    ASSERT_EQ(rest.size(), 1U);
    EXPECT_EQ(first[0].bytes, std::vector<std::uint8_t>{1});
    EXPECT_EQ(first[1].bytes, std::vector<std::uint8_t>{2});
    EXPECT_EQ(rest[0].bytes, std::vector<std::uint8_t>{3});
}

TEST(UdpSocket, OversizedDatagramIsDroppedAndCounted)
{
    net::UdpSocket receiver(0);
    receiver.start();
    asio::io_context ioContext;
    auto sender = makeSender(ioContext);
    const auto destination = receiver.localEndpoint();
    const std::vector<std::uint8_t> oversized(net::Packet::MAX_SIZE + 100, 0);

    sendDatagram(sender, destination, oversized);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_TRUE(receiver.receiveAll().empty());
    EXPECT_EQ(receiver.dropped(), 1U);
}

TEST(UdpSocket, FloodIsBoundedAndAccountsForApplicationDrops)
{
    constexpr std::size_t sentCount = 10'000;
    net::UdpSocket receiver(0);
    receiver.start();
    asio::io_context ioContext;
    auto sender = makeSender(ioContext);
    const auto destination = receiver.localEndpoint();
    const std::vector<std::uint8_t> bytes{42};

    for (std::size_t index = 0; index < sentCount; ++index)
        sendDatagram(sender, destination, bytes);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto queued = receiver.receiveAll(sentCount + 1).size();

    EXPECT_LE(queued, 4096U);
    EXPECT_LE(queued + receiver.dropped(), sentCount);
}
