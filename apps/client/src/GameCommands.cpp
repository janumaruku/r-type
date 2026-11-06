/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameCommands.cpp
*/

#include "GameCommands.hpp"

#include "ClientNetworkAdapter.hpp"
#include "rtype/protocol/Decode.tpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace rtype::client {
//////////////////////////////////////////////////////////
bool SnapshotCommand::execute(ClientNetworkAdapter &adapter,
    net::Packet &packet)
{
    auto snapshot   = protocol::decode<protocol::Snapshot>(packet);
    auto &assembler = adapter.assembler();
    auto &world     = adapter.world();

    if (assembler.isStale(snapshot.serverTick))
        return true;

    for (const auto &state: snapshot.entities)
        world.applyEntity(snapshot.serverTick, state);

    if (snapshot.chunkIndex == 0) {
        world.applyRoster(snapshot.roster);
        world.setLastProcessedInput(snapshot.lastProcessedInput);
    }

    if (auto completeTick = assembler.add(snapshot))
        world.removeNotSeenSince(completeTick.value());

    return true;
}

bool SnapshotCommand::operator()(ClientNetworkAdapter &adapter,
    net::Packet &packet)
{
    return execute(adapter, packet);
}

std::unique_ptr<IGameCommand> SnapshotCommand::create()
{
    return std::make_unique<SnapshotCommand>();
}
///////////////////////////////////////////////////////////

bool EventCommand::execute(ClientNetworkAdapter &adapter, net::Packet &packet)
{
    auto event = protocol::decode<protocol::Event>(packet);
    if (adapter.seenEvents().firstTime(event.eventId))
        adapter.events().publish(event);
    return true;
}

bool EventCommand::operator()(ClientNetworkAdapter &adapter,
    net::Packet &packet)
{
    return execute(adapter, packet);
}

std::unique_ptr<IGameCommand> EventCommand::create()
{
    return std::make_unique<EventCommand>();
}
////////////////////////////////////////////////////////////

bool ShutdownCommand::execute(ClientNetworkAdapter &adapter, net::Packet &)
{
    adapter.leave(LeaveCause::SHUTDOWN);
    return true;
}

bool ShutdownCommand::operator()(ClientNetworkAdapter &adapter,
    net::Packet &packet)
{
    return execute(adapter, packet);
}

std::unique_ptr<IGameCommand> ShutdownCommand::create()
{
    return std::make_unique<ShutdownCommand>();
}

////////////////////////////////////////////////////////////

bool KickCommand::execute(ClientNetworkAdapter &adapter, net::Packet &packet)
{
    auto kick = protocol::decode<protocol::Kick>(packet);

    auto cause = LeaveCause::NO_CAUSE;

    switch (kick.reason) {
    case protocol::KickReason::PROTOCOL_ERROR:
        cause = LeaveCause::KICKED_PROTOCOL;
        break;
    case protocol::KickReason::RATE_LIMITED:
        cause = LeaveCause::KICKED_RATE;
        break;
    default:
        cause = LeaveCause::NO_CAUSE;
        break;
    }
    adapter.leave(cause);
    return true;
}

bool KickCommand::operator()(ClientNetworkAdapter &adapter, net::Packet &packet)
{
    return execute(adapter, packet);
}

std::unique_ptr<IGameCommand> KickCommand::create()
{
    return std::make_unique<KickCommand>();
}
/////////////////////////////////////////////////////////////
} // namespace rtype::client
