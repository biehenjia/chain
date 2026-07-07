#pragma once 
#include <string>


#include "chains/arena.hpp"
#include "chains/node.hpp"

namespace chains { 
    uint32_t add(Arena& a, uint32_t x, uint32_t y);
    uint32_t mul(Arena& a, uint32_t x, uint32_t y);
    
}