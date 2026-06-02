#include "betting_by_time/core/utilities.hpp"

// This file is intentionally minimal as most utilities are header-only templates
// and inline functions for performance optimization.

namespace betting {
Vector32i flatnonzero(const Vector32i& vec) {
    std::vector<Int32> indices;
    indices.reserve(vec.size());

    for (Int32 i = 0; i < vec.size(); ++i) {
        if (vec(i) != 0) {
            indices.push_back(i);
        }
    }

    return Eigen::Map<Vector32i>(indices.data(), indices.size()).eval();
}
Vector32i gen_times(Float32 start, Float32 base, Float32 end) {
    Float32 log_start = std::log(start);
    Float32 log_end = std::log(end);
    Float32 log_base = std::log(base);
    
    Int32 i_start = static_cast<Int32>(std::ceil(log_start / log_base));
    Int32 i_end = static_cast<Int32>(std::floor(log_end / log_base));
    
    Int32 num_points = i_end - i_start + 1;
    std::vector<Int32> times_vec;
    times_vec.reserve(num_points + 2);
    
    times_vec.push_back(0);
    
    for (Int32 i = i_start; i <= i_end; ++i) {
        times_vec.push_back(static_cast<Int32>(std::ceil(std::pow(base, i))));
    }
    
    times_vec.push_back(static_cast<Int32>(end));
    
    // Sort and remove duplicates
    std::sort(times_vec.begin(), times_vec.end());
    auto last = std::unique(times_vec.begin(), times_vec.end());
    times_vec.erase(last, times_vec.end());
    
    return Eigen::Map<Vector32i>(times_vec.data(), times_vec.size());
}

}
