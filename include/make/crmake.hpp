#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <symengine/add.h>
#include <symengine/basic.h>
#include <symengine/constants.h>
#include <symengine/functions.h>
#include <symengine/mul.h>
#include <symengine/number.h>
#include <symengine/pow.h>
#include <symengine/symbol.h>

#include <cr/domain/symbolic.hpp>
#include <cr/intern/intern.hpp>
#include <cr/ir/node.hpp>

#include "../algebra/router.hpp"

namespace cr::make {

    class CRmake {
        public:
            CRmake(cr::algebra::Router<cr::Symbolic>& router, const std::vector<cr::Symbolic>& axes) : router_(router) {
                for (std::size_t v = 0; v < axes.size(); ++v)
                    axis_.emplace(SymEngine::down_cast<const SymEngine::Symbol&>(*axes[v]).get_name(), uint8_t(v + 1));
            }

            uint32_t operator()(const cr::Symbolic& node) {
                const std::size_t h = node->hash();
                return cr::cons(memo_, h,
                    [&](const std::pair<cr::Symbolic, uint32_t>& hit) { return SymEngine::eq(*hit.first, *node); },
                    [&] { return std::pair{node, build(node)}; }).second;
            }

        private:
            uint32_t build(const cr::Symbolic& node) {
                using namespace SymEngine;

                if (is_a_Number(*node)) return router_.leaf(0, node);

                if (is_a<Symbol>(*node)) {
                    const std::string& name = down_cast<const Symbol&>(*node).get_name();
                    const uint8_t v = axis_.at(name);
                    const uint32_t x0 = router_.leaf(0, symbol(name + "_0"));
                    const uint32_t xh = router_.leaf(0, symbol(name + "_h"));
                    const std::array<uint32_t, 2> ops{x0, xh};
                    return router_.emit(cr::Kind::Sum, v, ops);
                }

                if (is_a<Add>(*node)) return fold(node->get_args(), [&](uint32_t a, uint32_t b) { return router_.add(a, b); });
                if (is_a<Mul>(*node)) return fold(node->get_args(), [&](uint32_t a, uint32_t b) { return router_.mul(a, b); });

                if (is_a<Pow>(*node)) {
                    const auto& p = down_cast<const Pow&>(*node);
                    return router_.pow((*this)(p.get_base()), (*this)(p.get_exp()));
                }

                if (is_a<Sin>(*node)) return router_.sin((*this)(down_cast<const Sin&>(*node).get_arg()));
                if (is_a<Cos>(*node)) return router_.cos((*this)(down_cast<const Cos&>(*node).get_arg()));
                if (is_a<Tan>(*node)) return router_.tan((*this)(down_cast<const Tan&>(*node).get_arg()));
                if (is_a<Cot>(*node)) return router_.cot((*this)(down_cast<const Cot&>(*node).get_arg()));

                if (is_a<Log>(*node)) {
                    const uint32_t arg = (*this)(down_cast<const Log&>(*node).get_arg());
                    return router_.log(arg, router_.leaf(0, E));
                }

                throw std::runtime_error("crmake: unhandled node " + node->__str__());
            }

            template <class F>
            uint32_t fold(const SymEngine::vec_basic& args, F&& combine) {
                uint32_t res = (*this)(args[0]);
                for (std::size_t i = 1; i < args.size(); ++i) res = combine(res, (*this)(args[i]));
                return res;
            }

            cr::algebra::Router<cr::Symbolic>& router_;
            std::unordered_map<std::string, uint8_t> axis_;
            std::unordered_multimap<std::size_t, std::pair<cr::Symbolic, uint32_t>> memo_;
    };

}
