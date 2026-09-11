#pragma once
#include "../simplification/simplify.hpp"
#include "add.hpp"
#include "cos.hpp"
#include "cot.hpp"
#include "log.hpp"
#include "mul.hpp"
#include "pow.hpp"
#include "sin.hpp"
#include "sub.hpp"
#include "tan.hpp"
#include <cr/intern/intern.hpp>
#include <cr/ir/arena.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
#include <span>
#include <utility>
namespace cr::algebra {
template <class T>
class Router {
public:
    explicit Router(cr::Arena<T>& arena) : arena_(arena), interner_(arena_) {}
    uint32_t leaf(uint8_t var, T value) { return interner_.intern_leaf(var, std::move(value)); }
    uint32_t emit(cr::Kind kind, uint8_t var, std::span<const uint32_t> operands) {
        return interner_.intern_node(kind, var, operands);
    }
    uint32_t add(uint32_t lhs, uint32_t rhs) { return simplify(cr::algebra::add(interner_, lhs, rhs)); }
    uint32_t sub(uint32_t lhs, uint32_t rhs) { return simplify(cr::algebra::sub(interner_, lhs, rhs)); }
    uint32_t mul(uint32_t lhs, uint32_t rhs) { return simplify(cr::algebra::mul(interner_, lhs, rhs)); }
    uint32_t pow(uint32_t lhs, uint32_t rhs) { return simplify(cr::algebra::pow(interner_, lhs, rhs)); }
    uint32_t log(uint32_t x, uint32_t base) { return simplify(cr::algebra::log(interner_, x, base)); }
    uint32_t sin(uint32_t x) { return simplify(cr::algebra::sin(interner_, x)); }
    uint32_t cos(uint32_t x) { return simplify(cr::algebra::cos(interner_, x)); }
    uint32_t tan(uint32_t x) { return simplify(cr::algebra::tan(interner_, x)); }
    uint32_t cot(uint32_t x) { return simplify(cr::algebra::cot(interner_, x)); }

private:
    uint32_t simplify(uint32_t id) { return cr::simplify::simplify(interner_, id); }
    cr::Arena<T>& arena_;

public:
    cr::Interner<T> interner_;
};
} // namespace cr::algebra
