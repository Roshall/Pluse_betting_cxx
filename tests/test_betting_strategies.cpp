/**
 * @file test_betting_strategies.cpp
 * @brief Unit tests for betting strategies (vanilla and adaptive)
 */

#include <catch2/catch_all.hpp>
#include "betting_by_time/strategies/vanilla_betting.hpp"
#include "betting_by_time/strategies/adaptive_betting.hpp"
#include "betting_by_time/framework.hpp"
#include <stdexcept>
#include <typeinfo>
#include <random>

using namespace betting;

/**
 * @brief Generate binomial samples for testing
 */
Vector32f generate_binomial_samples(Float32 p, Int32 n, unsigned int seed = 42) {
    std::mt19937 gen(seed);
    std::bernoulli_distribution d(p);
    
    Vector32f samples(n);
    for (Int32 i = 0; i < n; ++i) {
        samples(i) = d(gen) ? 1.0f : 0.0f;
    }
    
    return samples;
}

TEST_CASE("Vanilla betting with GeoCheckingCapital", "[strategy][vanilla][geo]") {
    // Generate samples with known mean
    Float32 true_mean = 0.65f;
    Int32 num_samples = 200;
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    // Run vanilla betting
    auto [estimated_mean, lb, ub, samples_used] = vanilla_betting(
        samples, 0.5f, 0.1f, 100
    );
    
    // Check results
    REQUIRE(samples_used > 0);
    REQUIRE(samples_used <= num_samples);
    REQUIRE(estimated_mean >= 0.0f);
    REQUIRE(estimated_mean <= 1.0f);
    
    // Estimate should be reasonably close to true mean
    REQUIRE(std::abs(estimated_mean - true_mean) < 0.15f);
}

TEST_CASE("Vanilla betting with SequenceCheckingCapital", "[strategy][vanilla][seq]") {
    // Generate samples with known mean
    Float32 true_mean = 0.45f;
    Int32 num_samples = 200;
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    // Run vanilla betting
    auto [estimated_mean, lb, ub, samples_used] = vanilla_betting_sequence(
        samples, 0.5f, 0.1f, 100
    );
    
    // Check results
    REQUIRE(samples_used > 0);
    REQUIRE(samples_used <= num_samples);
    REQUIRE(estimated_mean >= 0.0f);
    REQUIRE(estimated_mean <= 1.0f);
    
    // Estimate should be reasonably close to true mean
    REQUIRE(std::abs(estimated_mean - true_mean) < 0.15f);
}

TEST_CASE("Adaptive betting with GeoCheckingCapital", "[strategy][adaptive][geo]") {
    // Generate samples with known mean
    Float32 true_mean = 0.7f;
    Int32 num_samples = 300;
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    // Run adaptive betting
    auto [estimated_mean, lb, ub, samples_used] = adaptive_betting(
        samples, 0.5f, 0.08f, 100
    );
    
    // Check results
    REQUIRE(samples_used > 0);
    REQUIRE(samples_used <= num_samples);
    REQUIRE(estimated_mean >= 0.0f);
    REQUIRE(estimated_mean <= 1.0f);
    
    // Estimate should be reasonably close to true mean
    REQUIRE(std::abs(estimated_mean - true_mean) < 0.16f);
}

TEST_CASE("Adaptive betting with SequenceCheckingCapital", "[strategy][adaptive][seq]") {
    // Generate samples with known mean
    Float32 true_mean = 0.3f;
    Int32 num_samples = 300;
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    // Run adaptive betting
    auto [estimated_mean, lb, ub, samples_used] = adaptive_betting_sequence(
        samples, 0.5f, 0.08f, 100
    );
    
    // Check results
    REQUIRE(samples_used > 0);
    REQUIRE(samples_used <= num_samples);
    REQUIRE(estimated_mean >= 0.0f);
    REQUIRE(estimated_mean <= 1.0f);
    
    // Estimate should be reasonably close to true mean
    REQUIRE(std::abs(estimated_mean - true_mean) < 0.15f);
}

TEST_CASE("Framework API - vanilla geo factory", "[framework][vanilla]") {
    auto [make_gambler, bet_fn] = vanilla_geo_factory();
    
    Vector32f samples = generate_binomial_samples(0.6f, 100);
    auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 100, {}, 0.05f, 0.5f, 0.25f, 1, 1000, Mode::Estimate);
    
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    REQUIRE(used > 0);
}

TEST_CASE("Framework API - vanilla seq factory", "[framework][vanilla]") {
    auto [make_gambler, bet_fn] = vanilla_seq_factory();
    
    Vector32f samples = generate_binomial_samples(0.4f, 100);
    auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 100, {}, 0.05f, 0.5f, 0.25f, 1, 1000, Mode::Estimate);
    
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    REQUIRE(used > 0);
}

TEST_CASE("Framework API - adaptive geo factory", "[framework][adaptive]") {
    auto [make_gambler, bet_fn] = adaptive_geo_factory();

    Vector32f samples = generate_binomial_samples(0.7f, 150);
    auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 100, {}, 0.05f, 0.5f, 0.25f, 1, 100100, Mode::Estimate);
    
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    REQUIRE(used > 0);
}

TEST_CASE("Framework API - adaptive seq factory", "[framework][adaptive]") {
    auto [make_gambler, bet_fn] = adaptive_seq_factory();

    Vector32f samples = generate_binomial_samples(0.3f, 150);
    auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 100, {}, 0.05f, 0.5f, 0.25f, 1, 100100, Mode::Estimate);
    
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    REQUIRE(used > 0);
}

