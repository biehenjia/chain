#pragma once

#include <array>
#include <cstdint>

#include <cr/domain/field.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>

#include "rot.hpp"

namespace cr::algebra {
    
    template <class T>
    uint32_t tan_leaf(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t x, uint8_t var) {
        const T v = cr::Field<T>::tan(arena.value(arena.node(x).ops));
        return interner.intern_leaf(var, v);
    }

    template <class T>
    uint32_t tan_sum(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t u, uint8_t var) {
        return trig_sum(arena, interner, u, var, cr::Kind::Tan);
    }

    template <class T>
    uint32_t tan_default(const cr::Arena<T>&, cr::Interner<T>& interner, uint32_t x, uint8_t var) {
        const std::array<uint32_t, 1> ops{x};
        return interner.intern_node(cr::Kind::ETan, var, ops);
    }

    template <class T>
    uint32_t tan(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t x) {
        const cr::Kind xk = arena.node(x).kind;
        const uint8_t var = arena.node(x).var;

        switch (xk) {
            case cr::Kind::Leaf: return tan_leaf(arena, interner, x, var);
            case cr::Kind::Sum:  return tan_sum(arena, interner, x, var);
            default: break;
        }
        return tan_default(arena, interner, x, var);
    }

}
