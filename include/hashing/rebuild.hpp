#pragma once

#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include <core/core.hpp>
#include "intern.hpp"

namespace chains {

    namespace rebuild_detail {

        template<class LeafT>
        inline uint32_t visit(const Arena<LeafT>& src,
                              Interner<LeafT>& out,
                              std::unordered_map<uint32_t, uint32_t>& map,
                              uint32_t id) {
            if (auto it = map.find(id); it != map.end()) return it->second;

            const Node& n = src.node(id);
            assert(!is_connector_kind(n.kind) && "rebuild input arena must not contain Connector nodes");

            uint32_t new_id;
            if (is_leaf_kind(n.kind)) {
                new_id = out.intern(src.leaf(id));
            } else {
                auto ops = src.operands(id);
                std::vector<uint32_t> new_ops;
                new_ops.reserve(ops.size());
                for (uint32_t child : ops) new_ops.push_back(visit(src, out, map, child));
                new_id = out.intern(n.kind, n.var, new_ops);
            }

            map.emplace(id, new_id);
            return new_id;
        }

    }

    template<class LeafT>
    inline Interner<LeafT> rebuild_with_consing(const Arena<LeafT>& src,
                                                uint32_t root,
                                                uint32_t& out_root) {
        Interner<LeafT> out;
        out.set_suffix_probe_enabled(true);
        std::unordered_map<uint32_t, uint32_t> map;
        out_root = rebuild_detail::visit(src, out, map, root);
        return out;
    }

}
