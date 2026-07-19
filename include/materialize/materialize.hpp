#pragma once

#include <cstdint>
#include <unordered_set>

#include <core/core.hpp>
#include <shift/shift.hpp>

namespace chains {

    namespace materialize_detail {

        inline double head_value(const Arena<double>& a, const ShiftPlan& p, uint32_t id) {
            const Node& n = a.node(id);
            if (is_leaf_kind(n.kind)) return a.leaf(id);
            if (is_connector_kind(n.kind)) return p.tape[p.node_to_offset[n.slot_a] + n.slot_b];
            if (is_chain_kind(n.kind)) return access(n.kind, p.tape.data(), p.node_to_offset[id], n.slot_b);

            auto ops = a.operands(id);
            switch (n.kind) {
                case Kind::EAdd: {
                    double r = head_value(a, p, ops[0]);
                    for (std::size_t k = 1; k < ops.size(); ++k) r = combine_add(r, head_value(a, p, ops[k]));
                    return r;
                }
                case Kind::EMul: {
                    double r = head_value(a, p, ops[0]);
                    for (std::size_t k = 1; k < ops.size(); ++k) r = combine_mul(r, head_value(a, p, ops[k]));
                    return r;
                }
                case Kind::EPow: return combine_pow(head_value(a, p, ops[0]), head_value(a, p, ops[1]));
                case Kind::ELog: return combine_log(head_value(a, p, ops[0]), head_value(a, p, ops[1]));
                case Kind::ESin: return apply_sin(head_value(a, p, ops[0]));
                case Kind::ECos: return apply_cos(head_value(a, p, ops[0]));
                case Kind::ETan: return apply_tan(head_value(a, p, ops[0]));
                case Kind::ECot: return apply_cot(head_value(a, p, ops[0]));
                default: return 0.0;
            }
        }

        inline void visit(const Arena<double>& a, ShiftPlan& p, uint32_t id, std::unordered_set<uint32_t>& seen) {
            if (!seen.insert(id).second) return;
            const Node& n = a.node(id);
            if (is_leaf_kind(n.kind)) return;
            if (is_connector_kind(n.kind)) { visit(a, p, n.slot_a, seen); return; }

            auto ops = a.operands(id);
            for (uint32_t child : ops) visit(a, p, child, seen);

            if (!is_chain_kind(n.kind)) return;

            uint32_t off = p.node_to_offset[id];
            for (uint32_t k = 0; k < ops.size(); ++k) {
                p.tape[off + k] = head_value(a, p, ops[k]);
            }
        }

    }

    inline double eval_head(const Arena<double>& a, const ShiftPlan& p, uint32_t id) {
        return materialize_detail::head_value(a, p, id);
    }

    inline void materialize(const Arena<double>& a, ShiftPlan& p, uint32_t root) {
        std::unordered_set<uint32_t> seen;
        materialize_detail::visit(a, p, root, seen);
        snapshot(p);
    }

}
