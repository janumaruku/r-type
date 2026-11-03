/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SnapshotAssembler.cpp
*/


#include <optional>
#include <cstdint>

#include "rtype/protocol/Snapshot.hpp"
#include "SnapshotAssembler.hpp"

namespace rtype::client {

bool SnapshotAssembler::isStale(std::uint32_t serverTick) const noexcept
{
    return serverTick <= _lastCompleteTick;

}

std::optional<std::uint32_t> SnapshotAssembler::add(const protocol::Snapshot &chunk)
{
    if (isStale(chunk.serverTick)) {
        return std::nullopt;
    }

    auto &set = _partial[chunk.serverTick];
    set.received.set(chunk.chunkIndex);
    set.count++;

    if (set.count == chunk.chunkCount) {
        _lastCompleteTick = chunk.serverTick;
        _partial.erase(chunk.serverTick);
        return chunk.serverTick;
    }
    return std::nullopt;
}

std::uint32_t SnapshotAssembler::lastCompleteTick() const noexcept
{
    return _lastCompleteTick;
}

}

