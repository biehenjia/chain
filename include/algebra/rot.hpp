#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>

namespace cr::algebra {

    template <class T>
    uint32_t sin(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t x);
    template <class T>
    uint32_t cos(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t x);

    template <class T>
    uint32_t trig_rot(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t u, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(u);
        std::vector<uint32_t> pairs(ops.size());
        for (std::size_t i = 0; i < ops.size(); ++i) {
            const uint32_t s = sin(arena, interner, ops[i]);
            const uint32_t c = cos(arena, interner, ops[i]);
            pairs[i] = interner.intern_pair(var, s, c);
        }
        return interner.intern_node(cr::Kind::Rot, var, pairs);
    }

    template <class T>
    uint32_t trig_sum(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t u, uint8_t var, cr::Kind target) {
        const uint32_t rot = trig_rot(arena, interner, u, var);
        const std::array<uint32_t, 1> ops{rot};
        return interner.intern_node(target, var, ops);
    }

}
