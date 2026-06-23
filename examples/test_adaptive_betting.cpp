/**
 * @file test_adaptive_betting.cpp
 * @brief Example demonstrating adaptive betting strategy
#include <chrono>
 */

#include "betting_by_time/framework.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <iomanip>

using namespace betting;

/**
 * @brief Generate binomial samples for testing
 */
Vector32f generate_binomial_samples(Float32 p, Int32 n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution d(p);
    
    Vector32f samples(n);
    for (Int32 i = 0; i < n; ++i) {
        samples(i) = d(gen) ? 1.0f : 0.0f;
    }
    
    return samples;
}

/**
 * @brief Run single experiment with given parameters
 */
void run_experiment(const std::string& name,
                   Float32 true_mean,
                   Float32 prior_mean,
                   Float32 delta,
                   Int32 num_samples,
                   BetStrategy strategy,
                   CapitalType capital_type) {
    std::cout << "\n=== " << name << " ===" << std::endl;
    std::cout << "True mean: " << true_mean << std::endl;
    std::cout << "Prior mean: " << prior_mean << std::endl;
    std::cout << "Delta: " << delta << std::endl;
    std::cout << "Samples: " << num_samples << std::endl;
    std::cout << "Strategy: " << (strategy == BetStrategy::Vanilla ? "Vanilla" : "Adaptive") << std::endl;
    std::cout << "Capital: " << (capital_type == CapitalType::Geo ? "Geo" : "Seq") << std::endl;
    
    // Create factory
    auto [make_gambler, bet_fn] = betting_factory(strategy, capital_type);
    
    // Initialize gambler
    constexpr Float32 alpha = 0.05f;
    constexpr Float32 trunc_scale = 0.5f;
    constexpr Int32 grid_num = 1000;
    
    auto gambler = make_gambler(alpha, trunc_scale, grid_num);
    
    // Generate samples
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    // Run betting strategy
    auto start_time = std::chrono::high_resolution_clock::now();
    // Call updated bet_fn signature: add breakpoints, gambler params, and mode
    auto [estimated_mean, samples_used] = bet_fn(samples, prior_mean, delta, grid_num,
                                                std::vector<Int32>{}, // breakpoints
                                                alpha,                 // gambler_alpha
                                                trunc_scale,           // gambler_trunc_scale
                                                0.25f,                 // gambler_prior_var
                                                1,                     // gambler_num
                                                grid_num,              // gambler_sample_num
                                                Mode::Estimate);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Report results
    std::cout << "\nResults:" << std::endl;
    std::cout << "  Estimated mean: " << std::fixed << std::setprecision(4) << estimated_mean << std::endl;
    std::cout << "  True mean:      " << true_mean << std::endl;
    std::cout << "  Error:          " << std::abs(estimated_mean - true_mean) << std::endl;
    std::cout << "  Samples used:   " << samples_used << " / " << num_samples << std::endl;
    std::cout << "  Time:           " << duration.count() << " microseconds" << std::endl;
    
    if (std::abs(estimated_mean - true_mean) < 0.05f) {
        std::cout << "  ✓ SUCCESS: Estimate within 0.05 of true mean!" << std::endl;
    } else {
        std::cout << "  ✗ WARNING: Estimate far from true mean" << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Adaptive Betting Strategy Comparison" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Test parameters
    constexpr Float32 true_mean = 0.65f;
    constexpr Float32 prior_mean = 0.5f;  // Biased prior
    constexpr Float32 delta = 0.1f;
    constexpr Int32 num_samples = 500;
    
    std::cout << "\nTest Configuration:" << std::endl;
    std::cout << "  True mean: " << true_mean << std::endl;
    std::cout << "  Prior: " << prior_mean << " (biased)" << std::endl;
    std::cout << "  Delta: " << delta << std::endl;
    std::cout << "  Samples: " << num_samples << std::endl;
    
    // Compare vanilla vs adaptive with GeoCheckingCapital
    run_experiment("Vanilla + Geo", 
                   true_mean, prior_mean, delta, num_samples,
                   BetStrategy::Vanilla, CapitalType::Geo);
    
    run_experiment("Adaptive + Geo", 
                   true_mean, prior_mean, delta, num_samples,
                   BetStrategy::Ada, CapitalType::Geo);
    
    // Compare vanilla vs adaptive with SequenceCheckingCapital
    run_experiment("Vanilla + Seq", 
                   true_mean, prior_mean, delta, num_samples,
                   BetStrategy::Vanilla, CapitalType::Seq);
    
    run_experiment("Adaptive + Seq", 
                   true_mean, prior_mean, delta, num_samples,
                   BetStrategy::Ada, CapitalType::Seq);
    
    // Test with different priors
    std::cout << "\n\n========================================" << std::endl;
    std::cout << " Testing with Different Priors" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::vector<Float32> priors = {0.3f, 0.5f, 0.7f};
    
    for (Float32 prior : priors) {
        std::cout << "\n--- Prior = " << prior << " ---" << std::endl;
        
        run_experiment("Adaptive + Geo", 
                       true_mean, prior, delta, num_samples,
                       BetStrategy::Ada, CapitalType::Geo);
    }
    
    // Test sample efficiency
    std::cout << "\n\n========================================" << std::endl;
    std::cout << " Sample Efficiency Test" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::vector<Int32> sample_sizes = {100, 200, 500, 1000};
    
    for (Int32 n : sample_sizes) {
        std::cout << "\n--- " << n << " samples ---" << std::endl;
        
        run_experiment("Vanilla + Geo", 
                       true_mean, prior_mean, delta, n,
                       BetStrategy::Vanilla, CapitalType::Geo);
        
        run_experiment("Adaptive + Geo", 
                       true_mean, prior_mean, delta, n,
                       BetStrategy::Ada, CapitalType::Geo);
    }
    
    // Demonstrate incremental behavior
    std::cout << "\n\n========================================" << std::endl;
    std::cout << " Incremental Behavior Demo" << std::endl;
    std::cout << "========================================" << std::endl;
    
    auto [make_gambler, bet_fn] = adaptive_geo_factory();
    
    constexpr Float32 alpha = 0.05f;
    constexpr Float32 trunc_scale = 0.5f;
    constexpr Int32 grid_num = 1000;
    
    auto gambler = make_gambler(alpha, trunc_scale, grid_num);
    
    Vector32f samples = generate_binomial_samples(true_mean, 200);
    
    std::cout << "\nRunning adaptive betting incrementally..." << std::endl;
    
    // Add samples in batches
    Int32 batch_size = 50;
    for (Int32 batch = 0; batch < 4; ++batch) {
        std::cout << "\nBatch " << (batch + 1) << ": Adding " << batch_size << " samples" << std::endl;
        
        // Extract batch
        Int32 start_idx = batch * batch_size;
        Int32 end_idx = std::min(start_idx + batch_size, static_cast<Int32>(samples.size()));
        
        Vector32f batch_samples(end_idx - start_idx);
        for (Int32 i = start_idx; i < end_idx; ++i) {
            batch_samples(i - start_idx) = samples(i);
        }
        
        // Run on accumulated samples
        auto [est, used] = bet_fn(samples.head(end_idx), prior_mean, delta, grid_num,
                     std::vector<Int32>{}, // breakpoints
                     alpha,                 // gambler_alpha
                     trunc_scale,           // gambler_trunc_scale
                     0.25f,                 // gambler_prior_var
                     1,                     // gambler_num
                     grid_num,              // gambler_sample_num
                     Mode::Estimate);
        
        std::cout << "  Total samples: " << end_idx << std::endl;
        std::cout << "  Current estimate: " << std::fixed << std::setprecision(4) << est << std::endl;
        std::cout << "  Error: " << std::abs(est - true_mean) << std::endl;
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << " All Tests Complete!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
