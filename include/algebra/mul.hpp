#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include <cr/domain/field.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>

#include "add.hpp"

namespace cr::algebra {

    template <class T>
    uint32_t mul(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs);

    template <class T>
    uint32_t mul_leaf_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const T prod = cr::Field<T>::mul(arena.value(arena.node(lhs).ops), arena.value(arena.node(rhs).ops));
        return interner.intern_leaf(var, prod);
    }

    template <class T>
    uint32_t mul_sum_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(lhs);
        std::vector<uint32_t> out(ops.size());
        for (std::size_t i = 0; i < ops.size(); ++i) out[i] = mul(arena, interner, ops[i], rhs);
        return interner.intern_node(cr::Kind::Sum, var, out);
    }

    template <class T>
    uint32_t mul_sum_sum(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        std::span<const uint32_t> lops = arena.operands(lhs);
        std::span<const uint32_t> rops = arena.operands(rhs);
        if (rops.size() > lops.size()) std::swap(lops, rops);

        const std::size_t n = lops.size() - 1;
        const std::size_t m = rops.size() - 1;
        const uint32_t zero = interner.intern_leaf(0, cr::Field<T>::zero());

        std::vector<uint32_t> out(n + m + 1);
        for (std::size_t i = 0; i <= n + m; ++i) {
            uint32_t r1 = zero;
            const std::size_t j_lo = i > m ? i - m : 0;
            const std::size_t j_hi = std::min(i, n);
            for (std::size_t j = j_lo; j <= j_hi; ++j) {
                uint32_t r2 = zero;
                const std::size_t k_hi = std::min(i, m);
                for (std::size_t k = i - j; k <= k_hi; ++k) {
                    const uint32_t coeff = interner.intern_leaf(0, field_from_uint<T>(binomial(j, i - k)));
                    r2 = add(arena, interner, r2, mul(arena, interner, coeff, rops[k]));
                }
                const uint32_t outer_coeff = interner.intern_leaf(0, field_from_uint<T>(binomial(i, j)));
                r2 = mul(arena, interner, r2, outer_coeff);
                r1 = add(arena, interner, r1, mul(arena, interner, lops[j], r2));
            }
            out[i] = r1;
        }
        return interner.intern_node(cr::Kind::Sum, var, out);
    }

    template <class T>
    uint32_t mul_prod_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(lhs);
        std::vector<uint32_t> out(ops.begin(), ops.end());
        out[0] = mul(arena, interner, out[0], rhs);
        return interner.intern_node(cr::Kind::Prod, var, out);
    }

    template <class T>
    uint32_t mul_prod_prod(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        std::span<const uint32_t> lops = arena.operands(lhs);
        std::span<const uint32_t> rops = arena.operands(rhs);
        if (rops.size() > lops.size()) std::swap(lops, rops);

        std::vector<uint32_t> out(lops.size());
        for (std::size_t i = 0; i < lops.size(); ++i)
            out[i] = i < rops.size() ? mul(arena, interner, lops[i], rops[i]) : lops[i];
        return interner.intern_node(cr::Kind::Prod, var, out);
    }

    template <class T>
    uint32_t mul_pair_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t pair_id, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(pair_id);
        return interner.intern_pair(var, mul(arena, interner, ops[0], rhs), mul(arena, interner, ops[1], rhs));
    }

    template <class T>
    uint32_t mul_rot_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t rot, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> pairs = arena.operands(rot);
        std::vector<uint32_t> new_pairs(pairs.begin(), pairs.end());
        new_pairs[0] = mul_pair_leaf(arena, interner, pairs[0], rhs, var);
        return interner.intern_node(cr::Kind::Rot, var, new_pairs);
    }

    template <class T>
    uint32_t mul_rot_prod(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t rot, uint32_t prod, uint8_t var) {
        const std::span<const uint32_t> pairs = arena.operands(rot);
        const std::span<const uint32_t> scale = arena.operands(prod);

        const std::size_t newlength = std::max(pairs.size(), scale.size());
        const uint32_t zero_id = interner.intern_leaf(0, cr::Field<T>::zero());
        const uint32_t one_id  = interner.intern_leaf(0, cr::Field<T>::one());
        const uint32_t identity_pair = interner.intern_pair(var, zero_id, one_id);

        std::vector<uint32_t> new_pairs(newlength);
        for (std::size_t i = 0; i < newlength; ++i) {
            const uint32_t pair_i  = i < pairs.size() ? pairs[i] : identity_pair;
            const uint32_t scale_i = i < scale.size() ? scale[i] : one_id;
            new_pairs[i] = mul_pair_leaf(arena, interner, pair_i, scale_i, var);
        }
        return interner.intern_node(cr::Kind::Rot, var, new_pairs);
    }


    template <class T>
    uint32_t mul_trig_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const cr::Kind trig_kind = arena.node(lhs).kind;
        const uint32_t rot = arena.operands(lhs)[0];
        const uint32_t new_rot = mul_rot_leaf(arena, interner, rot, rhs, var);
        const std::array<uint32_t, 1> ops{new_rot};
        return interner.intern_node(trig_kind, var, ops);
    }

    template <class T>
    uint32_t mul_trig_prod(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const cr::Kind trig_kind = arena.node(lhs).kind;
        const uint32_t rot = arena.operands(lhs)[0];
        const uint32_t new_rot = mul_rot_prod(arena, interner, rot, rhs, var);
        const std::array<uint32_t, 1> ops{new_rot};
        return interner.intern_node(trig_kind, var, ops);
    }

    template <class T>
    uint32_t mul_default(const cr::Arena<T>&, cr::Interner<T>& interner, uint32_t x, uint32_t y, uint8_t var) {
        const std::array<uint32_t, 2> ops{x, y};
        return interner.intern_node(cr::Kind::EMul, var, ops);
    }

    template <class T>
    uint32_t mul(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs) {
        const uint8_t var = std::max(arena.node(lhs).var, arena.node(rhs).var);
        cr::Kind lk = dispatch_kind(arena, lhs, var);
        cr::Kind rk = dispatch_kind(arena, rhs, var);
        if (lk == cr::Kind::Leaf && rk != cr::Kind::Leaf) {
            std::swap(lhs, rhs);
            std::swap(lk, rk);
        }

        switch (pair(lk, rk)) {
            case pair(cr::Kind::Leaf, cr::Kind::Leaf): return mul_leaf_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sum, cr::Kind::Leaf): return mul_sum_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sum, cr::Kind::Sum):  return mul_sum_sum(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Prod, cr::Kind::Leaf): return mul_prod_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Prod, cr::Kind::Prod): return mul_prod_prod(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Pair, cr::Kind::Leaf): return mul_pair_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Rot, cr::Kind::Leaf): return mul_rot_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Rot, cr::Kind::Prod): return mul_rot_prod(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sin, cr::Kind::Leaf):
            case pair(cr::Kind::Cos, cr::Kind::Leaf): return mul_trig_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sin, cr::Kind::Prod):
            case pair(cr::Kind::Cos, cr::Kind::Prod): return mul_trig_prod(arena, interner, lhs, rhs, var);
        }
        return mul_default(arena, interner, lhs, rhs, var);
    }

}
