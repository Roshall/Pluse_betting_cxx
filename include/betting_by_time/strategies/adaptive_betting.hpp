#ifndef BETTING_BY_TIME_STRATEGIES_ADAPTIVE_BETTING_HPP
#define BETTING_BY_TIME_STRATEGIES_ADAPTIVE_BETTING_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/bet_on_estimate.hpp"
#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"
#include <cmath>
#include <algorithm>

namespace betting {



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
     * @brief Construct an AdaptiveBetting instance with gambler parameters.
     * 
     * @param prior_mean Prior estimate of the mean (also used as gambler prior mean)
     * @param delta Confidence level parameter (smaller = more confident)
     * @param grid_num Number of grid points for hypothesis testing
     * @param gambler_alpha Delta parameter for gambler (default: 0.05f)
     * @param gambler_trunc_scale Truncation scale for gambler (default: 0.5f)
     * @param gambler_prior_var Prior variance for gambler (default: 0.25f)
     * @param gambler_num Initial time step for gambler (default: 1)
     * @param gambler_sample_num Pre-allocated sample capacity for gambler (default: 100100)
     */
    AdaptiveBetting(Float32 prior_mean, Float32 delta, Int32 grid_num,
                    Float32 gambler_alpha = 0.05f,
                    Float32 gambler_trunc_scale = 0.5f,
                    Float32 gambler_prior_var = 0.25f,
                    Int32 gambler_num = 1,
                  Int32 gambler_sample_num = 100100,
                  Mode mode = Mode::Estimate)
        : gambler_(gambler_alpha, gambler_trunc_scale, grid_num, prior_mean,
                   gambler_prior_var, gambler_num, gambler_sample_num),
          prior_mean_(prior_mean),
          delta_(delta),
          grid_num_(grid_num),
          stride_(1.0f / static_cast<Float32>(grid_num)),
            win_(2.0f * delta),
            mode_(mode)
    {
        Float32 l = std::min(std::max(prior_mean - delta, 0.0f), 1.0f - 2.0f * delta);
        l_ = l;
        u_ = l + 2.0f * delta;
        li_ = static_cast<Int32>(std::round(l_ * grid_num));
        ui_ = static_cast<Int32>(std::round(u_ * grid_num));
    }

    /**
     * @brief Submit a single sample for processing.
     * 
     * Processes the sample according to the current phase of the algorithm.
     * 
     * @param sample The sample to process
     */
    void submit_sample(Float32 sample) {
        Vector32f sample_vec(1);
        sample_vec(0) = sample;
        submit_samples(sample_vec);
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
    Int32 get_samples_used() const { return gambler_.used_sample_num(); }

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
        return std::make_pair(estimated_mean_, gambler_.s_ptr());
    }

    /**
     * @brief Reset the state to initial conditions.
     * 
     * Allows reusing the same instance with the same parameters but fresh state.
     * 
     * @param prior_mean Prior mean for gambler
     * @param prior_var Prior variance for gambler
     * @param prior_num Initial time step for gambler
     */
    void reset(Float32 prior_mean, Float32 prior_var, Int32 prior_num) {
        Float32 l = std::min(std::max(prior_mean - delta_, 0.0f), 1.0f - 2.0f * delta_);
        l_ = l;
        u_ = l + 2.0f * delta_;
        li_ = static_cast<Int32>(std::round(l_ * grid_num_));
        ui_ = static_cast<Int32>(std::round(u_ * grid_num_));
        touch_ = Touch::None;
        phase_ = Phase::DirectionDetection;
        s2_is_up_ = false;
        estimated_mean_ = prior_mean;
        finalized_ = false;
        gambler_.reset(prior_mean, prior_var, prior_num);
    }

private:
    // Phases of the adaptive betting algorithm
    enum class Phase {
        DirectionDetection,
        BoundsRefinement,
        FinalEstimation
    };

    CapitalProcess gambler_;  // Internally owned gambler
    
    // Configuration parameters (immutable after construction)
    const Float32 prior_mean_;
    const Float32 delta_;
    const Int32 grid_num_;
    const Float32 stride_;
    const Float32 win_;
    
