#pragma once
#include <vector>
#include <span>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <chains/node.hpp>
#include <chains/hasher.hpp>
#include <symengine/basic.h>
#include <symengine/integer.h>

using Symbolic = SymEngine::RCP<const SymEngine::Basic>;

namespace chains {

class Arena;

std::optional<uint32_t> simplify_chain(Arena& a, Kind k, uint8_t var, std::span<const uint32_t> ops);
std::optional<uint32_t> simplify_algebraic(Arena& a, Kind k, uint8_t var, std::span<const uint32_t> ops);

class Arena {
    public:
        /**
        attempts to insert a new symbolic object (by construction via CRnum algebraic rules) and returns id to an equivalent of that node
        */
        uint32_t push_symbolic(Symbolic s) {
            // compute symbolic hash at expression level
            SymEngine::hash_t h = s->hash(); 
            auto range = symbolic_intern_.equal_range(h);

            // equality check
            for (auto it = range.first; it != range.second; ++it) {
                if (SymEngine::eq(*symbolics_[it->second], *s)) {
                    return it->second;
                }
            }

            uint32_t id = uint32_t(symbolics_.size());
            symbolics_.push_back(std::move(s));
            symbolic_intern_.emplace(h, id);
            return id;
        }

        const Symbolic& symbolic(uint32_t id) const {
            return symbolics_[id];
        }

        uint32_t push_leaf(uint32_t sym_id) {
            uint32_t h = hash_leaf(sym_id);

            auto range = intern_.equal_range(h);
            for (auto it = range.first; it != range.second; ++it) {
                const Node& e = nodes_[it->second];
                if (e.kind == Kind::Leaf && e.slot_a == sym_id) {
                    return it->second;
                }
            }

            return commit(Node{Kind::Leaf, 0, 0, sym_id, 0, h});
        }

        uint32_t intern_algebraic(Kind k, uint8_t var, std::span<const uint32_t> ops) {
            if (auto s = simplify_algebraic(*this, k, var, ops)) return *s;
            std::vector<uint32_t> chs = child_hashes(ops);
            uint32_t h = hash_algebraic(k, var, chs);

            if (auto id = lookup(k, var, h, ops)) return *id;

            uint32_t opstart = append_operands(ops);
            suffix_hashes_.resize(operands_.size(), 0);
            uint32_t oplen = uint32_t(ops.size());
            auto n = Node{k, var, 0, opstart, oplen, h};
            return commit(n);
        }

        uint32_t intern_chain(Kind k, uint8_t var, std::span<const uint32_t> ops) {
            if (auto s = simplify_chain(*this, k, var, ops)) return *s;
            std::vector<uint32_t> chs = child_hashes(ops);
            ChainHash ch = hash_chain(k, var, chs);

            if (auto id = lookup(k, var, ch.head, ops)) {
                return *id;
            }

            uint32_t opstart = append_operands(ops);
            suffix_hashes_.insert(suffix_hashes_.end(), ch.suffixes.begin(), ch.suffixes.end());
            uint32_t oplen = uint32_t(ops.size());
            auto n = Node{k, var, 0, opstart, oplen, ch.head};
            return commit(n);
        }

        // Interns a Connector node referring to (origin, offset) — used only by prepare.
        // Variable mirrors the origin's variable so shift dispatch sees the right axis.
        uint32_t intern_connector(uint32_t origin, uint32_t offset) {
            uint32_t h = hash_connector(origin, offset);
            auto range = intern_.equal_range(h);
            for (auto it = range.first; it != range.second; ++it) {
                const Node& e = nodes_[it->second];
                if (e.kind == Kind::Connector && e.slot_a == origin && e.slot_b == offset) {
                    return it->second;
                }
            }
            uint8_t var = nodes_[origin].variable;
            return commit(Node{Kind::Connector, var, 0, origin, offset, h});
        }

        const Node& node(uint32_t id) const { return nodes_[id]; }

        std::span<const uint32_t> operands(uint32_t id) const {
            const Node& n = nodes_[id];
            return {operands_.data() + n.slot_a, n.slot_b};
        }

        const Symbolic& leaf_symbolic(uint32_t id) const {
            return symbolics_[nodes_[id].slot_a];
        }

        uint32_t suffix_hash_at(uint32_t op_index) const { return suffix_hashes_[op_index]; }
        uint32_t num_nodes() const { return uint32_t(nodes_.size()); }
        uint32_t operand_count() const { return uint32_t(operands_.size()); }

        // lazy construction of consed zero and one leaves
        std::pair<Arena, uint32_t> compact(uint32_t root) const;

        uint32_t zero_leaf_id() {
            if (!zero_leaf_id_) zero_leaf_id_ = push_leaf(push_symbolic(SymEngine::integer(0)));
            return *zero_leaf_id_;
        }
        uint32_t one_leaf_id() {
            if (!one_leaf_id_) one_leaf_id_ = push_leaf(push_symbolic(SymEngine::integer(1)));
            return *one_leaf_id_;
        }

    private:
        

        // determines if a description of a node exists within the interning table already, returns false on absence
        std::optional<uint32_t> lookup(Kind k, uint8_t var, uint32_t h, std::span<const uint32_t> ops) const {
            auto range = intern_.equal_range(h);
            for (auto it = range.first; it != range.second; ++it) {

                const Node& n = nodes_[it->second];

                if (n.kind != k || n.variable != var || n.slot_b != ops.size()) {
                    continue;
                }
                
                // perform node id checks, by above interning variant that node ids are equal iff they have the same contents
                bool eq = true;
                for (uint32_t i = 0; i < ops.size(); ++i) {
                    if (operands_[n.slot_a + i] != ops[i]) { 
                        eq = false; 
                        break; 
                    }
                }
                if (eq) {
                    return it->second;
                }
            }
            return std::nullopt;
        }

        // insertion protocol for nodes; pointer moves node struct and creates id:node_index key value pairing
        // returns index of inserted node 
        uint32_t commit(Node n) {
            uint32_t id = uint32_t(nodes_.size());
            nodes_.push_back(n);
            intern_.emplace(n.hash, id);
            return id;
        }
        
        // returns mapped hash of a slice
        std::vector<uint32_t> child_hashes(std::span<const uint32_t> ops) const {
            std::vector<uint32_t> out(ops.size());

            for (size_t i = 0; i < ops.size(); ++i) {
                out[i] = nodes_[ops[i]].hash;
            }
            return out;
        }
        
        // Post-order DFS from `root`, appending each reachable node id once. Post-order
        // guarantees children appear before parents in `topo`, so the compact rewrite can
        // remap operand ids in a single pass. Definition in src/arena.cpp.
        void collect_live(uint32_t id, std::unordered_set<uint32_t>& live, std::vector<uint32_t>& topo) const;

        // n-ary operand appending into operands_ vector; returns insertion point
        uint32_t append_operands(std::span<const uint32_t> ops) {
            uint32_t opstart = uint32_t(operands_.size());
            operands_.insert(operands_.end(), ops.begin(), ops.end());
            return opstart;
        }

        std::vector<Node> nodes_;
        std::vector<uint32_t> operands_;
        std::vector<uint32_t> suffix_hashes_; // parallel to operands_
        std::vector<Symbolic> symbolics_;
        std::unordered_multimap<uint32_t, uint32_t> intern_;
        std::unordered_multimap<SymEngine::hash_t, uint32_t> symbolic_intern_;
        std::optional<uint32_t> zero_leaf_id_;
        std::optional<uint32_t> one_leaf_id_;
};

}
