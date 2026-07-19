// archived, use odometer now

#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

#include <core/core.hpp>
#include "plan.hpp"
#include "kernels.hpp"

namespace chains {

    inline void reset(ShiftPlan& p, uint8_t axis) {
        if (p.reset_programs.empty()) return;
        if (axis >= p.reset_programs.size()) return;
        double* t = p.tape.data();
        const double* base = p.baseline.data();
        uint8_t last = uint8_t(p.reset_programs.size() - 1);
        for (uint8_t a = axis; a <= last; ++a) {
            const ResetProgram& rp = p.reset_programs[a];
            for (auto [s, l] : rp.baseline_ranges) {
                std::memcpy(t + s, base + s, l * sizeof(double));
            }
            for (const Op& f : rp.fetches) {
                t[f.start] = access(f.src_kind, t, f.src, f.length);
            }
        }
    }

    inline void refresh(ShiftPlan& p, uint8_t axis) {
        if (axis >= p.refresh_programs.size()) return;
        double* t = p.tape.data();
        for (const Op& f : p.refresh_programs[axis]) {
            t[f.start] = access(f.src_kind, t, f.src, f.length);
        }
    }

    inline void shift(ShiftPlan& p, uint8_t axis) {
        if (axis >= p.programs.size()) return;
        double* t = p.tape.data();
        for (const Op& op : p.programs[axis]) {
            switch (op.op) {
                case OpKind::Sum: kernel::run_sum (t, op.start, op.length); break;
                case OpKind::Prod: kernel::run_prod(t, op.start, op.length); break;
                case OpKind::Trig: kernel::run_trig(t, op.start, op.length); break;
                case OpKind::Fetch: break; // NOT USED ANYMORE! 
            }
        }
    }

}
