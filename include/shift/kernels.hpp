#pragma once

#include <cstdint>

namespace chains::kernel {

    inline void run_sum(double* t, uint32_t s, uint32_t L) {
        for (uint32_t i = 0; i + 1 < L; ++i) t[s + i] += t[s + i + 1];
    }

    inline void run_prod(double* t, uint32_t s, uint32_t L) {
        for (uint32_t i = 0; i + 1 < L; ++i) t[s + i] *= t[s + i + 1];
    }
    inline void run_trig(double* t, uint32_t s, uint32_t L) {
        const uint32_t half = L / 2;
        for (uint32_t i = 0; i + 1 < half; ++i) {
            double sin_i = t[s + i];
            double cos_i = t[s + half + i];
            double sin_ip1 = t[s + i + 1];
            double cos_ip1 = t[s + half + i + 1];
            t[s + i] = sin_i * cos_ip1 + cos_i * sin_ip1;
            t[s + half + i] = cos_i * cos_ip1 - sin_i * sin_ip1;
        }
    }

}
