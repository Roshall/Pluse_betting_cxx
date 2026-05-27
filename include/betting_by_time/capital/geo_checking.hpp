#ifndef BETTING_BY_TIME_CAPITAL_GEO_CHECKING_HPP
#define BETTING_BY_TIME_CAPITAL_GEO_CHECKING_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/capital/time_slice_lambda.hpp"
#include <Eigen/Dense>

namespace betting {

/**
 * @brief Geometric checking capital process for betting-based confidence sequences.
 * 
 * This class implements a capital process that tracks wealth for multiple hypotheses
 * about the mean using geometric betting strategies. It maintains twin capitals
 * (positive and negative bets) for each hypothesis to construct two-sided confidence sets.
 */
class GeoCheckingCapital {
private:
    Matrix64d cum_cap_twins_;     ///< Cumulative capitals for each hypothesis (grid_num+1 x 2)
    Matrix32i cum_cap_pos_;       ///< Position tracking for each hypothesis (grid_num+1 x 2)
    Vector32f capitals_;          ///< Capital values per sample
    Vector32f samples_;           ///< All samples stored sequentially
    Vector32i s_pos_;             ///< Position indices for sample ranges
    TimeSliceOptLambda cap_mine_; ///< Lambda optimizer
    Float32 trunc_scale_;         ///< Truncation scale for bet limiting
    Float32 threshold_;           ///< Capital threshold for rejection
    Int32 grid_num_;              ///< Number of grid points
    Int32 s_ptr_;                 ///< Current sample pointer

public:
    /**
     * @brief Construct a new Geo Checking Capital object.
     * 
     * @param alpha Confidence parameter
     * @param trunc_scale Truncation scale for bets
     * @param grid_num Number of grid points for hypothesis testing
     * @param prior_mean Prior estimate of mean
     * @param prior_var Prior estimate of variance
     * @param num Initial time step
     * @param sample_num Pre-allocated sample capacity
     */
    explicit GeoCheckingCapital(Float32 alpha = 0.05f,
                                Float32 trunc_scale = 0.5f,
                                Int32 grid_num = 1000,
                                Float32 prior_mean = 0.5f,
                                Float32 prior_var = 0.25f,
                                Int32 num = 1,
                                Int32 sample_num = 100010)
        : cum_cap_twins_(Matrix64d::Ones(grid_num + 1, 2)),
          cum_cap_pos_(Matrix32i::Zero(grid_num + 1, 2)),
          capitals_(Vector32f::Zero(1000)),
          samples_(Vector32f::Zero(sample_num)),
          s_pos_(Vector32i::Zero(1000)),
          cap_mine_(prior_mean, prior_var, static_cast<Float32>(num), alpha * 0.5f),
          trunc_scale_(trunc_scale),
          threshold_(1.0f / alpha),
          grid_num_(grid_num),
          s_ptr_(0) {
        s_pos_(0) = 0;
    }

    /**
     * @brief Get the capacity for storing samples.
     * 
     * @return Int32 Maximum number of samples
     */
    [[nodiscard]] Int32 capacity() const {
        return static_cast<Int32>(samples_.size());
    }

    /**
     * @brief Reset the capital process to initial state.
     * 
     * @param prior_mean New prior mean
     * @param prior_var New prior variance
     * @param num Initial time step
     */
    void reset(Float32 prior_mean = 0.5f, Float32 prior_var = 0.25f, Int32 num = 1) {
        cum_cap_pos_.setZero();
        cum_cap_twins_.setOnes();
        capitals_.setZero();
        s_ptr_ = 0;
        cap_mine_.reset(prior_mean, prior_var, static_cast<Float32>(num));
        samples_.setZero();
        s_pos_.setZero();
        s_pos_(0) = 0;
    }

    /**
     * @brief Update the confidence parameter a.
     * 
     * @param alpha New alpha value
     */
    void set_alpha(Float32 alpha) {
        threshold_ = 1.0f / alpha;
        cap_mine_.set_alpha(alpha);
    }

    /**
     * @brief Get samples for a given index range.
     * 
     * @param idx Index into the sample sequence
     * @return Eigen::Map<Vector32f> View into the sample array
     */
    Eigen::Map<Vector32f> sample(Int32 idx) {
        Int32 start = s_pos_(idx);
        return {&samples_(start), s_pos_(idx + 1) - start};
    }

    /**
     * @brief Add a single sample to the process.
     * 
     * @param sample Sample value
     */
    void add_sample(Float32 sample) {
        Vector32f sample_batch(1);
        sample_batch(0) = sample;
        add_sample(sample_batch);
    }

    /**
     * @brief Add a batch of samples to the process.
     * 
     * @param samples Sample batch
     */
    void add_sample(const Vector32f& samples) {
        const Int32 start = s_pos_(s_ptr_);
        samples_.segment(start, samples.size()) = samples;
        capitals_(s_ptr_) = cap_mine_.advance(&samples_(start), samples.size());
        s_ptr_ += 1;
        s_pos_(s_ptr_) = start + static_cast<Int32>(samples.size());
    }

    /**
     * @brief Get the last sample added.
     * 
     * @return Eigen::Map<Vector32f> View of the last sample
     */
    Eigen::Map<Vector32f> last_sample() {
        return sample(s_ptr_ - 1);
    }

    /**
     * @brief Get the latest capital value.
     * 
     * @return Float32 Latest capital
     */
    [[nodiscard]] Float32 lat_capital() const {
        return capitals_(s_ptr_ - 1);
    }

    /**
     * @brief Advance the capital process with a new sample across multiple hypotheses.
     * 
     * @param samples New sample value
     * @param m_lst Vector of hypothesized means to test
     */
    void advance(Float32 sample, const Vector32f& m_lst) {
        Vector32f sample_batch(1);
        sample_batch(0) = sample;
        advance(sample_batch, m_lst);
    }

    /**
     * @brief Advance the capital process with a new sample batch across multiple hypotheses.
     * 
     * @param samples Sample batch
     * @param m_lst Vector of hypothesized means to test
     */
    void advance(const Vector32f& samples, const Vector32f& m_lst) {
        add_sample(samples);
        Float32 capital = lat_capital();
        
        // Get the last sample as an Eigen map
        auto last_samp = last_sample();
        
        // Update cumulative capitals for all hypotheses
        for (Int32 mi = 0; mi < m_lst.size(); ++mi) {
            Float32 m = m_lst(mi);
            
            // Import the single bet function (defined in bet_once.hpp)
            extern Float64 geo_single_bet_on(Float32, Float32, const Vector32f&, Float64, Float32);
            
            cum_cap_twins_(mi, 0) = geo_single_bet_on(trunc_scale_, m, last_samp, 
                                                      cum_cap_twins_(mi, 0), capital);
            cum_cap_twins_(mi, 1) = geo_single_bet_on(trunc_scale_, m, last_samp,
                                                      cum_cap_twins_(mi, 1), -capital);
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

#endif // BETTING_BY_TIME_CAPITAL_GEO_CHECKING_HPP
