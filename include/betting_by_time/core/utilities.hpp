#ifndef BETTING_BY_TIME_CORE_UTILITIES_HPP
#define BETTING_BY_TIME_CORE_UTILITIES_HPP

#include "betting_by_time/core/types.hpp"
#include <cmath>
#include <algorithm>

namespace betting {

/**
 * @brief Computes the confidence scaling constant c for a given delta.
 * 
 * @param delta Confidence parameter (typically 0.01 to 0.1)
 * @return The constant c = 2 * ln(2/delta) used in capital process calculations
 * 
 * @note This function is based on the theoretical bounds from the paper
 * "Testing by Betting: A Framework for Statistical Hypothesis Testing"
 */
template<typename T>
constexpr T cal_c(T delta) {
    return 2.0 * std::log(2.0 / delta);
}

/**
 * @brief Creates evenly spaced values between start and stop.
 * 
 * @param start Start value
 * @param stop Stop value
 * @param num Number of points to generate
 * @return Vector32f with linearly spaced values
 * 
 * @note Uses Eigen's optimized LinSpaced function
 */
inline Vector32f linspace(Float32 start, Float32 stop, Int32 num) {
    return Vector32f::LinSpaced(num, start, stop);
}

/**
 * @brief Finds the index of the minimum element in a vector.
 * 
 * @param vec Input vector
 * @return Index of the minimum element
 */
inline Int32 argmin(const Vector64d& vec) {
    Eigen::Index min_idx;
    vec.minCoeff(&min_idx);
    return static_cast<Int32>(min_idx);
}

/**
 * @brief Finds indices of non-zero elements in a boolean vector.
 * 
 * @param vec Boolean vector
 * @return Vector32i containing indices where vec is true/non-zero
 */
Vector32i flatnonzero(const Vector32i& vec);

/**
 * @brief Generates geometric time sequence.
 * 
 * @param start Starting value
 * @param base Base for geometric progression
 * @param end Maximum value
 * @return Vector32i containing unique time points
 * 
 * @note Generates times as ceil(base^i) for i such that start <= base^i <= end
 */
Vector32i gen_times(Float32 start, Float32 base, Float32 end = 100000.0f);

/**
 * @brief Intersects interval [a[0], a[1]] with [l, u] in-place.
 * 
 * @param a Array to modify (must have at least 2 elements)
 * @param l Lower bound
 * @param u Upper bound
 */
inline void intersect(Array2f& a, Float32 l, Float32 u) {
    a(0) = std::max(a(0), l);
    a(1) = std::min(a(1), u);
}

} // namespace betting

#endif // BETTING_BY_TIME_CORE_UTILITIES_HPP
