#pragma once
#include <vector>
#include <span>
#include <unordered_map>
#include <chains/node.hpp>
#include <chains/hasher.hpp>
#include <symengine/basic.h>

using Symbolic = SymEngine::RCP<const SymEngine::Basic>;

namespace chains {

class Arena {
    public:
        static constexpr uint32_t INVALID = 0xFFFFFFFFu;

        uint32_t push_symbolic(Symbolic s) {
            // compute the symbolic hash
            SymEngine::hash_t h = s->hash(); 
            auto range = symbolic_intern_.equal_range(h);

            // equality check
            for (auto it = range.first; it != range.second; ++it) {
                if (SymEngine::eq(*symbolics_[it->second], *s)) return it->second;
            }

            uint32_t id = uint32_t(symbolics_.size());
            symbolics_.push_back(std::move(s));
            symbolic_intern_.emplace(h, id);
            return id;
        }

        const Symbolic& symbolic(uint32_t id) const {
            return symbolics_[id];
        }

        uint32_t push_leaf(uint8_t var, uint32_t sym_id) {
            uint32_t h = hash_leaf(var, sym_id);
            auto range = intern_.equal_range(h);
            for (auto it = range.first; it != range.second; ++it) {
                const Node& e = nodes_[it->second];
                if (e.kind == Kind::Leaf && e.variable == var && e.slot_a == sym_id) return it->second;
            }
            return commit(Node{Kind::Leaf, var, 0, sym_id, 0, h});
        }

        uint32_t intern_algebraic(Kind k, uint8_t var, std::span<const uint32_t> ops) {
            uint32_t h = hash_algebraic(k, var, child_hashes(ops));
            if (uint32_t id = lookup(k, var, h, ops); id != INVALID) return id;
            uint32_t opstart = append_operands(ops);
            suffix_hashes_.resize(operands_.size(), 0);
            return commit(Node{k, var, 0, opstart, uint32_t(ops.size()), h});
        }

        uint32_t intern_chain(Kind k, uint8_t var, std::span<const uint32_t> ops) {
            std::vector<uint32_t> chs = child_hashes(ops);
            ChainHash ch = hash_chain(k, var, chs);
            if (uint32_t id = lookup(k, var, ch.head, ops); id != INVALID) return id;
            uint32_t opstart = append_operands(ops);
            suffix_hashes_.insert(suffix_hashes_.end(), ch.suffixes.begin(), ch.suffixes.end());
            return commit(Node{k, var, 0, opstart, uint32_t(ops.size()), ch.head});
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

    private:
        uint32_t lookup(Kind k, uint8_t var, uint32_t h, std::span<const uint32_t> ops) const {
            auto range = intern_.equal_range(h);
            for (auto it = range.first; it != range.second; ++it) {
                const Node& n = nodes_[it->second];
                if (n.kind != k || n.variable != var || n.slot_b != ops.size()) continue;
                bool eq = true;
                for (uint32_t i = 0; i < ops.size(); ++i) {
                    if (operands_[n.slot_a + i] != ops[i]) { eq = false; break; }
                }
                if (eq) return it->second;
            }
            return INVALID;
        }

        uint32_t commit(Node n) {
            uint32_t id = uint32_t(nodes_.size());
            nodes_.push_back(n);
            intern_.emplace(n.hash, id);
            return id;
        }

        std::vector<uint32_t> child_hashes(std::span<const uint32_t> ops) const {
            std::vector<uint32_t> out(ops.size());
            for (size_t i = 0; i < ops.size(); ++i) out[i] = nodes_[ops[i]].hash;
            return out;
        }

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
};

}
