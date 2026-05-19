/**
 * @file test_bet_on_estimate.cpp
 * @brief Example demonstrating bet_on and estimate functions
 */

#include "betting_by_time/bet_on_estimate.hpp"
#include <iostream>
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
    std::cout << "=== Testing bet_on and estimate Functions ===" << std::endl;
    
    // Create a GeoCheckingCapital process
    constexpr Float32 delta = 0.05f;
    constexpr Float32 trunc_scale = 0.5f;
    constexpr Int32 grid_num = 1000;
    
    GeoCheckingCapital gambler(delta, trunc_scale, grid_num);
    
    std::cout << "\nInitialized GeoCheckingCapital:" << std::endl;
    std::cout << "  Grid size: " << grid_num << std::endl;
    std::cout << "  Threshold: " << gambler.threshold() << std::endl;
    
    // Generate test samples
    constexpr Int32 num_samples = 200;
    constexpr Float32 true_mean = 0.65f;
    
    std::cout << "\nGenerating " << num_samples << " samples with true mean " 
              << true_mean << "..." << std::endl;
    
    Vector32f samples = generate_binomial_samples(true_mean, num_samples);
    
    // Add samples to gambler
    for (Int32 i = 0; i < num_samples; ++i) {
        gambler.add_sample(samples(i));
    }
    
    std::cout << "Added " << gambler.s_ptr() << " samples to gambler" << std::endl;
    
    // Test bet_on function
    std::cout << "\n--- Testing bet_on Function ---" << std::endl;
    
    // Test hypothesis m = 0.5 (should be rejected since true mean is 0.65)
    Float32 test_m1 = 0.5f;
    Int32 test_mi1 = static_cast<Int32>(test_m1 * grid_num);
    
    std::cout << "\nTesting hypothesis m = " << test_m1 << " (index " << test_mi1 << ")" << std::endl;
    std::cout << "Initial capitals: pos=" << gambler.cum_cap_twins()(test_mi1, 0)
              << ", neg=" << gambler.cum_cap_twins()(test_mi1, 1) << std::endl;
    
    bool rejected1 = bet_on(gambler, test_m1, test_mi1, 2);
    
    std::cout << "After bet_on:" << std::endl;
    std::cout << "  Capitals: pos=" << gambler.cum_cap_twins()(test_mi1, 0)
              << ", neg=" << gambler.cum_cap_twins()(test_mi1, 1) << std::endl;
    std::cout << "  Rejected (capital > threshold)? " << (rejected1 ? "YES" : "NO") << std::endl;
    
    // Test hypothesis m = 0.65 (close to true mean, should NOT be rejected)
    Float32 test_m2 = 0.65f;
    Int32 test_mi2 = static_cast<Int32>(test_m2 * grid_num);
    
    std::cout << "\nTesting hypothesis m = " << test_m2 << " (index " << test_mi2 << ")" << std::endl;
    std::cout << "Initial capitals: pos=" << gambler.cum_cap_twins()(test_mi2, 0)
              << ", neg=" << gambler.cum_cap_twins()(test_mi2, 1) << std::endl;
    
    bool rejected2 = bet_on(gambler, test_m2, test_mi2, 2);
    
    std::cout << "After bet_on:" << std::endl;
    std::cout << "  Capitals: pos=" << gambler.cum_cap_twins()(test_mi2, 0)
              << ", neg=" << gambler.cum_cap_twins()(test_mi2, 1) << std::endl;
    std::cout << "  Rejected (capital > threshold)? " << (rejected2 ? "YES" : "NO") << std::endl;
    
    // Test estimate function
    std::cout << "\n--- Testing estimate Function ---" << std::endl;
    
    Float32 lower_bound = 0.5f;
    Float32 upper_bound = 0.8f;
    
    std::cout << "\nEstimating mean in range [" << lower_bound << ", " << upper_bound << "]" << std::endl;
    
    Float32 estimated = estimate(gambler, lower_bound, upper_bound);
    
    std::cout << "Estimated mean: " << estimated << std::endl;
    std::cout << "True mean: " << true_mean << std::endl;
    std::cout << "Error: " << std::abs(estimated - true_mean) << std::endl;
    
    if (std::abs(estimated - true_mean) < 0.05f) {
        std::cout << "✓ SUCCESS: Estimate within 0.05 of true mean!" << std::endl;
    } else {
        std::cout << "✗ WARNING: Estimate far from true mean" << std::endl;
    }
    
    // Test incremental updates
    std::cout << "\n--- Testing Incremental Updates ---" << std::endl;
    
    // Reset gambler
    gambler.reset();
    
    // Add samples in batches and test incrementally
    Int32 batch_size = 50;
    for (Int32 batch = 0; batch < 4; ++batch) {
        std::cout << "\nBatch " << (batch + 1) << ": Adding " << batch_size << " samples" << std::endl;
        
        for (Int32 i = 0; i < batch_size; ++i) {
            Int32 idx = batch * batch_size + i;
            if (idx < num_samples) {
                gambler.add_sample(samples(idx));
            }
        }
        
        // Test bet_on incrementally
        Float32 test_m = 0.6f;
        Int32 test_mi = static_cast<Int32>(test_m * grid_num);
        
        bool rejected = bet_on(gambler, test_m, test_mi, 2);
        
        std::cout << "  Samples so far: " << gambler.s_ptr() << std::endl;
        std::cout << "  Hypothesis m=" << test_m << " rejected? " << (rejected ? "YES" : "NO") << std::endl;
        std::cout << "  Capitals: pos=" << gambler.cum_cap_twins()(test_mi, 0)
                  << ", neg=" << gambler.cum_cap_twins()(test_mi, 1) << std::endl;
    }
    
    // Final estimate
    std::cout << "\nFinal estimate after all batches:" << std::endl;
    Float32 final_estimate = estimate(gambler, 0.5f, 0.8f);
    std::cout << "  Estimated: " << final_estimate << std::endl;
    std::cout << "  True: " << true_mean << std::endl;
    std::cout << "  Error: " << std::abs(final_estimate - true_mean) << std::endl;
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    
    return 0;
}
