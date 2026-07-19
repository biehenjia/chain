#pragma once

#include <cstdint>
#include <span>

#include "mix.hpp"
#include <core/core.hpp>

namespace chains {
    template<class LeafT>
    inline uint32_t hash_chain(Kind k, uint8_t var, std::span<const uint32_t> ops, const Arena<LeafT>& a, std::span<uint32_t> suffix_out) {
        bool trig = is_trig_kind(k);
        uint32_t kseed = trig ? hmix(uint32_t(Kind::Trig), var) : hmix(uint32_t(k), var);
        size_t n = ops.size();
        size_t stride = trig ? n / 2 : 0;
        size_t steps = trig ? stride : n;

        uint32_t running = 0;
        for (int i = int(steps) - 1; i >= 0; --i) {
            uint32_t child = trig ? hmix(a.node(ops[i]).hash, a.node(ops[i + stride]).hash) : a.node(ops[i]).hash;
            uint32_t inner = hmix(child, running);
            running = hmix(kseed, inner);
            suffix_out[i] = running;
        }
        return trig ? hmix(hmix(uint32_t(k), var), running) : running;
    }

}
