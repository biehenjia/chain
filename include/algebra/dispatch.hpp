#pragma once
#include <cr/intern/intern.hpp>
#include <cr/ir/node.hpp>
#include <cstdint>
namespace cr::algebra {
inline constexpr uint16_t pair(cr::Kind a, cr::Kind b) {
    return uint16_t(uint16_t(a) << 8 | uint16_t(b));
}
template <class T>
cr::Kind dispatch_kind(const cr::Interner<T>& interner, uint32_t id, uint8_t v) {
    const cr::Node& n = interner.node(id);
    return n.var < v ? cr::Kind::Leaf : n.kind;
}
} // namespace cr::algebra
