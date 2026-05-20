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
 * This class implements an adaptive betting strategy that:
 * 1. First detects which direction (upper/lower) to explore
 * 2. Then performs binary search to find tight bounds
 * 3. Finally estimates the mean within the refined interval
 * 
 * Unlike the function-based approach, this class supports incremental sample
 * feeding while maintaining state between calls.
 * 
 * @tparam CapitalProcess Type of capital process (GeoCheckingCapital or SequenceCheckingCapital)
 */
template<typename CapitalProcess>
class AdaptiveBetting {
public:
    /**
     * @brief Construct an AdaptiveBetting instance.
     * 
     * @param prior_mean Prior estimate of the mean
     * @param delta Confidence level parameter (smaller = more confident)
     * @param grid_num Number of grid points for hypothesis testing
     * @param gambler Reference to capital process instance
     */
    AdaptiveBetting(Float32 prior_mean, Float32 delta, Int32 grid_num, CapitalProcess& gambler)
        : gambler_(gambler),
          prior_mean_(prior_mean),
          delta_(delta),
          grid_num_(grid_num),
          stride_(1.0f / static_cast<Float32>(grid_num)),
          l_(std::max(0.0f, prior_mean - delta)),
          u_(std::min(1.0f, prior_mean + delta)),
          li_(static_cast<Int32>(std::round(l_ * grid_num))),
          ui_(static_cast<Int32>(std::round(u_ * grid_num))),
          touch_(Touch::None),
          samples_used_(0),
          phase_(Phase::DirectionDetection),
          estimated_mean_(prior_mean),
          finalized_(false) {}

    /**
     * @brief Submit a single sample for processing.
     * 
     * Processes the sample according to the current phase of the algorithm.
     * 
     * @param sample The sample to process
     */
    void submit_sample(Float32 sample) {
        submit_samples(Vector32f(sample));
    }

    /**
     * @brief Submit multiple samples for processing.
     * 
     * This method adds all samples to the gambler in batch, then processes
     * the algorithm logic in phases. This is more efficient than processing
     * samples one by one.
     * 
     * @param samples Vector of samples to process
     */
    void submit_samples(const Vector32f& samples) {
        if (finalized_) {
            return;
        }

        // Add all samples to the gambler in batch
        gambler_.add_sample(samples);
        switch (phase_) {
            case Phase::DirectionDetection:
                process_direction_detection();
                break;
            case Phase::BoundsRefinement:
                process_bounds_refinement();
                break;
            case Phase::FinalEstimation:
                process_final_estimation(); 
                break;
            default:
                break;
        }
    }

    /**
     * @brief Get the current lower bound of the confidence interval.
     */
    Float32 get_lower_bound() const { return l_; }

    /**
     * @brief Get the current upper bound of the confidence interval.
     */
    Float32 get_upper_bound() const { return u_; }

    /**
     * @brief Get the width of the current confidence interval.
     */
    Float32 get_interval_width() const { return u_ - l_; }

    /**
     * @brief Get the current estimated mean.
     */
    Float32 get_estimated_mean() const { return estimated_mean_; }

    /**
     * @brief Get the number of samples used so far.
     */
    Int32 get_samples_used() const { return samples_used_; }

    /**
     * @brief Check if the algorithm has completed.
     */
    bool is_finalized() const { return finalized_; }

    /**
     * @brief Get the current phase of the algorithm.
     */
    Int32 get_current_phase() const { return static_cast<Int32>(phase_); }

    /**
     * @brief Finalize the algorithm and compute the final estimate.
     * 
     * @return Pair of (estimated_mean, samples_used)
     */
    std::pair<Float32, Int32> finalize() {
        if (!finalized_) {
            // Ensure final estimation is done
            phase_ = Phase::FinalEstimation;
            process_final_estimation();
        }
        return std::make_pair(estimated_mean_, samples_used_);
    }

    /**
     * @brief Reset the state to initial conditions.
     * 
     * Allows reusing the same instance with the same parameters but fresh state.
     */
    void reset() {
        l_ = std::max(0.0f, prior_mean_ - delta_);
        u_ = std::min(1.0f, prior_mean_ + delta_);
        li_ = static_cast<Int32>(std::round(l_ * grid_num_));
        ui_ = static_cast<Int32>(std::round(u_ * grid_num_));
        touch_ = Touch::None;
        samples_used_ = 0;
        phase_ = Phase::DirectionDetection;
        estimated_mean_ = prior_mean_;
        finalized_ = false;
        // Note: gambler_ state is not reset as it's owned externally
    }

private:
    // Phases of the adaptive betting algorithm
    enum class Phase {
        DirectionDetection = 0,
        BoundsRefinement = 1,
        FinalEstimation = 2
    };

    CapitalProcess& gambler_;
    
    // Configuration parameters (immutable after construction)
    const Float32 prior_mean_;
    const Float32 delta_;
    const Int32 grid_num_;
    const Float32 stride_;
    
    // Current state
    Float32 l_;           // Lower bound
    Float32 u_;           // Upper bound
    Int32 li_;            // Lower bound index
    Int32 ui_;            // Upper bound index
    Touch touch_;         // Touch flags
    Int32 samples_used_;  // Number of samples processed
    Phase phase_;         // Current algorithm phase
    Float32 estimated_mean_;  // Current estimated mean
    bool finalized_;      // Whether algorithm has completed

