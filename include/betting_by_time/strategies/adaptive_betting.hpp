#ifndef BETTING_BY_TIME_STRATEGIES_ADAPTIVE_BETTING_HPP
#define BETTING_BY_TIME_STRATEGIES_ADAPTIVE_BETTING_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/bet_on_estimate.hpp"
#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"
#include <cmath>
#include <algorithm>

namespace betting {

// Touch flags for adaptive betting direction detection
enum class Touch : Int32 { None = 0, Lower = 1, Upper = 2, Both = 3 };

inline Touch operator|(Touch a, Touch b) {
    return static_cast<Touch>(static_cast<Int32>(a) | static_cast<Int32>(b));
}

inline Touch& operator|=(Touch& a, Touch b) {
    a = a | b;
    return a;
}

inline Touch operator&(Touch a, Touch b) {
    return static_cast<Touch>(static_cast<Int32>(a) & static_cast<Int32>(b));
}

/**
 * @brief Adaptive betting strategy with dynamic confidence interval refinement.
 * 
 * This strategy adaptively narrows down the confidence interval by:
 * 1. First detecting which direction (upper/lower) to explore
 * 2. Then performing binary search to find tight bounds
 * 3. Finally estimating the mean within the refined interval
 * 
 * @tparam CapitalProcess Type of capital process (GeoCheckingCapital or SequenceCheckingCapital)
 * @param samples Input data samples
 * @param prior_mean Prior estimate of the mean
 * @param delta Confidence level parameter (smaller = more confident)
 * @param grid_num Number of grid points for hypothesis testing
 * @param gambler Reference to capital process instance
 * @return Pair of (estimated_mean, samples_used)
 * 
 * @note This is more sample-efficient than vanilla betting but requires
 * careful implementation to maintain statistical guarantees.
 */
template<typename CapitalProcess>
std::pair<Float32, Int32> adaptive_betting(const Vector32f& samples,
                                            Float32 prior_mean,
                                            Float32 delta,
                                            Int32 grid_num,
                                            CapitalProcess& gambler) {
    // Initialize parameters
    Float32 stride = 1.0f / static_cast<Float32>(grid_num);
    
    // Initial confidence interval around prior
    Float32 l = std::max(0.0f, prior_mean - delta);
    Float32 u = std::min(1.0f, prior_mean + delta);
    
    // Convert to indices
    Int32 li = static_cast<Int32>(std::round(l * grid_num));
    Int32 ui = static_cast<Int32>(std::round(u * grid_num));
    
    // Touch flags: bit 0 = lower touched, bit 1 = upper touched
    Touch touch = Touch::None;
    
    // Phase 1: Detect direction by advancing until one boundary is touched
    Int32 i = 0;
    while (i < samples.size()) {
        gambler.add_sample(samples(i++));
        
        // Test if we've touched either boundary
        if ((touch & Touch::Lower) == Touch::None) {
            // Check lower bound
            if (bet_on(gambler, l, li, 0)) {
                touch |= Touch::Lower;  // Mark lower as touched
            }
        }

        if ((touch & Touch::Upper) == Touch::None) {
            // Check upper bound
            if (bet_on(gambler, u, ui, 1)) {
                touch |= Touch::Upper;  // Mark upper as touched
            }
        }
        
        // If both touched or one touched, we can proceed
        if (touch != Touch::None) {
            break;
        }
    }
    
    // Handle edge cases
    if (i >= samples.size()) {
        // Ran out of samples without touching anything
        return std::make_pair(prior_mean, static_cast<Int32>(samples.size()));
    }
    
    // Phase 2: Refine bounds based on which direction was detected
    if (touch == Touch::Lower) {
        // Only lower bound touched - need to find new lower bound
        
        // Binary search for lower bound
        while (li + 1 < ui) {
            Int32 mid = (li + ui) / 2;
            Float32 m = mid * stride;
            
            // Continue adding samples during search
            bool found_mid = false;
            while (i < samples.size()) {
                gambler.add_sample(samples(i++));
                
                if (bet_on(gambler, m, mid, 0)) {
                    // Midpoint rejected as lower bound
                    li = mid;
                    found_mid = true;
                    break;
                } else {
                    // Midpoint still plausible
                    ui = mid;
                    found_mid = true;
                    break;
                }
            }
            
            if (!found_mid) {
                // Ran out of samples
                break;
            }
        }
        
        // Update lower bound
        l = ui * stride;
        
    } else if (touch == Touch::Upper) {
        // Only upper bound touched - need to find new upper bound
        
        // Binary search for upper bound
        while (li + 1 < ui) {
            Int32 mid = (li + ui) / 2;
            Float32 m = mid * stride;
            
            // Continue adding samples during search
            bool found_mid = false;
            while (i < samples.size()) {
                gambler.add_sample(samples(i++));
                
                if (bet_on(gambler, m, mid, 1)) {
                    // Midpoint rejected as upper bound
                    ui = mid;
                    found_mid = true;
                    break;
                } else {
                    // Midpoint still plausible
                    li = mid;
                    found_mid = true;
                    break;
                }
            }
            
            if (!found_mid) {
                // Ran out of samples
                break;
            }
        }
        
        // Update upper bound
        u = li * stride;
        
    } else if (touch == Touch::Both) {
        // Both bounds touched - interval is already tight
        // Could perform additional refinement here if needed
    }
    
    // Phase 3: Final estimation within refined interval
    // Use remaining samples for final estimate
    while (i < samples.size()) {
        gambler.add_sample(samples(i++));
    }
    
    // Get final estimate using the estimate function
    Float32 estimated_mean = estimate(gambler, l, u);
    
    // Fallback if estimate fails
    if (estimated_mean < 0.0f) {
        estimated_mean = (l + u) / 2.0f;
    }
    
    Int32 samples_used = gambler.s_ptr();
    
    return std::make_pair(estimated_mean, samples_used);
}

/**
 * @brief Convenience wrapper for GeoCheckingCapital.
 */
inline std::pair<Float32, Int32> adaptive_betting(const Vector32f& samples,
                                                   Float32 prior_mean,
                                                   Float32 delta,
                                                   Int32 grid_num,
                                                   GeoCheckingCapital& gambler) {
    return adaptive_betting<GeoCheckingCapital>(samples, prior_mean, delta, 
                                                 grid_num, gambler);
}

/**
 * @brief Convenience wrapper for SequenceCheckingCapital.
 */
inline std::pair<Float32, Int32> adaptive_betting(const Vector32f& samples,
                                                   Float32 prior_mean,
                                                   Float32 delta,
                                                   Int32 grid_num,
                                                   SequenceCheckingCapital& gambler) {
    return adaptive_betting<SequenceCheckingCapital>(samples, prior_mean, delta, 
                                                      grid_num, gambler);
}

} // namespace betting

#endif // BETTING_BY_TIME_STRATEGIES_ADAPTIVE_BETTING_HPP
