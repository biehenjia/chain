#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include <core/core.hpp>
#include <hashing/hashing.hpp>

namespace chains {

    template<class LeafT> using BinaryRule = uint32_t (*)(Interner<LeafT>&, uint32_t, uint32_t);
    template<class LeafT> using UnaryRule  = uint32_t (*)(Interner<LeafT>&, uint32_t);

    template<class LeafT>
    struct BinaryEntry {
        Kind lk;
        Kind rk;
        BinaryRule<LeafT> fn;
        bool commutative;
    };

    template<class LeafT>
    struct UnaryEntry {
        Kind k;
        UnaryRule<LeafT> fn;
    };

    template<class LeafT>
    inline uint8_t pick_var(const Interner<LeafT>& i, uint32_t x, uint32_t y) {
        return std::max(i.arena().node(x).var, i.arena().node(y).var);
    }

    template<class LeafT>
    inline uint32_t default_binary(Interner<LeafT>& i, Kind k, uint32_t x, uint32_t y) {
        uint32_t ops[] = {x, y};
        return i.intern(k, pick_var(i, x, y), {ops, 2});
    }

    template<class LeafT>
    inline uint32_t default_unary(Interner<LeafT>& i, Kind k, uint32_t x) {
        uint32_t ops[] = {x};
        return i.intern(k, i.arena().node(x).var, {ops, 1});
    }

    template<class LeafT>
    inline void canonicalize_by_var(const Interner<LeafT>& i, uint32_t x, uint32_t y, Kind& lk, Kind& rk) {
        uint8_t lv = i.arena().node(x).var;
        uint8_t rv = i.arena().node(y).var;
        if (lv > rv) rk = Kind::Leaf;
        else if (lv < rv) lk = Kind::Leaf;
    }

    template<class LeafT>
    inline uint32_t dispatch_binary(Interner<LeafT>& i, uint32_t x, uint32_t y, std::span<const BinaryEntry<LeafT>> rules, Kind default_kind) {
        Kind lk = i.arena().node(x).kind;
        Kind rk = i.arena().node(y).kind;
        canonicalize_by_var(i, x, y, lk, rk);

        for (const auto& e : rules) {
            if (e.lk == lk && e.rk == rk) return e.fn(i, x, y);
        }
        for (const auto& e : rules) {
            if (e.commutative && e.lk == rk && e.rk == lk) return e.fn(i, y, x);
        }
        return default_binary(i, default_kind, x, y);
    }

    template<class LeafT>
    inline uint32_t dispatch_unary(Interner<LeafT>& i, uint32_t x, std::span<const UnaryEntry<LeafT>> rules, Kind default_kind) {
        Kind k = i.arena().node(x).kind;
        for (const auto& e : rules) {
            if (e.k == k) return e.fn(i, x);
        }
        return default_unary(i, default_kind, x);
    }

    template<class LeafT, class Mutator>
    inline uint32_t rebuild(Interner<LeafT>& i, uint32_t x, Kind result_kind, Mutator mut) {
        auto ops = i.arena().operands(x);
        std::vector<uint32_t> new_ops(ops.begin(), ops.end());
        mut(new_ops);
        uint8_t var = i.arena().node(x).var;
        return i.intern(result_kind, var, new_ops);
    }

    template<class LeafT, class Op>
    inline uint32_t elementwise_combine(Interner<LeafT>& i, uint32_t x, uint32_t y, Kind result_kind, Op op) {
        if (i.arena().node(y).slot_b > i.arena().node(x).slot_b) std::swap(x, y);
        auto y_ops = i.arena().operands(y);
        return rebuild(i, x, result_kind, [&](std::vector<uint32_t>& ops) {
            for (std::size_t k = 0; k < y_ops.size(); ++k) ops[k] = op(i, ops[k], y_ops[k]);
        });
    }

    template<class LeafT> uint32_t add(Interner<LeafT>& i, uint32_t x, uint32_t y);
    template<class LeafT> uint32_t mul(Interner<LeafT>& i, uint32_t x, uint32_t y);
    template<class LeafT> uint32_t pow(Interner<LeafT>& i, uint32_t x, uint32_t y);
    template<class LeafT> uint32_t log(Interner<LeafT>& i, uint32_t x, uint32_t y);
    template<class LeafT> uint32_t sin(Interner<LeafT>& i, uint32_t x);
    template<class LeafT> uint32_t cos(Interner<LeafT>& i, uint32_t x);
    template<class LeafT> uint32_t tan(Interner<LeafT>& i, uint32_t x);
    template<class LeafT> uint32_t cot(Interner<LeafT>& i, uint32_t x);

}
