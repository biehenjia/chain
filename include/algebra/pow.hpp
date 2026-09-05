#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include <cr/domain/field.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>

#include "add.hpp"
#include "mul.hpp"

namespace cr::algebra {
    
    template <class T>
    uint32_t pow(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs);

    template <class T>
    uint32_t pow_default(const cr::Arena<T>&, cr::Interner<T>& interner, uint32_t x, uint32_t y, uint8_t var) {
        const std::array<uint32_t, 2> ops{x, y};
        return interner.intern_node(cr::Kind::EPow, var, ops);
    }

    template <class T>
    uint32_t pow_leaf_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const T v = cr::Field<T>::pow(arena.value(arena.node(lhs).ops), arena.value(arena.node(rhs).ops));
        return interner.intern_leaf(var, v);
    }

    template <class T>
    uint32_t pow_leaf_sum(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(rhs);
        std::vector<uint32_t> out(ops.size());
        for (std::size_t i = 0; i < ops.size(); ++i) out[i] = pow(arena, interner, lhs, ops[i]);
        return interner.intern_node(cr::Kind::Prod, var, out);
    }

    template <class T>
    uint32_t pow_sum_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const auto whole = cr::Field<T>::whole(arena.value(arena.node(rhs).ops));
        if (!whole) return pow_default(arena, interner, lhs, rhs, var);

        uint32_t result = interner.intern_leaf(0, cr::Field<T>::one());
        uint32_t base = lhs;
        for (long v = *whole; v > 0; v >>= 1) {
            if (v & 1) result = mul(arena, interner, result, base);
            if (v > 1) base = mul(arena, interner, base, base);
        }
        return result;
    }

    template <class T>
    uint32_t pow_prod_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(lhs);
        std::vector<uint32_t> out(ops.size());
        for (std::size_t i = 0; i < ops.size(); ++i) out[i] = pow(arena, interner, ops[i], rhs);
        return interner.intern_node(cr::Kind::Prod, var, out);
    }

    template <class T>
    uint32_t pow_prod_sum(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> lops = arena.operands(lhs);
        const std::span<const uint32_t> rops = arena.operands(rhs);

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
                    const uint32_t coeff = interner.intern_leaf(0, field_from_uint<T>(binomial(j, i - k) * binomial(i, j)));
                    const uint32_t base_pow = pow(arena, interner, lops[j], rops[k]);
                    r2 = mul(arena, interner, r2, pow(arena, interner, base_pow, coeff));
                }
                r1 = mul(arena, interner, r1, r2);
            }
            out[i] = r1;
        }
        return interner.intern_node(cr::Kind::Prod, var, out);
    }

    template <class T>
    uint32_t pow(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs) {
        const uint8_t var = std::max(arena.node(lhs).var, arena.node(rhs).var);
        const cr::Kind lk = dispatch_kind(arena, lhs, var);
        const cr::Kind rk = dispatch_kind(arena, rhs, var);

        switch (pair(lk, rk)) {
            case pair(cr::Kind::Leaf, cr::Kind::Leaf): return pow_leaf_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Leaf, cr::Kind::Sum):  return pow_leaf_sum(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sum,  cr::Kind::Leaf): return pow_sum_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Prod, cr::Kind::Leaf): return pow_prod_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Prod, cr::Kind::Sum):  return pow_prod_sum(arena, interner, lhs, rhs, var);
        }
        return pow_default(arena, interner, lhs, rhs, var);
    }

}
