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
#include <cr/ir/leaf.hpp>
#include <cr/ir/node.hpp>

namespace cr::algebra {

    using cr::is_zero_leaf;
    using cr::is_one_leaf;

    inline constexpr uint16_t pair(cr::Kind a, cr::Kind b) {
        return uint16_t(uint16_t(a) << 8 | uint16_t(b));
    }

    template <class T>
    cr::Kind dispatch_kind(const cr::Arena<T>& arena, uint32_t id, uint8_t v) {
        const cr::Node& n = arena.node(id);
        return n.var < v ? cr::Kind::Leaf : n.kind;
    }

    template <class T>
    T field_from_uint(std::uint64_t n) {
        T result = cr::Field<T>::zero();
        T base = cr::Field<T>::one();
        while (n) {
            if (n & 1) result = cr::Field<T>::add(result, base);
            base = cr::Field<T>::add(base, base);
            n >>= 1;
        }
        return result;
    }

    inline std::uint64_t binomial(std::uint64_t n, std::uint64_t k) {
        if (k > n) return 0;
        k = std::min(k, n - k);
        std::uint64_t result = 1;
        for (std::uint64_t i = 0; i < k; ++i) result = result * (n - i) / (i + 1);
        return result;
    }

    template <class T>
    uint32_t add(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs);

    template <class T>
    uint32_t add_leaf_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const T sum = cr::Field<T>::add(arena.value(arena.node(lhs).ops), arena.value(arena.node(rhs).ops));
        return interner.intern_leaf(var, sum);
    }

    template <class T>
    uint32_t add_sum_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(lhs);
        std::vector<uint32_t> out(ops.begin(), ops.end());
        out[0] = add(arena, interner, out[0], rhs);
        return interner.intern_node(cr::Kind::Sum, var, out);
    }


    template <class T>
    uint32_t add_sum_sum(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        std::span<const uint32_t> lops = arena.operands(lhs);
        std::span<const uint32_t> rops = arena.operands(rhs);
        if (rops.size() > lops.size()) std::swap(lops, rops);

        std::vector<uint32_t> out(lops.size());
        for (std::size_t i = 0; i < lops.size(); ++i)
            out[i] = i < rops.size() ? add(arena, interner, lops[i], rops[i]) : lops[i];
        return interner.intern_node(cr::Kind::Sum, var, out);
    }

    template <class T>
    uint32_t add_default(const cr::Arena<T>&, cr::Interner<T>& interner, uint32_t x, uint32_t y, uint8_t var) {
        const std::array<uint32_t, 2> ops{x, y};
        return interner.intern_node(cr::Kind::EAdd, var, ops);
    }

    template <class T>
    uint32_t add(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs) {
        const uint8_t var = std::max(arena.node(lhs).var, arena.node(rhs).var);
        cr::Kind lk = dispatch_kind(arena, lhs, var);
        cr::Kind rk = dispatch_kind(arena, rhs, var);
        if (lk == cr::Kind::Leaf && rk != cr::Kind::Leaf) {
            std::swap(lhs, rhs);
            std::swap(lk, rk);
        }

        if (rk == cr::Kind::Leaf && is_zero_leaf(arena, rhs)) return lhs;
        if (lk == cr::Kind::Leaf && is_zero_leaf(arena, lhs)) return rhs;

        switch (pair(lk, rk)) {
            case pair(cr::Kind::Leaf, cr::Kind::Leaf): return add_leaf_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sum,  cr::Kind::Leaf): return add_sum_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sum,  cr::Kind::Sum):  return add_sum_sum(arena, interner, lhs, rhs, var);
        }
        return add_default(arena, interner, lhs, rhs, var);
    }

}