    /**
     * @brief Process direction detection phase for single sample.
     */
    bool process_direction_detection() {
        // Test if we've touched either boundary
        if ((touch_ & Touch::Lower) == Touch::None) {
            if (bet_on(gambler_, l_, li_, 0)) {
                touch_ |= Touch::Lower;
            }
        }

        if ((touch_ & Touch::Upper) == Touch::None) {
            if (bet_on(gambler_, u_, ui_, 1)) {
                touch_ |= Touch::Upper;
            }
        }

        // If either boundary touched, move to refinement phase
        if (touch_ != Touch::None) {
            phase_ = Phase::BoundsRefinement;
        }

        return false;
    }


    /**
     * @brief Process bounds refinement phase for single sample.
     */
    bool process_bounds_refinement() {
        if (li_ + 1 >= ui_) {
            // Interval is already tight enough
            phase_ = Phase::FinalEstimation;
            return process_final_estimation();
        }

        Int32 mid = (li_ + ui_) / 2;
        Float32 m = mid * stride_;

        if (touch_ == Touch::Lower) {
            // Refining lower bound
            if (bet_on(gambler_, m, mid, 0)) {
                // Midpoint rejected as lower bound
                li_ = mid;
            } else {
                // Midpoint still plausible
                ui_ = mid;
            }
        } else if (touch_ == Touch::Upper) {
            // Refining upper bound
            if (bet_on(gambler_, m, mid, 1)) {
                // Midpoint rejected as upper bound
                ui_ = mid;
            } else {
                // Midpoint still plausible
                li_ = mid;
            }
        } else if (touch_ == Touch::Both) {
            // Both bounds touched - interval is already tight
            phase_ = Phase::FinalEstimation;
            return process_final_estimation();
        }

        // Update bounds based on indices
        if (touch_ == Touch::Lower) {
            l_ = ui_ * stride_;
        } else if (touch_ == Touch::Upper) {
            u_ = li_ * stride_;
        }

        return false;
    }


    /**
     * @brief Process final estimation phase.
     */
    bool process_final_estimation() {
        estimated_mean_ = estimate(gambler_, l_, u_);

        if (estimated_mean_ < 0.0f) {
            estimated_mean_ = (l_ + u_) / 2.0f;
        }

        finalized_ = true;
        return true;
    }
};

// Convenience typedefs
using GeoAdaptiveBetting = AdaptiveBetting<GeoCheckingCapital>;
using SequenceAdaptiveBetting = AdaptiveBetting<SequenceCheckingCapital>;

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
 * @param breakpoints Vector of indices indicating batch boundaries. Samples are submitted in batches:
 *                    [breakpoints[0], breakpoints[1]), [breakpoints[1], breakpoints[2]), ..., 
 *                    [breakpoints[n-2], breakpoints[n-1])
 *                    Example: for 10 samples with breakpoints [0, 3, 7, 10], 
 *                    submits samples [0,3), then [3,7), then [7,10)
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
                                            CapitalProcess& gambler,
                                            const std::vector<Int32>& breakpoints = {}) {
    AdaptiveBetting<CapitalProcess> ab(prior_mean, delta, grid_num, gambler);
    
    if (breakpoints.empty()) {
        // No breakpoints provided, submit all samples at once
        ab.submit_samples(samples);
    } else {
        // Submit samples in batches according to breakpoints
        for (size_t i = 0; i < breakpoints.size() - 1; ++i) {
            Int32 start = breakpoints[i];
            Int32 end = breakpoints[i + 1];
            
            // Validate indices
            if (start < 0 || end > static_cast<Int32>(samples.size()) || start >= end) {
                continue;
            }
            
            // Extract batch and submit
            Vector32f batch(samples.begin() + start, samples.begin() + end);
            ab.submit_samples(batch);
        }
    }
    
    return ab.finalize();
}

/**
 * @brief Convenience wrapper for GeoCheckingCapital.
 */
inline std::pair<Float32, Int32> adaptive_betting(const Vector32f& samples,
                                                   Float32 prior_mean,
                                                   Float32 delta,
                                                   Int32 grid_num,
                                                   GeoCheckingCapital& gambler,
                                                   const std::vector<Int32>& breakpoints = {}) {
    return adaptive_betting<GeoCheckingCapital>(samples, prior_mean, delta, 
                                                 grid_num, gambler, breakpoints);
}

/**
 * @brief Convenience wrapper for SequenceCheckingCapital.
 */
inline std::pair<Float32, Int32> adaptive_betting(const Vector32f& samples,
                                                   Float32 prior_mean,
                                                   Float32 delta,
                                                   Int32 grid_num,
                                                   SequenceCheckingCapital& gambler,
                                                   const std::vector<Int32>& breakpoints = {}) {
    return adaptive_betting<SequenceCheckingCapital>(samples, prior_mean, delta, 
                                                      grid_num, gambler, breakpoints);
}

} // namespace betting

#endif // BETTING_BY_TIME_STRATEGIES_ADAPTIVE_BETTING_HPP
