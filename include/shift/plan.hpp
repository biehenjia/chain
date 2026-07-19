#pragma once

#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <utility>
#include <vector>

#include <core/core.hpp>


namespace chains {
    enum class OpKind : uint8_t { Sum, Prod, Trig, Fetch };
    struct Op {
        OpKind op;
        Kind src_kind; 
        uint32_t start; 
        uint32_t length;
        uint32_t src;
    };

    struct ResetProgram {
        std::vector<std::pair<uint32_t, uint32_t>> baseline_ranges;
        std::vector<Op> fetches;
    };

    struct ShiftPlan {
        std::vector<double> tape;
        std::vector<double> baseline;
        std::vector<uint32_t> node_to_offset;
        std::vector<std::vector<Op>> programs;
        std::vector<ResetProgram> reset_programs;
        std::vector<std::vector<Op>> refresh_programs;
        Reader root_reader = &read_zero;
        uint32_t root_offset = 0;
        uint32_t root_length = 0;
    };
    inline constexpr uint32_t kNoOffset = ~uint32_t(0);

}

namespace chains::plan_detail {
    template<class LeafT>
    inline void postorder_visit(const Arena<LeafT>& a, uint32_t id, std::unordered_set<uint32_t>& seen, ShiftPlan& p) {
        if (!seen.insert(id).second) return;

        const Node& n = a.node(id);
        if (is_connector_kind(n.kind)) {
            postorder_visit(a, n.slot_a, seen, p);
            return;
        }
        if (!is_leaf_kind(n.kind)) {
            for (uint32_t child : a.operands(id)) postorder_visit(a, child, seen, p);
        }

        if (!is_chain_kind(n.kind)) return;

        uint32_t offset = uint32_t(p.tape.size());
        uint32_t length = n.slot_b;
        p.tape.resize(offset + length);
        p.node_to_offset[id] = offset;


        OpKind chain_op = is_trig_kind(n.kind) ? OpKind::Trig : (n.kind == Kind::Prod ? OpKind::Prod : OpKind::Sum);
        uint8_t axis = n.var;
        if (axis >= p.programs.size()) p.programs.resize(axis + 1);
        if (axis >= p.reset_programs.size()) p.reset_programs.resize(axis + 1);
        if (axis >= p.refresh_programs.size()) p.refresh_programs.resize(axis + 1);
        p.programs[axis].push_back({.op = chain_op, .src_kind = Kind::Leaf, .start = offset, .length = length, .src = 0});
        p.reset_programs[axis].baseline_ranges.emplace_back(offset, length);
        auto ops = a.operands(id);
        for (uint32_t k = 0; k < ops.size(); ++k) {
            const Node& child = a.node(ops[k]);
            if (is_connector_kind(child.kind)) {
                Op fetch{OpKind::Fetch, Kind::Sum, offset + k, 1, p.node_to_offset[child.slot_a] + child.slot_b};
                p.reset_programs[axis].fetches.push_back(fetch);
                p.refresh_programs[axis].push_back(fetch);
                continue;
            }
            if (!is_chain_kind(child.kind)) continue;
            Op fetch{OpKind::Fetch, child.kind, offset + k, child.slot_b, p.node_to_offset[ops[k]]};
            p.reset_programs[axis].fetches.push_back(fetch);
            if (child.var == n.var) p.refresh_programs[axis].push_back(fetch);
        }
    }

    inline void coalesce_baseline_ranges(ResetProgram& rp) {
        auto& r = rp.baseline_ranges;
        if (r.empty()) return;
        std::sort(r.begin(), r.end());
        std::size_t w = 0;
        for (std::size_t i = 1; i < r.size(); ++i) {
            auto& cur = r[w];
            if (r[i].first == cur.first + cur.second) {
                cur.second += r[i].second;
            } else {
                r[++w] = r[i];
            }
        }
        r.resize(w + 1);
    }

}

namespace chains {

    template<class LeafT>
    inline ShiftPlan build_shift_plan(const Arena<LeafT>& a, uint32_t root) {
        ShiftPlan p;
        p.node_to_offset.assign(a.num_nodes(), kNoOffset);

        std::unordered_set<uint32_t> seen;
        plan_detail::postorder_visit(a, root, seen, p);
        p.baseline.resize(p.tape.size());
        for (ResetProgram& rp : p.reset_programs) plan_detail::coalesce_baseline_ranges(rp);

        const Node& root_node = a.node(root);
        p.root_reader = reader_for(root_node.kind);
        p.root_offset = is_chain_kind(root_node.kind) ? p.node_to_offset[root] : 0;
        p.root_length = is_chain_kind(root_node.kind) ? root_node.slot_b : 0;
        return p;
    }

    inline void snapshot(ShiftPlan& p) {
        p.baseline = p.tape;
    }

}
