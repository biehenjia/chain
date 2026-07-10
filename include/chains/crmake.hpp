#pragma once
#include <cstdint>
#include <vector>
#include <chains/arena.hpp>
#include <symengine/symbol.h>

namespace chains {

    uint32_t crmake(Arena& a, const Symbolic& expr, const std::vector<SymEngine::RCP<const SymEngine::Symbol>>& symbols);

}
