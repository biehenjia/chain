#include <chains/simplification.hpp>
#include <chains/arena.hpp>
#include <vector>

namespace chains {

std::optional<uint32_t> simplify_chain(Arena& a, Kind k, uint8_t var, std::span<const uint32_t> ops) {
    const uint32_t z = a.zero_leaf_id();
    const uint32_t o = a.one_leaf_id();

    switch (k) {
        case Kind::Sum: {
            size_t n = ops.size();
            while (n > 1 && ops[n - 1] == z) --n;
            if (n == 1) return ops[0]; // collapses all-zero chains (ops[0]==z) too
            if (n == ops.size()) return std::nullopt;
            return a.intern_chain(k, var, ops.first(n));
        }
        case Kind::Prod: {
            // z cannot appear in the tail if the entire thing is not z itself

            // while [a,b,c,z,e,f,z,z] exists and it will return to be a,b,z,z,e,z,z,z -> a,z,z,z,z,z,z,z -> z,z,z,z,z,z,z,z which isnt entirely z 
            // throughout the entire evaluation, this is actually impossible by construction of exponential functions so i will ignore it!

            for (uint32_t id : ops) if (id == z) return z;
            size_t n = ops.size();
            while (n > 1 && ops[n - 1] == o) --n;
            if (n == 1) return ops[0];
            if (n == ops.size()) return std::nullopt;
            return a.intern_chain(k, var, ops.first(n));
        }
        case Kind::Sin:
        case Kind::Cos:
        case Kind::Tan:
        case Kind::Cot: {
            // consider the trig type CR that is of the form A:= [a,b,z,  c,d,z]. When shifting the second pair of intermediaries is (b * z), (z * d), and (b * z), (z * d), so 
            // we can immediately know that the pairs containing suffix z's are dead.
            
            size_t n = ops.size() / 2;
            size_t new_n = n;
            while (new_n > 0 && ops[new_n - 1] == z && ops[new_n - 1 + n] == o) --new_n;
            if (new_n == n) return std::nullopt;
            if (new_n == 0) {
                if (k == Kind::Sin || k == Kind::Tan) return z;
                if (k == Kind::Cos) return o;
                return std::nullopt;
            }
            std::vector<uint32_t> new_ops(2 * new_n);
            for (size_t i = 0; i < new_n; ++i) {
                new_ops[i]         = ops[i];
                new_ops[i + new_n] = ops[i + n];
            }
            return a.intern_chain(k, var, new_ops);
        }
        default:
            return std::nullopt;
    }
}

// Cheap n-ary identity collapses only. Room for more (log identities, negation-aware Add/Mul,
// distributive interactions with existing operands, EPow with EMul/EAdd base, etc.) — add as
// concrete workloads demand them.
std::optional<uint32_t> simplify_algebraic(Arena& a, Kind k, uint8_t var, std::span<const uint32_t> ops) {
    const uint32_t z = a.zero_leaf_id();
    const uint32_t o = a.one_leaf_id();

    switch (k) {
        case Kind::EAdd: {
            std::vector<uint32_t> new_ops;
            new_ops.reserve(ops.size());
            for (uint32_t id : ops) if (id != z) new_ops.push_back(id);
            if (new_ops.size() == ops.size()) return std::nullopt;
            if (new_ops.empty()) return z;
            if (new_ops.size() == 1) return new_ops[0];
            return a.intern_algebraic(k, var, new_ops);
        }
        case Kind::EMul: {
            for (uint32_t id : ops) if (id == z) return z;
            std::vector<uint32_t> new_ops;
            new_ops.reserve(ops.size());
            for (uint32_t id : ops) if (id != o) new_ops.push_back(id);
            if (new_ops.size() == ops.size()) return std::nullopt;
            if (new_ops.empty()) return o;
            if (new_ops.size() == 1) return new_ops[0];
            return a.intern_algebraic(k, var, new_ops);
        }
        case Kind::EPow: {
            if (ops.size() == 2) {
                if (ops[1] == z) return o;   // x^0 = 1
                if (ops[1] == o) return ops[0]; // x^1 = x
            }
            return std::nullopt;
        }
        default:
            return std::nullopt;
    }
}

}