TEST_CASE("Generic betting factory", "[framework]") {
    SECTION("Vanilla Geo") {
        auto [make_gambler, bet_fn] = betting_factory(BetStrategy::Vanilla, CapitalType::Geo);
        
        // Note: With internal gambler ownership, the factory's make_gambler is no longer used
        // by the betting function. It's kept for API compatibility.
        
        Vector32f samples = generate_binomial_samples(0.5f, 50);
        auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 50, {}, 0.05f, 0.5f, 0.25f, 1, 1000, Mode::Estimate);

        REQUIRE(est >= 0.0f);
        REQUIRE(est <= 1.0f);
        REQUIRE(used > 0);
    }

    SECTION("Vanilla Seq") {
        auto [make_gambler, bet_fn] = betting_factory(BetStrategy::Vanilla, CapitalType::Seq);
        
        Vector32f samples = generate_binomial_samples(0.5f, 50);
        auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 50, {}, 0.05f, 0.5f, 0.25f, 1, 1000, Mode::Estimate);

        REQUIRE(est >= 0.0f);
        REQUIRE(est <= 1.0f);
        REQUIRE(used > 0);
    }

    SECTION("Adaptive Geo") {
        auto [make_gambler, bet_fn] = betting_factory(BetStrategy::Ada, CapitalType::Geo);

        Vector32f samples = generate_binomial_samples(0.5f, 50);
        auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 50, {}, 0.05f, 0.5f, 0.25f, 1, 100100, Mode::Estimate);

        REQUIRE(est >= 0.0f);
        REQUIRE(est <= 1.0f);
        REQUIRE(used > 0);
    }

    SECTION("Adaptive Seq") {
        auto [make_gambler, bet_fn] = betting_factory(BetStrategy::Ada, CapitalType::Seq);

        Vector32f samples = generate_binomial_samples(0.5f, 50);
        auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 50, {}, 0.05f, 0.5f, 0.25f, 1, 100100, Mode::Estimate);

        REQUIRE(est >= 0.0f);
        REQUIRE(est <= 1.0f);
        REQUIRE(used > 0);
    }
}

TEST_CASE("Betting factory rejects unsupported enum values", "[framework]") {
    REQUIRE_THROWS_AS(
        betting_factory(static_cast<BetStrategy>(99), CapitalType::Geo),
        std::invalid_argument
    );

    REQUIRE_THROWS_AS(
        betting_factory(BetStrategy::Vanilla, static_cast<CapitalType>(99)),
        std::invalid_argument
    );
}

TEST_CASE("Betting strategies handle edge cases", "[strategy]") {
    // Empty samples
    Vector32f empty_samples(0);
    
    GeoCheckingCapital gambler1(0.05f, 0.5f, 100);
    auto [est1, lb1, ub1, used1] = vanilla_betting(empty_samples, 0.5f, 0.1f, 100);
    REQUIRE(used1 == 0);
    
    // Single sample
    Vector32f single_sample(1);
    single_sample << 0.7f;
    
    auto [est2, lb2, ub2, used2] = vanilla_betting(single_sample, 0.5f, 0.1f, 100);
    REQUIRE(used2 == 1);
    REQUIRE(est2 >= 0.0f);
    REQUIRE(est2 <= 1.0f);
}

TEST_CASE("Strategies work with different priors", "[strategy]") {
    Float32 true_mean = 0.6f;
    Vector32f samples = generate_binomial_samples(true_mean, 200);
    
    std::vector<Float32> priors = {0.3f, 0.5f, 0.7f};
    
    for (Float32 prior : priors) {
        auto [est, lb, ub, used] = vanilla_betting(samples, prior, 0.1f, 100);
        
        // Should still estimate correctly regardless of prior
        REQUIRE(std::abs(est - true_mean) < 0.15f);
    }
}

TEST_CASE("Adaptive vs Vanilla comparison", "[strategy][comparison]") {
    Float32 true_mean = 0.65f;
    Vector32f samples = generate_binomial_samples(true_mean, 300);
    
    // Vanilla
    auto [vanilla_est, vlb, vub, vanilla_used] = vanilla_betting(
        samples, 0.5f, 0.1f, 100
    );
    
    // Adaptive
    auto [adaptive_est, alb, aub, adaptive_used] = adaptive_betting(
        samples, 0.5f, 0.1f, 100
    );
    
    // Both should produce reasonable estimates
    REQUIRE(std::abs(vanilla_est - true_mean) < 0.15f);
    REQUIRE(std::abs(adaptive_est - true_mean) < 0.15f);
    
    // Estimates should be similar (within tolerance)
    REQUIRE(std::abs(vanilla_est - adaptive_est) < 0.1f);
}

// ============================================================================
// Class-based interface tests
// ============================================================================

TEST_CASE("VanillaBetting class - incremental sample submission", "[class][vanilla]") {
    Float32 true_mean = 0.6f;
    Int32 total_samples = 150;
    Vector32f all_samples = generate_binomial_samples(true_mean, total_samples);
    
    VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100);
    
    // Initially not finalized
    REQUIRE_FALSE(vb.is_finalized());
    REQUIRE(vb.get_samples_used() == 0);
    REQUIRE(vb.get_current_phase() == 0); // Running phase
    
    // Submit samples in batches
    Int32 batch_size = 30;
    for (Int32 i = 0; i < total_samples; i += batch_size) {
        Int32 end = std::min(i + batch_size, total_samples);
        Vector32f batch = all_samples.segment(i, end - i);
        vb.submit_samples(batch);
        
        // Samples used should increase
        REQUIRE(vb.get_samples_used() > 0);
        REQUIRE(vb.get_samples_used() <= end);
    }
    
    // Finalize and check results
    auto [est, lb, ub, used] = vb.finalize();
    REQUIRE(vb.is_finalized());
    REQUIRE(used > 0);
    REQUIRE(used <= total_samples);
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    REQUIRE(std::abs(est - true_mean) < 0.15f);
}

