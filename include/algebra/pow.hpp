#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <core/core.hpp>
#include <hashing/hashing.hpp>
#include "router.hpp"

namespace chains {

    template<class LeafT>
    uint32_t pow_leaf_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return i.intern(combine_pow(i.arena().leaf(x), i.arena().leaf(y)));
    }

    template<class LeafT>
    uint32_t pow_leaf_sum(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return rebuild(i, y, Kind::Prod, [&](std::vector<uint32_t>& ops) {
            for (auto& o : ops) o = pow(i, x, o);
        });
    }

    template<class LeafT>
    uint32_t pow_prod_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return rebuild(i, x, Kind::Prod, [&](std::vector<uint32_t>& ops) {
            for (auto& o : ops) o = pow(i, o, y);
        });
    }

    template<class LeafT>
    uint32_t pow_sum_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        if (auto exp = as_nonneg_int(i.arena().leaf(y))) {
            uint32_t result = i.one();
            uint32_t base = x;
            long v = *exp;
            while (v > 0) {
                if (v & 1) result = mul(i, result, base);
                v >>= 1;
                if (v > 0) base = mul(i, base, base);
            }
            return result;
        }
        uint32_t ops[] = {x, y};
        uint8_t var = pick_var(i, x, y);
        return i.intern(Kind::EPow, var, {ops, 2});
    }

    template<class LeafT>
    uint32_t pow_prod_sum(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        auto l_ops = i.arena().operands(x);
        auto r_ops = i.arena().operands(y);
        std::vector<uint32_t> l(l_ops.begin(), l_ops.end());
        std::vector<uint32_t> r(r_ops.begin(), r_ops.end());

        std::size_t n = (l.size() > r.size() ? l.size() : r.size()) - 1;
        std::size_t m = (l.size() < r.size() ? l.size() : r.size()) - 1;
        uint8_t var = i.arena().node(x).var;

        auto binom_leaf = [&](std::size_t nn, std::size_t kk) {
            return i.intern(make_binom<LeafT>(nn, kk));
        };

        uint32_t one = i.one();
        std::vector<uint32_t> new_ops(n + m + 1);

        for (std::size_t t = 0; t < n + m + 1; ++t) {
            uint32_t r1 = one;
            std::size_t j_lo = (t > m) ? t - m : 0;
            std::size_t j_hi = std::min(t, n);
            for (std::size_t j = j_lo; j <= j_hi; ++j) {
                uint32_t r2 = one;
                std::size_t k_lo = t - j;
                std::size_t k_hi = std::min(t, m);
                for (std::size_t k = k_lo; k <= k_hi; ++k) {
                    uint32_t coeff = mul(i, binom_leaf(j, t - k), binom_leaf(t, j));
                    uint32_t term  = mul(i, pow(i, l[j], r[k]), coeff);
                    r2 = mul(i, r2, term);
                }
                r1 = mul(i, r1, r2);
            }
            new_ops[t] = r1;
        }

        return i.intern(Kind::Prod, var, new_ops);
    }

}
