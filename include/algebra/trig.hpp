#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <core/core.hpp>
#include <hashing/hashing.hpp>
#include "router.hpp"

namespace chains {

    template<class LeafT> uint32_t sin_leaf(Interner<LeafT>& i, uint32_t x) { return i.intern(apply_sin(i.arena().leaf(x))); }
    template<class LeafT> uint32_t cos_leaf(Interner<LeafT>& i, uint32_t x) { return i.intern(apply_cos(i.arena().leaf(x))); }
    template<class LeafT> uint32_t tan_leaf(Interner<LeafT>& i, uint32_t x) { return i.intern(apply_tan(i.arena().leaf(x))); }
    template<class LeafT> uint32_t cot_leaf(Interner<LeafT>& i, uint32_t x) { return i.intern(apply_cot(i.arena().leaf(x))); }

    template<class LeafT>
    uint32_t trig_sum(Interner<LeafT>& i, uint32_t x, Kind trig_kind) {
        auto ops_span = i.arena().operands(x);
        std::vector<uint32_t> ops(ops_span.begin(), ops_span.end());
        std::size_t n = ops.size();
        std::vector<uint32_t> new_ops(2 * n);
        for (std::size_t k = 0; k < n; ++k) {
            new_ops[k] = sin(i, ops[k]);
            new_ops[k + n] = cos(i, ops[k]);
        }
        uint8_t var = i.arena().node(x).var;
        return i.intern(trig_kind, var, new_ops);
    }

    template<class LeafT> uint32_t sin_sum(Interner<LeafT>& i, uint32_t x) { return trig_sum(i, x, Kind::Sin); }
    template<class LeafT> uint32_t cos_sum(Interner<LeafT>& i, uint32_t x) { return trig_sum(i, x, Kind::Cos); }
    template<class LeafT> uint32_t tan_sum(Interner<LeafT>& i, uint32_t x) { return trig_sum(i, x, Kind::Tan); }
    template<class LeafT> uint32_t cot_sum(Interner<LeafT>& i, uint32_t x) { return trig_sum(i, x, Kind::Cot); }

}
