#ifndef BETTING_BY_TIME_CAPITAL_SEQUENCE_CHECKING_HPP
#define BETTING_BY_TIME_CAPITAL_SEQUENCE_CHECKING_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/capital/sequence_lambda.hpp"
#include <Eigen/Dense>

namespace betting {

/**
 * @brief Sequence checking capital process for betting-based confidence sequences.
 * 
 * This class implements a capital process that tracks wealth for multiple hypotheses
 * using sequential betting strategies. Simpler than GeoCheckingCapital as it stores
 * individual samples rather than sample ranges.
 */
class SequenceCheckingCapital {
private:
    Matrix64d cum_cap_twins_;     ///< Cumulative capitals (grid_num+1 x 2)
    Matrix32i cum_cap_pos_;       ///< Position tracking (grid_num+1 x 2)
    Lambda cap_mine_;             ///< Lambda optimizer for sequence checking
    Float32 trunc_scale_;         ///< Truncation scale for bet limiting
    Float32 threshold_;           ///< Capital threshold for rejection
    Vector32f samples_;           ///< Individual samples
    Vector32f capitals_;          ///< Capital values per sample
    Int32 s_ptr_;                 ///< Current sample pointer
    Int32 grid_num_;              ///< Number of grid points

public:
    /**
     * @brief Construct a new Sequence Checking Capital object.
     * 
     * @param delta Confidence parameter
     * @param trunc_scale Truncation scale for bets
     * @param grid_num Number of grid points
     * @param prior_mean Prior estimate of mean
     * @param prior_var Prior estimate of variance
     * @param num Initial time step
     * @param sample_num Pre-allocated sample capacity
     */
    explicit SequenceCheckingCapital(Float32 delta = 0.05f,
                                     Float32 trunc_scale = 0.5f,
                                     Int32 grid_num = 1000,
                                     Float32 prior_mean = 0.5f,
                                     Float32 prior_var = 0.25f,
                                     Int32 num = 1,
                                     Int32 sample_num = 100100)
        : cum_cap_twins_(Matrix64d::Ones(grid_num + 1, 2)),
          cum_cap_pos_(Matrix32i::Zero(grid_num + 1, 2)),
          cap_mine_(prior_mean, prior_var, num, delta * 0.5f),
          trunc_scale_(trunc_scale),
          threshold_(1.0f / delta),
          samples_(Vector32f::Zero(sample_num)),
          capitals_(Vector32f::Zero(sample_num)),
          s_ptr_(0),
          grid_num_(grid_num) {}

    /**
     * @brief Get the capacity for storing samples.
     * 
     * @return Int32 Maximum number of samples
     */
    Int32 capacity() const {
        return static_cast<Int32>(samples_.size());
    }

    /**
     * @brief Reset the capital process to initial state.
     * 
     * @param prior_mean New prior mean
     * @param prior_diff2 New prior variance
     * @param num Initial time step
     */
    void reset(Float32 prior_mean = 0.5f, Float32 prior_diff2 = 0.25f, Int32 num = 1) {
        // Zero out used portion of samples
        samples_.head(s_ptr_).setZero();
        s_ptr_ = 0;
        cum_cap_pos_.setZero();
        cum_cap_twins_.setOnes();
        cap_mine_.reset(prior_mean, prior_diff2, num);
    }

    /**
     * @brief Update the confidence parameter delta.
     * 
     * @param d New delta value
     */
    void set_delta(Float32 d) {
        threshold_ = 1.0f / d;
        cap_mine_.set_delta(d);
    }

    /**
     * @brief Placeholder for candidate setting (not used).
     * 
     * @param cand Candidate value (unused)
     */
    void set_cand(Float32 cand) {
        // No-op for sequence checking
        (void)cand;
    }

    /**
     * @brief Get a sample by index.
     * 
     * @param idx Sample index
     * @return Float32 Sample value
     */
    Float32 sample(Int32 idx) const {
        return samples_(idx);
    }

    /**
     * @brief Get the last sample added.
     * 
     * @return Float32 Last sample value
     */
    Float32 last_sample() const {
        return samples_(s_ptr_ - 1);
    }

    /**
     * @brief Add a single sample to the process.
     * 
     * @param sample Sample value
     */
    void add_sample(Float32 sample) {
        samples_(s_ptr_) = sample;
        capitals_(s_ptr_) = cap_mine_.advance(sample);
        s_ptr_ += 1;
    }

    /**
     * @brief Advance the capital process with a new sample across multiple hypotheses.
     * 
     * @param x New sample value
     * @param m_lst Vector of hypothesized means to test
     */
    void advance(Float32 x, const Vector32f& m_lst) {
        // Add the sample to the sequence and obtain the latest lambda/capital
        add_sample(x);
        Float32 lbd = capitals_(s_ptr_ - 1);
        
        // Update cumulative capitals for all hypotheses
        for (Int32 mi = 0; mi < m_lst.size(); ++mi) {
            Float32 m = m_lst(mi);
            
            // Skip if sample equals hypothesis (martingale property)
            if (std::abs(x - m) < 1e-9f) {
                continue;
            }
            
            // Update positive twin (bet on mean > m)
            Float64& cum_capital_pos = cum_cap_twins_(mi, 0);
            if (std::abs(cum_capital_pos) > 1e-16) {
                Float32 lbd_m = std::clamp(lbd,
                                          -trunc_scale_ / (1.0f + 1e-9f - m),
                                          trunc_scale_ / (m + 1e-9f));
                cum_capital_pos *= 1.0 + static_cast<Float64>(lbd_m) * static_cast<Float64>(x - m);
            }
            
            // Update negative twin (bet on mean < m)
            Float64& cum_capital_neg = cum_cap_twins_(mi, 1);
            if (std::abs(cum_capital_neg) > 1e-16) {
                Float32 lbd_m = std::clamp(-lbd,
                                          -trunc_scale_ / (1.0f + 1e-9f - m),
                                          trunc_scale_ / (m + 1e-9f));
                cum_capital_neg *= 1.0 + static_cast<Float64>(lbd_m) * static_cast<Float64>(x - m);
            }
        }
    }

    // Accessors
    const Matrix64d& cum_cap_twins() const { return cum_cap_twins_; }
    Matrix64d& cum_cap_twins() { return cum_cap_twins_; }

    const Matrix32i& cum_cap_pos() const { return cum_cap_pos_; }
    Matrix32i& cum_cap_pos() { return cum_cap_pos_; }

    const Vector32f& capitals() const { return capitals_; }
    Vector32f& capitals() { return capitals_; }

    Int32 s_ptr() const { return s_ptr_; }
    Float32 trunc_scale() const { return trunc_scale_; }
    Float32 threshold() const { return threshold_; }
    Int32 grid_num() const { return grid_num_; }
};

} // namespace betting

#endif // BETTING_BY_TIME_CAPITAL_SEQUENCE_CHECKING_HPP
