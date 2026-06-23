#ifndef BETTING_BY_TIME_STRATEGIES_VANILLA_BETTING_HPP
#define BETTING_BY_TIME_STRATEGIES_VANILLA_BETTING_HPP

#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"
#include "betting_by_time/core/types.hpp"
#include "betting_by_time/core/utilities.hpp"

namespace betting {

/**
 * @brief Vanilla betting strategy implementation.
 * 
 * This strategy tests all hypotheses simultaneously and narrows down
 * the confidence set by eliminating hypotheses whose capital exceeds threshold.
 * 
 * @tparam CapitalProcess Type of capital process (GeoCheckingCapital or SequenceCheckingCapital)
 * @param samples Input sample array
 * @param prior_mean Prior estimate of mean (unused in vanilla)
 * @param delta Confidence parameter
 * @param grid_num Number of grid points
 * @param gambler_params Parameters for constructing the internal gambler
 * @return Pair of (estimated_mean, num_samples_used)
 */
// Class-based vanilla betting strategy with incremental sample feeding
template<typename CapitalProcess>
class VanillaBetting {
public:
    /**
     * @brief Construct a VanillaBetting instance with gambler parameters.
     * 
     * @param prior_mean Prior estimate of mean (also used as gambler prior mean)
     * @param delta Confidence parameter
     * @param grid_num Number of grid points
     * @param gambler_alpha Delta parameter for gambler (default: 0.05f)
     * @param gambler_trunc_scale Truncation scale for gambler (default: 0.5f)
     * @param gambler_prior_var Prior variance for gambler (default: 0.25f)
     * @param gambler_num Initial time step for gambler (default: 1)
     * @param gambler_sample_num Pre-allocated sample capacity for gambler (default: 100010)
     */
    VanillaBetting(Float32 prior_mean,
                   Float32 delta,
                   Int32 grid_num,
                   Float32 gambler_alpha = 0.05f,
                   Float32 gambler_trunc_scale = 0.5f,
                   Float32 gambler_prior_var = 0.25f,
                   Int32 gambler_num = 1,
                   Int32 gambler_sample_num = 100010,
                   Mode mode = Mode::Estimate)
        : gambler_(gambler_alpha, gambler_trunc_scale, grid_num, prior_mean, 
                   gambler_prior_var, gambler_num, gambler_sample_num),
          grid_num_(grid_num),
          m_possible(linspace(0.0f, 1.0f, grid_num + 1)),
          eps_(delta * 2.0f - 1.0f / grid_num),
          cs_bound_(),
          threshold_(gambler_.threshold() * 2.0f),
          finalized_(false),
          estimated_mean_(prior_mean),
                    phase_(Phase::Running),
                    mode_(mode) {
        cs_bound_ << 0.0f, 1.0f;
    }

    void submit_sample(Float32 sample) {
        Vector32f sample_vec(1);
        sample_vec(0) = sample;
        submit_samples(sample_vec);
    }

    void submit_samples(const Vector32f& samples) {
        if (finalized_) return;
        gambler_.advance(samples, m_possible);

        Vector64d cap_sums = gambler_.cum_cap_twins().rowwise().sum();

        std::vector<Int32> new_indices;
        for (Int32 i = 0; i < cap_sums.size(); ++i) {
            if (cap_sums(i) < threshold_) {
                new_indices.push_back(i);
            }
        }
        if (new_indices.empty()) {
            finalize_internal();
            return;
        }

        intersect(cs_bound_, m_possible(new_indices.front()), m_possible(new_indices.back()));

        if (mode_ == Mode::Estimate && cs_bound_(1) - cs_bound_(0) < eps_) {
            finalize_internal();
            return;
        }
    }

    Float32 get_lower_bound() const { return cs_bound_(0); }
    Float32 get_upper_bound() const { return cs_bound_(1); }
    Float32 get_interval_width() const { return cs_bound_(1) - cs_bound_(0); }
    Float32 get_estimated_mean() const { return estimated_mean_; }
    Int32 get_samples_used() const { return gambler_.used_sample_num(); }
    bool is_finalized() const { return finalized_; }
    Int32 get_current_phase() const { return static_cast<Int32>(phase_); }

