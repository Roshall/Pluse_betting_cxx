#ifndef BETTING_BY_TIME_BET_ONCE_HPP
#define BETTING_BY_TIME_BET_ONCE_HPP

#include "betting_by_time/core/types.hpp"
#include <algorithm>
#include <cmath>
#include <cassert>

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
    const auto m_f64 = static_cast<Float64>(m);
    const Float64 lower_bound =
        -static_cast<Float64>(trunc_scale) / (1.0 + 1e-9 - m_f64);
    const Float64 upper_bound =
        static_cast<Float64>(trunc_scale) / (m_f64 + 1e-9);
    const Float64 bet = std::clamp(static_cast<Float64>(capital), lower_bound, upper_bound);
    
    // Vectorized computation: product of (1 + (samples - m) * bet)
    const Float64 product =
        (1.0 + (samples.cast<Float64>().array() - m_f64) * bet).prod();
    assert(product >= 0.0 && "Product of earnings should be non-negative");
    
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



    const auto sample_f64 = static_cast<Float64>(sample);
    const auto m_f64 = static_cast<Float64>(m);

    if (std::abs(sample_f64 - m_f64) < 1e-9 || std::abs(cum_cap) < 1e-16) {
        return cum_cap;
    }

    const auto trunc_f64 = static_cast<Float64>(trunc_scale);
    const auto lower_bound = -trunc_f64 / (1.0 + 1e-9 - m_f64);
    const auto upper_bound = trunc_f64 / (m_f64 + 1e-9);

    const auto lbd_m = std::clamp(static_cast<Float64>(capital),
                                  lower_bound,
                                  upper_bound);

    Float64 earn = 1.0 + lbd_m * (sample_f64 - m_f64);

    if (earn < 0.0) {
        earn = 0.5;
    }

    return cum_cap * earn;
}

} // namespace betting

#endif // BETTING_BY_TIME_BET_ONCE_HPP
