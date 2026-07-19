#pragma once

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include "node.hpp"

namespace chains {

    template<class LeafT>
    class Arena {
        public:
            uint32_t append(Node n, std::span<const uint32_t> ops) {
                n.slot_a = uint32_t(operands_.size());
                n.slot_b = uint32_t(ops.size());
                operands_.insert(operands_.end(), ops.begin(), ops.end());
                uint32_t id = uint32_t(nodes_.size());
                nodes_.push_back(n);
                return id;
            }

            uint32_t append(Node n) {
                uint32_t id = uint32_t(nodes_.size());
                nodes_.push_back(n);
                return id;
            }

            uint32_t append(Node n, LeafT v) {
                n.slot_a = uint32_t(leaves_.size());
                n.slot_b = 0;
                leaves_.push_back(std::move(v));
                uint32_t id = uint32_t(nodes_.size());
                nodes_.push_back(n);
                return id;
            }

            const Node& node(uint32_t id) const { return nodes_[id]; }
            std::span<const uint32_t> operands(uint32_t id) const {
                const Node& n = nodes_[id];
                return {operands_.data() + n.slot_a, n.slot_b};
            }

            const LeafT& leaf(uint32_t id) const { return leaves_[nodes_[id].slot_a]; }
            uint32_t num_nodes() const { return uint32_t(nodes_.size()); }
            uint32_t num_operands() const { return uint32_t(operands_.size()); }
            uint32_t num_leaves() const { return uint32_t(leaves_.size()); }

        private:
            std::vector<Node> nodes_;
            std::vector<uint32_t> operands_;
            std::vector<LeafT> leaves_;
    };

}
