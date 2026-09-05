#pragma once

#include <cstdint>
#include <span>

#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/leaf.hpp>
#include <cr/ir/node.hpp>

namespace cr::simplify {
    
    template <class T>
    uint32_t simplify_prod(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t id) {
        const std::span<const uint32_t> ops = arena.operands(id);

        std::size_t n = ops.size();
        for (std::size_t i = 0; i < ops.size(); ++i) {
            if (cr::is_zero_leaf(arena, ops[i])) { n = i + 1; break; }
        }

        std::size_t j = n - 1;
        while (j > 0 && cr::is_one_leaf(arena, ops[j])) --j;

        if (j == 0) return ops[0];
        if (j == ops.size() - 1) return id;
        return interner.intern_node(cr::Kind::Prod, arena.node(id).var, ops.subspan(0, j + 1));
    }

}