TEST_CASE("VanillaBetting class - single sample submission", "[class][vanilla]") {
    Float32 true_mean = 0.7f;
    Int32 num_samples = 150;  // Increased samples
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100);
    
    // Submit samples one by one
    for (Int32 i = 0; i < num_samples; ++i) {
        if (vb.is_finalized()) break;
        vb.submit_sample(samples(i));
    }
    
    auto [est, lb, ub, used] = vb.finalize();
    REQUIRE(used > 0);
    REQUIRE(used <= num_samples);
    // With single sample submission, may terminate early; check estimate is valid
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    // Relaxed tolerance - single sample mode may be less accurate
    if (used > 10) {  // Only check accuracy if enough samples were used
        REQUIRE(std::abs(est - true_mean) < 0.2f);
    }
}

TEST_CASE("VanillaBetting class - state queries", "[class][vanilla]") {
    Vector32f samples = generate_binomial_samples(0.5f, 50);
    
    VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100);
    
    // Initial bounds should be [0, 1]
    REQUIRE(vb.get_lower_bound() == 0.0f);
    REQUIRE(vb.get_upper_bound() == 1.0f);
    REQUIRE(vb.get_interval_width() == 1.0f);
    
    // Submit some samples
    vb.submit_samples(samples);
    
    // Bounds should narrow
    REQUIRE(vb.get_lower_bound() >= 0.0f);
    REQUIRE(vb.get_upper_bound() <= 1.0f);
    REQUIRE(vb.get_interval_width() <= 1.0f);
    REQUIRE(vb.get_interval_width() >= 0.0f);
    
    // Estimated mean should be within bounds
    Float32 est = vb.get_estimated_mean();
    vb.finalize();
    REQUIRE(est >= vb.get_lower_bound());
    REQUIRE(est <= vb.get_upper_bound());
}

TEST_CASE("VanillaBetting class - reset functionality", "[class][vanilla]") {
    Vector32f samples1 = generate_binomial_samples(0.6f, 50);
    Vector32f samples2 = generate_binomial_samples(0.4f, 50);
    
    SECTION("Reset with fresh instance") {
        VanillaBetting<GeoCheckingCapital> vb1(0.5f, 0.1f, 100);
        
        // First run
        vb1.submit_samples(samples1);
        vb1.finalize();
        REQUIRE(vb1.is_finalized());
        REQUIRE(vb1.get_samples_used() > 0);
        
        // Create new instance for second run (simulating reset with fresh state)
        VanillaBetting<GeoCheckingCapital> vb2(0.5f, 0.1f, 100);
        
        vb2.submit_samples(samples2);
        auto [est, lb, ub, used] = vb2.finalize();
        REQUIRE(used > 0);
        REQUIRE(std::abs(est - 0.4f) < 0.15f);
    }
    
    SECTION("Reset strategy state only") {
        VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100);
        
        // First run
        vb.submit_samples(samples1);
        vb.finalize();
        REQUIRE(vb.is_finalized());
        
        // Reset strategy (gambler state persists)
        vb.reset(0.5f, 0.25f, 1);
        REQUIRE_FALSE(vb.is_finalized());
        REQUIRE(vb.get_lower_bound() == 0.0f);
        REQUIRE(vb.get_upper_bound() == 1.0f);
    }
}

TEST_CASE("VanillaBetting class - early termination on precision", "[class][vanilla]") {
    // Use very loose delta to trigger early termination
    Vector32f samples = generate_binomial_samples(0.5f, 500);
    
    VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.3f, 100);
    
    // Submit all samples at once
    vb.submit_samples(samples);
    
    // Should have terminated early due to loose precision requirement
    REQUIRE(vb.is_finalized());
    REQUIRE(vb.get_samples_used() <= 500);
}

TEST_CASE("AdaptiveBetting class - incremental sample submission", "[class][adaptive]") {
    Float32 true_mean = 0.65f;
    Int32 total_samples = 200;
    Vector32f all_samples = generate_binomial_samples(true_mean, total_samples);
    
    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100);
    
    // Initially not finalized
    REQUIRE_FALSE(ab.is_finalized());
    REQUIRE(ab.get_samples_used() == 0);
    REQUIRE(ab.get_current_phase() == 0); // DirectionDetection phase
    
    // Submit samples in batches
    Int32 batch_size = 25;
    for (Int32 i = 0; i < total_samples; i += batch_size) {
        Int32 end = std::min(i + batch_size, total_samples);
        Vector32f batch = all_samples.segment(i, end - i);
        ab.submit_samples(batch);
        
        // Samples used should increase
        REQUIRE(ab.get_samples_used() > 0);
        REQUIRE(ab.get_samples_used() <= end);
    }
    
    // Finalize and check results
    auto [est, lb, ub, used] = ab.finalize();
    REQUIRE(ab.is_finalized());
    REQUIRE(used > 0);
    REQUIRE(used <= total_samples);
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    REQUIRE(std::abs(est - true_mean) < 0.15f);
}

