/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FactoryTemplateTests.cpp
*/

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "engine/core/FactoryTemplate.hpp"

namespace {

class Command {
public:
    virtual ~Command() = default;
    virtual int execute(int value) const = 0;
};

class AddCommand : public Command {
public:
    static std::unique_ptr<Command> create(int amount)
    {
        return std::make_unique<AddCommand>(amount);
    }

    explicit AddCommand(int amount) : _amount(amount) {}

    int execute(int value) const override
    {
        return value + _amount;
    }

    int operator()(int value) const
    {
        return execute(value);
    }

private:
    int _amount;
};

class MultiplyCommand : public Command {
public:
    static std::unique_ptr<Command> create(int factor)
    {
        return std::make_unique<MultiplyCommand>(factor);
    }

    explicit MultiplyCommand(int factor) : _factor(factor) {}

    int execute(int value) const override
    {
        return value * _factor;
    }

    int operator()(int value) const
    {
        return execute(value);
    }

private:
    int _factor;
};

class InvalidCommand {
};

using CommandFactory = designPattern::FactoryTemplate<Command, std::string, int>;

static_assert(designPattern::HasCreateMethod<Command, AddCommand, int>);
static_assert(!designPattern::HasCreateMethod<Command, InvalidCommand, int>);

}

TEST(FactoryTemplate, RegisterAndCreateReturnsConcreteType)
{
    CommandFactory factory;
    factory.registerCreator<AddCommand>("add");

    std::unique_ptr<Command> command = factory.create("add", 5);

    EXPECT_NE(dynamic_cast<AddCommand *>(command.get()), nullptr);
    EXPECT_EQ(command->execute(7), 12);
}

TEST(FactoryTemplate, CommandCallOperatorExecutesCommand)
{
    AddCommand add(5);
    MultiplyCommand multiply(3);

    EXPECT_EQ(add(7), add.execute(7));
    EXPECT_EQ(multiply(7), multiply.execute(7));
    EXPECT_EQ(add(7), 12);
    EXPECT_EQ(multiply(7), 21);
}

TEST(FactoryTemplate, HasKeyOnlyForRegisteredKeys)
{
    CommandFactory factory;

    EXPECT_FALSE(factory.hasKey("add"));
    factory.registerCreator<AddCommand>("add");
    EXPECT_TRUE(factory.hasKey("add"));
    EXPECT_FALSE(factory.hasKey("multiply"));
}

TEST(FactoryTemplate, UnknownKeyThrowsWithKeyInMessage)
{
    CommandFactory factory;

    try {
        factory.create("missing", 1);
        FAIL() << "Expected create to throw";
    } catch (const std::runtime_error &error) {
        EXPECT_NE(std::string(error.what()).find("missing"), std::string::npos);
    }
}

TEST(FactoryTemplate, UnregisterRemovesKey)
{
    CommandFactory factory;
    factory.registerCreator<AddCommand>("add");
    ASSERT_TRUE(factory.hasKey("add"));

    factory.unregisterCreator("add");

    EXPECT_FALSE(factory.hasKey("add"));
}