    // Current state
    Float32 l_;           // Lower bound
    Float32 u_;           // Upper bound
    Int32 li_;            // Lower bound index
    Int32 ui_;            // Upper bound index
    enum class Touch : Int32 {
        None = -1,
        Lower = 0,
        Upper = 1,
        Both = 2
    };
    Touch touch_ = Touch::None;    // Touch state for direction detection
    Phase phase_ = Phase::DirectionDetection;
    bool s2_is_up_ = false;  // Direction for state 2 refinement
    Float32 estimated_mean_ = 0.0f;
    bool finalized_ = false;
    const Mode mode_;

    /**
     * @brief Perform binary search to find a tight bound.
     * 
     * @param low_idx Lower grid index
     * @param high_idx Upper grid index
     * @param twin_idx Which twin to test (0 for lower bound, 1 for upper bound)
     * @return Tightest index found
     */
    Int32 binary_search_bound(Int32 low_idx, Int32 high_idx, Int32 twin_idx) {
        while (low_idx + 1 < high_idx) {
            Int32 mid = (low_idx + high_idx) / 2;
            if (bet_on(gambler_, mid * stride_, mid, twin_idx)) {
                if (twin_idx == 0) {
                    low_idx = mid;
                } else {
                    high_idx = mid;
                }
            } else {
                if (twin_idx == 0) {
                    high_idx = mid;
                } else {
                    low_idx = mid;
                }
            }
        }
        return (twin_idx == 0) ? high_idx : low_idx;
    }

    /**
     * @brief Resolve overshoot where both bounds are rejected in the same direction.
     * 
     * @param which Direction of overshoot (0=below mean, 1=above mean)
     */
    void resolve_overshoot(Int32 which) {
        touch_ = static_cast<Touch>(which);
        Float32 width = win_ + stride_;
        Int32 w_stride = static_cast<Int32>(std::round(width * grid_num_));

        if (which == 1) {
            // Both above the mean, slide down
            li_ = static_cast<Int32>(std::round(l_ * grid_num_));
            while (l_ >= 0.0f) {
                l_ -= width;
                li_ -= w_stride;
                if (!bet_on(gambler_, l_, li_, 1)) {
                    if (!bet_on(gambler_, l_, li_, 0)) {
                        touch_ = Touch::Upper;
                    }
                    break;
                }
            }
            if (l_ < 0.0f) {
                l_ = 0.0f;
            }
            u_ = std::min(l_ + width, 1.0f);
        } else {
            // Both below the mean, slide up
            ui_ = static_cast<Int32>(std::round(u_ * grid_num_));
            while (u_ <= 1.0f) {
                u_ += width;
                ui_ += w_stride;
                if (!bet_on(gambler_, u_, ui_, 0)) {
                    if (!bet_on(gambler_, u_, ui_, 1)) {
                        touch_ = Touch::Lower;
                    }
                    break;
                }
            }
            if (u_ > 1.0f) {
                u_ = 1.0f;
            }
            l_ = std::max(u_ - width, 0.0f);
        }
    }

