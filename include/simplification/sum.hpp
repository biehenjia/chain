#pragma once
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/leaf.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
#include <span>
namespace cr::simplify {
template <class T>
uint32_t simplify_sum(cr::Interner<T>& interner, uint32_t id) {
    const std::span<const uint32_t> ops = interner.operands(id);
    std::size_t j = ops.size() - 1;
    while (j > 0 && cr::is_zero_leaf(interner.arena, ops[j]))
        --j;
    if (j == 0)
        return ops[0];
    if (j == ops.size() - 1)
        return id;
    return interner.intern_node(cr::Kind::Sum, interner.node(id).var, ops.subspan(0, j + 1));
}
} // namespace cr::simplify
