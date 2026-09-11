#pragma once
#include "rot.hpp"
#include <array>
#include <cr/domain/field.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
namespace cr::algebra {
template <class T>
uint32_t sin_leaf(cr::Interner<T>& interner, uint32_t x, uint8_t var) {
    const T v = cr::Field<T>::sin(interner.value(interner.node(x).ops));
    return interner.intern_leaf(var, v);
}
template <class T>
uint32_t sin_sum(cr::Interner<T>& interner, uint32_t u, uint8_t var) {
    return trig_sum(interner, u, var, cr::Kind::Sin);
}
template <class T>
uint32_t sin_default(cr::Interner<T>& interner, uint32_t x, uint8_t var) {
    const std::array<uint32_t, 1> ops{x};
    return interner.intern_node(cr::Kind::ESin, var, ops);
}
template <class T>
uint32_t sin(cr::Interner<T>& interner, uint32_t x) {
    const cr::Kind xk = interner.node(x).kind;
    const uint8_t var = interner.node(x).var;
    switch (xk) {
        case cr::Kind::Leaf:
            return sin_leaf(interner, x, var);
        case cr::Kind::Sum:
            return sin_sum(interner, x, var);
        default:
            break;
    }
    return sin_default(interner, x, var);
}
} // namespace cr::algebra
