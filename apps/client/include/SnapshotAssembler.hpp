/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SnapshotAssembler.hpp
*/

#ifndef SNAPSHOT_ASSEMBLER_HPP
#define SNAPSHOT_ASSEMBLER_HPP

#include <bitset>
#include <cstdint>
#include <map>
#include <optional>

#include "rtype/protocol/Snapshot.hpp"

namespace rtype::client {
class SnapshotAssembler {
public:
    static constexpr std::size_t MAX_PARTIAL_TICKS = 8;

    [[nodiscard]] bool isStale(std::uint32_t serverTick) const noexcept;
    [[nodiscard]] std::optional<std::uint32_t> add(const protocol::Snapshot
            &chunk);
    [[nodiscard]] std::uint32_t lastCompleteTick() const noexcept;

private:
    struct ChunkSet {
        std::uint8_t count = 0;
        std::bitset<16> received;
    };
    std::uint32_t _lastCompleteTick = 0;
    std::map<std::uint32_t, ChunkSet> _partial;
};
} // namespace rtype::client
#endif /* !SNAPSHOT_ASSEMBLER_HPP_ */
