#include "betting_by_time/core/utilities.hpp"

// This file is intentionally minimal as most utilities are header-only templates
// and inline functions for performance optimization.

namespace betting {
    // Explicit template instantiations if needed for faster compilation
    template Float32 cal_c<Float32>(Float32);
    template Float64 cal_c<Float64>(Float64);
}
