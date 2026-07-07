#include <cassert>
#include <cstdio>
#include <chains/arena.hpp>
#include <symengine/symbol.h>

using namespace chains;
using SymEngine::symbol;

int main() {
    Arena a;

    uint32_t s_x = a.push_symbolic(symbol("x"));
    uint32_t s_y = a.push_symbolic(symbol("y"));

    // Symbolic tape interning: pushing an equal expression returns the same id.
    uint32_t s_x_again = a.push_symbolic(symbol("x"));
    assert(s_x_again == s_x);

    // Leaves: same (var, symbolic span) intern to same id.
    uint32_t lx1 = a.push_leaf(0, s_x);
    uint32_t lx2 = a.push_leaf(0, s_x);
    assert(lx1 == lx2);

    // Different symbolic → different leaf.
    uint32_t ly = a.push_leaf(0, s_y);
    assert(ly != lx1);

    // Different variable → different leaf.
    uint32_t lx_v1 = a.push_leaf(1, s_x);
    assert(lx_v1 != lx1);

    // Chain intern: same ordered ops → same NodeId.
    uint32_t ops_xy[] = {lx1, ly};
    uint32_t c_xy_1 = a.intern_chain(Kind::Sum, 0, {ops_xy, 2});
    uint32_t c_xy_2 = a.intern_chain(Kind::Sum, 0, {ops_xy, 2});
    assert(c_xy_1 == c_xy_2);

    // Different order → different chain.
    uint32_t ops_yx[] = {ly, lx1};
    uint32_t c_yx = a.intern_chain(Kind::Sum, 0, {ops_yx, 2});
    assert(c_yx != c_xy_1);

    // Different kind → different chain.
    uint32_t c_prod = a.intern_chain(Kind::Prod, 0, {ops_xy, 2});
    assert(c_prod != c_xy_1);

    // Whole-chain hash matches the suffix hash at position 0.
    const Node& n_xy = a.node(c_xy_1);
    assert(n_xy.hash == a.suffix_hash_at(n_xy.slot_a));

    // Suffix reuse: build (lx1, lx1, ly). Its suffix from position 1 should
    // hash-match the whole chain (lx1, ly) → confirms suffix hashes align.
    uint32_t ops_xxy[] = {lx1, lx1, ly};
    uint32_t c_xxy = a.intern_chain(Kind::Sum, 0, {ops_xxy, 3});
    const Node& n_xxy = a.node(c_xxy);
    assert(a.suffix_hash_at(n_xxy.slot_a + 1) == n_xy.hash);

    // Terminal suffix hash — position N-1 of any chain hashes just its last element.
    // Sanity: two Sum chains ending in the same element share their last suffix hash.
    uint32_t last_xy  = a.suffix_hash_at(n_xy.slot_a + 1);
    uint32_t last_xxy = a.suffix_hash_at(n_xxy.slot_a + 2);
    assert(last_xy == last_xxy);

    // Algebraic intern.
    uint32_t alg1 = a.intern_algebraic(Kind::EAdd, 0, {ops_xy, 2});
    uint32_t alg2 = a.intern_algebraic(Kind::EAdd, 0, {ops_xy, 2});
    assert(alg1 == alg2);
    // Algebraic Add is distinct from Sum chain even with identical operands.
    assert(alg1 != c_xy_1);

    // Leaf accessor returns the backing symbolic.
    assert(SymEngine::eq(*a.leaf_symbolic(lx1), *a.symbolic(s_x)));

    // Chain operand span.
    auto xy_ops = a.operands(c_xy_1);
    assert(xy_ops.size() == 2);
    assert(xy_ops[0] == lx1 && xy_ops[1] == ly);

    std::printf("ok — all arena tests passed (%u nodes)\n", a.num_nodes());
    return 0;
}
