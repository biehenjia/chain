#pragma once
#include <cstdint>
#include <utility>
#include <vector>
#include <chains/algebra.hpp>
#include <chains/arena.hpp>
#include <symengine/mul.h>

namespace chains{

    inline uint32_t mul_leaf_leaf(Arena& a, uint32_t x, uint32_t y ){
        auto prod = SymEngine::mul(a.leaf_symbolic(x), a.leaf_symbolic(y));
        uint32_t sym_id = a.push_symbolic(prod);
        return a.push_leaf(sym_id);
    }

    inline uint32_t mul_sum_leaf(Arena& a, uint32_t x, uint32_t y){ 
        auto ops = a.operands(x);
        std::vector<uint32_t> new_ops(ops.begin(), ops.end());

        for (size_t i = 0; i < ops.size(); ++i){
            
        }
    }


}