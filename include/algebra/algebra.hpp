#pragma once

#include <array>
#include <cstdint>
#include <span>

#include <core/core.hpp>
#include <hashing/hashing.hpp>

#include "algebra/router.hpp"
#include "algebra/rules.hpp"

namespace chains {

    template<class L>
    inline constexpr auto add_rules_v = std::to_array<BinaryEntry<L>>({
        {Kind::Leaf, Kind::Leaf, add_leaf_leaf<L>, true},
        {Kind::Sum, Kind::Leaf, add_sum_leaf<L>, true},
        {Kind::Sum, Kind::Sum, add_sum_sum<L>, true},
    });

    template<class L>
    inline constexpr auto mul_rules_v = std::to_array<BinaryEntry<L>>({
        {Kind::Leaf, Kind::Leaf, mul_leaf_leaf<L>, true},
        {Kind::Sum,  Kind::Leaf, mul_sum_leaf<L>, true},
        {Kind::Sum,  Kind::Sum,  mul_sum_sum<L>, true},
        {Kind::Prod, Kind::Leaf, mul_prod_leaf<L>, true},
        {Kind::Prod, Kind::Prod, mul_prod_prod<L>, true},
        {Kind::Sin, Kind::Leaf, mul_sin_leaf<L>, true},
        {Kind::Cos, Kind::Leaf, mul_cos_leaf<L>, true},
        {Kind::Prod, Kind::Sin,  mul_prod_sin<L>, true},
        {Kind::Prod, Kind::Cos,  mul_prod_cos<L>, true},
    });

    template<class L>
    inline constexpr auto pow_rules_v = std::to_array<BinaryEntry<L>>({
        {Kind::Leaf, Kind::Leaf, pow_leaf_leaf<L>, false},
        {Kind::Leaf, Kind::Sum, pow_leaf_sum<L>, false},
        {Kind::Sum, Kind::Leaf, pow_sum_leaf<L>, false},
        {Kind::Prod, Kind::Leaf, pow_prod_leaf<L>, false},
        {Kind::Prod, Kind::Sum, pow_prod_sum<L>, false},
    });

    template<class L>
    inline constexpr auto log_rules_v = std::to_array<BinaryEntry<L>>({
        {Kind::Leaf, Kind::Leaf, log_leaf_leaf<L>, false},
        {Kind::Prod, Kind::Leaf, log_prod_leaf<L>, false},
    });

    template<class L> inline constexpr auto sin_rules_v = std::to_array<UnaryEntry<L>>({ {Kind::Leaf, sin_leaf<L>}, {Kind::Sum, sin_sum<L>} });
    template<class L> inline constexpr auto cos_rules_v = std::to_array<UnaryEntry<L>>({ {Kind::Leaf, cos_leaf<L>}, {Kind::Sum, cos_sum<L>} });
    template<class L> inline constexpr auto tan_rules_v = std::to_array<UnaryEntry<L>>({ {Kind::Leaf, tan_leaf<L>}, {Kind::Sum, tan_sum<L>} });
    template<class L> inline constexpr auto cot_rules_v = std::to_array<UnaryEntry<L>>({ {Kind::Leaf, cot_leaf<L>}, {Kind::Sum, cot_sum<L>} });

    template<class L> uint32_t add(Interner<L>& i, uint32_t x, uint32_t y) { return dispatch_binary(i, x, y, std::span<const BinaryEntry<L>>{add_rules_v<L>}, Kind::EAdd); }
    template<class L> uint32_t mul(Interner<L>& i, uint32_t x, uint32_t y) { return dispatch_binary(i, x, y, std::span<const BinaryEntry<L>>{mul_rules_v<L>}, Kind::EMul); }
    template<class L> uint32_t pow(Interner<L>& i, uint32_t x, uint32_t y) { return dispatch_binary(i, x, y, std::span<const BinaryEntry<L>>{pow_rules_v<L>}, Kind::EPow); }
    template<class L> uint32_t log(Interner<L>& i, uint32_t x, uint32_t y) { return dispatch_binary(i, x, y, std::span<const BinaryEntry<L>>{log_rules_v<L>}, Kind::ELog); }

    template<class L> uint32_t sin(Interner<L>& i, uint32_t x) { return dispatch_unary(i, x, std::span<const UnaryEntry<L>>{sin_rules_v<L>}, Kind::ESin); }
    template<class L> uint32_t cos(Interner<L>& i, uint32_t x) { return dispatch_unary(i, x, std::span<const UnaryEntry<L>>{cos_rules_v<L>}, Kind::ECos); }
    template<class L> uint32_t tan(Interner<L>& i, uint32_t x) { return dispatch_unary(i, x, std::span<const UnaryEntry<L>>{tan_rules_v<L>}, Kind::ETan); }
    template<class L> uint32_t cot(Interner<L>& i, uint32_t x) { return dispatch_unary(i, x, std::span<const UnaryEntry<L>>{cot_rules_v<L>}, Kind::ECot); }

}
