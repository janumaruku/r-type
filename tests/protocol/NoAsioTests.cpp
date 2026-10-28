/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** NoAsioTests
*/

#include <gtest/gtest.h>

#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/Event.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/MessageType.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/ProtocolError.hpp"
#include "rtype/protocol/Snapshot.hpp"

/*
** network links Asio publicly, so its include path reaches this file: only the
** headers above can define ASIO_VERSION. Every new protocol header is added to
** the list, and this file stops compiling the day one of them pulls Asio in.
*/
#ifdef ASIO_VERSION
#error "a protocol header includes Asio"
#endif

TEST(NoAsioTests, TheProtocolHeadersDontIncludeAsio)
{
    SUCCEED();
}
