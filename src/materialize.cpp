#include <chains/materialize.hpp>
#include <chains/arena.hpp>
#include <unordered_set>

namespace chains {

namespace {

Symbolic value_of(const Arena& a, const Materialized& m, uint32_t id) {
    const Node& n = a.node(id);
    if (n.kind == Kind::Leaf) return a.leaf_symbolic(id);
    // Connector: not resolved in this pass. Left for prepare/shift wiring.
    std::span<const Symbolic> slot(m.values.data() + n.slot_a, n.slot_b);
    return access(n.kind, slot);
}

void materialize_rec(const Arena& a, uint32_t id, Materialized& m, std::unordered_set<uint32_t>& done) {
    if (!done.insert(id).second) return;
    const Node& n = a.node(id);
    if (n.kind == Kind::Leaf || n.kind == Kind::Connector) return;
    auto ops = a.operands(id);
    for (uint32_t i = 0; i < ops.size(); ++i) {
        materialize_rec(a, ops[i], m, done);
        m.values[n.slot_a + i] = value_of(a, m, ops[i]);
    }
}

}

Materialized materialize(const Arena& a, uint32_t root) {
    Materialized m;
    m.values.resize(a.operand_count());
    std::unordered_set<uint32_t> done;
    materialize_rec(a, root, m, done);
    return m;
}

}
