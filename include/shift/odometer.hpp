#pragma once

#include <cstdint>
#include <vector>

#include "plan.hpp"
#include "kernels.hpp"

namespace chains {


    struct OdometerKernel {
        struct Step { OpKind kind; uint32_t start; uint32_t length; };
        uint8_t axis = 0;
        std::vector<Step> steps;
    };

    inline OdometerKernel build_odometer(const ShiftPlan& p, uint8_t axis) {
        OdometerKernel k;
        k.axis = axis;
        if (axis >= p.programs.size()) return k;
        for (const Op& op : p.programs[axis]) {
            if (op.op == OpKind::Fetch) continue; 
            k.steps.push_back({op.op, op.start, op.length});
        }
        return k;
    }

    inline void shift_odometer(ShiftPlan& p, const OdometerKernel& k) {
        double* t = p.tape.data();
        for (const auto& step : k.steps) {
            switch (step.kind) {
                case OpKind::Sum: kernel::run_sum (t, step.start, step.length); break;
                case OpKind::Prod: kernel::run_prod(t, step.start, step.length); break;
                case OpKind::Trig: kernel::run_trig(t, step.start, step.length); break;
                case OpKind::Fetch: break;
            }
        }
    }

}
