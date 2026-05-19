#ifndef BETTING_BY_TIME_FRAMEWORK_HPP
#define BETTING_BY_TIME_FRAMEWORK_HPP

#include "betting_by_time/core/types.hpp"
#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"
#include "betting_by_time/strategies/vanilla_betting.hpp"
#include "betting_by_time/strategies/adaptive_betting.hpp"
#include <functional>
#include <memory>
#include <stdexcept>

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
    Ada       ///< Adaptive betting strategy (to be implemented)
};

/**
 * @brief Type alias for vanilla betting function
 */
using VanillaBettingFn = std::function<std::pair<Float32, Int32>(
    const Vector32f&, Float32, Float32, Int32, GeoCheckingCapital&)>;

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
 * 
 * @note Currently only implements vanilla strategy. Adaptive strategy is TODO.
 * 
 * Example usage:
 * @code
 * auto [make_gambler, bet_fn] = betting_factory(BetStrategy::Vanilla, CapitalType::Geo);
 * auto gambler = make_gambler(0.5f, 0.1f);  // alpha, trunc_scale
 * auto result = bet_fn(samples, prior_mean, delta, grid_num, gambler);
 * @endcode
 */
inline auto betting_factory(BetStrategy strategy, CapitalType capital) {
    switch (strategy) {
        case BetStrategy::Vanilla: {
            switch (capital) {
                case CapitalType::Geo: {
                    // Return factory for GeoCheckingCapital and vanilla betting
                    auto make_gambler = [](Float32 alpha, Float32 trunc_scale, 
                                          Int32 grid_num = 1000,
                                          Float32 prior_mean = 0.5f,
                                          Float32 prior_var = 0.25f,
                                          Int32 num = 1,
                                          Int32 sample_num = 100010) {
                        return GeoCheckingCapital(alpha, trunc_scale, grid_num,
                                                  prior_mean, prior_var, num,
                                                  sample_num);
                    };
                    
                    VanillaBettingFn bet_fn = [](const Vector32f& samples,
                                                  Float32 prior_mean,
                                                  Float32 delta,
                                                  Int32 grid_num,
                                                  GeoCheckingCapital& gambler) {
                        return vanilla_betting(samples, prior_mean, delta, grid_num, gambler);
                    };
                    
                    return std::make_pair(make_gambler, bet_fn);
                }
                
                case CapitalType::Seq: {
                    // TODO: Fix return type consistency - adaptive factories temporarily disabled
                    throw std::invalid_argument("Sequence checking with vanilla betting not yet fully implemented");
                    /*
                    // Provide a factory and bet function for SequenceCheckingCapital
                    auto make_gambler_seq = [](Float32 delta, Float32 trunc_scale,
                                                Int32 grid_num = 1000,
                                                Float32 prior_mean = 0.5f,
                                                Float32 prior_var = 0.25f,
                                                Int32 num = 1,
                                                Int32 sample_num = 100100) {
                        return SequenceCheckingCapital(delta, trunc_scale, grid_num,
                                                       prior_mean, prior_var, num,
                                                       sample_num);
                    };

                    auto bet_fn_seq = [](const Vector32f& samples,
                                          Float32 prior_mean,
                                          Float32 delta,
                                          Int32 grid_num,
                                          SequenceCheckingCapital& gambler) {
                        return vanilla_betting(samples, prior_mean, delta, grid_num, gambler);
                    };

                    return std::make_pair(make_gambler_seq, bet_fn_seq);
                    */
                }
                
                default:
                    throw std::invalid_argument("Unsupported capital type");
            }
        }
        
        case BetStrategy::Ada: {
            // TODO: Fix return type consistency - adaptive factories temporarily disabled
            throw std::invalid_argument("Adaptive betting strategy factory not yet fully implemented (return type issues)");
            /*
            if (capital == CapitalType::Geo) {
                using MakeGamblerFn = std::function<GeoCheckingCapital(Float32, Float32, Int32)>;
                MakeGamblerFn make_gambler = [](Float32 alpha, Float32 trunc_scale, Int32 grid_num) {
                    return GeoCheckingCapital(alpha, trunc_scale, grid_num);
                };
                
                using AdaBettingFn = std::function<std::pair<Float32, Int32>(
                    const Vector32f&, Float32, Float32, Int32, GeoCheckingCapital&)>;
                
                AdaBettingFn bet_fn = [](const Vector32f& samples,
                                         Float32 prior_mean,
                                         Float32 delta,
                                         Int32 grid_num,
                                         GeoCheckingCapital& gambler) {
                    return adaptive_betting(samples, prior_mean, delta, grid_num, gambler);
                };
                
                return std::make_pair(make_gambler, bet_fn);
                
            } else {
                using MakeGamblerFn = std::function<SequenceCheckingCapital(Float32, Float32, Int32)>;
                MakeGamblerFn make_gambler = [](Float32 alpha, Float32 trunc_scale, Int32 grid_num) {
                    return SequenceCheckingCapital(alpha, trunc_scale, grid_num);
                };
                
                using AdaBettingFn = std::function<std::pair<Float32, Int32>(
                    const Vector32f&, Float32, Float32, Int32, SequenceCheckingCapital&)>;
                
                AdaBettingFn bet_fn = [](const Vector32f& samples,
                                         Float32 prior_mean,
                                         Float32 delta,
                                         Int32 grid_num,
                                         SequenceCheckingCapital& gambler) {
                    return adaptive_betting(samples, prior_mean, delta, grid_num, gambler);
                };
                
                return std::make_pair(make_gambler, bet_fn);
            }
            */
        }
        
        default:
            throw std::invalid_argument("Unsupported betting strategy");
    }
}

/**
 * @brief Convenience function to create GeoCheckingCapital with vanilla betting.
 * 
 * @return Pair of (factory function, betting function)
 */
inline auto vanilla_geo_factory() {
    return betting_factory(BetStrategy::Vanilla, CapitalType::Geo);
}

/**
 * @brief Convenience function to create SequenceCheckingCapital with vanilla betting.
 * 
 * @return Pair of (factory function, betting function)
 */
inline auto vanilla_seq_factory() {
    auto make_gambler_seq = [](Float32 delta, Float32 trunc_scale,
                                Int32 grid_num = 1000,
                                Float32 prior_mean = 0.5f,
                                Float32 prior_var = 0.25f,
                                Int32 num = 1,
                                Int32 sample_num = 100100) {
        return SequenceCheckingCapital(delta, trunc_scale, grid_num,
                                       prior_mean, prior_var, num, sample_num);
    };

    auto bet_fn_seq = [](const Vector32f& samples,
                          Float32 prior_mean,
                          Float32 delta,
                          Int32 grid_num,
                          SequenceCheckingCapital& gambler) {
        return vanilla_betting(samples, prior_mean, delta, grid_num, gambler);
    };

    return std::make_pair(make_gambler_seq, bet_fn_seq);
}

/**
 * @brief Convenience function to create GeoCheckingCapital with adaptive betting.
 * 
 * @return Pair of (factory function, betting function)
 */
inline auto adaptive_geo_factory() {
    return betting_factory(BetStrategy::Ada, CapitalType::Geo);
}

/**
 * @brief Convenience function to create SequenceCheckingCapital with adaptive betting.
 * 
 * @return Pair of (factory function, betting function)
 */
inline auto adaptive_seq_factory() {
    return betting_factory(BetStrategy::Ada, CapitalType::Seq);
}

} // namespace betting

#endif // BETTING_BY_TIME_FRAMEWORK_HPP