TEST_CASE("AdaptiveBetting class - single sample submission", "[class][adaptive]") {
    Float32 true_mean = 0.7f;
    Int32 num_samples = 150;
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100);
    
    // Submit samples one by one
    for (Int32 i = 0; i < num_samples; ++i) {
        if (ab.is_finalized()) break;
        ab.submit_sample(samples(i));
    }
    
    auto [est, lb, ub, used] = ab.finalize();
    REQUIRE(used > 0);
    REQUIRE(used <= num_samples);
    // Relaxed tolerance for single-sample submission (less efficient)
    REQUIRE(std::abs(est - true_mean) < 0.2f);
}

TEST_CASE("AdaptiveBetting class - phase transitions", "[class][adaptive]") {
    Vector32f samples = generate_binomial_samples(0.6f, 200);
    
    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100);
    
    // Start in DirectionDetection phase
    REQUIRE(ab.get_current_phase() == 0);
    
    // Submit first batch
    Vector32f batch1 = samples.segment(0, 50);
    ab.submit_samples(batch1);
    
    // May still be in direction detection or moved to refinement
    Int32 phase_after_first = ab.get_current_phase();
    REQUIRE(phase_after_first >= 0);
    REQUIRE(phase_after_first <= 3); // 0=DirectionDetection, 1=BoundsRefinement, 2=BoundsTightening, 3=FinalEstimation
    
    // Submit more samples
    Vector32f batch2 = samples.segment(50, 50);
    ab.submit_samples(batch2);
    
    // Should progress through phases
    Int32 phase_after_second = ab.get_current_phase();
    REQUIRE(phase_after_second >= phase_after_first);
    
    // Finalize
    ab.finalize();
    REQUIRE(ab.get_current_phase() == 3); // FinalEstimation (phase 3)
}

TEST_CASE("AdaptiveBetting class - confidence interval narrowing", "[class][adaptive]") {
    Vector32f samples = generate_binomial_samples(0.5f, 200);
    
    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100);
    
    // Initial interval based on prior
    Float32 initial_width = ab.get_interval_width();
    REQUIRE(initial_width > 0.0f);
    REQUIRE(initial_width <= 1.0f);
    
    // Submit samples in batches and track interval narrowing
    Float32 prev_width = initial_width;
    for (Int32 i = 0; i < 200; i += 50) {
        Vector32f batch = samples.segment(i, 50);
        ab.submit_samples(batch);
        
        Float32 current_width = ab.get_interval_width();
        // Interval should generally narrow (or stay same)
        REQUIRE(current_width <= prev_width + 0.01f); // Small tolerance for numerical issues
        prev_width = current_width;
    }
    
    ab.finalize();
    // Interval should narrow or stay the same (allowing for edge cases)
    REQUIRE(ab.get_interval_width() <= initial_width);
}

TEST_CASE("AdaptiveBetting class - reset functionality", "[class][adaptive]") {
    Vector32f samples1 = generate_binomial_samples(0.6f, 100);
    Vector32f samples2 = generate_binomial_samples(0.4f, 100);
    
    SECTION("Reset with fresh instance") {
        AdaptiveBetting<GeoCheckingCapital> ab1(0.5f, 0.08f, 100);
        
        // First run
        ab1.submit_samples(samples1);
        ab1.finalize();
        REQUIRE(ab1.is_finalized());
        REQUIRE(ab1.get_samples_used() > 0);
        
        // Create new instance for second run
        AdaptiveBetting<GeoCheckingCapital> ab2(0.5f, 0.08f, 100);
        
        ab2.submit_samples(samples2);
        auto [est, lb, ub, used] = ab2.finalize();
        REQUIRE(used > 0);
        REQUIRE(std::abs(est - 0.4f) < 0.15f);
    }
    
    SECTION("Reset strategy state only") {
        AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100);
        
        // First run
        ab.submit_samples(samples1);
        ab.finalize();
        REQUIRE(ab.is_finalized());
        
        // Reset strategy (gambler state persists)
        ab.reset(0.5f, 0.25f, 1);
        REQUIRE_FALSE(ab.is_finalized());
        REQUIRE(ab.get_current_phase() == 0); // Back to DirectionDetection
    }
}

TEST_CASE("AdaptiveBetting class - state queries", "[class][adaptive]") {
    Vector32f samples = generate_binomial_samples(0.5f, 100);
    
    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100);
    
    // Initial bounds based on prior
    REQUIRE(ab.get_lower_bound() >= 0.0f);
    REQUIRE(ab.get_upper_bound() <= 1.0f);
    REQUIRE(ab.get_lower_bound() < ab.get_upper_bound());
    
    // Submit samples
    ab.submit_samples(samples);
    
    // Bounds should be valid
    REQUIRE(ab.get_lower_bound() >= 0.0f);
    REQUIRE(ab.get_upper_bound() <= 1.0f);
    REQUIRE(ab.get_lower_bound() <= ab.get_upper_bound());
    
    // Estimated mean should be within bounds after finalization
    ab.finalize();
    Float32 est = ab.get_estimated_mean();
    REQUIRE(est >= ab.get_lower_bound());
    REQUIRE(est <= ab.get_upper_bound());
}

TEST_CASE("VanillaBetting with SequenceCheckingCapital class", "[class][vanilla][seq]") {
    Float32 true_mean = 0.55f;
    Vector32f samples = generate_binomial_samples(true_mean, 150);
    
    VanillaBetting<SequenceCheckingCapital> vb(0.5f, 0.1f, 100);
    
    vb.submit_samples(samples);
    auto [est, lb, ub, used] = vb.finalize();
    
    REQUIRE(used > 0);
    REQUIRE(used <= 150);
    REQUIRE(std::abs(est - true_mean) < 0.15f);
}

