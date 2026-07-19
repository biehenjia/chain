#pragma once

#include <cstdint>
#include <vector>

#include <core/core.hpp>
#include <hashing/hashing.hpp>
#include "router.hpp"

namespace chains {

    template<class LeafT>
    uint32_t log_leaf_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return i.intern(combine_log(i.arena().leaf(x), i.arena().leaf(y)));
    }

    template<class LeafT>
    uint32_t log_prod_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return rebuild(i, x, Kind::Sum, [&](std::vector<uint32_t>& ops) {
            for (auto& o : ops) o = log(i, o, y);
        });
    }

}
