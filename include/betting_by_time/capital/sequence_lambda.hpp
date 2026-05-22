#ifndef BETTING_BY_TIME_CAPITAL_SEQUENCE_LAMBDA_HPP
#define BETTING_BY_TIME_CAPITAL_SEQUENCE_LAMBDA_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/core/utilities.hpp"
#include <cmath>
#include <algorithm>

namespace betting {

/**
 * @brief Lambda calculator for sequence checking capital process.
 * 
 * This class implements a lambda calculation strategy based on sequential
 * hypothesis testing with logarithmic confidence bounds.
 */
class Lambda {
private:
    Float32 cum_;           ///< Cumulative sum of samples
    Float32 cum_diff2_;     ///< Cumulative sum of squared differences
    Int32 t_;               ///< Current time step
    Float32 alpha_;         ///< Confidence parameter
    Float32 c_;             ///< Confidence constant c = 2*ln(2/alpha)
    Int32 fake_;            ///< Fake time offset

public:
    /**
     * @brief Construct a new Lambda object.
     * 
     * @param prior_mean Prior estimate of mean (default: 0.5)
     * @param prior_var Prior estimate of variance (default: 0.25)
     * @param num Initial time step (default: 1)
     * @param alpha Confidence parameter (default: 0.05)
     */
    Lambda(Float32 prior_mean = 0.5f,
           Float32 prior_var = 0.25f,
           Int32 num = 1,
           Float32 alpha = 0.05f)
        : cum_(prior_mean * num),
          cum_diff2_(std::max(prior_var, 1e-6f) * num),
          t_(num),
          alpha_(alpha),
          c_(cal_c(alpha)),
          fake_(num - 1) {}

    /**
     * @brief Reset the lambda calculator with new priors.
     * 
     * @param prior_mean New prior mean
     * @param prior_diff2 New prior variance
     * @param num Initial time step
     */
    void reset(Float32 prior_mean, Float32 prior_diff2, Int32 num) {
        t_ = num;
        cum_ = prior_mean * num;
        cum_diff2_ = std::max(prior_diff2, 1e-6f) * num;
        fake_ = num - 1;
    }

    /**
     * @brief Update the confidence parameter.
     * 
     * @param alpha New alpha value
     */
    void set_alpha(Float32 alpha) {
        c_ = cal_c(alpha);
    }

    /**
     * @brief Advance lambda calculation with a new sample.
     * 
     * @param x New sample value
     * @return Computed lambda value
     */
    Float32 advance(Float32 x) {
        Int32 t = t_;
        Float32 sigma2 = cum_diff2_ / t;
        Int32 ft = t - fake_;
        
        // Lambda formula with logarithmic confidence bound
        Float32 lbd = std::sqrt(c_ / (sigma2 * ft * std::log(ft + 1)));
        
        // Update cumulative statistics
        cum_diff2_ += (x - cum_ / t) * (x - cum_ / t);
        cum_ += x;
        t_ = t + 1;
        
        return lbd;
    }

    // Getters
    Float32 cum() const { return cum_; }
    Float32 cum_diff2() const { return cum_diff2_; }
    Int32 t() const { return t_; }
    Float32 alpha() const { return alpha_; }
    Float32 c() const { return c_; }
    Int32 fake() const { return fake_; }
};

} // namespace betting

#endif // BETTING_BY_TIME_CAPITAL_SEQUENCE_LAMBDA_HPP
