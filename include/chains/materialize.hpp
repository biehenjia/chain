#pragma once
#include <vector>
#include <cstdint>
#include <chains/access.hpp>

namespace chains {

    class Arena;

    struct Materialized {
        std::vector<Symbolic> values;
    };

    Materialized materialize(const Arena& a, uint32_t root);

}
