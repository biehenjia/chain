#pragma once
#include "add.hpp"
#include "binomial.hpp"
#include <algorithm>
#include <array>
#include <cr/domain/field.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>
namespace cr::algebra {
template <class T>
uint32_t mul(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs);
template <class T>
uint32_t mul_leaf_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const T prod = cr::Field<T>::mul(interner.value(interner.node(lhs).ops), interner.value(interner.node(rhs).ops));
    return interner.intern_leaf(var, prod);
}
template <class T>
uint32_t mul_sum_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const std::vector<uint32_t> ops(interner.operands(lhs).begin(), interner.operands(lhs).end());
    std::vector<uint32_t> out(ops.size());
    for (std::size_t i = 0; i < ops.size(); ++i)
        out[i] = mul(interner, ops[i], rhs);
    return interner.intern_node(cr::Kind::Sum, var, out);
}
template <class T>
uint32_t mul_sum_sum(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    std::vector<uint32_t> lops(interner.operands(lhs).begin(), interner.operands(lhs).end());
    std::vector<uint32_t> rops(interner.operands(rhs).begin(), interner.operands(rhs).end());
    if (rops.size() > lops.size())
        std::swap(lops, rops);
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
                const uint32_t coeff = interner.intern_leaf(0, cr::field_from_uint<T>(binomial(j, i - k)));
                r2 = add(interner, r2, mul(interner, coeff, rops[k]));
            }
            const uint32_t outer_coeff = interner.intern_leaf(0, cr::field_from_uint<T>(binomial(i, j)));
            r2 = mul(interner, r2, outer_coeff);
            r1 = add(interner, r1, mul(interner, lops[j], r2));
        }
        out[i] = r1;
    }
    return interner.intern_node(cr::Kind::Sum, var, out);
}
template <class T>
uint32_t mul_prod_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const std::vector<uint32_t> ops(interner.operands(lhs).begin(), interner.operands(lhs).end());
    std::vector<uint32_t> out(ops.begin(), ops.end());
    out[0] = mul(interner, out[0], rhs);
    return interner.intern_node(cr::Kind::Prod, var, out);
}
template <class T>
uint32_t mul_prod_prod(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    std::vector<uint32_t> lops(interner.operands(lhs).begin(), interner.operands(lhs).end());
    std::vector<uint32_t> rops(interner.operands(rhs).begin(), interner.operands(rhs).end());
    if (rops.size() > lops.size())
        std::swap(lops, rops);
    std::vector<uint32_t> out(lops.size());
    for (std::size_t i = 0; i < lops.size(); ++i)
        out[i] = i < rops.size() ? mul(interner, lops[i], rops[i]) : lops[i];
    return interner.intern_node(cr::Kind::Prod, var, out);
}
template <class T>
uint32_t mul_pair_leaf(cr::Interner<T>& interner, uint32_t pair_id, uint32_t rhs, uint8_t var) {
    const std::vector<uint32_t> ops(interner.operands(pair_id).begin(), interner.operands(pair_id).end());
    return interner.intern_pair(var, mul(interner, ops[0], rhs), mul(interner, ops[1], rhs));
}
template <class T>
uint32_t mul_rot_leaf(cr::Interner<T>& interner, uint32_t rot, uint32_t rhs, uint8_t var) {
    const std::vector<uint32_t> pairs(interner.operands(rot).begin(), interner.operands(rot).end());
    std::vector<uint32_t> new_pairs(pairs.begin(), pairs.end());
    new_pairs[0] = mul_pair_leaf(interner, pairs[0], rhs, var);
    return interner.intern_node(cr::Kind::Rot, var, new_pairs);
}
template <class T>
uint32_t mul_rot_prod(cr::Interner<T>& interner, uint32_t rot, uint32_t prod, uint8_t var) {
    const std::vector<uint32_t> pairs(interner.operands(rot).begin(), interner.operands(rot).end());
    const std::vector<uint32_t> scale(interner.operands(prod).begin(), interner.operands(prod).end());
    const std::size_t newlength = std::max(pairs.size(), scale.size());
    const uint32_t zero_id = interner.intern_leaf(0, cr::Field<T>::zero());
    const uint32_t one_id = interner.intern_leaf(0, cr::Field<T>::one());
    const uint32_t identity_pair = interner.intern_pair(var, zero_id, one_id);
    std::vector<uint32_t> new_pairs(newlength);
    for (std::size_t i = 0; i < newlength; ++i) {
        const uint32_t pair_i = i < pairs.size() ? pairs[i] : identity_pair;
        const uint32_t scale_i = i < scale.size() ? scale[i] : one_id;
        new_pairs[i] = mul_pair_leaf(interner, pair_i, scale_i, var);
    }
    return interner.intern_node(cr::Kind::Rot, var, new_pairs);
}
template <class T>
uint32_t mul_trig_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const cr::Kind trig_kind = interner.node(lhs).kind;
    const uint32_t rot = interner.operands(lhs)[0];
    const uint32_t new_rot = mul_rot_leaf(interner, rot, rhs, var);
    const std::array<uint32_t, 1> ops{new_rot};
    return interner.intern_node(trig_kind, var, ops);
}
template <class T>
uint32_t mul_trig_prod(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const cr::Kind trig_kind = interner.node(lhs).kind;
    const uint32_t rot = interner.operands(lhs)[0];
    const uint32_t new_rot = mul_rot_prod(interner, rot, rhs, var);
    const std::array<uint32_t, 1> ops{new_rot};
    return interner.intern_node(trig_kind, var, ops);
}
template <class T>
uint32_t mul_default(cr::Interner<T>& interner, uint32_t x, uint32_t y, uint8_t var) {
    const std::array<uint32_t, 2> ops{x, y};
    return interner.intern_node(cr::Kind::EMul, var, ops);
}
template <class T>
uint32_t mul(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs) {
    const uint8_t var = std::max(interner.node(lhs).var, interner.node(rhs).var);
    cr::Kind lk = dispatch_kind(interner, lhs, var);
    cr::Kind rk = dispatch_kind(interner, rhs, var);
    if (lk == cr::Kind::Leaf && rk != cr::Kind::Leaf) {
        std::swap(lhs, rhs);
        std::swap(lk, rk);
    }
    switch (pair(lk, rk)) {
        case pair(cr::Kind::Leaf, cr::Kind::Leaf):
            return mul_leaf_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Sum, cr::Kind::Leaf):
            return mul_sum_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Sum, cr::Kind::Sum):
            return mul_sum_sum(interner, lhs, rhs, var);
        case pair(cr::Kind::Prod, cr::Kind::Leaf):
            return mul_prod_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Prod, cr::Kind::Prod):
            return mul_prod_prod(interner, lhs, rhs, var);
        case pair(cr::Kind::Pair, cr::Kind::Leaf):
            return mul_pair_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Rot, cr::Kind::Leaf):
            return mul_rot_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Prod, cr::Kind::Rot):
            return mul_rot_prod(interner, rhs, lhs, var);
        case pair(cr::Kind::Rot, cr::Kind::Prod):
            return mul_rot_prod(interner, lhs, rhs, var);
        case pair(cr::Kind::Sin, cr::Kind::Leaf):
        case pair(cr::Kind::Cos, cr::Kind::Leaf):
            return mul_trig_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Prod, cr::Kind::Sin):
        case pair(cr::Kind::Prod, cr::Kind::Cos):
            return mul_trig_prod(interner, rhs, lhs, var);
        case pair(cr::Kind::Sin, cr::Kind::Prod):
        case pair(cr::Kind::Cos, cr::Kind::Prod):
            return mul_trig_prod(interner, lhs, rhs, var);
    }
    return mul_default(interner, lhs, rhs, var);
}
} // namespace cr::algebra