    /**
     * @brief State 1: direction detection with bounds refinement.
     * 
     * Tests both the lower and upper bounds with both capital twins.
     * On detection, immediately performs binary search to nail down the 
     * tight bound, then switches to state 2.
     * 
     * Matches Python's advance_betting_s1 logic.
     */
    void process_direction_detection() {
        // Test both bounds with both twins simultaneously (which=2)
        if (bet_on(gambler_, l_, li_, 2)) {
            touch_ = Touch::Lower;
        }
        if (bet_on(gambler_, u_, ui_, 2)) {
            touch_ = (touch_ == Touch::Lower) ? Touch::Both : Touch::Upper;
        }

        if (touch_ == Touch::None) {
            return;  // No direction detected yet, need more samples
        }

        if (touch_ == Touch::Both) {
            // Both bounds touched simultaneously - double-check for overshoot
            auto& cct = gambler_.cum_cap_twins();
            Float64 cap_l_pos = cct(li_, 0);
            Float64 cap_l_neg = cct(li_, 1);
            Int32 which_l = (cap_l_pos > cap_l_neg) ? 0 : 1;
            Float64 cap_u_pos = cct(ui_, 0);
            Float64 cap_u_neg = cct(ui_, 1);
            Int32 which_u = (cap_u_pos > cap_u_neg) ? 0 : 1;

            if (which_l == which_u) {
                // Overshoot: both rejected in same direction, slide window
                resolve_overshoot(which_l);

                if (touch_ == Touch::Both) {
                    if (mode_ == Mode::Estimate) {
                        phase_ = Phase::FinalEstimation;
                        process_final_estimation();
                    } else {
                        s2_is_up_ = (which_l == 0);
                        phase_ = Phase::BoundsRefinement;
                    }
                    return;
                }
            }
        }

        if (touch_ == Touch::Lower || touch_ == Touch::Upper) {
            Int32 twin_idx = static_cast<Int32>(touch_);
            li_ = static_cast<Int32>(std::round(l_ * grid_num_));
            ui_ = static_cast<Int32>(std::round(u_ * grid_num_));
            Int32 target_idx = binary_search_bound(li_, ui_, twin_idx);
            
            if (twin_idx == 0) {
                // Lower bound rejected, binary search for tight lower bound
                l_ = target_idx * stride_;
                u_ = std::min(l_ + win_, 1.0f);
                if (u_ == 1.0f || bet_on(gambler_, u_, static_cast<Int32>(std::round(u_ * grid_num_)), 1)) {
                    touch_ = Touch::Both;
                    if (mode_ == Mode::Estimate) {
                        phase_ = Phase::FinalEstimation;
                        process_final_estimation();
                    } else {
                        s2_is_up_ = true;
                        phase_ = Phase::BoundsRefinement;
                    }
                    return;
                }
            } else {
                // Upper bound rejected, binary search for tight upper bound
                u_ = std::min(target_idx * stride_, 1.0f);
                l_ = std::max(u_ - win_, 0.0f);
                if (l_ == 0.0f || bet_on(gambler_, l_, static_cast<Int32>(std::round(l_ * grid_num_)), 0)) {
                    touch_ = Touch::Both;
                    if (mode_ == Mode::Estimate) {
                        phase_ = Phase::FinalEstimation;
                        process_final_estimation();
                    } else {
                        s2_is_up_ = false;
                        phase_ = Phase::BoundsRefinement;
                    }
                    return;
                }
            }
        }

        // Move to state 2
        s2_is_up_ = (touch_ == Touch::Lower);
        phase_ = Phase::BoundsRefinement;
    }

    /**
     * @brief Refine the opposite bound sequentially.
     * 
     * @param refine_up If true, refine upper bound sequentially. If false, refine lower bound sequentially.
     */
    void refine_bound_sequentially(bool refine_up) {
        Int32 twin_idx = refine_up ? 0 : 1;
        Float32& bound = refine_up ? l_ : u_;
        Int32 mi = static_cast<Int32>(std::round(bound * grid_num_));
        
        while ((refine_up ? bound < 1.0f - win_ : bound > win_) && bet_on(gambler_, bound, mi, twin_idx)) {
            if (refine_up) {
                bound += stride_;
                mi += 1;
            } else {
                bound -= stride_;
                mi -= 1;
            }
        }
        if (refine_up) {
            l_ = bound;
            u_ = std::min(l_ + win_, 1.0f);
        } else {
            u_ = bound;
            l_ = std::max(u_ - win_, 0.0f);
        }
            
    }

    /**
     * @brief Find terminal bound by sliding window in a direction.
     * 
     * @param slide_up If true, find upper bound by sliding up (State 2 up).
     *                 If false, find lower bound by sliding down (State 2 down).
     * @return true if finalized.
     */
    bool process_bounds_refinement_dir(bool slide_up) {
        Float32 width = win_ + stride_;
        Int32 i_stride = static_cast<Int32>(std::round(width / stride_));
        Int32 twin_idx = slide_up ? 0 : 1;

        if (slide_up) {
            ui_ = static_cast<Int32>(std::round(u_ / stride_));
            while (u_ <= 1.0f) {
                if (!bet_on(gambler_, u_, ui_, twin_idx)) {
                    l_ = std::max(u_ - win_, 0.0f);
                    break;
                }
                u_ += width;
                ui_ += i_stride;
            }
            if (u_ >= 1.0f) {
                u_ = 1.0f;
                l_ = std::max(u_ - width, 0.0f);
                return true;
            }
            refine_bound_sequentially(true);
            return u_ == 1.0f || bet_on(gambler_, u_, static_cast<Int32>(std::round(u_ * grid_num_)), 1);
        } else {
            li_ = static_cast<Int32>(std::round(l_ / stride_));
            while (l_ >= 0.0f) {
                if (!bet_on(gambler_, l_, li_, twin_idx)) {
                    u_ = std::min(l_ + win_, 1.0f);
                    break;
                }
                l_ -= width;
                li_ -= i_stride;
            }
            if (l_ <= 0.0f) {
                l_ = 0.0f;
                u_ = std::min(l_ + width, 1.0f);
                return true;
            } 
            refine_bound_sequentially(false);
            return l_ == 0.0f || bet_on(gambler_, l_, static_cast<Int32>(std::round(l_ * grid_num_)), 0);
        }
    }

