#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <core/core.hpp>
#include <hashing/hashing.hpp>
#include "router.hpp"

namespace chains {

    template<class LeafT>
    uint32_t mul_leaf_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return i.intern(combine_mul(i.arena().leaf(x), i.arena().leaf(y)));
    }

    template<class LeafT>
    uint32_t mul_sum_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return rebuild(i, x, Kind::Sum, [&](std::vector<uint32_t>& ops) {
            for (auto& o : ops) o = mul(i, o, y);
        });
    }

    template<class LeafT>
    uint32_t mul_prod_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return rebuild(i, x, Kind::Prod, [&](std::vector<uint32_t>& ops) {
            ops[0] = mul(i, ops[0], y);
        });
    }

    template<class LeafT>
    uint32_t mul_prod_prod(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return elementwise_combine(i, x, y, Kind::Prod,
            [](Interner<LeafT>& in, uint32_t a, uint32_t b) { return mul(in, a, b); });
    }

    template<class LeafT>
    uint32_t mul_trig_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y, Kind trig_kind) {
        return rebuild(i, x, trig_kind, [&](std::vector<uint32_t>& ops) {
            std::size_t hl = ops.size() / 2;
            ops[0]  = mul(i, ops[0],  y);
            ops[hl] = mul(i, ops[hl], y);
        });
    }

    template<class LeafT> uint32_t mul_sin_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) { return mul_trig_leaf(i, x, y, Kind::Sin); }
    template<class LeafT> uint32_t mul_cos_leaf(Interner<LeafT>& i, uint32_t x, uint32_t y) { return mul_trig_leaf(i, x, y, Kind::Cos); }

    template<class LeafT>
    uint32_t mul_prod_trig_impl(Interner<LeafT>& i, uint32_t p, uint32_t t, Kind trig_kind) {
        auto p_ops = i.arena().operands(p);
        auto t_ops = i.arena().operands(t);
        std::size_t pn  = p_ops.size();
        std::size_t thl = t_ops.size() / 2;

        std::vector<uint32_t> pv(p_ops.begin(), p_ops.end());
        std::vector<uint32_t> tv(t_ops.begin(), t_ops.end());
        std::size_t newlength;

        if (thl > pn) {
            pv.resize(thl, i.one());
            newlength = thl;
        } else if (thl < pn) {
            uint32_t zero = i.zero();
            uint32_t one  = i.one();
            std::vector<uint32_t> new_tv(2 * pn);
            for (std::size_t k = 0; k < pn; ++k) new_tv[k]      = (k < thl) ? tv[k]       : zero;
            for (std::size_t k = 0; k < pn; ++k) new_tv[k + pn] = (k < thl) ? tv[k + thl] : one;
            tv = std::move(new_tv);
            newlength = pn;
        } else {
            newlength = thl;
        }

        std::vector<uint32_t> new_ops(newlength * 2);
        for (std::size_t k = 0; k < newlength * 2; ++k) {
            std::size_t j = (k < newlength) ? k : (k - newlength);
            new_ops[k] = mul(i, tv[k], pv[j]);
        }
        uint8_t var = i.arena().node(t).var;
        return i.intern(trig_kind, var, new_ops);
    }

    template<class LeafT>
    uint32_t mul_prod_sin(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return mul_prod_trig_impl(i, x, y, Kind::Sin);
    }

    template<class LeafT>
    uint32_t mul_prod_cos(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return mul_prod_trig_impl(i, x, y, Kind::Cos);
    }

    template<class LeafT>
    uint32_t mul_sum_sum(Interner<LeafT>& i, uint32_t x, uint32_t y) {
        if (i.arena().node(y).slot_b > i.arena().node(x).slot_b) {
            std::swap(x, y);
        }

        auto l_ops = i.arena().operands(x);
        auto r_ops = i.arena().operands(y);
        std::vector<uint32_t> l(l_ops.begin(), l_ops.end());
        std::vector<uint32_t> r(r_ops.begin(), r_ops.end());

        std::size_t n = l.size() - 1;
        std::size_t m = r.size() - 1;
        uint8_t var = i.arena().node(x).var;

        auto binom_leaf = [&](std::size_t nn, std::size_t kk) {
            return i.intern(make_binom<LeafT>(nn, kk));
        };

        std::vector<uint32_t> new_ops(n + m + 1);

        for (std::size_t t = 0; t < n + m + 1; ++t) {
            std::optional<uint32_t> r1;
            std::size_t j_lo = (t > m) ? t - m : 0;
            std::size_t j_hi = std::min(t, n);
            for (std::size_t j = j_lo; j <= j_hi; ++j) {
                std::optional<uint32_t> r2;
                std::size_t k_lo = t - j;
                std::size_t k_hi = std::min(t, m);
                for (std::size_t k = k_lo; k <= k_hi; ++k) {
                    uint32_t coeff = binom_leaf(j, t - k);
                    uint32_t term = mul(i, coeff, r[k]);
                    r2 = r2 ? add(i, *r2, term) : term;
                }
                uint32_t outer_coeff = binom_leaf(t, j);
                uint32_t scaled = mul(i, *r2, outer_coeff);
                uint32_t contrib = mul(i, l[j], scaled);
                r1 = r1 ? add(i, *r1, contrib) : contrib;
            }
            new_ops[t] = *r1;
        }

        return i.intern(Kind::Sum, var, new_ops);
    }

}
