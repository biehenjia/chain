#pragma once

#include <cstdint>
#include <cstring>

#include "node.hpp"


namespace chains::leaf_hash_detail {
    inline uint32_t hmix(uint32_t a, uint32_t b) { return a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2)); }
}

namespace chains {

    inline uint32_t hash_leaf(double v) {
        uint64_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        if (v == 0.0) bits = 0; 
        uint32_t lo = uint32_t(bits);
        uint32_t hi = uint32_t(bits >> 32);
        return leaf_hash_detail::hmix(leaf_hash_detail::hmix(uint32_t(Kind::Leaf), lo), hi);
    }

    inline bool leaf_equal(double a, double b) { return a == b; }

}
