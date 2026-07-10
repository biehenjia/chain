#include <chains/arena.hpp>

namespace chains {

void Arena::collect_live(uint32_t id, std::unordered_set<uint32_t>& live, std::vector<uint32_t>& topo) const {
    if (!live.insert(id).second) return;
    const Node& n = nodes_[id];
    if (n.kind != Kind::Leaf) {
        for (uint32_t i = 0; i < n.slot_b; ++i) {
            collect_live(operands_[n.slot_a + i], live, topo);
        }
    }
    topo.push_back(id);
}

std::pair<Arena, uint32_t> Arena::compact(uint32_t root) const {
    std::unordered_set<uint32_t> live;
    std::vector<uint32_t> topo;
    topo.reserve(nodes_.size());
    collect_live(root, live, topo);

    Arena out;
    out.symbolics_       = symbolics_;
    out.symbolic_intern_ = symbolic_intern_;

    std::unordered_map<uint32_t, uint32_t> remap;
    remap.reserve(topo.size());

    for (uint32_t old_id : topo) {
        const Node& src = nodes_[old_id];
        Node n = src;
        if (src.kind != Kind::Leaf) {
            n.slot_a = uint32_t(out.operands_.size());
            for (uint32_t i = 0; i < src.slot_b; ++i) {
                out.operands_.push_back(remap.at(operands_[src.slot_a + i]));
                out.suffix_hashes_.push_back(suffix_hashes_[src.slot_a + i]);
            }
        }
        uint32_t new_id = uint32_t(out.nodes_.size());
        out.nodes_.push_back(n);
        out.intern_.emplace(n.hash, new_id);
        remap.emplace(old_id, new_id);
    }

    if (zero_leaf_id_ && live.count(*zero_leaf_id_)) out.zero_leaf_id_ = remap[*zero_leaf_id_];
    if (one_leaf_id_  && live.count(*one_leaf_id_)) out.one_leaf_id_  = remap[*one_leaf_id_];

    return {std::move(out), remap.at(root)};
}

}
