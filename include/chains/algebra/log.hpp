#pragma once
#include <cstdint>
#include <vector>
#include <chains/algebra.hpp>
#include <chains/arena.hpp>
#include <symengine/functions.h>
#include <symengine/mul.h>

namespace chains {

    inline uint32_t log_leaf_leaf(Arena& a, uint32_t x, uint32_t y) {
        const auto& xs = a.leaf_symbolic(x);
        const auto& ys = a.leaf_symbolic(y);
        // log base ys of xs = log(xs) / log(ys)
        auto l = SymEngine::log(xs, ys);
        uint32_t sym_id = a.push_symbolic(l);
        return a.push_leaf(sym_id);
    }

    inline uint32_t log_prod_leaf(Arena& a, uint32_t x, uint32_t y) {
        auto ops = a.operands(x);
        std::vector<uint32_t> new_ops(ops.size());
        for (size_t i = 0; i < ops.size(); ++i) {
            new_ops[i] = log(a, ops[i], y);
        }
        uint8_t var = a.node(x).variable;
        return a.intern_chain(Kind::Sum, var, new_ops);
    }

}
