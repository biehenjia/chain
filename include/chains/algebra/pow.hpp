#pragma once
#include <cstdint>
#include <vector>
#include <chains/algebra.hpp>
#include <chains/arena.hpp>
#include <symengine/pow.h>
#include <symengine/integer.h>
#include <symengine/ntheory.h>

namespace chains {

    inline uint32_t pow_leaf_leaf(Arena& a, uint32_t x, uint32_t y) {
        const auto& xs = a.leaf_symbolic(x);
        const auto& ys = a.leaf_symbolic(y);
        auto p = SymEngine::pow(xs, ys);
        uint32_t sym_id = a.push_symbolic(p);
        return a.push_leaf(sym_id);
    }

    inline uint32_t pow_leaf_sum(Arena& a, uint32_t x, uint32_t y) {
        auto ops = a.operands(y);
        std::vector<uint32_t> new_ops(ops.size());
        for (size_t i = 0; i < ops.size(); ++i) {
            new_ops[i] = pow(a, x, ops[i]);
        }
        uint8_t var = a.node(y).variable;
        return a.intern_chain(Kind::Prod, var, new_ops);
    }

    inline uint32_t pow_prod_leaf(Arena& a, uint32_t x, uint32_t y) {
        auto ops = a.operands(x);
        std::vector<uint32_t> new_ops(ops.size());
        for (size_t i = 0; i < ops.size(); ++i) {
            new_ops[i] = pow(a, ops[i], y);
        }
        uint8_t var = a.node(x).variable;
        return a.intern_chain(Kind::Prod, var, new_ops);
    }

    inline uint32_t pow_sum_leaf(Arena& a, uint32_t x, uint32_t y) {
        const auto& ys = a.leaf_symbolic(y);
        if (SymEngine::is_a<SymEngine::Integer>(*ys)) {
            const auto& yi = SymEngine::down_cast<const SymEngine::Integer&>(*ys);
            if (!yi.is_negative()) {
                long v = SymEngine::mp_get_si(yi.as_integer_class());
                uint32_t one_sym = a.push_symbolic(SymEngine::integer(1));
                uint32_t result = a.push_leaf(one_sym);
                uint32_t base = x;
                while (v > 0) {
                    if (v & 1) result = mul(a, result, base);
                    v >>= 1;
                    if (v > 0) base = mul(a, base, base);
                }
                return result;
            }
        }
        uint32_t ops[] = {x, y};
        uint8_t var = pick_var(a, x, y);
        return a.intern_algebraic(Kind::EPow, var, {ops, 2});
    }

    inline uint32_t pow_prod_sum(Arena& a, uint32_t x, uint32_t y) {
        auto l_ops = a.operands(x);
        auto r_ops = a.operands(y);
        std::vector<uint32_t> l(l_ops.begin(), l_ops.end());
        std::vector<uint32_t> r(r_ops.begin(), r_ops.end());

        size_t n = (l.size() > r.size() ? l.size() : r.size()) - 1;
        size_t m = (l.size() < r.size() ? l.size() : r.size()) - 1;
        uint8_t var = a.node(x).variable;

        auto binom_leaf = [&](size_t nn, size_t kk) {
            auto n_int = SymEngine::integer(static_cast<long>(nn));
            auto k_ul = static_cast<unsigned long>(kk);
            auto b = SymEngine::binomial(*n_int, k_ul);
            uint32_t sym_id = a.push_symbolic(b);
            return a.push_leaf(sym_id);
        };

        uint32_t one = a.push_leaf(a.push_symbolic(SymEngine::integer(1)));
        std::vector<uint32_t> new_ops(n + m + 1);

        for (size_t i = 0; i < n + m + 1; ++i) {
            uint32_t r1 = one;
            size_t j_lo = (i > m) ? i - m : 0;
            size_t j_hi = std::min(i, n);
            for (size_t j = j_lo; j <= j_hi; ++j) {
                uint32_t r2 = one;
                size_t k_lo = i - j;
                size_t k_hi = std::min(i, m);
                for (size_t k = k_lo; k <= k_hi; ++k) {
                    uint32_t coeff = mul(a, binom_leaf(j, i - k), binom_leaf(i, j));
                    uint32_t term = mul(a, pow(a, l[j], r[k]), coeff);
                    r2 = mul(a, r2, term);
                }
                r1 = mul(a, r1, r2);
            }
            new_ops[i] = r1;
        }

        return a.intern_chain(Kind::Prod, var, new_ops);
    }

}
