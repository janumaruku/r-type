/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameCommands.hpp
*/

#ifndef GAME_COMMANDS_HPP
#define GAME_COMMANDS_HPP

#include "core/ICommand.hpp"

using IGameCommand = rtype::core::ICommand<rtype::protocol::Event>;

class SnapshotCommand : public IGameCommand { /* execute, create */ };
class EventCommand : public IGameCommand { /* execute, create */ };
class ShutdownCommand : public IGameCommand { /* execute, create */ };
class KickCommand : public IGameCommand { /* execute, create */ };

#endif /* !GAME_COMMANDS_HPP_ */