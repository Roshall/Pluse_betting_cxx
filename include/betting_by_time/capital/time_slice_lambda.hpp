#ifndef BETTING_BY_TIME_CAPITAL_TIME_SLICE_LAMBDA_HPP
#define BETTING_BY_TIME_CAPITAL_TIME_SLICE_LAMBDA_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/core/utilities.hpp"
#include <cmath>
#include <Eigen/Dense>

namespace betting {

/**
 * @brief Time-slice optimized lambda calculator for geometric checking.
 * 
 * This class implements an adaptive lambda calculation strategy that optimizes
 * the betting fraction based on observed variance and cumulative statistics.
 * Based on the paper "Testing by Betting: A Framework for Statistical Hypothesis Testing".
 */
class TimeSliceOptLambda {
private:
    Float64 cum_;           ///< Cumulative sum of samples
    Float64 cum_diff2_;     ///< Cumulative sum of squared differences
    Float64 c_;             ///< Confidence constant (depends on alpha)
    Float64 lbd_cum_;       ///< Cumulative lambda
    Float64 lbd2_cum_;      ///< Cumulative lambda squared
    Float64 t_;               ///< Time step counter

public:
    /**
     * @brief Construct a new Time Slice Opt Lambda object.
     * 
     * @param prior_mean Prior estimate of the mean (default: 0.5)
     * @param prior_var Prior estimate of the variance (default: 0.25)
     * @param num Initial time step (default: 1)
     * @param alpha Confidence parameter (default: 0.05)
     */
    explicit TimeSliceOptLambda(Float32 prior_mean = 0.5f,
                                Float32 prior_var = 0.25f,
                                Float32 num = 1,
                                Float32 alpha = 0.05f)
        : cum_(prior_mean * num),
          cum_diff2_(prior_var * num),
          c_(cal_c(alpha)),
          lbd_cum_(std::sqrt(c_ / (prior_var * num)) * num),
          lbd2_cum_(lbd_cum_ * std::sqrt(c_ / (prior_var * num))),
          t_(num - 1) {}

    /**
     * @brief Reset the lambda calculator with new priors.
     * 
     * @param prior_mean New prior mean
     * @param prior_var New prior variance
     * @param num Initial time step
     */
    void reset(Float32 prior_mean, Float32 prior_var, Float32 num) {
        cum_diff2_ = prior_var * num;
        const Float64 lbd = std::sqrt(c_ / cum_diff2_);
        lbd_cum_ = lbd * num;
        lbd2_cum_ = lbd_cum_ * lbd;
        t_ = num - 1;
        cum_ = prior_mean * num;
    }

    /**
     * @brief Update the confidence parameter alpha.
     * 
     * @param d New alpha value
     */
    void set_alpha(Float32 d) {
        c_ = cal_c(d);
    }

    /**
     * @brief Advance the lambda calculation with new samples.
     * 
     * @param samples Pointer to sample array
     * @param n Number of samples
     * @return The computed lambda value for this step
     */
    Float32 advance(const Float32* samples, Float32 n) {
        t_ += n;
        const Float64 t = t_;
        
        // Update cumulative statistics using Eigen
        Eigen::Map<const Eigen::VectorXf> sample_vec(samples, static_cast<Int32>(n));
        cum_ += sample_vec.sum();
        
        // Update cumulative squared differences using Eigen
        const Eigen::VectorXd centered = sample_vec.cast<Float64>().array() - cum_ / t;
        cum_diff2_ += centered.squaredNorm();
        
        // Compute optimal lambda
        const Float64 sigma2 = cum_diff2_ / t;
        const Float64 a = sigma2 * lbd2_cum_ + c_;
        const Float64 b = lbd_cum_ / n;
        
        // lambda* formula from the paper
        const Float32 lbd = std::sqrt(b * b + a / (sigma2 * n)) - b;
        
        // Update cumulative lambdas
        lbd_cum_ += lbd * n;
        lbd2_cum_ += lbd * lbd * n;
        
        return lbd;
    }

    // Getters
    [[nodiscard]] Float32 cum() const { return cum_; }
    [[nodiscard]] Float32 cum_diff2() const { return cum_diff2_; }
    [[nodiscard]] Float32 c() const { return c_; }
    [[nodiscard]] Float32 lbd_cum() const { return lbd_cum_; }
    [[nodiscard]] Float32 lbd2_cum() const { return lbd2_cum_; }
    [[nodiscard]] Int32 t() const { return static_cast<Int32>(t_); }
};

} // namespace betting

#endif // BETTING_BY_TIME_CAPITAL_TIME_SLICE_LAMBDA_HPP
