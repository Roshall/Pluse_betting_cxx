#ifndef BETTING_BY_TIME_CORE_TYPES_HPP
#define BETTING_BY_TIME_CORE_TYPES_HPP

#include <Eigen/Dense>
#include <cstdint>

/**
 * @namespace betting
 * @brief Main namespace for betting-by-time algorithms
 */
namespace betting {

/// @brief Single-precision floating point type (matches numpy float32)
using Float32 = float;

/// @brief Double-precision floating point type (matches numpy float64)
using Float64 = double;

/// @brief 32-bit integer type (matches numpy int32)
using Int32 = int32_t;

// Eigen-based types replacing NumPy arrays

/// @brief 1D vector of Float32 values
using Vector32f = Eigen::VectorXf;

/// @brief 1D vector of Float64 values
using Vector64d = Eigen::VectorXd;

/// @brief 1D vector of Int32 values
using Vector32i = Eigen::VectorXi;

/// @brief 2D matrix of Float32 values
using Matrix32f = Eigen::MatrixXf;

/// @brief 2D matrix of Float64 values
using Matrix64d = Eigen::MatrixXd;

/// @brief 2D matrix of Int32 values
using Matrix32i = Eigen::MatrixXi;

/// @brief Row vector of Float32 values
using RowVector32f = Eigen::RowVectorXf;

// Fixed-size types for small arrays (stack-allocated for performance)

/// @brief Fixed-size 2-element Float32 array
using Array2f = Eigen::Array<Float32, 2, 1>;

/// @brief Fixed-size 2-element Int32 array
using Array2i = Eigen::Array<Int32, 2, 1>;

/// @brief Fixed-size 2-element Float64 array
using Array2d = Eigen::Array<Float64, 2, 1>;

} // namespace betting

#endif // BETTING_BY_TIME_CORE_TYPES_HPP
