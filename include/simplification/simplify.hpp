#pragma once

#include <cstdint>

#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>

#include "prod.hpp"
#include "sum.hpp"
#include "trig.hpp"

namespace cr::simplify {

    template <class T>
    uint32_t simplify(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t id) {
        switch (arena.node(id).kind) {
            case cr::Kind::Sum: return simplify_sum(arena, interner, id);
            case cr::Kind::Prod: return simplify_prod(arena, interner, id);
            case cr::Kind::Sin:
            case cr::Kind::Cos:
            case cr::Kind::Tan:
            case cr::Kind::Cot: return simplify_trig(arena, interner, id);
            default: return id;
        }
    }

}
