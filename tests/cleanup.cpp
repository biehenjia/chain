#include <cr/domain/interval.hpp>
#include <cr/domain/mpreal.hpp>
#include <cr/domain/real.hpp>
#include <limits>
#include <make/crmake.hpp>
#include <stdexcept>

struct MemoKey {
    unsigned value;
};
template <>
struct cr::Field<MemoKey> {
    static uint32_t hash(MemoKey) { return 0; }
    static bool equal(MemoKey a, MemoKey b) { return a.value == b.value; }
};

void require(bool condition) {
    if (!condition)
        throw std::runtime_error("cleanup regression failed");
}

int main() {
    cr::Memo<MemoKey, unsigned> memo;
    unsigned calls = 0;
    require(memo.memoize({1}, [&] {
        ++calls;
        return 10u;
    }) == 10);
    require(memo.memoize({1}, [&] {
        ++calls;
        return 99u;
    }) == 10);
    require(calls == 1);
    require(memo.memoize({2}, [&] {
        for (unsigned i = 3; i < 256; ++i)
            require(memo.memoize({i}, [i] { return i; }) == i);
        return 20u;
    }) == 20);
    require(memo.memoize({1}, [] { return 99u; }) == 10);
    require(memo.memoize({2}, [] { return 99u; }) == 20);

    using namespace SymEngine;
    require(cr::field_from_uint<double>(0) == 0);
    require(cr::field_from_uint<double>(37) == 37);
    const auto interval = cr::field_from_uint<cr::Interval<double>>(37);
    require(interval.lo == 37 && interval.hi == 37);
    require(cr::Field<cr::MpReal>::equal(cr::field_from_uint<cr::MpReal>(37), cr::MpReal(37L)));
    const auto maximum = std::numeric_limits<uint64_t>::max();
    const auto expected = sub(pow(integer(2), integer(64)), integer(1));
    require(eq(*cr::field_from_uint<cr::Symbolic>(maximum), *expected));

    cr::Arena<cr::Symbolic> arena;
    cr::algebra::Router<cr::Symbolic> router(arena);
    const auto x = symbol("x");
    cr::make::CRmake make(router, {x});
    const auto id = make(x);
    require(arena.node(id).kind == cr::Kind::Sum && arena.node(id).var == 1);
    require(make(x) == id);
    require(arena.operands(id).size() == 2);
    require(arena.node(make(integer(7))).kind == cr::Kind::Leaf);

    require(make(add(x, integer(2))) == router.add(id, make(integer(2))));
    require(make(mul(integer(3), x)) == router.mul(make(integer(3)), id));
    require(make(pow(integer(2), x)) == router.pow(make(integer(2)), id));
    require(make(sin(x)) == router.sin(id));
    require(make(cos(x)) == router.cos(id));
    require(make(tan(x)) == router.tan(id));
    require(make(cot(x)) == router.cot(id));
    require(make(log(x)) == router.log(id, router.leaf(0, E)));
    const auto nested = sin(log(x));
    require(make(nested) == router.sin(make(log(x))));
    const auto nested_id = make(nested);
    require(make(nested) == nested_id);
}
