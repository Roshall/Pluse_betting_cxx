#ifndef BETTING_BY_TIME_FRAMEWORK_HPP
#define BETTING_BY_TIME_FRAMEWORK_HPP

#include "betting_by_time/core/types.hpp"
#include <any>
#include <functional>

namespace betting {

/**
 * @brief Enum for capital process types
 */
enum class CapitalType {
    Geo,  ///< Geometric checking capital process
    Seq   ///< Sequence checking capital process
};

/**
 * @brief Enum for betting strategy types
 */
enum class BetStrategy {
    Vanilla,  ///< Vanilla betting strategy
    Ada       ///< Adaptive betting strategy
};

/**
 * @brief Type erased factory function for any capital process
 */
using AnyGamblerFactory = std::function<std::any(Float32, Float32, Int32)>;

/**
 * @brief Type erased betting function
 */
using AnyBettingFn = std::function<std::pair<Float32, Int32>(
    const Vector32f&, Float32, Float32, Int32, const std::vector<Int32>&,
    Float32, Float32, Float32, Int32, Int32, Mode)>;

/**
 * @brief Return type for betting factory
 */
using BettingFactoryResult = std::pair<AnyGamblerFactory, AnyBettingFn>;

/**
 * @brief Factory function to create betting strategy and capital process.
 * 
 * This function mimics the Python betting_factory API, returning appropriate
 * capital process type and betting function based on strategy selection.
 * 
 * @param strategy Betting strategy to use
 * @param capital Capital process type to use
 * @return A pair containing a factory function for the capital process and the betting function
 * 
 * @throws std::invalid_argument if unsupported strategy or capital type is requested
 */
BettingFactoryResult betting_factory(BetStrategy strategy, CapitalType capital);

/**
 * @brief Convenience function to create GeoCheckingCapital with vanilla betting.
 * 
 * @return Pair of (factory function, betting function)
 */
inline BettingFactoryResult vanilla_geo_factory() {
    return betting_factory(BetStrategy::Vanilla, CapitalType::Geo);
}

/**
 * @brief Convenience function to create SequenceCheckingCapital with vanilla betting.
 * 
 * @return Pair of (factory function, betting function)
 */
inline BettingFactoryResult vanilla_seq_factory() {
    return betting_factory(BetStrategy::Vanilla, CapitalType::Seq);
}

/**
 * @brief Convenience function to create GeoCheckingCapital with adaptive betting.
 * 
 * @return Pair of (factory function, betting function)
 */
inline BettingFactoryResult adaptive_geo_factory() {
    return betting_factory(BetStrategy::Ada, CapitalType::Geo);
}

/**
 * @brief Convenience function to create SequenceCheckingCapital with adaptive betting.
 * 
 * @return Pair of (factory function, betting function)
 */
inline BettingFactoryResult adaptive_seq_factory() {
    return betting_factory(BetStrategy::Ada, CapitalType::Seq);
}

} // namespace betting

#endif // BETTING_BY_TIME_FRAMEWORK_HPP