    std::pair<Float32, Int32> finalize() {
        if (!finalized_) finalize_internal();
        return std::make_pair(estimated_mean_, get_samples_used());
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
        cs_bound_ << 0.0f, 1.0f;
        finalized_ = false;
        estimated_mean_ = prior_mean;
        phase_ = Phase::Running;
        gambler_.reset(prior_mean, prior_var, prior_num);
    }

private:
    enum class Phase { Running = 0, FinalEstimation = 1 };

    CapitalProcess gambler_;  // Internally owned gambler
    const Int32 grid_num_;
    const Vector32f m_possible;
    const Float32 eps_;
    Array2f cs_bound_;
    const Float32 threshold_;
    bool finalized_;
    Float32 estimated_mean_;
    Phase phase_;
    const Mode mode_;

    void finalize_internal() {
        if (mode_ == Mode::Estimate) {
            Vector64d final_sums = gambler_.cum_cap_twins().rowwise().sum();
            Int32 min_idx = argmin(final_sums);
            estimated_mean_ = static_cast<Float32>(min_idx) / grid_num_;
        } else {
            estimated_mean_ = (cs_bound_(0) + cs_bound_(1)) / 2.0f;
        }
        finalized_ = true;
        phase_ = Phase::FinalEstimation;
    }
};

// Convenience function that matches previous free-function signature
template<typename CapitalProcess>
std::pair<Float32, Int32> vanilla_betting(const Vector32f& samples,
                                          Float32 prior_mean,
                                          Float32 delta,
                                          Int32 grid_num,
                                          const std::vector<Int32>& breakpoints = {},
                                          Float32 gambler_alpha = 0.05f,
                                          Float32 gambler_trunc_scale = 0.5f,
                                          Float32 gambler_prior_var = 0.25f,
                                          Int32 gambler_num = 1,
                                          Int32 gambler_sample_num = 100010,
                                          Mode mode = Mode::Estimate) {
    VanillaBetting<CapitalProcess> vb(prior_mean, delta, grid_num, 
                                      gambler_alpha, gambler_trunc_scale,
                                      gambler_prior_var, gambler_num, gambler_sample_num, mode);

    if (breakpoints.empty()) {
        vb.submit_samples(samples);
    } else {
        for (size_t i = 0; i < breakpoints.size() - 1; ++i) {
            Int32 start = breakpoints[i];
            Int32 end = breakpoints[i + 1];

            if (start < 0 || end > static_cast<Int32>(samples.size()) || start >= end) {
                continue;
            }

            Vector32f batch = samples.segment(start, end - start);
            vb.submit_samples(batch);
        }
    }

    return vb.finalize();
}

// Convenience overloads for Geo/Sequence capital processes
inline std::pair<Float32, Int32> vanilla_betting(const Vector32f& samples,
                                                 Float32 prior_mean,
                                                 Float32 delta,
                                                 Int32 grid_num,
                                                 const std::vector<Int32>& breakpoints = {},
                                                 Float32 gambler_alpha = 0.05f,
                                                 Float32 gambler_trunc_scale = 0.5f,
                                                 Float32 gambler_prior_var = 0.25f,
                                                 Int32 gambler_num = 1,
                                                 Int32 gambler_sample_num = 100010,
                                                 Mode mode = Mode::Estimate) {
    return vanilla_betting<GeoCheckingCapital>(samples, prior_mean, delta, grid_num, 
                                               breakpoints, gambler_alpha, gambler_trunc_scale,
                                               gambler_prior_var, gambler_num, gambler_sample_num, mode);
}

inline std::pair<Float32, Int32> vanilla_betting_sequence(const Vector32f& samples,
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
    return vanilla_betting<SequenceCheckingCapital>(samples, prior_mean, delta, grid_num, 
                                                    breakpoints, gambler_alpha, gambler_trunc_scale,
                                                    gambler_prior_var, gambler_num, gambler_sample_num, mode);
}

} // namespace betting

#endif // BETTING_BY_TIME_STRATEGIES_VANILLA_BETTING_HPP
