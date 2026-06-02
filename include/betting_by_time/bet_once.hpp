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
Float64 geo_single_bet_on(Float32 trunc_scale, Float32 m, 
                          const Vector32f& samples,
                          Float64 cum_cap, Float32 capital);

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
Float64 seq_single_bet_on(Float32 trunc_scale, Float32 m, 
                          Float32 sample,
                          Float64 cum_cap, Float32 capital);

} // namespace betting

#endif // BETTING_BY_TIME_BET_ONCE_HPP
