#ifndef BETTING_BY_TIME_STRATEGIES_VANILLA_BETTING_HPP
#define BETTING_BY_TIME_STRATEGIES_VANILLA_BETTING_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/core/utilities.hpp"
#include "betting_by_time/capital/geo_checking.hpp"

namespace betting {

/**
 * @brief Vanilla betting strategy implementation.
 * 
 * This strategy tests all hypotheses simultaneously and narrows down
 * the confidence set by eliminating hypotheses whose capital exceeds threshold.
 * 
 * @param samples Input sample array
 * @param prior_mean Prior estimate of mean (unused in vanilla)
 * @param delta Confidence parameter
 * @param grid_num Number of grid points
 * @param gambler Reference to capital process (GeoCheckingCapital or SequenceCheckingCapital)
 * @return Pair of (estimated_mean, num_samples_used)
 */
template<typename CapitalProcess>
std::pair<Float32, Int32> vanilla_betting(const Vector32f& samples,
                                           Float32 prior_mean,
                                           Float32 delta,
                                           Int32 grid_num,
                                           CapitalProcess& gambler) {
    // Generate linspace of possible means
    Vector32f m_possible = linspace(0.0f, 1.0f, grid_num + 1);
    
    Float32 eps = delta * 2.0f - 1.0f / grid_num;
    Array2f cs_bound;
    cs_bound << 0.0f, 1.0f;
    
    Float32 threshold = gambler.threshold() * 2.0f;
    
    // Create index array for active hypotheses
    Vector32i rang_idx = Vector32i::LinSpaced(grid_num + 1, 0, grid_num);
    
    Int32 i = 0;
    for (i = 0; i < samples.size(); ++i) {
        // Advance gambler with current sample and full hypothesis grid
        // (advance implementations expect row indices matching grid rows)
        gambler.advance(samples(i), m_possible);
        
        // Find hypotheses still below threshold
        Vector64d cap_sums = gambler.cum_cap_twins().rowwise().sum();
        
        std::vector<Int32> new_indices;
        for (Int32 j = 0; j < cap_sums.size(); ++j) {
            if (cap_sums(j) < threshold) {
                new_indices.push_back(j);
            }
        }
        
        if (new_indices.empty()) break;
        
        rang_idx.resize(new_indices.size());
        for (size_t j = 0; j < new_indices.size(); ++j) {
            rang_idx(j) = new_indices[j];
        }
        
        // Update confidence set bounds
        intersect(cs_bound, m_possible(rang_idx(0)), m_possible(rang_idx(rang_idx.size() - 1)));
        
        // Check if bound is tight enough
        if (cs_bound(1) - cs_bound(0) < eps) {
            break;
        }
    }
    
    // Find hypothesis with minimum capital
    Vector64d final_sums = gambler.cum_cap_twins().rowwise().sum();
    Int32 min_idx = argmin(final_sums);

    Int32 used = (i < samples.size()) ? (i + 1) : i;
    return std::make_pair(static_cast<Float32>(min_idx) / grid_num, used);
}

} // namespace betting

#endif // BETTING_BY_TIME_STRATEGIES_VANILLA_BETTING_HPP
