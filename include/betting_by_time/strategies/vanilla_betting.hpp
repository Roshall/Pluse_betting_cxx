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
 * @param samples Input sample array
 * @param prior_mean Prior estimate of mean (unused in vanilla)
 * @param delta Confidence parameter
 * @param grid_num Number of grid points
 * @param gambler Reference to capital process (GeoCheckingCapital or SequenceCheckingCapital)
 * @return Pair of (estimated_mean, num_samples_used)
 */
// Class-based vanilla betting strategy with incremental sample feeding
template<typename CapitalProcess>
class VanillaBetting {
public:
    VanillaBetting([[maybe_unused]] Float32 prior_mean,
                   Float32 delta,
                   Int32 grid_num,
                   CapitalProcess& gambler)
        : gambler_(gambler),
          prior_mean_(prior_mean),
          delta_(delta),
          grid_num_(grid_num),
          m_possible(linspace(0.0f, 1.0f, grid_num + 1)),
          eps_(delta * 2.0f - 1.0f / grid_num),
          cs_bound_(),
          threshold_(gambler.threshold() * 2.0f),
          rang_idx_(Vector32i::LinSpaced(grid_num + 1, 0, grid_num)),
          samples_used_(0),
          finalized_(false),
          estimated_mean_(prior_mean),
          phase_(Phase::Running) {
        cs_bound_ << 0.0f, 1.0f;
    }

    void submit_sample(Float32 sample) {
        submit_samples(Vector32f(sample));
    }

    void submit_samples(const Vector32f& samples) {
        if (finalized_) return;

        Int32 i = 0;
        for (i = 0; i < samples.size(); ++i) {
            gambler_.advance(samples(i), m_possible);

            Vector64d cap_sums = gambler_.cum_cap_twins().rowwise().sum();

            std::vector<Int32> new_indices;
            for (Int32 j = 0; j < cap_sums.size(); ++j) {
                if (cap_sums(j) < threshold_) {
                    new_indices.push_back(j);
                }
            }

            if (new_indices.empty()) {
                samples_used_ = i + 1;
                finalize_internal();
                return;
            }

            rang_idx_.resize(new_indices.size());
            for (size_t j = 0; j < new_indices.size(); ++j) {
                rang_idx_(j) = new_indices[j];
            }

            intersect(cs_bound_, m_possible(rang_idx_(0)), m_possible(rang_idx_(rang_idx_.size() - 1)));

            if (cs_bound_(1) - cs_bound_(0) < eps_) {
                samples_used_ = i + 1;
                finalize_internal();
                return;
            }
        }

        samples_used_ = i;
        finalize_internal();
    }

    Float32 get_lower_bound() const { return cs_bound_(0); }
    Float32 get_upper_bound() const { return cs_bound_(1); }
    Float32 get_interval_width() const { return cs_bound_(1) - cs_bound_(0); }
    Float32 get_estimated_mean() const { return estimated_mean_; }
    Int32 get_samples_used() const { return samples_used_; }
    bool is_finalized() const { return finalized_; }
    Int32 get_current_phase() const { return static_cast<Int32>(phase_); }

    std::pair<Float32, Int32> finalize() {
        if (!finalized_) finalize_internal();
        return std::make_pair(estimated_mean_, samples_used_);
    }

    void reset() {
        cs_bound_ << 0.0f, 1.0f;
        rang_idx_ = Vector32i::LinSpaced(grid_num_ + 1, 0, grid_num_);
        samples_used_ = 0;
        finalized_ = false;
        estimated_mean_ = prior_mean_;
        phase_ = Phase::Running;
        // gambler_ state is external and not reset here
    }

private:
    enum class Phase { Running = 0, FinalEstimation = 1 };

    CapitalProcess& gambler_;
    const Float32 prior_mean_;
    const Float32 delta_;
    const Int32 grid_num_;
    const Vector32f m_possible;
    const Float32 eps_;
    Array2f cs_bound_;
    const Float32 threshold_;
    Vector32i rang_idx_;
    Int32 samples_used_;
    bool finalized_;
    Float32 estimated_mean_;
    Phase phase_;

    void finalize_internal() {
        Vector64d final_sums = gambler_.cum_cap_twins().rowwise().sum();
        Int32 min_idx = argmin(final_sums);
        estimated_mean_ = static_cast<Float32>(min_idx) / grid_num_;
        finalized_ = true;
        phase_ = Phase::FinalEstimation;
    }
};

// Convenience function that matches previous free-function signature
template<typename CapitalProcess>
std::pair<Float32, Int32> vanilla_betting(const Vector32f& samples,
                                           [[maybe_unused]] Float32 prior_mean,
                                           Float32 delta,
                                           Int32 grid_num,
                                           CapitalProcess& gambler,
                                           const std::vector<Int32>& breakpoints = {}) {
    VanillaBetting<CapitalProcess> vb(prior_mean, delta, grid_num, gambler);

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
                                                 [[maybe_unused]] Float32 prior_mean,
                                                 Float32 delta,
                                                 Int32 grid_num,
                                                 GeoCheckingCapital& gambler,
                                                 const std::vector<Int32>& breakpoints = {}) {
    return vanilla_betting<GeoCheckingCapital>(samples, prior_mean, delta, grid_num, gambler, breakpoints);
}

inline std::pair<Float32, Int32> vanilla_betting(const Vector32f& samples,
                                                 [[maybe_unused]] Float32 prior_mean,
                                                 Float32 delta,
                                                 Int32 grid_num,
                                                 SequenceCheckingCapital& gambler,
                                                 const std::vector<Int32>& breakpoints = {}) {
    return vanilla_betting<SequenceCheckingCapital>(samples, prior_mean, delta, grid_num, gambler, breakpoints);
}

} // namespace betting

#endif // BETTING_BY_TIME_STRATEGIES_VANILLA_BETTING_HPP
