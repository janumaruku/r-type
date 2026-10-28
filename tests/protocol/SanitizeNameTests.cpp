/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SanitizeNameTests
*/

#include <gtest/gtest.h>

#include <string>

#include "rtype/protocol/Messages.hpp"

using rtype::protocol::sanitizeName;

TEST(SanitizeNameTests, AControlByteIsReplaced)
{
    EXPECT_EQ(sanitizeName("Ab\x01" "c", 0), "Ab?c");
}

TEST(SanitizeNameTests, BytesOutsidePrintableAsciiAreReplaced)
{
    EXPECT_EQ(sanitizeName("\x1F" "a\x7F" "b\x80" "c\xFF", 0), "?a?b?c?");
}

TEST(SanitizeNameTests, TheBoundsOfPrintableAsciiAreKept)
{
    EXPECT_EQ(sanitizeName(" ~", 0), " ~");
}

TEST(SanitizeNameTests, AnEmptyNameIsNumberedFromOne)
{
    EXPECT_EQ(sanitizeName("", 0), "Player 1");
    EXPECT_EQ(sanitizeName("", 2), "Player 3");
}

TEST(SanitizeNameTests, ASixteenByteNameIsKeptWhole)
{
    EXPECT_EQ(sanitizeName("ABCDEFGHIJKLMNOP", 0), "ABCDEFGHIJKLMNOP");
}

TEST(SanitizeNameTests, ALongerNameIsCutToSixteenBytes)
{
    EXPECT_EQ(sanitizeName("ABCDEFGHIJKLMNOPQRS", 0), "ABCDEFGHIJKLMNOP");
}
