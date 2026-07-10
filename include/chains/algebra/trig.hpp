#pragma once
#include <cstdint>
#include <vector>
#include <chains/algebra.hpp>
#include <chains/arena.hpp>
#include <symengine/functions.h>

namespace chains {

    using SymUnary = SymEngine::RCP<const SymEngine::Basic> (*)(const SymEngine::RCP<const SymEngine::Basic>&);

    inline uint32_t trig_leaf(Arena& a, uint32_t x, SymUnary fn) {
        const auto& xs = a.leaf_symbolic(x);
        auto v = fn(xs);
        uint32_t sym_id = a.push_symbolic(v);
        return a.push_leaf(sym_id);
    }

    inline uint32_t trig_sum(Arena& a, uint32_t x, Kind trig_kind) {
        auto ops = a.operands(x);
        size_t n = ops.size();
        std::vector<uint32_t> new_ops(2 * n);
        for (size_t i = 0; i < n; ++i) {
            new_ops[i] = sin(a, ops[i]);
            new_ops[i + n] = cos(a, ops[i]);
        }
        uint8_t var = a.node(x).variable;
        return a.intern_chain(trig_kind, var, new_ops);
    }

    inline uint32_t sin_leaf(Arena& a, uint32_t x) { return trig_leaf(a, x, SymEngine::sin); }
    inline uint32_t cos_leaf(Arena& a, uint32_t x) { return trig_leaf(a, x, SymEngine::cos); }
    inline uint32_t tan_leaf(Arena& a, uint32_t x) { return trig_leaf(a, x, SymEngine::tan); }
    inline uint32_t cot_leaf(Arena& a, uint32_t x) { return trig_leaf(a, x, SymEngine::cot); }

    inline uint32_t sin_sum(Arena& a, uint32_t x) { return trig_sum(a, x, Kind::Sin); }
    inline uint32_t cos_sum(Arena& a, uint32_t x) { return trig_sum(a, x, Kind::Cos); }
    inline uint32_t tan_sum(Arena& a, uint32_t x) { return trig_sum(a, x, Kind::Tan); }
    inline uint32_t cot_sum(Arena& a, uint32_t x) { return trig_sum(a, x, Kind::Cot); }

}
