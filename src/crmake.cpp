#include <cassert>
#include <string>
#include <unordered_map>
#include <chains/crmake.hpp>
#include <chains/algebra.hpp>
#include <symengine/basic.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/symbol.h>
#include <symengine/integer.h>
#include <symengine/number.h>
#include <symengine/constants.h>
#include <symengine/functions.h>

namespace chains {

    namespace {

        bool ends_with(const std::string& s, const char* suffix) {
            size_t n = std::string_view(suffix).size();
            return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
        }

        struct Ctx {
            Arena& a;
            const std::vector<SymEngine::RCP<const SymEngine::Symbol>>& symbols;
            std::unordered_map<std::string, uint8_t> var_of;
            std::unordered_map<std::string, uint32_t> symbol_cr; // cached CRsum id per symbol name

            uint32_t leaf(const Symbolic& s) {
                return a.push_leaf(a.push_symbolic(s));
            }

            uint32_t materialize_symbol(const std::string& name) {
                auto it = symbol_cr.find(name);
                if (it != symbol_cr.end()) return it->second;
                uint8_t var = var_of.at(name);
                uint32_t x0 = leaf(SymEngine::symbol(name + "_0"));
                uint32_t xh = leaf(SymEngine::symbol(name + "_h"));
                uint32_t ops[] = {x0, xh};
                uint32_t id = a.intern_chain(Kind::Sum, var, {ops, 2});
                symbol_cr.emplace(name, id);
                return id;
            }

            uint32_t walk(const Symbolic& e) {
                using namespace SymEngine;
                const Basic& b = *e;

                if (is_a_Number(b) || is_a<Constant>(b)) return leaf(e);

                if (is_a<Symbol>(b)) {
                    const auto& sym = down_cast<const Symbol&>(b);
                    auto it = var_of.find(sym.get_name());
                    if (it == var_of.end()) return leaf(e); // free/parameter symbol
                    return materialize_symbol(sym.get_name());
                }

                if (is_a<Add>(b)) {
                    auto args = b.get_args();
                    uint32_t r = walk(args[0]);
                    for (size_t i = 1; i < args.size(); ++i) r = add(a, r, walk(args[i]));
                    return r;
                }
                if (is_a<Mul>(b)) {
                    auto args = b.get_args();
                    uint32_t r = walk(args[0]);
                    for (size_t i = 1; i < args.size(); ++i) r = mul(a, r, walk(args[i]));
                    return r;
                }
                if (is_a<Pow>(b)) {
                    auto args = b.get_args();
                    return pow(a, walk(args[0]), walk(args[1]));
                }
                if (is_a<Sin>(b)) return sin(a, walk(b.get_args()[0]));
                if (is_a<Cos>(b)) return cos(a, walk(b.get_args()[0]));
                if (is_a<Tan>(b)) return tan(a, walk(b.get_args()[0]));
                if (is_a<Cot>(b)) return cot(a, walk(b.get_args()[0]));
                if (is_a<Log>(b)) {

                    uint32_t arg = walk(b.get_args()[0]);
                    uint32_t base = leaf(E);
                    return log(a, arg, base);
                }
                //fallback 
                return leaf(e);
            }
        };

    } // namespace of chains

    uint32_t crmake(Arena& a, const Symbolic& expr, const std::vector<SymEngine::RCP<const SymEngine::Symbol>>& symbols) {
        assert(symbols.size() <= 256 && "variable id is uint8_t");

        Ctx ctx{a, symbols, {}, {}};
        for (size_t i = 0; i < symbols.size(); ++i) {
            const std::string& name = symbols[i]->get_name();
            assert(!ends_with(name, "_0") && "symbol name may not end in _0");
            assert(!ends_with(name, "_h") && "symbol name may not end in _h");
            ctx.var_of.emplace(name, static_cast<uint8_t>(i));
        }
        return ctx.walk(expr);
    }

}