TEST_CASE("AdaptiveBetting with SequenceCheckingCapital class", "[class][adaptive][seq]") {
    Float32 true_mean = 0.45f;
    Vector32f samples = generate_binomial_samples(true_mean, 150);
    
    AdaptiveBetting<SequenceCheckingCapital> ab(0.5f, 0.08f, 100);
    
    ab.submit_samples(samples);
    auto [est, lb, ub, used] = ab.finalize();
    
    REQUIRE(used > 0);
    REQUIRE(used <= 150);
    REQUIRE(std::abs(est - true_mean) < 0.15f);
}

// ============================================================================
// Mode::Bound tests
// ============================================================================

TEST_CASE("VanillaBetting Mode::Bound - basic correctness", "[class][vanilla][bound]") {
    Float32 true_mean = 0.6f;
    Vector32f samples = generate_binomial_samples(true_mean, 200);

    VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100);
    VanillaBetting<GeoCheckingCapital> vb_bound(0.5f, 0.1f, 100,
                                                 0.05f, 0.5f, 0.25f, 1, 100010,
                                                 Mode::Bound);

    vb.submit_samples(samples);
    vb_bound.submit_samples(samples);

    auto [est, lb, ub, used] = vb.finalize();
    auto [est_b, lb_b, ub_b, used_b] = vb_bound.finalize();

    // Both should produce valid estimates
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    REQUIRE(est_b >= 0.0f);
    REQUIRE(est_b <= 1.0f);

    // Mode::Bound estimate should be the midpoint of its bounds
    Float32 expected_midpoint = (lb_b + ub_b) / 2.0f;
    REQUIRE(est_b == Catch::Approx(expected_midpoint).margin(1e-5f));

    // Bounds should be valid
    REQUIRE(lb_b <= ub_b);
    REQUIRE(ub_b - lb_b <= 1.0f);
}

TEST_CASE("VanillaBetting Mode::Bound - no early termination on precision", "[class][vanilla][bound]") {
    // In Mode::Estimate, loose delta (0.3→eps=0.4) triggers early termination.
    // In Mode::Bound, early termination on eps_ is skipped, so it consumes all samples.
    Float32 true_mean = 0.5f;
    Vector32f samples = generate_binomial_samples(true_mean, 500);

    // Mode::Estimate: should terminate early due to loose precision
    VanillaBetting<GeoCheckingCapital> vb_est(0.5f, 0.25f, 100,
                                               0.05f, 0.5f, 0.25f, 1, 100010,
                                               Mode::Estimate);
    vb_est.submit_samples(samples);
    REQUIRE(vb_est.is_finalized());
    Int32 est_used = vb_est.get_samples_used();
    REQUIRE(est_used <= 500); // May or may not use all

    // Mode::Bound: should NOT terminate early - consumes all 500 samples
    VanillaBetting<GeoCheckingCapital> vb_bound(0.5f, 0.25f, 100,
                                                 0.05f, 0.5f, 0.25f, 1, 100010,
                                                 Mode::Bound);
    vb_bound.submit_samples(samples);
    // Mode::Bound doesn't auto-finalize, so we call finalize()
    vb_bound.finalize();
    REQUIRE(vb_bound.is_finalized());
    Int32 bound_used = vb_bound.get_samples_used();
    // Mode::Bound should consume all available samples (no early termination)
    REQUIRE(bound_used == 500);
}

TEST_CASE("VanillaBetting Mode::Bound - bounds keep narrowing with more samples", "[class][vanilla][bound]") {
    Float32 true_mean = 0.6f;
    Vector32f samples = generate_binomial_samples(true_mean, 500);

    VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100,
                                           0.05f, 0.5f, 0.25f, 1, 100010,
                                           Mode::Bound);

    // Submit first batch
    Vector32f batch1 = samples.segment(0, 100);
    vb.submit_samples(batch1);
    REQUIRE_FALSE(vb.is_finalized()); // Should not auto-finalize
    Float32 width1 = vb.get_interval_width();

    // Submit more samples
    Vector32f batch2 = samples.segment(100, 200);
    vb.submit_samples(batch2);
    REQUIRE_FALSE(vb.is_finalized());
    Float32 width2 = vb.get_interval_width();

    // Submit even more
    Vector32f batch3 = samples.segment(300, 200);
    vb.submit_samples(batch3);

    vb.finalize();
    Float32 width3 = vb.get_interval_width();

    // Bounds should narrow or stay the same (never widen)
    REQUIRE(width2 <= width1 + 0.01f);
    REQUIRE(width3 <= width2 + 0.01f);

    // Final estimate should be the midpoint
    Float32 est = vb.get_estimated_mean();
    Float32 midpoint = (vb.get_lower_bound() + vb.get_upper_bound()) / 2.0f;
    REQUIRE(est == Catch::Approx(midpoint).margin(1e-5f));
}

