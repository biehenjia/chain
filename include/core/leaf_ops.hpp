#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <symengine/basic.h>
#include <symengine/integer.h>
#include <symengine/ntheory.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/functions.h>
#include <symengine/printers.h>

#include "node.hpp"
#include "leaf_hash.hpp"

namespace chains {

    using Symbolic = SymEngine::RCP<const SymEngine::Basic>;

    inline uint32_t hash_leaf(const Symbolic& s) {
        std::size_t sh = s->hash();
        uint32_t folded = uint32_t(sh) ^ uint32_t(sh >> 32);
        return leaf_hash_detail::hmix(uint32_t(Kind::Leaf), folded);
    }

    inline bool leaf_equal(const Symbolic& a, const Symbolic& b) {
        return SymEngine::eq(*a, *b);
    }

    template<class LeafT> LeafT make_int(long v);
    template<> inline double make_int<double>(long v) { return double(v); }
    template<> inline Symbolic make_int<Symbolic>(long v) { return SymEngine::integer(v); }

    template<class LeafT> LeafT make_binom(std::size_t n, std::size_t k);
    template<> inline double make_binom<double>(std::size_t n, std::size_t k) {
        double r = 1.0;
        for (std::size_t s = 1; s <= k; ++s) r = r * double(n - s + 1) / double(s);
        return r;
    }
    template<> inline Symbolic make_binom<Symbolic>(std::size_t n, std::size_t k) {
        auto n_int = SymEngine::integer(long(n));
        return SymEngine::binomial(*n_int, static_cast<unsigned long>(k));
    }

    inline double combine_add(double a, double b) { return a + b; }
    inline Symbolic combine_add(const Symbolic& a, const Symbolic& b) { return SymEngine::add(a, b); }

    inline double combine_mul(double a, double b) { return a * b; }
    inline Symbolic combine_mul(const Symbolic& a, const Symbolic& b) { return SymEngine::mul(a, b); }

    inline double combine_pow(double a, double b) { return std::pow(a, b); }
    inline Symbolic combine_pow(const Symbolic& a, const Symbolic& b) { return SymEngine::pow(a, b); }

    inline double combine_log(double a, double b) { return std::log(a) / std::log(b); }
    inline Symbolic combine_log(const Symbolic& a, const Symbolic& b) { return SymEngine::log(a, b); }

    inline double apply_sin(double v) { return std::sin(v); }
    inline Symbolic apply_sin(const Symbolic& s) { return SymEngine::sin(s); }

    inline double apply_cos(double v) { return std::cos(v); }
    inline Symbolic apply_cos(const Symbolic& s) { return SymEngine::cos(s); }

    inline double apply_tan(double v) { return std::tan(v); }
    inline Symbolic apply_tan(const Symbolic& s) { return SymEngine::tan(s); }

    inline double apply_cot(double v) { return 1.0 / std::tan(v); }
    inline Symbolic apply_cot(const Symbolic& s) { return SymEngine::cot(s); }

    inline std::optional<long> as_nonneg_int(double v) {
        if (v < 0 || v != std::floor(v)) return std::nullopt;
        return long(v);
    }
    inline std::optional<long> as_nonneg_int(const Symbolic& s) {
        if (!SymEngine::is_a<SymEngine::Integer>(*s)) return std::nullopt;
        const auto& yi = SymEngine::down_cast<const SymEngine::Integer&>(*s);
        if (yi.is_negative()) return std::nullopt;
        return SymEngine::mp_get_si(yi.as_integer_class());
    }

    inline std::string format_leaf(double v)          { return std::to_string(v); }
    inline std::string format_leaf(const Symbolic& s) { return SymEngine::str(*s); }

}
