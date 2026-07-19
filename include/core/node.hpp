#pragma once

#include <cstdint>
#include <type_traits>

namespace chains {

    enum class Kind : uint8_t {
        Leaf = 0,
        Sum, Prod,
        Trig, Sin, Cos, Tan, Cot,
        EAdd, EMul, EPow, ELog, ESin, ECos, ETan, ECot,
        Connector,
    };

    inline bool is_leaf_kind(Kind k) { return k == Kind::Leaf; }
    inline bool is_trig_kind(Kind k) { return k == Kind::Sin || k == Kind::Cos || k == Kind::Tan || k == Kind::Cot; }

    inline bool is_chain_kind(Kind k) { return k >= Kind::Sum && k <= Kind::Cot; }
    inline bool is_algebraic_kind(Kind k) { return k >= Kind::EAdd && k <= Kind::ECot; }
    inline bool is_connector_kind(Kind k) { return k == Kind::Connector; }

    struct Node {
        Kind kind;

        uint8_t var;
        uint16_t _pad;

        uint32_t slot_a;
        uint32_t slot_b;

        uint32_t hash;
    };

    static_assert(sizeof(Node) == 16);
    static_assert(alignof(Node) == 4);
    static_assert(std::is_trivially_copyable_v<Node>);
    static_assert(std::is_standard_layout_v<Node>);

}
