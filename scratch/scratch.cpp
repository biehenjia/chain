// Playground for building & manipulating CR objects through the algebra.
// Build with the rest of the tree (see CMakeLists.txt: target `scratch`).
// Run: ./build/scratch

#include <iostream>
#include <vector>

#include <chains/arena.hpp>
#include <chains/algebra.hpp>
#include <chains/crmake.hpp>
#include <chains/print.hpp>

#include <symengine/symbol.h>
#include <symengine/integer.h>
#include <symengine/pow.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/functions.h>

using namespace chains;
using SymEngine::symbol;
using SymEngine::integer;

static void show(const char* label, const Arena& a, uint32_t id) {
    std::cout << label << " (id=" << id << ") = " << to_string(a, id) << "\n";
}

int main() {
    Arena a;

    // ---- Leaves --------------------------------------------------------
    uint32_t c2 = a.push_leaf(a.push_symbolic(integer(2)));
    uint32_t c3 = a.push_leaf(a.push_symbolic(integer(3)));
    show("leaf 2", a, c2);
    show("leaf 3", a, c3);

    // ---- Direct algebra on leaves --------------------------------------
    uint32_t s = add(a, c2, c3);
    show("2 + 3", a, s);

    uint32_t p = mul(a, c2, c3);
    show("2 * 3", a, p);

    // ---- CR from a SymEngine expression --------------------------------
    // e = x^2 + 3x + 1 over variable x.
    auto x = symbol("x");
    auto expr = SymEngine::add(
        SymEngine::add(
            SymEngine::pow(x, integer(2)),
            SymEngine::mul(integer(3), x)),
        integer(1));

    std::vector<SymEngine::RCP<const SymEngine::Symbol>> syms = { x };
    uint32_t poly = crmake(a, expr, syms);
    show("x^2 + 3x + 1", a, poly);

    // ---- Multiply two CR chains ----------------------------------------
    auto y_expr = SymEngine::add(x, integer(1));       // x + 1
    uint32_t y = crmake(a, y_expr, syms);
    show("x + 1", a, y);

    uint32_t product = mul(a, poly, y);
    show("(x^2 + 3x + 1) * (x + 1)", a, product);

    // ---- Trig ----------------------------------------------------------
    auto sinx = SymEngine::sin(x);
    uint32_t sx = crmake(a, sinx, syms);
    show("sin(x)", a, sx);

    uint32_t sx_scaled = mul(a, c2, sx);
    show("2 * sin(x)", a, sx_scaled);

    // ---- Playground area -----------------------------------------------
    // Add anything below to poke at the arena.

    std::cout << "\nArena: " << a.num_nodes() << " nodes, "
              << a.operand_count() << " operand slots\n";
    return 0;
}
