#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

#include <core/core.hpp>
#include "algebraic.hpp"
#include "chain.hpp"

namespace chains {

    template<class LeafT> class Interner;
    template<class LeafT> std::optional<uint32_t> simplify_chain(Interner<LeafT>&, Kind, uint8_t, std::span<const uint32_t>);
    template<class LeafT> std::optional<uint32_t> simplify_algebraic(Interner<LeafT>&, Kind, uint8_t, std::span<const uint32_t>);

    template<class LeafT>
    class Interner {

        public:
            uint32_t intern(LeafT v) {
                uint32_t h = hash_leaf(v);
                auto [begin, end] = table_.equal_range(h);

                for (auto it = begin; it != end; ++it) {
                    const Node& n = arena_.node(it->second);
                    if (n.kind == Kind::Leaf && leaf_equal(arena_.leaf(it->second), v)) {
                        return it->second;
                    }
                }

                Node n{};
                n.kind = Kind::Leaf;
                n.hash = h;

                uint32_t id = arena_.append(n, std::move(v));
                table_.emplace(h, id);
                return id;
            }

            uint32_t intern(Kind k, uint8_t var, std::span<const uint32_t> ops) {
                bool chain = is_chain_kind(k);
                if (chain) {
                    if (auto s = simplify_chain(*this, k, var, ops)) return *s;
                } else if (is_algebraic_kind(k)) {
                    if (auto s = simplify_algebraic(*this, k, var, ops)) return *s;
                }

                uint32_t h;
                std::size_t suffix_start = suffix_hashes_.size();

                if (chain) {
                    suffix_hashes_.resize(suffix_start + ops.size());
                    h = hash_chain(k, var, ops, arena_, std::span<uint32_t>{suffix_hashes_.data() + suffix_start, ops.size()});

                    if (suffix_probe_enabled_ && !is_trig_kind(k) && ops.size() >= 3) {
                        for (std::size_t i = 1; i + 1 < ops.size(); ++i) {
                            uint32_t sh = suffix_hashes_[suffix_start + i];
                            auto range = suffix_index_.equal_range(sh);
                            for (auto it = range.first; it != range.second; ++it) {
                                uint32_t owner = it->second.first;
                                uint32_t off = it->second.second;
                                const Node& on = arena_.node(owner);
                                if (on.kind != k || on.var != var) continue;
                                std::size_t suf_len = ops.size() - i;
                                if (off + suf_len > on.slot_b) continue;
                                auto stored = arena_.operands(owner);
                                bool eq = true;
                                for (std::size_t j = 0; j < suf_len; ++j) {
                                    if (stored[off + j] != ops[i + j]) { eq = false; break; }
                                }
                                if (!eq) continue;
                                suffix_hashes_.resize(suffix_start);
                                uint32_t conn = intern_connector(owner, uint32_t(off));
                                std::vector<uint32_t> new_ops(ops.begin(), ops.begin() + i);
                                new_ops.push_back(conn);
                                return intern(k, var, new_ops);
                            }
                        }
                    }
                } else {
                    h = hash_algebraic(k, var, ops, arena_);
                }

                auto [begin, end] = table_.equal_range(h);
                for (auto it = begin; it != end; ++it) {
                    const Node& n = arena_.node(it->second);
                    if (n.kind != k || n.var != var || n.slot_b != ops.size()) continue;
                    auto stored = arena_.operands(it->second);

                    bool eq = true;
                    for (std::size_t i = 0; i < ops.size(); ++i) {
                        if (stored[i] != ops[i]) { eq = false; break; }
                    }
                    if (eq) {
                        if (chain) suffix_hashes_.resize(suffix_start);
                        return it->second;
                    }
                }

                if (!chain) suffix_hashes_.resize(suffix_start + ops.size());

                Node n{};
                n.kind = k;
                n.var = var;
                n.hash = h;

                uint32_t id = arena_.append(n, ops);
                table_.emplace(h, id);

                if (chain && !is_trig_kind(k) && ops.size() >= 3) {
                    bool has_connector = false;
                    for (uint32_t op : ops) {
                        if (arena_.node(op).kind == Kind::Connector) { has_connector = true; break; }
                    }
                    if (!has_connector) {
                        for (std::size_t i = 1; i + 1 < ops.size(); ++i) {
                            suffix_index_.emplace(suffix_hashes_[suffix_start + i], std::make_pair(id, uint32_t(i)));
                        }
                    }
                }
                return id;
            }

            uint32_t intern_connector(uint32_t origin, uint32_t offset) {
                uint32_t h = hmix(hmix(uint32_t(Kind::Connector), arena_.node(origin).hash), offset);
                auto [begin, end] = table_.equal_range(h);
                for (auto it = begin; it != end; ++it) {
                    const Node& n = arena_.node(it->second);
                    if (n.kind == Kind::Connector && n.slot_a == origin && n.slot_b == offset) {
                        return it->second;
                    }
                }

                Node n{};
                n.kind = Kind::Connector;
                n.slot_a = origin;
                n.slot_b = offset;
                n.hash = h;

                uint32_t id = arena_.append(n);
                table_.emplace(h, id);
                return id;
            }

            const Arena<LeafT>& arena() const { return arena_; }
            Arena<LeafT>& arena() { return arena_; }
            void set_suffix_probe_enabled(bool on) { suffix_probe_enabled_ = on; }
            std::span<const uint32_t> suffix_hashes() const { return suffix_hashes_; }
            uint32_t zero() { if (zero_ == uint32_t(-1)) zero_ = intern(make_int<LeafT>(0)); return zero_; }
            uint32_t one()  { if (one_  == uint32_t(-1)) one_  = intern(make_int<LeafT>(1)); return one_;  }

        private:
            Arena<LeafT> arena_;
            std::unordered_multimap<uint32_t, uint32_t> table_;
            std::unordered_multimap<uint32_t, std::pair<uint32_t, uint32_t>> suffix_index_;
            std::vector<uint32_t> suffix_hashes_;
            uint32_t zero_ = uint32_t(-1);
            uint32_t one_  = uint32_t(-1);
            bool suffix_probe_enabled_ = false;
    };

}
