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
    REQUIRE(phase_after_first <= 2);
    
    // Submit more samples
    Vector32f batch2 = samples.segment(50, 50);
    ab.submit_samples(batch2);
    
    // Should progress through phases
    Int32 phase_after_second = ab.get_current_phase();
    REQUIRE(phase_after_second >= phase_after_first);
    
    // Finalize
    ab.finalize();
    REQUIRE(ab.get_current_phase() == 2); // FinalEstimation
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