    /**
     * @brief Process bounds refinement phase (state 2).
     */
    void process_bounds_refinement() {
        if (process_bounds_refinement_dir(s2_is_up_) && mode_ == Mode::Estimate) {
            phase_ = Phase::FinalEstimation;
            process_final_estimation();
        }
    }

    /**
     * @brief Process final estimation phase.
     */
    bool process_final_estimation() {
        if (mode_ == Mode::Estimate) {
            estimated_mean_ = estimate(gambler_, l_, u_);

            if (estimated_mean_ < 0.0f) {
                estimated_mean_ = (l_ + u_) / 2.0f;
            }
        } else {
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
 * @param breakpoints Vector of indices indicating batch boundaries. Samples are submitted in batches:
 *                    [breakpoints[0], breakpoints[1]), [breakpoints[1], breakpoints[2]), ..., 
 *                    [breakpoints[n-2], breakpoints[n-1])
 *                    Example: for 10 samples with breakpoints [0, 3, 7, 10], 
 *                    submits samples [0,3), then [3,7), then [7,10)
 * @param gambler_alpha Delta parameter for gambler (default: 0.05f)
 * @param gambler_trunc_scale Truncation scale for gambler (default: 0.5f)
 * @param gambler_prior_mean Prior mean for gambler (default: 0.5f)
 * @param gambler_prior_var Prior variance for gambler (default: 0.25f)
 * @param gambler_num Initial time step for gambler (default: 1)
 * @param gambler_sample_num Pre-allocated sample capacity for gambler (default: 100100)
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
                                           const std::vector<Int32>& breakpoints = {},
                                           Float32 gambler_alpha = 0.05f,
                                           Float32 gambler_trunc_scale = 0.5f,
                                           Float32 gambler_prior_var = 0.25f,
                                           Int32 gambler_num = 1,
                                           Int32 gambler_sample_num = 100100,
                                           Mode mode = Mode::Estimate) {
    AdaptiveBetting<CapitalProcess> ab(prior_mean, delta, grid_num, 
                                       gambler_alpha, gambler_trunc_scale,
                                       gambler_prior_var, gambler_num, gambler_sample_num, mode);
    
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
            Vector32f batch = samples.segment(start, end - start);
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
                                                  const std::vector<Int32>& breakpoints = {},
                                                  Float32 gambler_alpha = 0.05f,
                                                  Float32 gambler_trunc_scale = 0.5f,
                                                  Float32 gambler_prior_var = 0.25f,
                                                  Int32 gambler_num = 1,
                                                  Int32 gambler_sample_num = 100100,
                                                  Mode mode = Mode::Estimate) {
    return adaptive_betting<GeoCheckingCapital>(samples, prior_mean, delta, 
                                                grid_num, breakpoints, gambler_alpha, gambler_trunc_scale,
                                                gambler_prior_var, gambler_num, gambler_sample_num, mode);
}

/**
 * @brief Convenience wrapper for SequenceCheckingCapital.
 */
inline std::pair<Float32, Int32> adaptive_betting_sequence(const Vector32f& samples,
                                                           Float32 prior_mean,
                                                           Float32 delta,
                                                           Int32 grid_num,
                                                           const std::vector<Int32>& breakpoints = {},
                                                           Float32 gambler_alpha = 0.05f,
                                                           Float32 gambler_trunc_scale = 0.5f,
                                                           Float32 gambler_prior_var = 0.25f,
                                                           Int32 gambler_num = 1,
                                                           Int32 gambler_sample_num = 100100,
                                                           Mode mode = Mode::Estimate) {
    return adaptive_betting<SequenceCheckingCapital>(samples, prior_mean, delta, 
                                                     grid_num, breakpoints, gambler_alpha, gambler_trunc_scale,
                                                     gambler_prior_var, gambler_num, gambler_sample_num, mode);
}

} // namespace betting

#endif // BETTING_BY_TIME_STRATEGIES_ADAPTIVE_BETTING_HPP
