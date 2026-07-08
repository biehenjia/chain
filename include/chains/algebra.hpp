#pragma once
#include <algorithm>
#include <cstdint>

#include "chains/arena.hpp"
#include "chains/node.hpp"

namespace chains {
    using BinaryRule = uint32_t (*)(Arena&, uint32_t, uint32_t);
    using UnaryRule = uint32_t (*)(Arena&, uint32_t);

    inline uint8_t pick_var(const Arena& a, uint32_t x, uint32_t y) {
        return std::max(a.node(x).variable, a.node(y).variable);
    }

    uint32_t add(Arena& a, uint32_t x, uint32_t y);
    uint32_t mul(Arena& a, uint32_t x, uint32_t y);
    uint32_t pow(Arena& a, uint32_t x, uint32_t y);
    uint32_t log(Arena& a, uint32_t x, uint32_t y);
    uint32_t sin(Arena& a, uint32_t x);
    uint32_t cos(Arena& a, uint32_t x);
    uint32_t tan(Arena& a, uint32_t x);
    uint32_t cot(Arena& a, uint32_t x);
}

