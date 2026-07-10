#pragma once
#include <span>
#include <chains/node.hpp>
#include <symengine/basic.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/functions.h>

// The access rule per Kind: given the operand-slot values of a node, returns the
// "current value" that node presents to its parent. Shared between materialization
// (initial tape population) and the shift dispatcher (hot interpreter path), so it
// lives in its own header, inline, with no Arena dependency.
//
// Leaf and Connector are intentionally not handled here:
//   Leaf      — has no tape entry; callers use Arena::leaf_symbolic(id) directly.
//   Connector — resolved by prepare/shift dispatch; the origin's tape slot at
//               the recorded offset is what the connector represents.

namespace chains {

using Symbolic = SymEngine::RCP<const SymEngine::Basic>;

inline Symbolic access(Kind k, std::span<const Symbolic> values) {
    using namespace SymEngine;
    switch (k) {
        case Kind::Sum:
        case Kind::Prod:
        case Kind::Sin:
        case Kind::Cos:
            return values[0];
        case Kind::Tan:
            return div(values[0], values[values.size() / 2]);
        case Kind::Cot:
            return div(values[values.size() / 2], values[0]);
        case Kind::EAdd: return SymEngine::add(values[0], values[1]);
        case Kind::EMul: return SymEngine::mul(values[0], values[1]);
        case Kind::EPow: return SymEngine::pow(values[0], values[1]);
        case Kind::ELog: return SymEngine::log(values[0], values[1]);
        case Kind::ESin: return SymEngine::sin(values[0]);
        case Kind::ECos: return SymEngine::cos(values[0]);
        case Kind::ETan: return SymEngine::tan(values[0]);
        case Kind::ECot: return SymEngine::cot(values[0]);
        case Kind::Leaf:
        case Kind::Connector:
            break;
    }
    return values[0];
}

}
