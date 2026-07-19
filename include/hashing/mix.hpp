#pragma once

#include <cstdint>

namespace chains {

    // swap for whatever
    inline uint32_t hmix(uint32_t a, uint32_t b) { return a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2)); }

}
