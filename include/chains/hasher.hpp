#pragma once
#include <cstdint>
#include <span>
#include <vector>
#include <chains/node.hpp>

namespace chains {

    // swap for whatever... 
    static inline uint32_t hmix(uint32_t a, uint32_t b) {
        return a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2));
    }

    // leaf: hash over kind and the symbolic id (leaves carry no variable)
    inline uint32_t hash_leaf(uint32_t sym_id) {
        return hmix(uint32_t(Kind::Leaf), sym_id);
    }

    // algebraic n-ary: hash over kind, variable, and each child's hash in order
    inline uint32_t hash_algebraic(Kind k, uint8_t var, std::span<const uint32_t> child_hashes) {

        uint32_t h = hmix(uint32_t(k), var);

        for (uint32_t ch : child_hashes) {
            h = hmix(h, ch);
        }
        return h;
    }

    struct ChainHash {
        uint32_t head; // hash of the full chain
        std::vector<uint32_t> suffixes;  // parallel to the operand span
    };

    // suffix hashes. For trig types, we only do up to len(CR)//2

    inline ChainHash hash_chain(Kind k, uint8_t var, std::span<const uint32_t> child_hashes) {

        uint32_t kseed = hmix(uint32_t(k), var);
        size_t n = child_hashes.size();
        bool trig = is_trig_kind(k);

        size_t stride = trig ? n / 2 : 0;
        size_t steps = trig ? stride : n;
        std::vector<uint32_t> suffixes(n, 0);
        uint32_t running = 0;

        for (int i = int(steps) - 1; i >= 0; --i) {
            uint32_t child = trig ? hmix(child_hashes[i], child_hashes[i + stride]) : child_hashes[i];

            running = hmix(kseed, hmix(child, running));
            suffixes[i] = running;
        }
        return {running, std::move(suffixes)};
    }

}
