#pragma once
#include <array>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
#include <span>
#include <vector>
namespace cr::algebra {
template <class T>
uint32_t sin(cr::Interner<T>& interner, uint32_t x);
template <class T>
uint32_t cos(cr::Interner<T>& interner, uint32_t x);
template <class T>
uint32_t trig_rot(cr::Interner<T>& interner, uint32_t u, uint8_t var) {
    const auto operands = interner.operands(u);
    // Creating pairs can reallocate the arena's operand storage.
    const std::vector<uint32_t> ops(operands.begin(), operands.end());
    std::vector<uint32_t> pairs(ops.size());
    for (std::size_t i = 0; i < ops.size(); ++i) {
        const uint32_t s = sin(interner, ops[i]);
        const uint32_t c = cos(interner, ops[i]);
        pairs[i] = interner.intern_pair(var, s, c);
    }
    return interner.intern_node(cr::Kind::Rot, var, pairs);
}
template <class T>
uint32_t trig_sum(cr::Interner<T>& interner, uint32_t u, uint8_t var, cr::Kind target) {
    const uint32_t rot = trig_rot(interner, u, var);
    const std::array<uint32_t, 1> ops{rot};
    return interner.intern_node(target, var, ops);
}
} // namespace cr::algebra
