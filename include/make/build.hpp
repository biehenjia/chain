#pragma once
#include "../algebra/router.hpp"
#include <array>
#include <cr/domain/symbolic.hpp>
#include <cr/ir/node.hpp>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <symengine/add.h>
#include <symengine/basic.h>
#include <symengine/constants.h>
#include <symengine/functions.h>
#include <symengine/mul.h>
#include <symengine/number.h>
#include <symengine/pow.h>
#include <symengine/symbol.h>
#include <unordered_map>
namespace cr::make {
template <class Convert>
uint32_t build(cr::algebra::Router<cr::Symbolic>& router, const std::unordered_map<std::string, uint8_t>& axes,
               const cr::Symbolic& node, Convert&& convert) {
    using namespace SymEngine;
    auto fold = [&](const vec_basic& args, auto combine) {
        uint32_t res = convert(args[0]);
        for (std::size_t i = 1; i < args.size(); ++i)
            res = combine(res, convert(args[i]));
        return res;
    };
    if (is_a_Number(*node))
        return router.leaf(0, node);
    if (is_a<Symbol>(*node)) {
        const std::string& name = down_cast<const Symbol&>(*node).get_name();
        const uint8_t v = axes.at(name);
        const uint32_t x0 = router.leaf(0, symbol(name + "_0"));
        const uint32_t xh = router.leaf(0, symbol(name + "_h"));
        const std::array<uint32_t, 2> ops{x0, xh};
        return router.emit(cr::Kind::Sum, v, ops);
    }
    if (is_a<Add>(*node))
        return fold(node->get_args(), [&](uint32_t a, uint32_t b) { return router.add(a, b); });
    if (is_a<Mul>(*node))
        return fold(node->get_args(), [&](uint32_t a, uint32_t b) { return router.mul(a, b); });
    if (is_a<Pow>(*node)) {
        const auto& p = down_cast<const Pow&>(*node);
        return router.pow(convert(p.get_base()), convert(p.get_exp()));
    }
    if (is_a<Sin>(*node))
        return router.sin(convert(down_cast<const Sin&>(*node).get_arg()));
    if (is_a<Cos>(*node))
        return router.cos(convert(down_cast<const Cos&>(*node).get_arg()));
    if (is_a<Tan>(*node))
        return router.tan(convert(down_cast<const Tan&>(*node).get_arg()));
    if (is_a<Cot>(*node))
        return router.cot(convert(down_cast<const Cot&>(*node).get_arg()));
    if (is_a<Log>(*node)) {
        const uint32_t arg = convert(down_cast<const Log&>(*node).get_arg());
        return router.log(arg, router.leaf(0, E));
    }
    throw std::runtime_error("crmake: unhandled node " + node->__str__());
}
} // namespace cr::make
