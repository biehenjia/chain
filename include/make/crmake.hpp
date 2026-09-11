#pragma once
#include "build.hpp"
#include <cr/domain/symbolic.hpp>
#include <cr/intern/memo.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <symengine/symbol.h>
#include <unordered_map>
#include <vector>
namespace cr::make {
class CRmake {
public:
    CRmake(cr::algebra::Router<cr::Symbolic>& router, const std::vector<cr::Symbolic>& axes) : router_(router) {
        for (std::size_t v = 0; v < axes.size(); ++v)
            axis_.emplace(SymEngine::down_cast<const SymEngine::Symbol&>(*axes[v]).get_name(), uint8_t(v + 1));
    }
    uint32_t operator()(const cr::Symbolic& node) { return convert(node); }

private:
    uint32_t convert(const cr::Symbolic& node) {
        return memo_.memoize(node, [&] {
            return build(router_, axis_, node, [this](const cr::Symbolic& child) { return convert(child); });
        });
    }

    cr::algebra::Router<cr::Symbolic>& router_;
    std::unordered_map<std::string, uint8_t> axis_;
    cr::Memo<cr::Symbolic, uint32_t> memo_;
};
} // namespace cr::make
