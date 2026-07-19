#pragma once

#include <cstdint>

#include "node.hpp"

namespace chains {

    using Reader = double(*)(const double* tape, uint32_t start, uint32_t length);

    inline double read_head(const double* tape, uint32_t start, uint32_t /*length*/) {
        return tape[start];
    }
    inline double read_cos(const double* tape, uint32_t start, uint32_t length) {
        return tape[start + length / 2];
    }
    inline double read_tan(const double* tape, uint32_t start, uint32_t length) {
        return tape[start] / tape[start + length / 2];
    }
    inline double read_cot(const double* tape, uint32_t start, uint32_t length) {
        return tape[start + length / 2] / tape[start];
    }
    inline double read_zero(const double*, uint32_t, uint32_t) { return 0.0; }

    inline Reader reader_for(Kind k) {
        switch (k) {
            case Kind::Sum:
            case Kind::Prod:
            case Kind::Sin: return &read_head;
            case Kind::Cos: return &read_cos;
            case Kind::Tan: return &read_tan;
            case Kind::Cot: return &read_cot;
            default: return &read_zero;
        }
    }

    inline double access(Kind k, const double* tape, uint32_t start, uint32_t length) {
        return reader_for(k)(tape, start, length);
    }

}
