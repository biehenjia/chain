#pragma once
#include "prod.hpp"
#include "sum.hpp"
#include "trig.hpp"
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
namespace cr::simplify {
template <class T>
uint32_t simplify(cr::Interner<T>& interner, uint32_t id) {
    switch (interner.node(id).kind) {
        case cr::Kind::Sum:
            return simplify_sum(interner, id);
        case cr::Kind::Prod:
            return simplify_prod(interner, id);
        case cr::Kind::Sin:
        case cr::Kind::Cos:
        case cr::Kind::Tan:
        case cr::Kind::Cot:
            return simplify_trig(interner, id);
        default:
            return id;
    }
}
} // namespace cr::simplify
