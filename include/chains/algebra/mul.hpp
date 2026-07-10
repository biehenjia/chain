#pragma once
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>
#include <chains/algebra.hpp>
#include <chains/arena.hpp>
#include <symengine/mul.h>
#include <symengine/integer.h>
#include <symengine/ntheory.h>

namespace chains{

    inline uint32_t mul_leaf_leaf(Arena& a, uint32_t x, uint32_t y ){
        const auto& xs = a.leaf_symbolic(x);
        const auto& ys = a.leaf_symbolic(y);
        auto prod = SymEngine::mul(xs, ys);
        uint32_t sym_id = a.push_symbolic(prod);
        return a.push_leaf(sym_id);
    }

    inline uint32_t mul_sum_leaf(Arena& a, uint32_t x, uint32_t y){
        auto ops = a.operands(x);
        std::vector<uint32_t> new_ops(ops.begin(), ops.end());

        for (size_t i = 0; i < ops.size(); ++i){
            new_ops[i] = mul(a, new_ops[i], y);
        }
        uint8_t var = a.node(x).variable;
        return a.intern_chain(Kind::Sum, var, new_ops);
    }

    inline uint32_t mul_prod_leaf(Arena& a, uint32_t x, uint32_t y){
        auto ops = a.operands(x);
        std::vector<uint32_t> new_ops(ops.begin(), ops.end());
        new_ops[0] = mul(a, new_ops[0], y);
        uint8_t var = a.node(x).variable;
        return a.intern_chain(Kind::Prod, var, new_ops);
    }

    inline uint32_t mul_prod_prod(Arena& a, uint32_t x, uint32_t y){
        if (a.node(y).slot_b > a.node(x).slot_b){
            std::swap(x,y);
        }

        auto x_ops = a.operands(x);
        auto y_ops = a.operands(y);

        std::vector<uint32_t> new_ops(x_ops.begin(),x_ops.end());
        std::vector<uint32_t> r_ops(y_ops.begin(), y_ops.end());

        for (size_t i = 0; i < r_ops.size(); ++i){
            new_ops[i] = mul(a, new_ops[i], r_ops[i]);
        }

        uint8_t var = a.node(x).variable;
        return a.intern_chain(Kind::Prod, var, new_ops);

    }

    inline uint32_t mul_sin_leaf(Arena& a, uint32_t x, uint32_t y){
        auto ops = a.operands(x);
        std::vector<uint32_t> new_ops(ops.begin(), ops.end());
        size_t hl = new_ops.size() / 2;
        new_ops[0]  = mul(a, new_ops[0], y);
        new_ops[hl] = mul(a, new_ops[hl], y);
        uint8_t var = a.node(x).variable;
        return a.intern_chain(Kind::Sin, var, new_ops);
    }

    inline uint32_t mul_cos_leaf(Arena& a, uint32_t x, uint32_t y){
        auto ops = a.operands(x);
        std::vector<uint32_t> new_ops(ops.begin(), ops.end());
        size_t hl = new_ops.size() / 2;
        new_ops[0]  = mul(a, new_ops[0], y);
        new_ops[hl] = mul(a, new_ops[hl], y);
        uint8_t var = a.node(x).variable;
        return a.intern_chain(Kind::Cos, var, new_ops);
    }

    // shared kernel for trig types 
    inline uint32_t mul_prod_trig_impl(Arena& a, uint32_t p, uint32_t t, Kind trig_kind){
        auto p_ops = a.operands(p);
        auto t_ops = a.operands(t);
        size_t pn  = p_ops.size();
        size_t thl = t_ops.size() / 2;

        std::vector<uint32_t> pv(p_ops.begin(), p_ops.end());
        std::vector<uint32_t> tv(t_ops.begin(), t_ops.end());
        size_t newlength;

        auto int_leaf = [&](long v) {
            return a.push_leaf(a.push_symbolic(SymEngine::integer(v)));
        };

        if (thl > pn) {
            uint32_t one = int_leaf(1);
            pv.resize(thl, one);
            newlength = thl;
        } else if (thl < pn) {
            uint32_t zero = int_leaf(0);
            uint32_t one  = int_leaf(1);
            std::vector<uint32_t> new_tv(2 * pn);
            for (size_t i = 0; i < pn; ++i) new_tv[i]      = (i < thl) ? tv[i]       : zero;
            for (size_t i = 0; i < pn; ++i) new_tv[i + pn] = (i < thl) ? tv[i + thl] : one;
            tv = std::move(new_tv);
            newlength = pn;
        } else {
            newlength = thl;
        }

        std::vector<uint32_t> new_ops(newlength * 2);
        for (size_t i = 0; i < newlength * 2; ++i){
            size_t j = (i < newlength) ? i : (i - newlength);
            new_ops[i] = mul(a, tv[i], pv[j]);
        }
        uint8_t var = a.node(t).variable;
        return a.intern_chain(trig_kind, var, new_ops);
    }

    inline uint32_t mul_prod_sin(Arena& a, uint32_t x, uint32_t y){
        return mul_prod_trig_impl(a, x, y, Kind::Sin);
    }

    inline uint32_t mul_prod_cos(Arena& a, uint32_t x, uint32_t y){
        return mul_prod_trig_impl(a, x, y, Kind::Cos);
    }

    inline uint32_t mul_sum_sum(Arena& a, uint32_t x, uint32_t y){
        if (a.node(y).slot_b > a.node(x).slot_b){
            std::swap(x,y);
        }

        auto l_ops = a.operands(x);
        auto r_ops = a.operands(y);
        std::vector<uint32_t> l(l_ops.begin(), l_ops.end());
        std::vector<uint32_t> r(r_ops.begin(), r_ops.end());

        size_t n = l.size() - 1;
        size_t m = r.size() - 1;
        uint8_t var = a.node(x).variable;

        auto binom_leaf = [&](size_t nn, size_t kk) {
            auto n_int = SymEngine::integer(static_cast<long>(nn));
            auto k_ul = static_cast<unsigned long>(kk);
            auto b = SymEngine::binomial(*n_int, k_ul);
            uint32_t sym_id = a.push_symbolic(b);
            return a.push_leaf(sym_id);
        };
        
        std::vector<uint32_t> new_ops(n + m + 1);

        for (size_t i = 0; i < n + m + 1; ++i) {
            std::optional<uint32_t> r1;
            size_t j_lo = (i > m) ? i - m : 0;
            size_t j_hi = std::min(i, n);
            for (size_t j = j_lo; j <= j_hi; ++j) {
                std::optional<uint32_t> r2;
                size_t k_lo = i - j;
                size_t k_hi = std::min(i, m);
                for (size_t k = k_lo; k <= k_hi; ++k) {
                    uint32_t coeff = binom_leaf(j, i - k);
                    uint32_t term = mul(a, coeff, r[k]);
                    r2 = r2 ? add(a, *r2, term) : term;
                }
                uint32_t outer_coeff = binom_leaf(i, j);
                uint32_t scaled = mul(a, *r2, outer_coeff);
                uint32_t contrib = mul(a, l[j], scaled);
                r1 = r1 ? add(a, *r1, contrib) : contrib;
            }
            new_ops[i] = *r1;
        }

        return a.intern_chain(Kind::Sum, var, new_ops);
    }



}