#pragma once

#include <cstdint>
#include <vector>

#include <core/core.hpp>
#include <hashing/hashing.hpp>
#include "router.hpp"

namespace chains {

    template<class LeafT>
    uint32_t add_leaf_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return i.intern(combine_add(i.arena().leaf(x), i.arena().leaf(y)));
    }

    template<class LeafT>
    uint32_t add_sum_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return rebuild(i, x, Kind::Sum, [&](std::vector<uint32_t>& ops) {
            ops[0] = add(i, ops[0], y);
        });
    }

    template<class LeafT>
    uint32_t add_sum_sum(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return elementwise_combine(i, x, y, Kind::Sum,
            [](Interner<LeafT>& in, uint32_t a, uint32_t b) { return add(in, a, b); });
    }

}
