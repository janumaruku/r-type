/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SnapshotAssembler.cpp
*/

#include "rtype/protocol/Snapshot.hpp"

#include <cstdint>
#include <optional>

#include "network/UdpSocket.hpp"
#include "SnapshotAssembler.hpp"

namespace rtype::client {

bool SnapshotAssembler::isStale(std::uint32_t serverTick) const noexcept
{
    return serverTick <= _lastCompleteTick;
}

std::optional<std::uint32_t> SnapshotAssembler::add(const protocol::Snapshot
        &chunk)
{
    if (isStale(chunk.serverTick))
        return std::nullopt;
    if (chunk.chunkIndex >= chunk.chunkCount)
        return std::nullopt;

    auto it = _partial.find(chunk.serverTick);
    if (it == _partial.end()) {
        if (_partial.size() >= MAX_PARTIAL_TICKS)
            _partial.erase(_partial.begin());
        it               = _partial.emplace(chunk.serverTick, ChunkSet{}).first;
        it->second.count = chunk.chunkCount;
    } else if (it->second.count != chunk.chunkCount) {
        engine::core::log::Logger a;
        a.warn("Snapshot chunkCount changed within tick {}",
            std::to_string(chunk.serverTick));
        return std::nullopt;
    }

    auto &set = it->second;
    if (set.received.test(chunk.chunkIndex))
        return std::nullopt;
    set.received.set(chunk.chunkIndex);
    if (set.received.count() != chunk.chunkCount)
        return std::nullopt;

    _lastCompleteTick = chunk.serverTick;
    _partial.erase(_partial.begin(), std::next(it));
    return chunk.serverTick;
}

std::uint32_t SnapshotAssembler::lastCompleteTick() const noexcept
{
    return _lastCompleteTick;
}

} // namespace rtype::client
