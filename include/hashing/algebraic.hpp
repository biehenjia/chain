#pragma once

#include <cstdint>
#include <span>

#include "mix.hpp"
#include <core/core.hpp>

namespace chains {

    template<class LeafT>
    inline uint32_t hash_algebraic(Kind k, uint8_t var, std::span<const uint32_t> ops, const Arena<LeafT>& a) {
        uint32_t h = hmix(uint32_t(k), var);
        for (uint32_t op : ops) h = hmix(h, a.node(op).hash);
        return h;
    }

}