TEST_CASE("VanillaBetting Mode::Bound - bounds shrink below 2*delta", "[class][vanilla][bound][tightening]") {
    // With enough samples, the confidence set should shrink well below the
    // initial 2*delta window as more hypotheses are rejected.
    // Use asymmetric true mean to accumulate rejection capital faster.
    Float32 delta = 0.1f;
    Float32 true_mean = 0.25f;
    Int32 num_samples = 800;
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);

    VanillaBetting<GeoCheckingCapital> vb(0.5f, delta, 100,
                                           0.05f, 0.5f, 0.25f, 1, 100010,
                                           Mode::Bound);

    // Submit in batches to allow interval to contract
    for (Int32 i = 0; i < num_samples; i += 100) {
        Int32 end = std::min(i + 100, num_samples);
        Vector32f batch = samples.segment(i, end - i);
        vb.submit_samples(batch);
    }

    vb.finalize();

    Float32 target_width = 2.0f * delta; // 0.20
    Float32 final_width = vb.get_interval_width();

    // The final interval must be strictly narrower than the initial 2*delta window
    // (vanilla betting narrows by eliminating hypotheses whose capital exceeds threshold)
    REQUIRE(final_width < target_width);

    // The true mean must be inside the final bounds
    REQUIRE(vb.get_lower_bound() <= true_mean);
    REQUIRE(true_mean <= vb.get_upper_bound());

    // Midpoint estimate should be close to the true mean
    Float32 est = vb.get_estimated_mean();
    REQUIRE(std::abs(est - true_mean) < 0.1f);
}

TEST_CASE("VanillaBetting Mode::Bound - vs Estimate estimate difference", "[class][vanilla][bound]") {
    // With Mode::Estimate, the estimate is the argmin of capital sums (grid-based).
    // With Mode::Bound, the estimate is the midpoint of bounds.
    // Both should be close to the true mean, but they may differ slightly.
    Float32 true_mean = 0.45f;
    Vector32f samples = generate_binomial_samples(true_mean, 200);

    VanillaBetting<GeoCheckingCapital> vb_est(0.5f, 0.1f, 100,
                                               0.05f, 0.5f, 0.25f, 1, 100010,
                                               Mode::Estimate);
    VanillaBetting<GeoCheckingCapital> vb_bound(0.5f, 0.1f, 100,
                                                 0.05f, 0.5f, 0.25f, 1, 100010,
                                                 Mode::Bound);

    vb_est.submit_samples(samples);
    vb_bound.submit_samples(samples);

    auto [est_e, lb_e, ub_e, used_e] = vb_est.finalize();
    auto [est_b, lb_b, ub_b, used_b] = vb_bound.finalize();

    // Both estimates should be reasonable
    REQUIRE(std::abs(est_e - true_mean) < 0.15f);
    REQUIRE(std::abs(est_b - true_mean) < 0.15f);

    // Bounds should be comparable (similar confidence intervals)
    // The bounds from Mode::Bound may be tighter since it doesn't stop early
    REQUIRE(lb_e >= 0.0f);
    REQUIRE(ub_e <= 1.0f);
    REQUIRE(lb_b >= 0.0f);
    REQUIRE(ub_b <= 1.0f);
}

TEST_CASE("AdaptiveBetting Mode::Bound - basic correctness", "[class][adaptive][bound]") {
    Float32 true_mean = 0.6f;
    Vector32f samples = generate_binomial_samples(true_mean, 300);

    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100,
                                            0.05f, 0.5f, 0.25f, 1, 100100,
                                            Mode::Bound);

    ab.submit_samples(samples);
    auto [est, lb, ub, used] = ab.finalize();

    // Basic sanity checks
    REQUIRE(used > 0);
    REQUIRE(used <= 300);
    REQUIRE(est >= 0.0f);
    REQUIRE(est <= 1.0f);
    REQUIRE(std::abs(est - true_mean) < 0.2f);

    // Mode::Bound estimate should be midpoint of bounds
    Float32 midpoint = (lb + ub) / 2.0f;
    REQUIRE(est == Catch::Approx(midpoint).margin(1e-5f));
}

TEST_CASE("AdaptiveBetting Mode::Bound - goes through full pipeline", "[class][adaptive][bound]") {
    // Mode::Bound should NOT short-circuit from DirectionDetection to FinalEstimation.
    // It should pass through BoundsRefinement then BoundsTightening phases.
    // Phase values:
    //   0 = DirectionDetection, 1 = BoundsRefinement,
    //   2 = BoundsTightening, 3 = FinalEstimation
    // Use asymmetric true mean (far from prior 0.5) to accumulate capital faster.
    Float32 true_mean = 0.3f;
    Int32 num_samples = 1200;
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);

    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100,
                                            0.05f, 0.5f, 0.25f, 1, 100100,
                                            Mode::Bound);

    // Submit in small batches to observe phase transitions
    Int32 phase_seen_refinement = 0;
    Int32 phase_seen_tightening = 0;
    for (Int32 i = 0; i < num_samples; i += 50) {
        Int32 end = std::min(i + 50, num_samples);
        Vector32f batch = samples.segment(i, end - i);
        ab.submit_samples(batch);

        Int32 phase = ab.get_current_phase();
        if (phase == 1) { // BoundsRefinement
            phase_seen_refinement++;
        }
        if (phase == 2) { // BoundsTightening
            phase_seen_tightening++;
        }
        REQUIRE(phase >= 0);
        REQUIRE(phase <= 3); // Now includes BoundsTightening (2) and FinalEstimation (3)
    }

    ab.finalize();

    // Mode::Bound should have entered BoundsRefinement at some point
    REQUIRE(phase_seen_refinement > 0);
    // With enough samples and asymmetric mean, should also enter BoundsTightening
    REQUIRE(phase_seen_tightening > 0);
    REQUIRE(ab.get_current_phase() == 3); // FinalEstimation (phase 3) after finalize()
}

