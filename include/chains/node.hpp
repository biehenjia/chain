#pragma once

#include <cstdint>
#include <type_traits>

namespace chains {

    enum class Kind : uint8_t {
        Leaf = 0,
        // CR* (chain, flat span of element NodeIds in reverse/suffix order)
        Sum, Prod, Sin, Cos, Tan, Cot,
        // CRE* (algebraic n-ary, flat span of operand NodeIds)
        EAdd, EMul, EPow, ELog, ESin, ECos, ETan, ECot,
    };

    inline bool is_trig_kind(Kind k) {
        return k == Kind::Sin || k == Kind::Cos || k == Kind::Tan || k == Kind::Cot;
    }

    /*
    Kind: 1B
    variable: 1B
    _pad: 2B
    slot_a: 4B Leaf: symbolic id; CR or CRE: opstart
    slot_b: 4B Leaf: unused (0); CR or CRE: oplength
    hash: 4B whole-chain hash for CR; subtree hash otherwise (per-position suffix hashes live in a parallel side tape)
    */

    struct Node {
        Kind kind;
        uint8_t variable;
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
