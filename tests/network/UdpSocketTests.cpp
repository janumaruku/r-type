/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocketTests.cpp
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <iterator>
#include <thread>
#include <vector>

#include "engine/core/Errors.hpp"
#include "network/UdpSocket.hpp"

namespace {

asio::ip::udp::socket makeSender(asio::io_context &ioContext)
{
    return {ioContext,
            asio::ip::udp::endpoint(asio::ip::address_v4::loopback(), 0)};
}

asio::ip::udp::endpoint loopbackOf(const net::UdpSocket &socket)
{
    return {asio::ip::address_v4::loopback(), socket.localEndpoint().port()};
}

void sendDatagram(asio::ip::udp::socket &sender,
    const asio::ip::udp::endpoint &destination,
    const std::vector<std::uint8_t> &bytes)
{
    sender.send_to(asio::buffer(bytes), destination);
}

net::Packet makePacket(const std::vector<std::uint8_t> &bytes)
{
    net::Packet packet;
    packet.writeRaw(bytes.data(), bytes.size());
    return packet;
}

std::vector<net::Datagram> waitForDatagrams(net::UdpSocket &receiver,
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

TEST(UdpSocket, IsReceivingFollowsStartAndStop)
{
    net::UdpSocket socket(0);

    EXPECT_FALSE(socket.isReceiving());
    socket.start();
    EXPECT_TRUE(socket.isReceiving());
    socket.stop();
    EXPECT_FALSE(socket.isReceiving());
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
    const auto destination = loopbackOf(receiver);

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
    constexpr std::size_t sentCount = 10;
    constexpr std::size_t maximum = 2;
    net::UdpSocket receiver(0);
    receiver.start();
    asio::io_context ioContext;
    auto sender = makeSender(ioContext);
    const auto destination = loopbackOf(receiver);

    for (std::size_t index = 0; index < sentCount; ++index)
        sendDatagram(sender, destination, {static_cast<std::uint8_t>(index)});

    std::vector<net::Datagram> received;
    const auto deadline = std::chrono::steady_clock::now()
        + std::chrono::seconds(1);
    while (received.size() < sentCount
        && std::chrono::steady_clock::now() < deadline) {
        auto batch = receiver.receiveAll(maximum);
        EXPECT_LE(batch.size(), maximum);
        if (batch.empty())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        received.insert(received.end(),
            std::make_move_iterator(batch.begin()),
            std::make_move_iterator(batch.end()));
    }

    ASSERT_EQ(received.size(), sentCount);
    for (std::size_t index = 0; index < sentCount; ++index)
        EXPECT_EQ(received[index].bytes,
            std::vector{static_cast<std::uint8_t>(index)});
}

TEST(UdpSocket, OversizedDatagramIsDroppedAndCounted)
{
    net::UdpSocket receiver(0);
    receiver.start();
    asio::io_context ioContext;
    auto sender = makeSender(ioContext);
    const auto destination = loopbackOf(receiver);
    const std::vector<std::uint8_t> oversized(net::Packet::MAX_SIZE + 100, 0);

    sendDatagram(sender, destination, oversized);
    const auto deadline = std::chrono::steady_clock::now()
        + std::chrono::seconds(1);
    while (receiver.dropped() == 0
        && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    EXPECT_EQ(receiver.dropped(), 1U);
    EXPECT_TRUE(receiver.receiveAll().empty());
}

TEST(UdpSocket, DatagramsLargerThanTheBufferDoNotStopReceiving)
{
    constexpr std::size_t largeCount = 5;
    net::UdpSocket receiver(0);
    receiver.start();
    asio::io_context ioContext;
    auto sender = makeSender(ioContext);
    const auto destination = loopbackOf(receiver);
    const std::vector<std::uint8_t> large(2000, 0);

    for (std::size_t index = 0; index < largeCount; ++index)
        sendDatagram(sender, destination, large);
    sendDatagram(sender, destination, {7});

    const auto received = waitForDatagrams(receiver, 1);

    ASSERT_EQ(received.size(), 1U);
    EXPECT_EQ(received[0].bytes, std::vector<std::uint8_t>{7});
    EXPECT_EQ(receiver.dropped(), largeCount);
}

TEST(UdpSocket, FloodIsBoundedAndAccountsForApplicationDrops)
{
    constexpr std::size_t sentCount = 5000;
    constexpr std::size_t batchSize = 100;
    net::UdpSocket receiver(0);
    receiver.start();
    asio::io_context ioContext;
    auto sender = makeSender(ioContext);
    const auto destination = loopbackOf(receiver);
    const std::vector<std::uint8_t> bytes{42};

    for (std::size_t index = 0; index < sentCount; ++index) {
        sendDatagram(sender, destination, bytes);
        if ((index + 1) % batchSize == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    const auto deadline = std::chrono::steady_clock::now()
        + std::chrono::seconds(1);
    while (receiver.dropped() == 0
        && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    const auto queued = receiver.receiveAll(sentCount).size();

    EXPECT_EQ(queued, net::UdpSocket::MAX_QUEUED_DATAGRAMS);
    EXPECT_GE(receiver.dropped(), 1U);
    EXPECT_LE(queued + receiver.dropped(), sentCount);
}

TEST(UdpSocket, SendToDeliversPacketsInOrderWithIntactBytes)
{
    constexpr std::size_t packetCount = 1000;
    constexpr std::size_t batchSize = 50;
    static_assert(packetCount % batchSize == 0);
    net::UdpSocket receiver(0);
    net::UdpSocket sender(0);
    receiver.start();
    sender.start();

    std::vector<net::Datagram> received;
    const auto destination = loopbackOf(receiver);
    for (std::size_t first = 0; first < packetCount; first += batchSize) {
        for (std::size_t index = first; index < first + batchSize; ++index) {
            const std::vector bytes{
                static_cast<std::uint8_t>(index & 0xff),
                static_cast<std::uint8_t>((index >> 8) & 0xff),
            };
            sender.sendTo(destination, makePacket(bytes));
        }

        auto batch = waitForDatagrams(receiver, batchSize);
        received.insert(received.end(),
            std::make_move_iterator(batch.begin()),
            std::make_move_iterator(batch.end()));
    }

    ASSERT_EQ(received.size(), packetCount);
    for (std::size_t index = 0; index < packetCount; ++index) {
        const std::vector expectedBytes{
            static_cast<std::uint8_t>(index & 0xff),
            static_cast<std::uint8_t>((index >> 8) & 0xff),
        };
        EXPECT_EQ(received[index].bytes, expectedBytes);
    }
}

TEST(UdpSocket, ConcurrentSendToCallsDeliverIntactPackets)
{
    constexpr std::size_t packetsPerThread = 200;
    net::UdpSocket receiver(0);
    net::UdpSocket sender(0);
    receiver.start();
    sender.start();
    const auto destination = loopbackOf(receiver);

    auto sendPackets = [&](std::uint8_t threadId) {
        for (std::size_t index = 0; index < packetsPerThread; ++index) {
            auto packet = makePacket({
                threadId,
                static_cast<std::uint8_t>(index),
                0xa5,
            });
            sender.sendTo(destination, packet);
        }
    };

    std::thread first(sendPackets, 0);
    std::thread second(sendPackets, 1);
    first.join();
    second.join();

    const auto received = waitForDatagrams(receiver, packetsPerThread * 2);
    ASSERT_FALSE(received.empty());
    for (const auto &datagram: received) {
        ASSERT_EQ(datagram.bytes.size(), 3U);
        EXPECT_LT(datagram.bytes[0], 2U);
        EXPECT_LT(datagram.bytes[1], packetsPerThread);
        EXPECT_EQ(datagram.bytes[2], 0xa5);
    }
}

TEST(UdpSocket, SendToAfterStopIsHarmless)
{
    net::UdpSocket sender(0);
    sender.start();
    const auto destination = asio::ip::udp::endpoint(
        asio::ip::address_v4::loopback(), 1);
    const auto packet = makePacket({1, 2, 3});
    sender.stop();

    EXPECT_NO_THROW(sender.sendTo(destination, packet));
}
