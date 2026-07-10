#pragma once
#include <string>
#include <cstdint>
#include <chains/arena.hpp>

namespace chains {
    std::string to_string(const Arena& a, uint32_t id);
}
