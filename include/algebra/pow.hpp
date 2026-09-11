#pragma once
#include "add.hpp"
#include "binomial.hpp"
#include "mul.hpp"
#include <algorithm>
#include <array>
#include <cr/domain/field.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
#include <span>
#include <vector>
namespace cr::algebra {
template <class T>
uint32_t pow(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs);
template <class T>
uint32_t pow_default(cr::Interner<T>& interner, uint32_t x, uint32_t y, uint8_t var) {
    const std::array<uint32_t, 2> ops{x, y};
    return interner.intern_node(cr::Kind::EPow, var, ops);
}
template <class T>
uint32_t pow_leaf_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const T v = cr::Field<T>::pow(interner.value(interner.node(lhs).ops), interner.value(interner.node(rhs).ops));
    return interner.intern_leaf(var, v);
}
template <class T>
uint32_t pow_leaf_sum(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const std::vector<uint32_t> ops(interner.operands(rhs).begin(), interner.operands(rhs).end());
    std::vector<uint32_t> out(ops.size());
    for (std::size_t i = 0; i < ops.size(); ++i)
        out[i] = pow(interner, lhs, ops[i]);
    return interner.intern_node(cr::Kind::Prod, var, out);
}
template <class T>
uint32_t pow_sum_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const auto whole = cr::Field<T>::whole(interner.value(interner.node(rhs).ops));
    if (!whole)
        return pow_default(interner, lhs, rhs, var);
    uint32_t result = interner.intern_leaf(0, cr::Field<T>::one());
    uint32_t base = lhs;
    for (long v = *whole; v > 0; v >>= 1) {
        if (v & 1)
            result = mul(interner, result, base);
        if (v > 1)
            base = mul(interner, base, base);
    }
    return result;
}
template <class T>
uint32_t pow_prod_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const std::vector<uint32_t> ops(interner.operands(lhs).begin(), interner.operands(lhs).end());
    std::vector<uint32_t> out(ops.size());
    for (std::size_t i = 0; i < ops.size(); ++i)
        out[i] = pow(interner, ops[i], rhs);
    return interner.intern_node(cr::Kind::Prod, var, out);
}
template <class T>
uint32_t pow_prod_sum(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const std::vector<uint32_t> lops(interner.operands(lhs).begin(), interner.operands(lhs).end());
    const std::vector<uint32_t> rops(interner.operands(rhs).begin(), interner.operands(rhs).end());
    const std::size_t n = lops.size() - 1;
    const std::size_t m = rops.size() - 1;
    const uint32_t one = interner.intern_leaf(0, cr::Field<T>::one());
    std::vector<uint32_t> out(n + m + 1);
    for (std::size_t i = 0; i <= n + m; ++i) {
        uint32_t r1 = one;
        const std::size_t j_lo = i > m ? i - m : 0;
        const std::size_t j_hi = std::min(i, n);
        for (std::size_t j = j_lo; j <= j_hi; ++j) {
            uint32_t r2 = one;
            const std::size_t k_hi = std::min(i, m);
            for (std::size_t k = i - j; k <= k_hi; ++k) {
                const uint32_t coeff =
                    interner.intern_leaf(0, cr::field_from_uint<T>(binomial(j, i - k) * binomial(i, j)));
                const uint32_t base_pow = pow(interner, lops[j], rops[k]);
                r2 = mul(interner, r2, pow(interner, base_pow, coeff));
            }
            r1 = mul(interner, r1, r2);
        }
        out[i] = r1;
    }
    return interner.intern_node(cr::Kind::Prod, var, out);
}
template <class T>
uint32_t pow(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs) {
    const uint8_t var = std::max(interner.node(lhs).var, interner.node(rhs).var);
    const cr::Kind lk = dispatch_kind(interner, lhs, var);
    const cr::Kind rk = dispatch_kind(interner, rhs, var);
    switch (pair(lk, rk)) {
        case pair(cr::Kind::Leaf, cr::Kind::Leaf):
            return pow_leaf_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Leaf, cr::Kind::Sum):
            return pow_leaf_sum(interner, lhs, rhs, var);
        case pair(cr::Kind::Sum, cr::Kind::Leaf):
            return pow_sum_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Prod, cr::Kind::Leaf):
            return pow_prod_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Prod, cr::Kind::Sum):
            return pow_prod_sum(interner, lhs, rhs, var);
    }
    return pow_default(interner, lhs, rhs, var);
}
} // namespace cr::algebra
