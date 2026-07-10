#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <chains/node.hpp>

namespace chains {

class Arena;

    std::optional<uint32_t> simplify_chain(Arena& a, Kind k, uint8_t var, std::span<const uint32_t> ops);
    std::optional<uint32_t> simplify_algebraic(Arena& a, Kind k, uint8_t var, std::span<const uint32_t> ops);

}
