#pragma once
#include <cstdint>
#include <utility>
#include <vector>
#include <chains/algebra.hpp>
#include <chains/arena.hpp>
#include <symengine/add.h>

namespace chains {

    inline uint32_t add_leaf_leaf(Arena& a, uint32_t x, uint32_t y) {
        auto sum = SymEngine::add(a.leaf_symbolic(x), a.leaf_symbolic(y));
        uint32_t sym_id = a.push_symbolic(sum);
        return a.push_leaf(sym_id);
    }



    inline uint32_t add_sum_leaf(Arena& a, uint32_t x, uint32_t y) {
        auto ops = a.operands(x);
        std::vector<uint32_t> new_ops(ops.begin(), ops.end());
        new_ops[0] = add(a, new_ops[0], y);
        return a.intern_chain(Kind::Sum, a.node(x).variable, new_ops);
    }

    inline uint32_t add_sum_sum(Arena& a, uint32_t x, uint32_t y) {

        if (a.node(y).slot_b > a.node(x).slot_b) {
            std::swap(x, y);
        }

        auto x_ops = a.operands(x);
        auto y_ops = a.operands(y);

        std::vector<uint32_t> new_ops(x_ops.begin(), x_ops.end());
        std::vector<uint32_t> r_ops(y_ops.begin(), y_ops.end());

        for (size_t i = 0; i < r_ops.size(); ++i) {
            new_ops[i] = add(a, new_ops[i], r_ops[i]);
        }

        return a.intern_chain(Kind::Sum, a.node(x).variable, new_ops);
    }



}
