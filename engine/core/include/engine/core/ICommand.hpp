/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ICommand
*/

#ifndef ZAPPY_ICOMMAND_HPP
#define ZAPPY_ICOMMAND_HPP

template <typename... Args>
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual bool execute(Args... args) = 0;

    virtual bool operator()(Args... args) = 0;
};

#endif
