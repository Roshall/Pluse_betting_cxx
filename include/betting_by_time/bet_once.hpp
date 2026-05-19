#ifndef BETTING_BY_TIME_BET_ONCE_HPP
#define BETTING_BY_TIME_BET_ONCE_HPP

#include "betting_by_time/core/types.hpp"
#include <algorithm>
#include <cmath>

namespace betting {

/**
 * @brief Performs a single geometric betting operation on a sample batch.
 * 
 * @param trunc_scale Truncation scale for bet limiting
 * @param m Hypothesized mean value
 * @param samples Eigen vector of samples to bet on
 * @param cum_cap Current cumulative capital
 * @param capital Bet amount
 * @return Updated cumulative capital
 * 
 * @note Uses Eigen's vectorized operations for efficient computation
 */
inline Float64 geo_single_bet_on(Float32 trunc_scale, Float32 m, 
                                  const Vector32f& samples,
                                  Float64 cum_cap, Float32 capital) {
    if (cum_cap < 1e-16) return cum_cap;
    
    // Compute bet limits
    Float32 lower_bound = -trunc_scale / (1.0f + 1e-9f - m);
    Float32 upper_bound = trunc_scale / (m + 1e-9f);
    Float32 bet = std::clamp(capital, lower_bound, upper_bound);
    
    // Vectorized computation: product of (1 + (samples - m) * bet)
    Float64 product = static_cast<Float64>(
        (1.0f + (samples.array() - m) * bet).prod()
    );
    
    return cum_cap * product;
}

/**
 * @brief Performs a single sequence betting operation.
 * 
 * @param trunc_scale Truncation scale for bet limiting
 * @param m Hypothesized mean value
 * @param sample Single sample value
 * @param cum_cap Current cumulative capital
 * @param capital Bet amount
 * @return Updated cumulative capital
 */
inline Float64 seq_single_bet_on(Float32 trunc_scale, Float32 m, 
                                  Float32 sample,
                                  Float64 cum_cap, Float32 capital) {
    sample = std::clamp(sample, 0.0f, 1.0f);
    
    if (std::abs(sample - m) < 1e-9f || std::abs(cum_cap) < 1e-16) {
        return cum_cap;
    }
    
    Float32 lbd_m = std::clamp(capital,
                               -trunc_scale / (1.0f + 1e-9f - m),
                               trunc_scale / (m + 1e-9f));
    
    Float64 earn = 1.0 + static_cast<Float64>(lbd_m) * static_cast<Float64>(sample - m);
    
    if (earn < 0.0) {
        earn = 0.5;
    }
    
    return cum_cap * earn;
}

} // namespace betting

#endif // BETTING_BY_TIME_BET_ONCE_HPP
