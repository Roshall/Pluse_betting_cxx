/**
 * @file test_capital_processes.cpp
 * @brief Unit tests for capital process classes
 */

#include <catch2/catch_all.hpp>
#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"

#include <random>

using namespace betting;

TEST_CASE("GeoCheckingCapital initialization", "[capital][geo]") {
    Float32 alpha = 0.05f;
    Float32 trunc_scale = 0.5f;
    Int32 grid_num = 100;
    
    GeoCheckingCapital gambler(alpha, trunc_scale, grid_num);
    
    REQUIRE(gambler.s_ptr() == 0);
    REQUIRE(gambler.trunc_scale() == Catch::Approx(trunc_scale));
    REQUIRE(gambler.grid_num() == grid_num);
    REQUIRE(gambler.threshold() > 0);
}

TEST_CASE("GeoCheckingCapital add_sample", "[capital][geo]") {
    GeoCheckingCapital gambler(0.05f, 0.5f, 100);
    
    // Add samples
    gambler.add_sample(0.5f);
    REQUIRE(gambler.s_ptr() == 1);
    REQUIRE(gambler.sample(0)(0) == Catch::Approx(0.5f));
    
    gambler.add_sample(0.7f);
    REQUIRE(gambler.s_ptr() == 2);
    REQUIRE(gambler.sample(1)(0) == Catch::Approx(0.7f));
}

TEST_CASE("GeoCheckingCapital advance", "[capital][geo]") {
    GeoCheckingCapital gambler(0.05f, 0.5f, 10);
    
    // Add some samples first
    Vector32f samples(5);
    samples << 0.3f, 0.5f, 0.7f, 0.4f, 0.6f;
    
    for (Int32 i = 0; i < 5; ++i) {
        gambler.add_sample(samples(i));
    }
    
    // Create hypothesis list
    Vector32f m_lst = linspace(0.0f, 1.0f, 11);
    
    // Advance should not crash
    gambler.advance(0.8f, m_lst);
    REQUIRE(gambler.s_ptr() == 6);
}

TEST_CASE("GeoCheckingCapital reset", "[capital][geo]") {
    GeoCheckingCapital gambler(0.05f, 0.5f, 100);
    
    // Add samples
    gambler.add_sample(0.5f);
    gambler.add_sample(0.7f);
    REQUIRE(gambler.s_ptr() == 2);
    
    // Reset
    gambler.reset();
    REQUIRE(gambler.s_ptr() == 0);
}

TEST_CASE("SequenceCheckingCapital initialization", "[capital][seq]") {
    Float32 alpha = 0.05f;
    Float32 trunc_scale = 0.5f;
    Int32 grid_num = 100;
    
    SequenceCheckingCapital gambler(alpha, trunc_scale, grid_num);
    
    REQUIRE(gambler.s_ptr() == 0);
    REQUIRE(gambler.trunc_scale() == Catch::Approx(trunc_scale));
    REQUIRE(gambler.grid_num() == grid_num);
    REQUIRE(gambler.threshold() > 0);
}

TEST_CASE("SequenceCheckingCapital add_sample", "[capital][seq]") {
    SequenceCheckingCapital gambler(0.05f, 0.5f, 100);
    
    // Add samples
    gambler.add_sample(0.3f);
    REQUIRE(gambler.s_ptr() == 1);
    REQUIRE(gambler.sample(0) == Catch::Approx(0.3f));
    
    gambler.add_sample(0.9f);
    REQUIRE(gambler.s_ptr() == 2);
    REQUIRE(gambler.sample(1) == Catch::Approx(0.9f));
}

TEST_CASE("SequenceCheckingCapital advance", "[capital][seq]") {
    SequenceCheckingCapital gambler(0.05f, 0.5f, 10);
    
    // Add some samples first
    Vector32f samples(3);
    samples << 0.2f, 0.5f, 0.8f;
    
    for (Int32 i = 0; i < 3; ++i) {
        gambler.add_sample(samples(i));
    }
    
    // Create hypothesis list
    Vector32f m_lst = linspace(0.0f, 1.0f, 11);
    
    // Advance should not crash
    gambler.advance(0.6f, m_lst);
    REQUIRE(gambler.s_ptr() == 4);
}

TEST_CASE("SequenceCheckingCapital reset", "[capital][seq]") {
    SequenceCheckingCapital gambler(0.05f, 0.5f, 100);
    
    // Add samples
    gambler.add_sample(0.4f);
    gambler.add_sample(0.6f);
    REQUIRE(gambler.s_ptr() == 2);
    
    // Reset
    gambler.reset();
    REQUIRE(gambler.s_ptr() == 0);
}

TEST_CASE("Capital processes handle binary samples", "[capital]") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution d(0.6f);
    
    // Test GeoCheckingCapital
    GeoCheckingCapital geo_gambler(0.05f, 0.5f, 100);
    for (Int32 i = 0; i < 50; ++i) {
        Float32 sample = d(gen) ? 1.0f : 0.0f;
        geo_gambler.add_sample(sample);
    }
    REQUIRE(geo_gambler.s_ptr() == 50);
    
    // Test SequenceCheckingCapital
    SequenceCheckingCapital seq_gambler(0.05f, 0.5f, 100);
    gen.seed(rd());  // Reset seed
    for (Int32 i = 0; i < 50; ++i) {
        Float32 sample = d(gen) ? 1.0f : 0.0f;
        seq_gambler.add_sample(sample);
    }
    REQUIRE(seq_gambler.s_ptr() == 50);
}

TEST_CASE("Capital twins are initialized correctly", "[capital]") {
    Int32 grid_num = 10;
    
    // GeoCheckingCapital
    GeoCheckingCapital geo_gambler(0.05f, 0.5f, grid_num);
    auto& geo_twins = geo_gambler.cum_cap_twins();
    REQUIRE(geo_twins.rows() == grid_num + 1);
    REQUIRE(geo_twins.cols() == 2);
    // Initial capitals should be 1.0
    for (Int32 i = 0; i <= grid_num; ++i) {
        REQUIRE(geo_twins(i, 0) == Catch::Approx(1.0));
        REQUIRE(geo_twins(i, 1) == Catch::Approx(1.0));
    }
    
    // SequenceCheckingCapital
    SequenceCheckingCapital seq_gambler(0.05f, 0.5f, grid_num);
    auto& seq_twins = seq_gambler.cum_cap_twins();
    REQUIRE(seq_twins.rows() == grid_num + 1);
    REQUIRE(seq_twins.cols() == 2);
    // Initial capitals should be 1.0
    for (Int32 i = 0; i <= grid_num; ++i) {
        REQUIRE(seq_twins(i, 0) == Catch::Approx(1.0));
        REQUIRE(seq_twins(i, 1) == Catch::Approx(1.0));
    }
}
