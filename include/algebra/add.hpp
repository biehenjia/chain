#pragma once
#include "dispatch.hpp"
#include <algorithm>
#include <array>
#include <cr/domain/field.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/leaf.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>
namespace cr::algebra {
using cr::is_zero_leaf;
template <class T>
uint32_t add(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs);
template <class T>
uint32_t add_leaf_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const T sum = cr::Field<T>::add(interner.value(interner.node(lhs).ops), interner.value(interner.node(rhs).ops));
    return interner.intern_leaf(var, sum);
}
template <class T>
uint32_t add_sum_leaf(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    const std::span<const uint32_t> ops = interner.operands(lhs);
    std::vector<uint32_t> out(ops.begin(), ops.end());
    out[0] = add(interner, out[0], rhs);
    return interner.intern_node(cr::Kind::Sum, var, out);
}
template <class T>
uint32_t add_sum_sum(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs, uint8_t var) {
    std::vector<uint32_t> lops(interner.operands(lhs).begin(), interner.operands(lhs).end());
    std::vector<uint32_t> rops(interner.operands(rhs).begin(), interner.operands(rhs).end());
    if (rops.size() > lops.size())
        std::swap(lops, rops);
    std::vector<uint32_t> out(lops.size());
    for (std::size_t i = 0; i < lops.size(); ++i)
        out[i] = i < rops.size() ? add(interner, lops[i], rops[i]) : lops[i];
    return interner.intern_node(cr::Kind::Sum, var, out);
}
template <class T>
uint32_t add_default(cr::Interner<T>& interner, uint32_t x, uint32_t y, uint8_t var) {
    const std::array<uint32_t, 2> ops{x, y};
    return interner.intern_node(cr::Kind::EAdd, var, ops);
}
template <class T>
uint32_t add(cr::Interner<T>& interner, uint32_t lhs, uint32_t rhs) {
    const uint8_t var = std::max(interner.node(lhs).var, interner.node(rhs).var);
    cr::Kind lk = dispatch_kind(interner, lhs, var);
    cr::Kind rk = dispatch_kind(interner, rhs, var);
    if (lk == cr::Kind::Leaf && rk != cr::Kind::Leaf) {
        std::swap(lhs, rhs);
        std::swap(lk, rk);
    }
    if (rk == cr::Kind::Leaf && is_zero_leaf(interner.arena, rhs))
        return lhs;
    if (lk == cr::Kind::Leaf && is_zero_leaf(interner.arena, lhs))
        return rhs;
    switch (pair(lk, rk)) {
        case pair(cr::Kind::Leaf, cr::Kind::Leaf):
            return add_leaf_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Sum, cr::Kind::Leaf):
            return add_sum_leaf(interner, lhs, rhs, var);
        case pair(cr::Kind::Sum, cr::Kind::Sum):
            return add_sum_sum(interner, lhs, rhs, var);
    }
    return add_default(interner, lhs, rhs, var);
}
} // namespace cr::algebra
