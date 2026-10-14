/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocketTests.cpp
*/

#include <gtest/gtest.h>

#include "Errors.hpp"
#include "UdpSocket.hpp"

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
