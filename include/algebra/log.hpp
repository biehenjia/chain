#pragma once
#include "dispatch.hpp"
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
uint32_t log(cr::Interner<T>& interner, uint32_t x, uint32_t base);
template <class T>
uint32_t log_leaf_leaf(cr::Interner<T>& interner, uint32_t x, uint32_t base, uint8_t var) {
    const T v = cr::Field<T>::div(cr::Field<T>::log(interner.value(interner.node(x).ops)),
                                  cr::Field<T>::log(interner.value(interner.node(base).ops)));
    return interner.intern_leaf(var, v);
}
template <class T>
uint32_t log_prod_leaf(cr::Interner<T>& interner, uint32_t x, uint32_t base, uint8_t var) {
    const std::span<const uint32_t> ops = interner.operands(x);
    std::vector<uint32_t> out(ops.size());
    for (std::size_t i = 0; i < ops.size(); ++i)
        out[i] = log(interner, ops[i], base);
    return interner.intern_node(cr::Kind::Sum, var, out);
}
template <class T>
uint32_t log_default(cr::Interner<T>& interner, uint32_t x, uint32_t base, uint8_t var) {
    const std::array<uint32_t, 2> ops{x, base};
    return interner.intern_node(cr::Kind::ELog, var, ops);
}
template <class T>
uint32_t log(cr::Interner<T>& interner, uint32_t x, uint32_t base) {
    const uint8_t var = std::max(interner.node(x).var, interner.node(base).var);
    const cr::Kind xk = dispatch_kind(interner, x, var);
    const cr::Kind bk = dispatch_kind(interner, base, var);
    switch (pair(xk, bk)) {
        case pair(cr::Kind::Leaf, cr::Kind::Leaf):
            return log_leaf_leaf(interner, x, base, var);
        case pair(cr::Kind::Prod, cr::Kind::Leaf):
            return log_prod_leaf(interner, x, base, var);
    }
    return log_default(interner, x, base, var);
}
} // namespace cr::algebra