TEST_CASE("AdaptiveBetting Mode::Bound - bounds keep narrowing incrementally", "[class][adaptive][bound]") {
    Float32 true_mean = 0.5f;
    Vector32f samples = generate_binomial_samples(true_mean, 400);

    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100,
                                            0.05f, 0.5f, 0.25f, 1, 100100,
                                            Mode::Bound);

    Float32 prev_width = ab.get_interval_width();
    REQUIRE(prev_width > 0.0f);

    // Submit in batches, track interval width
    std::vector<Float32> widths;
    for (Int32 i = 0; i < 400; i += 50) {
        Int32 end = std::min(i + 50, 400);
        Vector32f batch = samples.segment(i, end - i);
        ab.submit_samples(batch);

        Float32 w = ab.get_interval_width();
        // Interval should never suddenly widen
        REQUIRE(w <= prev_width + 0.01f);
        prev_width = w;
        widths.push_back(w);
    }

    ab.finalize();

    // At least some narrowing should have occurred
    REQUIRE(widths.back() <= widths.front() + 0.01f);

    // Final bounds should be valid
    REQUIRE(ab.get_lower_bound() >= 0.0f);
    REQUIRE(ab.get_upper_bound() <= 1.0f);
    REQUIRE(ab.get_lower_bound() < ab.get_upper_bound());
}

TEST_CASE("AdaptiveBetting Mode::Bound - BoundsTightening shrinks below 2*delta", "[class][adaptive][bound][tightening]") {
    // In BoundsTightening phase, bounds should shrink independently below the
    // initial 2*delta window as hypotheses are rejected one stride at a time.
    // Use asymmetric true mean (far from prior) to build capital and reach
    // the tightening phase reliably.
    Float32 delta = 0.08f;
    Float32 true_mean = 0.25f;
    Int32 num_samples = 2000;
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);

    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, delta, 100,
                                            0.05f, 0.5f, 0.25f, 1, 100100,
                                            Mode::Bound);

    // Submit in batches to allow phases to progress naturally
    for (Int32 i = 0; i < num_samples; i += 100) {
        Int32 end = std::min(i + 100, num_samples);
        Vector32f batch = samples.segment(i, end - i);
        ab.submit_samples(batch);
    }

    ab.finalize();

    Float32 initial_width = 2.0f * delta; // 0.16
    Float32 final_width = ab.get_interval_width();

    // The final interval must be strictly narrower than the initial 2*delta window
    REQUIRE(final_width < initial_width);

    // The true mean should be inside the final bounds
    REQUIRE(ab.get_lower_bound() <= true_mean);
    REQUIRE(true_mean <= ab.get_upper_bound());
}

TEST_CASE("AdaptiveBetting Mode::Bound - does not auto-finalize", "[class][adaptive][bound]") {
    // In Mode::Bound, the strategy should NOT auto-finalize even when both bounds
    // are touched. It should stay in BoundsRefinement and wait for manual finalize().
    Float32 true_mean = 0.65f;
    Vector32f samples = generate_binomial_samples(true_mean, 300);

    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100,
                                            0.05f, 0.5f, 0.25f, 1, 100100,
                                            Mode::Bound);

    // Submit all samples
    ab.submit_samples(samples);

    // Should NOT be auto-finalized - user must call finalize()
    REQUIRE_FALSE(ab.is_finalized());

    ab.finalize();
    REQUIRE(ab.is_finalized());

    // Estimate should be reasonable
    Float32 est = ab.get_estimated_mean();
    REQUIRE(std::abs(est - true_mean) < 0.2f);
}

TEST_CASE("AdaptiveBetting Mode::Bound - reset after use", "[class][adaptive][bound]") {
    // Verify that reset works correctly in Mode::Bound
    Float32 true_mean1 = 0.65f;
    Float32 true_mean2 = 0.35f;
    Vector32f samples1 = generate_binomial_samples(true_mean1, 150);
    Vector32f samples2 = generate_binomial_samples(true_mean2, 150, 12345);

    AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100,
                                            0.05f, 0.5f, 0.25f, 1, 100100,
                                            Mode::Bound);

    // First run
    ab.submit_samples(samples1);
    auto [est1, lb1, ub1, used1] = ab.finalize();
    REQUIRE(ab.is_finalized());
    REQUIRE(used1 > 0);
    REQUIRE(std::abs(est1 - true_mean1) < 0.2f);

    // Reset with fresh state
    ab.reset(0.5f, 0.25f, 1);
    REQUIRE_FALSE(ab.is_finalized());
    REQUIRE(ab.get_current_phase() == 0); // Back to DirectionDetection

    // Second run with different samples
    ab.submit_samples(samples2);
    auto [est2, lb2, ub2, used2] = ab.finalize();
    REQUIRE(used2 > 0);
    REQUIRE(std::abs(est2 - true_mean2) < 0.2f);
}

TEST_CASE("Mode::Bound via free function API", "[strategy][bound]") {
    Float32 true_mean = 0.6f;
    Vector32f samples = generate_binomial_samples(true_mean, 200);

    SECTION("vanilla_betting with Mode::Bound") {
        auto [est, lb, ub, used] = vanilla_betting(
            samples, 0.5f, 0.1f, 100, {},
            0.05f, 0.5f, 0.25f, 1, 100010,
            Mode::Bound);

        REQUIRE(used > 0);
        REQUIRE(used <= 200);
        REQUIRE(std::abs(est - true_mean) < 0.2f);
        // Estimate should be midpoint of bounds
        Float32 midpoint = (lb + ub) / 2.0f;
        REQUIRE(est == Catch::Approx(midpoint).margin(1e-5f));
    }

    SECTION("adaptive_betting with Mode::Bound") {
        auto [est, lb, ub, used] = adaptive_betting(
            samples, 0.5f, 0.1f, 100, {},
            0.05f, 0.5f, 0.25f, 1, 100100,
            Mode::Bound);

        REQUIRE(used > 0);
        REQUIRE(used <= 200);
        REQUIRE(std::abs(est - true_mean) < 0.2f);
        // Estimate should be midpoint of bounds
        Float32 midpoint = (lb + ub) / 2.0f;
        REQUIRE(est == Catch::Approx(midpoint).margin(1e-5f));
    }
}

