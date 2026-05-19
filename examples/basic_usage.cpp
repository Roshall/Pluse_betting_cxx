/**
 * @file basic_usage.cpp
 * @brief Example demonstrating the framework API usage
#include <chrono>
 */

#include "betting_by_time/framework.hpp"
#include <iostream>
#include <chrono>
#include <random>

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

int main() {
    std::cout << "=== Betting-by-Time C++20 Framework Example ===" << std::endl;
    
    // Use the factory to get capital process creator and betting function
    auto [make_gambler, bet_fn] = vanilla_geo_factory();
    
    // Create a gambler with custom parameters
    constexpr Float32 alpha = 0.5f;
    constexpr Float32 trunc_scale = 0.1f;
    constexpr Int32 grid_num = 1000;
    
    auto gambler = make_gambler(alpha, trunc_scale, grid_num);
    
    std::cout << "\nInitialized GeoCheckingCapital via factory:" << std::endl;
    std::cout << "  Alpha: " << alpha << std::endl;
    std::cout << "  Truncation scale: " << trunc_scale << std::endl;
    std::cout << "  Grid size: " << grid_num << std::endl;
    std::cout << "  Delta: " << 0.05f << std::endl;
    std::cout << "  Threshold: " << std::any_cast<betting::GeoCheckingCapital&>(gambler).threshold() << std::endl;
    
    // Generate test samples
    constexpr Int32 num_samples = 500;
    constexpr Float32 true_mean = 0.72353f;
    
    std::cout << "\nGenerating " << num_samples << " samples with true mean " 
              << true_mean << "..." << std::endl;
    
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    // Run vanilla betting strategy
    constexpr Float32 prior_mean = 0.5f;
    constexpr Float32 delta = 0.05f;
    
    std::cout << "\nRunning vanilla betting strategy..." << std::endl;
    auto [estimated_mean, samples_used] = bet_fn(samples, prior_mean, delta, grid_num, gambler);
    
    std::cout << "\nResults:" << std::endl;
    std::cout << "  True mean: " << true_mean << std::endl;
    std::cout << "  Estimated mean: " << estimated_mean << std::endl;
    std::cout << "  Error: " << std::abs(estimated_mean - true_mean) << std::endl;
    std::cout << "  Samples used: " << samples_used << " / " << num_samples << std::endl;
    std::cout << "  Efficiency: " << (100.0f * samples_used / num_samples) << "%" << std::endl;
    
    // Verify accuracy
    Float32 error = std::abs(estimated_mean - true_mean);
    if (error <= delta) {
        std::cout << "\n✓ SUCCESS: Estimate within confidence bound!" << std::endl;
    } else {
        std::cout << "\n✗ WARNING: Estimate outside confidence bound" << std::endl;
    }
    
    std::cout << "\n=== Example Complete ===" << std::endl;
    
    return 0;
}
