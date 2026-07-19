#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include <core/core.hpp>
#include "intern.hpp"

namespace chains {

    template<class LeafT>
    inline std::optional<uint32_t> simplify_chain(Interner<LeafT>& i, Kind k, uint8_t var, std::span<const uint32_t> ops) {
        uint32_t z = i.zero();
        uint32_t o = i.one();

        switch (k) {
            case Kind::Sum: {
                std::size_t n = ops.size();
                while (n > 1 && ops[n - 1] == z) --n;
                if (n == 1 && ops[0] != z) return ops[0];
                if (n == ops.size()) return std::nullopt;
                return i.intern(k, var, ops.first(n));
            }
            case Kind::Prod: {
                std::size_t n = ops.size();
                while (n > 1 && ops[n - 1] == o) --n;
                if (n == 1 && ops[0] != z) return ops[0];
                if (n == ops.size()) return std::nullopt;
                return i.intern(k, var, ops.first(n));
            }
            case Kind::Sin:
            case Kind::Cos:
            case Kind::Tan:
            case Kind::Cot: {
                std::size_t n = ops.size() / 2;
                std::size_t new_n = n;
                while (new_n > 0 && ops[new_n - 1] == z && ops[new_n - 1 + n] == o) --new_n;
                if (new_n == n) return std::nullopt;
                if (new_n == 0) return std::nullopt;
                std::vector<uint32_t> new_ops(2 * new_n);
                for (std::size_t j = 0; j < new_n; ++j) {
                    new_ops[j]         = ops[j];
                    new_ops[j + new_n] = ops[j + n];
                }
                return i.intern(k, var, new_ops);
            }
            default:
                return std::nullopt;
        }
    }

    template<class LeafT>
    inline std::optional<uint32_t> simplify_algebraic(Interner<LeafT>& i, Kind k, uint8_t var, std::span<const uint32_t> ops) {
        uint32_t z = i.zero();
        uint32_t o = i.one();

        switch (k) {
            case Kind::EAdd: {
                std::vector<uint32_t> new_ops;
                new_ops.reserve(ops.size());
                for (uint32_t id : ops) if (id != z) new_ops.push_back(id);
                if (new_ops.size() == ops.size()) return std::nullopt;
                if (new_ops.empty()) return z;
                if (new_ops.size() == 1) return new_ops[0];
                return i.intern(k, var, new_ops);
            }
            case Kind::EMul: {
                for (uint32_t id : ops) if (id == z) return z;
                std::vector<uint32_t> new_ops;
                new_ops.reserve(ops.size());
                for (uint32_t id : ops) if (id != o) new_ops.push_back(id);
                if (new_ops.size() == ops.size()) return std::nullopt;
                if (new_ops.empty()) return o;
                if (new_ops.size() == 1) return new_ops[0];
                return i.intern(k, var, new_ops);
            }
            case Kind::EPow: {
                if (ops.size() == 2) {
                    if (ops[1] == z) return o;
                    if (ops[1] == o) return ops[0];
                }
                return std::nullopt;
            }
            default:
                return std::nullopt;
        }
    }

}
