#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include <symengine/basic.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/symbol.h>
#include <symengine/constants.h>
#include <symengine/functions.h>
#include <symengine/eval_double.h>

#include <core/core.hpp>
#include <hashing/hashing.hpp>
#include <algebra/algebra.hpp>

namespace chains::crmake_detail {

    template<class LeafT, class Policy>
    inline uint32_t walk(Interner<LeafT>& i, const Policy& pol, const Symbolic& e) {
        using namespace SymEngine;
        const Basic& b = *e;

        if (is_a_Number(b) || is_a<Constant>(b)) return pol.number_or_constant(i, e);
        if (is_a<Symbol>(b)) return pol.symbol(i, down_cast<const Symbol&>(b).get_name());

        if (is_a<Add>(b)) {
            auto args = b.get_args();
            uint32_t r = walk(i, pol, args[0]);
            for (std::size_t k = 1; k < args.size(); ++k) r = add(i, r, walk(i, pol, args[k]));
            return r;
        }
        if (is_a<Mul>(b)) {
            auto args = b.get_args();
            uint32_t r = walk(i, pol, args[0]);
            for (std::size_t k = 1; k < args.size(); ++k) r = mul(i, r, walk(i, pol, args[k]));
            return r;
        }
        if (is_a<Pow>(b)) {
            auto args = b.get_args();
            return pow(i, walk(i, pol, args[0]), walk(i, pol, args[1]));
        }
        if (is_a<Sin>(b)) return sin(i, walk(i, pol, b.get_args()[0]));
        if (is_a<Cos>(b)) return cos(i, walk(i, pol, b.get_args()[0]));
        if (is_a<Tan>(b)) return tan(i, walk(i, pol, b.get_args()[0]));
        if (is_a<Cot>(b)) return cot(i, walk(i, pol, b.get_args()[0]));
        if (is_a<Log>(b)) return log(i, walk(i, pol, b.get_args()[0]), pol.log_base(i));

        return pol.fallback(i, e);
    }

    using VarMap = std::unordered_map<std::string, uint8_t>;

    struct SymbolicPolicy {
        const VarMap& var_of;

        uint32_t number_or_constant(Interner<Symbolic>& i, const Symbolic& e) const {
            return i.intern(e);
        }
        uint32_t symbol(Interner<Symbolic>& i, const std::string& name) const {
            auto it = var_of.find(name);
            if (it == var_of.end()) return i.intern(SymEngine::symbol(name));
            uint32_t x0 = i.intern(SymEngine::symbol(name + "_0"));
            uint32_t xh = i.intern(SymEngine::symbol(name + "_h"));
            uint32_t ops[] = {x0, xh};
            return i.intern(Kind::Sum, it->second, std::span<const uint32_t>(ops, 2));
        }
        uint32_t log_base(Interner<Symbolic>& i) const { return i.intern(SymEngine::E); }
        uint32_t fallback(Interner<Symbolic>& i, const Symbolic& e) const { return i.intern(e); }
    };

    struct NumericAxis { uint8_t id; double start; double step; };
    using NumericMap = std::unordered_map<std::string, NumericAxis>;

    struct NumericPolicy {
        const NumericMap& var_of;

        uint32_t number_or_constant(Interner<double>& i, const Symbolic& e) const {
            return i.intern(SymEngine::eval_double(*e));
        }
        uint32_t symbol(Interner<double>& i, const std::string& name) const {
            auto it = var_of.find(name);
            assert(it != var_of.end() && "unbound symbol in numeric crmake");
            uint32_t x0 = i.intern(it->second.start);
            uint32_t xh = i.intern(it->second.step);
            uint32_t ops[] = {x0, xh};
            return i.intern(Kind::Sum, it->second.id, std::span<const uint32_t>(ops, 2));
        }
        uint32_t log_base(Interner<double>& i) const { return i.intern(std::exp(1.0)); }
        uint32_t fallback(Interner<double>&, const Symbolic&) const {
            assert(false && "unsupported SymEngine node in numeric crmake");
            return 0;
        }
    };

}

namespace chains {

    struct NumericVar {
        std::string name;
        double start;
        double step;
    };

    inline uint32_t crmake(Interner<Symbolic>& i, const Symbolic& expr, const std::vector<SymEngine::RCP<const SymEngine::Symbol>>& symbols) {
        assert(symbols.size() <= 256 && "variable id is uint8_t");

        crmake_detail::VarMap var_of;
        for (std::size_t k = 0; k < symbols.size(); ++k) {
            const std::string& name = symbols[k]->get_name();
            assert(!name.ends_with("_0") && "symbol name may not end in _0");
            assert(!name.ends_with("_h") && "symbol name may not end in _h");
            var_of.emplace(name, static_cast<uint8_t>(k));
        }
        return crmake_detail::walk(i, crmake_detail::SymbolicPolicy{var_of}, expr);
    }

    inline uint32_t crmake(Interner<double>& i, const Symbolic& expr, const std::vector<NumericVar>& vars) {
        assert(vars.size() <= 256 && "variable id is uint8_t");

        crmake_detail::NumericMap var_of;
        for (std::size_t k = 0; k < vars.size(); ++k) {
            var_of.emplace(vars[k].name, crmake_detail::NumericAxis{uint8_t(k), vars[k].start, vars[k].step});
        }
        return crmake_detail::walk(i, crmake_detail::NumericPolicy{var_of}, expr);
    }

}
