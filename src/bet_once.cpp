#include "betting_by_time/bet_once.hpp"

namespace betting {

Float64 geo_single_bet_on(Float32 trunc_scale, Float32 m, 
                          const Vector32f& samples,
                          Float64 cum_cap, Float32 capital) {
    if ((cum_cap < 1e-16) || (cum_cap > 1e16)) return cum_cap;
    
    const auto m_f64 = static_cast<Float64>(m);
    const Float64 lower_bound =
        -static_cast<Float64>(trunc_scale) / (1.0 + 1e-9 - m_f64);
    const Float64 upper_bound =
        static_cast<Float64>(trunc_scale) / (m_f64 + 1e-9);
    const Float64 bet = std::clamp(static_cast<Float64>(capital), lower_bound, upper_bound);
    
    const Float64 product =
        (1.0 + (samples.cast<Float64>().array() - m_f64) * bet).prod();
    assert(product >= 0.0 && "Product of earnings should be non-negative");
    
    return cum_cap * product;
}

Float64 seq_single_bet_on(Float32 trunc_scale, Float32 m, 
                          Float32 sample,
                          Float64 cum_cap, Float32 capital) {
    const auto sample_f64 = static_cast<Float64>(sample);
    const auto m_f64 = static_cast<Float64>(m);
    const auto abs_cum_cap = std::abs(cum_cap);

    if ((std::abs(sample_f64 - m_f64) < 1e-9) || (abs_cum_cap < 1e-16) || (abs_cum_cap > 1e16)) {
        return cum_cap;
    }

    const auto trunc_f64 = static_cast<Float64>(trunc_scale);
    const auto lower_bound = -trunc_f64 / (1.0 + 1e-9 - m_f64);
    const auto upper_bound = trunc_f64 / (m_f64 + 1e-9);

    const auto lbd_m = std::clamp(static_cast<Float64>(capital),
                                  lower_bound,
                                  upper_bound);
    assert (lbd_m * (sample_f64 - m_f64) > -1.0 && "Earnings factor should be non-negative");

    return cum_cap * (1.0 + lbd_m * (sample_f64 - m_f64));
}

} // namespace betting
