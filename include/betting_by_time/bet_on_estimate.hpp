#ifndef BETTING_BY_TIME_BET_ON_ESTIMATE_HPP
#define BETTING_BY_TIME_BET_ON_ESTIMATE_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/bet_once.hpp"
#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"
#include <algorithm>
#include <cmath>
#include <type_traits>

namespace betting {

/**
 * @brief Updates cumulative capitals for a gambler at a specific mean hypothesis.
 * 
 * This function advances the capital process for a single hypothesis m,
 * updating both positive and negative twin capitals based on samples
 * that haven't been processed yet for this hypothesis.
 * 
 * @tparam CapitalProcess Type of capital process (GeoCheckingCapital or SequenceCheckingCapital)
 * @param gambler Reference to the capital process
 * @param m Hypothesized mean value
 * @param mi Index of the hypothesis in the grid
 * @param which Which twin(s) to update: 0=positive, 1=negative, 2=both
 * @return true if either capital exceeds threshold, false otherwise
 * 
 * @note This is an incremental update function that tracks position to avoid
 * reprocessing samples. Essential for adaptive betting strategies.
 */
template<typename CapitalProcess>
bool bet_on(CapitalProcess& gambler, Float32 m, Int32 mi, Int32 which = 2) {
    auto& cum_cap_twins = gambler.cum_cap_twins();
    auto& cum_cap_pos = gambler.cum_cap_pos();
    // Helper to convert scalar or map samples into an Eigen::Map<Vector32f>
    auto to_map = [&](auto &v) -> Eigen::Map<Vector32f> {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, Eigen::Map<Vector32f>>) {
            return v;
        } else {
            return Eigen::Map<Vector32f>(&v, 1);
        }
    };

    // Per-hypothesis accessors for positions and twin capitals
    auto old_pos = [&](Int32 j) -> Int32& { return cum_cap_pos(mi, j); };
    auto cum_cap_twin = [&](Int32 j) -> Float64& { return cum_cap_twins(mi, j); };
    Float32 trunc_scale = gambler.trunc_scale();
    Float32 threshold = gambler.threshold() * 2.0f;
    Int32 cap_size = gambler.s_ptr();
    
    if (which == 2) {
        // Update both twins
        
        // First, advance any lagged behind capital
        if (cum_cap_pos(mi, 0) != cum_cap_pos(mi, 1)) {
            // Determine which one is behind
            Int32 idx = (old_pos(0) < old_pos(1)) ? 0 : 1;
            Int32 fidx = idx ^ 1;  // XOR to get the other index
            
            Float64& cum_cap = cum_cap_twin(idx);
            
            // Process samples from old position to catch up
            for (Int32 i = old_pos(idx); i < old_pos(fidx); ++i) {
                auto sample_tmp = gambler.sample(i);
                auto sample_map = to_map(sample_tmp);
                Float32 capital = gambler.capitals()(i);

                cum_cap = geo_single_bet_on(trunc_scale, m,
                                           sample_map,
                                           cum_cap, capital);
            }
            
            cum_cap_twin(idx) = cum_cap;
        } else {
            // Both at same position
        }
        
        // Now advance both from the front position to current size
        Int32 fidx = (old_pos(0) == old_pos(1)) ? 0 : 
                     ((old_pos(0) < old_pos(1)) ? 1 : 0);
        
        Float64 cap_p = cum_cap_twin(0);
        Float64 cap_n = cum_cap_twin(1);
        
        for (Int32 i = old_pos(fidx); i < cap_size; ++i) {
            auto sample_tmp = gambler.sample(i);
            auto sample_map = to_map(sample_tmp);
            Float32 capital = gambler.capitals()(i);

            // Update positive twin (bet on mean > m)
            cap_p = geo_single_bet_on(trunc_scale, m,
                                     sample_map,
                                     cap_p, capital);

            // Update negative twin (bet on mean < m)
            cap_n = geo_single_bet_on(trunc_scale, m,
                                     sample_map,
                                     cap_n, -capital);
            
            // Early exit if threshold exceeded
            if (std::max(cap_p, cap_n) > threshold) {
                break;
            }
        }
        
        cum_cap_twin(0) = cap_p;
        cum_cap_twin(1) = cap_n;
        old_pos(0) = cap_size;
        old_pos(1) = cap_size;
        
        return std::max(cap_p, cap_n) > threshold;
        
    } else {
        // Update only one twin
        Float64& cum_cap = cum_cap_twin(which);
        
        for (Int32 i = old_pos(which); i < cap_size; ++i) {
            auto sample_tmp = gambler.sample(i);
            auto sample_map = to_map(sample_tmp);
            Float32 capital = gambler.capitals()(i);

            cum_cap = geo_single_bet_on(trunc_scale, m,
                                       sample_map,
                                       cum_cap,
                                       (which == 0) ? capital : -capital);
            
            // Early exit if threshold exceeded
            if (cum_cap > threshold) {
                break;
            }
        }
        
        cum_cap_twin(which) = cum_cap;
        old_pos(which) = cap_size;
        
        return cum_cap > threshold;
    }
}

