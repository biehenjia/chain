#pragma once

#include <array>
#include <cstdint>
#include <span>

#include <cr/domain/field.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/leaf.hpp>
#include <cr/ir/node.hpp>

namespace cr::simplify {

    template <class T>
    bool is_identity_pair(const cr::Arena<T>& arena, uint32_t pair_id) {
        const std::span<const uint32_t> ops = arena.operands(pair_id);
        return cr::is_zero_leaf(arena, ops[0]) && cr::is_one_leaf(arena, ops[1]);
    }

    template <class T>
    uint32_t simplify_trig(const cr::Arena<T>& arena, cr::Interner<T>& interner, uint32_t id) {
        const uint32_t rot = arena.operands(id)[0];
        const std::span<const uint32_t> pairs = arena.operands(rot);
        const std::span<const uint32_t> pair0 = arena.operands(pairs[0]);

        if (cr::is_zero_leaf(arena, pair0[0]) && cr::is_zero_leaf(arena, pair0[1]))
            return interner.intern_leaf(0, cr::Field<T>::zero());

        std::size_t j = pairs.size() - 1;
        while (j > 0 && is_identity_pair(arena, pairs[j])) --j;
        if (j == pairs.size() - 1) return id;

        const uint32_t trimmed_rot = interner.intern_node(cr::Kind::Rot, arena.node(rot).var, pairs.subspan(0, j + 1));
        const std::array<uint32_t, 1> trig_ops{trimmed_rot};
        return interner.intern_node(arena.node(id).kind, arena.node(id).var, trig_ops);
    }

}
