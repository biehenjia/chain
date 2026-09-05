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

namespace cr::algebra {

    template <class T>
    uint32_t sub(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs);

    template <class T>
    uint32_t sub_leaf_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const T diff = cr::Field<T>::sub(arena.value(arena.node(lhs).ops), arena.value(arena.node(rhs).ops));
        return interner.intern_leaf(var, diff);
    }

    template <class T>
    uint32_t sub_sum_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(lhs);
        std::vector<uint32_t> out(ops.begin(), ops.end());
        out[0] = sub(arena, interner, out[0], rhs);
        return interner.intern_node(cr::Kind::Sum, var, out);
    }

    template <class T>
    uint32_t sub_leaf_sum(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> ops = arena.operands(rhs);
        const uint32_t zero = interner.intern_leaf(0, cr::Field<T>::zero());

        std::vector<uint32_t> out(ops.size());
        out[0] = sub(arena, interner, lhs, ops[0]);
        for (std::size_t i = 1; i < ops.size(); ++i)
            out[i] = sub(arena, interner, zero, ops[i]);
        return interner.intern_node(cr::Kind::Sum, var, out);
    }

    template <class T>
    uint32_t sub_sum_sum(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
        const std::span<const uint32_t> lops = arena.operands(lhs);
        const std::span<const uint32_t> rops = arena.operands(rhs);
        const std::size_t n = std::max(lops.size(), rops.size());
        const uint32_t zero = interner.intern_leaf(0, cr::Field<T>::zero());

        std::vector<uint32_t> out(n);
        for (std::size_t i = 0; i < n; ++i) {
            if (i < lops.size() && i < rops.size()) out[i] = sub(arena, interner, lops[i], rops[i]);
            else if (i < lops.size()) out[i] = lops[i];
            else out[i] = sub(arena, interner, zero, rops[i]);
        }
        return interner.intern_node(cr::Kind::Sum, var, out);
    }

    template <class T>
    uint32_t sub_default(const cr::Arena<T>&, cr::Interner<T>& interner, uint32_t x, uint32_t y, uint8_t var) {
        const std::array<uint32_t, 2> ops{x, y};
        return interner.intern_node(cr::Kind::ESub, var, ops);
    }

    template <class T>
    uint32_t sub(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs) {
        const uint8_t var = std::max(arena.node(lhs).var, arena.node(rhs).var);
        const cr::Kind lk = dispatch_kind(arena, lhs, var);
        const cr::Kind rk = dispatch_kind(arena, rhs, var);

        switch (pair(lk, rk)) {
            case pair(cr::Kind::Leaf, cr::Kind::Leaf): return sub_leaf_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sum,  cr::Kind::Leaf): return sub_sum_leaf(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Leaf, cr::Kind::Sum): return sub_leaf_sum(arena, interner, lhs, rhs, var);
            case pair(cr::Kind::Sum,  cr::Kind::Sum):  return sub_sum_sum(arena, interner, lhs, rhs, var);
        }
        return sub_default(arena, interner, lhs, rhs, var);
    }

}
