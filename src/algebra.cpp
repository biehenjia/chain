#include <array>
#include <span>
#include <chains/algebra.hpp>
#include <chains/algebra/add.hpp>
#include <chains/algebra/mul.hpp>
#include <chains/algebra/pow.hpp>
#include <chains/algebra/log.hpp>
#include <chains/algebra/trig.hpp>

namespace chains {
    struct BinaryEntry {
        Kind lk;
        Kind rk;
        BinaryRule fn;
        bool commutative;
    };

    struct UnaryEntry {
        Kind k;
        UnaryRule fn;
    };

    static uint32_t default_binary(Arena& a, Kind k, uint32_t x, uint32_t y) {
        uint32_t ops[] = {x, y};
        uint8_t var = pick_var(a, x, y);
        return a.intern_algebraic(k, var, {ops, 2});
    }

    static uint32_t default_unary(Arena& a, Kind k, uint32_t x) {
        uint32_t ops[] = {x};
        uint8_t var = a.node(x).variable;
        return a.intern_algebraic(k, var, {ops, 1});
    }

    static uint32_t dispatch_binary(Arena& a, uint32_t x, uint32_t y, Kind lk, Kind rk, std::span<const BinaryEntry> rules, Kind default_kind) {
        // Forward pass: prefer a direct (lk, rk) match in argument order.
        for (const auto& e : rules) {
            if (e.lk == lk && e.rk == rk) return e.fn(a, x, y);
        }
        // Fallback: try commutative entries with swapped args.
        for (const auto& e : rules) {
            if (e.commutative && e.lk == rk && e.rk == lk) return e.fn(a, y, x);
        }
        return default_binary(a, default_kind, x, y);
    }

    
    static void canonicalize_by_var(const Arena& a, uint32_t x, uint32_t y, Kind& lk, Kind& rk) {
        uint8_t lv = a.node(x).variable;
        uint8_t rv = a.node(y).variable;
        if (lv > rv)      rk = Kind::Leaf;
        else if (lv < rv) lk = Kind::Leaf;
    }

    static uint32_t dispatch_unary(Arena& a, uint32_t x, Kind k, std::span<const UnaryEntry> rules, Kind default_kind) {

        for (const auto& e : rules) {
            if (e.k == k) return e.fn(a, x);
        }
        return default_unary(a, default_kind, x);
    }

    static constexpr std::array add_rules = std::to_array<BinaryEntry>({
        {Kind::Leaf, Kind::Leaf, add_leaf_leaf, true},
        {Kind::Sum,  Kind::Leaf, add_sum_leaf,  true},
        {Kind::Sum,  Kind::Sum,  add_sum_sum,   true},
    });
    
    static constexpr std::array mul_rules = std::to_array<BinaryEntry>({
        {Kind::Leaf, Kind::Leaf, mul_leaf_leaf, true},
        {Kind::Sum,  Kind::Leaf, mul_sum_leaf,  true},
        {Kind::Sum,  Kind::Sum,  mul_sum_sum,   true},
        {Kind::Prod, Kind::Leaf, mul_prod_leaf, true},
        {Kind::Prod, Kind::Prod, mul_prod_prod, true},
        {Kind::Sin,  Kind::Leaf, mul_sin_leaf,  true},
        {Kind::Cos,  Kind::Leaf, mul_cos_leaf,  true},
        {Kind::Prod, Kind::Sin,  mul_prod_sin,  true},
        {Kind::Prod, Kind::Cos,  mul_prod_cos,  true},
    });
    static constexpr std::array pow_rules = std::to_array<BinaryEntry>({
        {Kind::Leaf, Kind::Leaf, pow_leaf_leaf, false},
        {Kind::Leaf, Kind::Sum,  pow_leaf_sum,  false},
        {Kind::Sum,  Kind::Leaf, pow_sum_leaf,  false},
        {Kind::Prod, Kind::Leaf, pow_prod_leaf, false},
        {Kind::Prod, Kind::Sum,  pow_prod_sum,  false},
    });

    static constexpr std::array log_rules = std::to_array<BinaryEntry>({
        {Kind::Leaf, Kind::Leaf, log_leaf_leaf, false},
        {Kind::Prod, Kind::Leaf, log_prod_leaf, false},
    });

    static constexpr std::array sin_rules = std::to_array<UnaryEntry>({
        {Kind::Leaf, sin_leaf},
        {Kind::Sum,  sin_sum},
    });
    static constexpr std::array cos_rules = std::to_array<UnaryEntry>({
        {Kind::Leaf, cos_leaf},
        {Kind::Sum,  cos_sum},
    });
    static constexpr std::array tan_rules = std::to_array<UnaryEntry>({
        {Kind::Leaf, tan_leaf},
        {Kind::Sum,  tan_sum},
    });
    static constexpr std::array cot_rules = std::to_array<UnaryEntry>({
        {Kind::Leaf, cot_leaf},
        {Kind::Sum,  cot_sum},
    });

    uint32_t add(Arena& a, uint32_t x, uint32_t y) {
        Kind lk = a.node(x).kind;
        Kind rk = a.node(y).kind;
        canonicalize_by_var(a, x, y, lk, rk);
        return dispatch_binary(a, x, y, lk, rk, add_rules, Kind::EAdd);
    }
    uint32_t mul(Arena& a, uint32_t x, uint32_t y) {
        Kind lk = a.node(x).kind;
        Kind rk = a.node(y).kind;
        canonicalize_by_var(a, x, y, lk, rk);
        return dispatch_binary(a, x, y, lk, rk, mul_rules, Kind::EMul);
    }

    uint32_t pow(Arena& a, uint32_t x, uint32_t y) {
        Kind lk = a.node(x).kind;
        Kind rk = a.node(y).kind;
        return dispatch_binary(a, x, y, lk, rk, pow_rules, Kind::EPow);
    }
    uint32_t log(Arena& a, uint32_t x, uint32_t y) {
        Kind lk = a.node(x).kind;
        Kind rk = a.node(y).kind;
        return dispatch_binary(a, x, y, lk, rk, log_rules, Kind::ELog);
    }

    uint32_t sin(Arena& a, uint32_t x) { 
        Kind k = a.node(x).kind;
        return dispatch_unary(a, x, k, sin_rules, Kind::ESin); 
    }
    uint32_t cos(Arena& a, uint32_t x) {
        Kind k = a.node(x).kind; 
        return dispatch_unary(a, x, k,  cos_rules, Kind::ECos); 
    }
    uint32_t tan(Arena& a, uint32_t x) { 
        Kind k = a.node(x).kind; 
        return dispatch_unary(a, x, k, tan_rules, Kind::ETan); 
    }
    uint32_t cot(Arena& a, uint32_t x) { 
        Kind k = a.node(x).kind;
        return dispatch_unary(a, x, k, cot_rules, Kind::ECot); 
    }

}
