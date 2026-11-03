/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ICommand
*/

#ifndef ICOMMAND_HPP
#define ICOMMAND_HPP

namespace designPattern {
template <typename... Args>
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual bool execute(Args... args) = 0;

    virtual bool operator()(Args... args) = 0;
};
}
#endif