/**
 * @brief Estimates the mean by finding the hypothesis with minimum capital in a range.
 * 
 * This function performs batch betting on all hypotheses in the range [l, u],
 * then returns the hypothesis with the minimum sum of twin capitals.
 * 
 * @tparam CapitalProcess Type of capital process
 * @param gambler Reference to the capital process
 * @param l Lower bound of search range
 * @param u Upper bound of search range
 * @return Estimated mean value
 * 
 * @note The estimate is the grid point with minimum capital, which corresponds
 * to the most likely true mean under the betting framework.
 */
template<typename CapitalProcess>
Float32 estimate(CapitalProcess& gambler, Float32 l, Float32 u) {
    // Clamp to valid range
    l = std::max(0.0f, l);
    u = std::min(1.0f, u);
    
    if (u < l) {
        // Invalid range
        return -1.0f;
    }
    
    Int32 grid_num = gambler.grid_num();
    Float32 stride = 1.0f / grid_num;
    
    if (u - l < stride) {
        // Range too small
        return l;
    }
    
    // Convert float bounds to integer grid indices (inclusive)
    Int32 li = static_cast<Int32>(std::ceil(l * grid_num));
    Int32 ui = static_cast<Int32>(std::floor(u * grid_num));

    if (ui < li) {
        return (l + u) / 2.0f;
    }

    // Batch bet on all hypotheses in range [li, ui]
    for (Int32 pi = li; pi <= ui; ++pi) {
        bet_on(gambler, static_cast<Float32>(pi) * stride, pi, 2);
    }

    // Find index with minimum sum of twin capitals
    auto& cum_cap_twins = gambler.cum_cap_twins();
    Vector64d sums = cum_cap_twins.block(li, 0, ui - li + 1, 2).rowwise().sum();

    Eigen::Index min_idx;
    sums.minCoeff(&min_idx);

    return static_cast<Float32>(li + min_idx) / static_cast<Float32>(grid_num);
}

/**
 * @brief Convenience overload for GeoCheckingCapital specifically.
 */
inline bool bet_on(GeoCheckingCapital& gambler, Float32 m, Int32 mi, Int32 which = 2) {
    return bet_on<GeoCheckingCapital>(gambler, m, mi, which);
}

/**
 * @brief Convenience overload for SequenceCheckingCapital specifically.
 */
inline bool bet_on(SequenceCheckingCapital& gambler, Float32 m, Int32 mi, Int32 which = 2) {
    return bet_on<SequenceCheckingCapital>(gambler, m, mi, which);
}

/**
 * @brief Convenience overload for GeoCheckingCapital estimate.
 */
inline Float32 estimate(GeoCheckingCapital& gambler, Float32 l, Float32 u) {
    return estimate<GeoCheckingCapital>(gambler, l, u);
}

/**
 * @brief Convenience overload for SequenceCheckingCapital estimate.
 */
inline Float32 estimate(SequenceCheckingCapital& gambler, Float32 l, Float32 u) {
    return estimate<SequenceCheckingCapital>(gambler, l, u);
}

} // namespace betting

#endif // BETTING_BY_TIME_BET_ON_ESTIMATE_HPP