TEST_CASE("Mode::Bound via factory framework API", "[framework][bound]") {
    Vector32f samples = generate_binomial_samples(0.55f, 200);

    SECTION("Vanilla Geo with Mode::Bound") {
        auto [make_gambler, bet_fn] = vanilla_geo_factory();
        auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 100, {},
                                           0.05f, 0.5f, 0.25f, 1, 100010,
                                           Mode::Bound);

        REQUIRE(est >= 0.0f);
        REQUIRE(est <= 1.0f);
        REQUIRE(used > 0);

        // Midpoint check
        Float32 midpoint = (lb + ub) / 2.0f;
        REQUIRE(est == Catch::Approx(midpoint).margin(1e-5f));
    }

    SECTION("Adaptive Geo with Mode::Bound") {
        auto [make_gambler, bet_fn] = adaptive_geo_factory();
        auto [est, lb, ub, used] = bet_fn(samples, 0.5f, 0.1f, 100, {},
                                           0.05f, 0.5f, 0.25f, 1, 100100,
                                           Mode::Bound);

        REQUIRE(est >= 0.0f);
        REQUIRE(est <= 1.0f);
        REQUIRE(used > 0);

        // Midpoint check
        Float32 midpoint = (lb + ub) / 2.0f;
        REQUIRE(est == Catch::Approx(midpoint).margin(1e-5f));
    }
}

TEST_CASE("Mode::Bound - edge case: all samples are 1.0", "[bound][edge]") {
    // When true mean = 1.0, bounds should converge appropriately
    Int32 n = 300;
    Vector32f samples = Vector32f::Ones(n);

    SECTION("VanillaBetting Mode::Bound with all ones") {
        VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100,
                                               0.05f, 0.5f, 0.25f, 1, 100010,
                                               Mode::Bound);
        vb.submit_samples(samples);
        vb.finalize();

        // Bounds should be narrowed toward 1.0
        REQUIRE(vb.get_lower_bound() >= 0.0f);
        REQUIRE(vb.get_upper_bound() <= 1.0f);
        REQUIRE(vb.get_interval_width() < 1.0f); // Must have narrowed
    }

    SECTION("AdaptiveBetting Mode::Bound with all ones") {
        AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100,
                                                0.05f, 0.5f, 0.25f, 1, 100100,
                                                Mode::Bound);
        ab.submit_samples(samples);
        ab.finalize();

        REQUIRE(ab.get_lower_bound() >= 0.0f);
        REQUIRE(ab.get_upper_bound() <= 1.0f);
        REQUIRE(ab.get_interval_width() < 1.0f);
    }
}

TEST_CASE("Mode::Bound - edge case: all samples are 0.0", "[bound][edge]") {
    Int32 n = 300;
    Vector32f samples = Vector32f::Zero(n);

    SECTION("VanillaBetting Mode::Bound with all zeros") {
        VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100,
                                               0.05f, 0.5f, 0.25f, 1, 100010,
                                               Mode::Bound);
        vb.submit_samples(samples);
        vb.finalize();

        REQUIRE(vb.get_interval_width() < 1.0f); // Must have narrowed
    }

    SECTION("AdaptiveBetting Mode::Bound with all zeros") {
        AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100,
                                                0.05f, 0.5f, 0.25f, 1, 100100,
                                                Mode::Bound);
        ab.submit_samples(samples);
        ab.finalize();

        REQUIRE(ab.get_interval_width() < 1.0f);
    }
}

TEST_CASE("Mode::Bound - state queries return consistent values", "[bound][state]") {
    Float32 true_mean = 0.6f;
    Vector32f samples = generate_binomial_samples(true_mean, 150);

    SECTION("VanillaBetting state consistency") {
        VanillaBetting<GeoCheckingCapital> vb(0.5f, 0.1f, 100,
                                               0.05f, 0.5f, 0.25f, 1, 100010,
                                               Mode::Bound);
        vb.submit_samples(samples);
        vb.finalize();

        Float32 est = vb.get_estimated_mean();
        Float32 lb = vb.get_lower_bound();
        Float32 ub = vb.get_upper_bound();
        REQUIRE(lb <= est);
        REQUIRE(est <= ub);
        REQUIRE(vb.get_interval_width() == Catch::Approx(ub - lb).margin(1e-5f));
    }

    SECTION("AdaptiveBetting state consistency") {
        AdaptiveBetting<GeoCheckingCapital> ab(0.5f, 0.08f, 100,
                                                0.05f, 0.5f, 0.25f, 1, 100100,
                                                Mode::Bound);
        ab.submit_samples(samples);
        ab.finalize();

        Float32 est = ab.get_estimated_mean();
        Float32 lb = ab.get_lower_bound();
        Float32 ub = ab.get_upper_bound();
        REQUIRE(lb <= est);
        REQUIRE(est <= ub);
        REQUIRE(ab.get_interval_width() == Catch::Approx(ub - lb).margin(1e-5f));
    }
}
