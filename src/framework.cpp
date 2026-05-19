#include "betting_by_time/framework.hpp"

#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"
#include "betting_by_time/strategies/vanilla_betting.hpp"
#include "betting_by_time/strategies/adaptive_betting.hpp"

#include <stdexcept>

namespace betting {
BettingFactoryResult betting_factory(BetStrategy strategy, CapitalType capital) {
    switch (strategy) {
        case BetStrategy::Vanilla: {
            switch (capital) {
                case CapitalType::Geo: {
                    AnyGamblerFactory make_gambler = [](Float32 alpha, Float32 trunc_scale, Int32 grid_num) -> std::any {
                        return std::make_any<GeoCheckingCapital>(alpha, trunc_scale, grid_num);
                    };
                    
                    AnyBettingFn bet_fn = [](const Vector32f& samples,
                                             Float32 prior_mean,
                                             Float32 delta,
                                             Int32 grid_num,
                                             std::any& gambler_any) {
                        auto& gambler = std::any_cast<GeoCheckingCapital&>(gambler_any);
                        return vanilla_betting(samples, prior_mean, delta, grid_num, gambler);
                    };
                    
                    return std::make_pair(make_gambler, bet_fn);
                }
                
                case CapitalType::Seq: {
                    AnyGamblerFactory make_gambler = [](Float32 alpha, Float32 trunc_scale, Int32 grid_num) -> std::any {
                        return std::make_any<SequenceCheckingCapital>(alpha, trunc_scale, grid_num);
                    };
                    
                    AnyBettingFn bet_fn = [](const Vector32f& samples,
                                             Float32 prior_mean,
                                             Float32 delta,
                                             Int32 grid_num,
                                             std::any& gambler_any) {
                        auto& gambler = std::any_cast<SequenceCheckingCapital&>(gambler_any);
                        return vanilla_betting(samples, prior_mean, delta, grid_num, gambler);
                    };
                    
                    return std::make_pair(make_gambler, bet_fn);
                }
                
                default:
                    throw std::invalid_argument("Unsupported capital type");
            }
        }
        
        case BetStrategy::Ada: {
            switch (capital) {
                case CapitalType::Geo: {
                    AnyGamblerFactory make_gambler = [](Float32 alpha, Float32 trunc_scale, Int32 grid_num) -> std::any {
                        return std::make_any<GeoCheckingCapital>(alpha, trunc_scale, grid_num);
                    };
                    
                    AnyBettingFn bet_fn = [](const Vector32f& samples,
                                             Float32 prior_mean,
                                             Float32 delta,
                                             Int32 grid_num,
                                             std::any& gambler_any) {
                        auto& gambler = std::any_cast<GeoCheckingCapital&>(gambler_any);
                        return adaptive_betting(samples, prior_mean, delta, grid_num, gambler);
                    };
                    
                    return std::make_pair(make_gambler, bet_fn);
                }
                
                case CapitalType::Seq: {
                    AnyGamblerFactory make_gambler = [](Float32 alpha, Float32 trunc_scale, Int32 grid_num) -> std::any {
                        return std::make_any<SequenceCheckingCapital>(alpha, trunc_scale, grid_num);
                    };
                    
                    AnyBettingFn bet_fn = [](const Vector32f& samples,
                                             Float32 prior_mean,
                                             Float32 delta,
                                             Int32 grid_num,
                                             std::any& gambler_any) {
                        auto& gambler = std::any_cast<SequenceCheckingCapital&>(gambler_any);
                        return adaptive_betting(samples, prior_mean, delta, grid_num, gambler);
                    };
                    
                    return std::make_pair(make_gambler, bet_fn);
                }
                
                default:
                    throw std::invalid_argument("Unsupported capital type");
            }
        }
        
        default:
            throw std::invalid_argument("Unsupported betting strategy");
    }
}

}
