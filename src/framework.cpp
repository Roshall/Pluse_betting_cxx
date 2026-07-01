#include "betting_by_time/framework.hpp"

#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"
#include "betting_by_time/strategies/vanilla_betting.hpp"
#include "betting_by_time/strategies/adaptive_betting.hpp"

#include <stdexcept>

namespace betting {
namespace {

using BettingSignature = std::tuple<Float32, Float32, Float32, Int32>(
    const Vector32f&, Float32, Float32, Int32, const std::vector<Int32>&,
    Float32, Float32, Float32, Int32, Int32, Mode);

std::any make_geo_gambler(Float32 alpha, Float32 trunc_scale, Int32 grid_num) {
    return std::make_any<GeoCheckingCapital>(alpha, trunc_scale, grid_num);
}

std::any make_seq_gambler(Float32 alpha, Float32 trunc_scale, Int32 grid_num) {
    return std::make_any<SequenceCheckingCapital>(alpha, trunc_scale, grid_num);
}

AnyGamblerFactory make_gambler_for(CapitalType capital) {
    switch (capital) {
        case CapitalType::Geo: return make_geo_gambler;
        case CapitalType::Seq: return make_seq_gambler;
        default: throw std::invalid_argument("Unsupported capital type");
    }
}

} // namespace

BettingFactoryResult betting_factory(BetStrategy strategy, CapitalType capital) {
    AnyGamblerFactory make_gambler = make_gambler_for(capital);

    AnyBettingFn bet_fn;
    switch (strategy) {
        case BetStrategy::Vanilla:
            bet_fn = (capital == CapitalType::Geo)
                ? AnyBettingFn(static_cast<BettingSignature*>(vanilla_betting))
                : AnyBettingFn(static_cast<BettingSignature*>(vanilla_betting_sequence));
            break;
        case BetStrategy::Ada:
            bet_fn = (capital == CapitalType::Geo)
                ? AnyBettingFn(static_cast<BettingSignature*>(adaptive_betting))
                : AnyBettingFn(static_cast<BettingSignature*>(adaptive_betting_sequence));
            break;
        default:
            throw std::invalid_argument("Unsupported betting strategy");
    }

    return {make_gambler, bet_fn};
}

}
