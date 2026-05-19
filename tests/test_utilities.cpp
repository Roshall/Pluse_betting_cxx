/**
 * @file test_utilities.cpp
 * @brief Unit tests for utility functions
 */

#include <catch2/catch_all.hpp>
#include "betting_by_time/core/utilities.hpp"
#include <cmath>
#include <limits>

using namespace betting;

TEST_CASE("cal_c computes correct values", "[utilities]") {
    // Test with delta = 0.05
    Float32 c1 = cal_c(0.05f);
    REQUIRE(c1 == Catch::Approx(2.0f * std::log(2.0f / 0.05f)).margin(1e-5));
    
    // Test with delta = 0.01
    Float32 c2 = cal_c(0.01f);
    REQUIRE(c2 == Catch::Approx(2.0f * std::log(2.0f / 0.01f)).margin(1e-5));
    
    // Test with delta = 0.1
    Float32 c3 = cal_c(0.1f);
    REQUIRE(c3 == Catch::Approx(2.0f * std::log(2.0f / 0.1f)).margin(1e-5));
}

TEST_CASE("linspace generates correct sequences", "[utilities]") {
    // Test basic linspace
    Vector32f v = linspace(0.0f, 1.0f, 5);
    REQUIRE(v.size() == 5);
    REQUIRE(v(0) == Catch::Approx(0.0f));
    REQUIRE(v(4) == Catch::Approx(1.0f));
    REQUIRE(v(1) == Catch::Approx(0.25f).margin(1e-5));
    REQUIRE(v(2) == Catch::Approx(0.5f));
    REQUIRE(v(3) == Catch::Approx(0.75f).margin(1e-5));
    
    // Test with different range
    Vector32f v2 = linspace(-1.0f, 1.0f, 3);
    REQUIRE(v2.size() == 3);
    REQUIRE(v2(0) == Catch::Approx(-1.0f));
    REQUIRE(v2(1) == Catch::Approx(0.0f));
    REQUIRE(v2(2) == Catch::Approx(1.0f));
}

TEST_CASE("argmin finds correct minimum index", "[utilities]") {
    // Test with simple vector
    Vector64d v1(5);
    v1 << 5.0, 3.0, 1.0, 4.0, 2.0;
    REQUIRE(argmin(v1) == 2);  // Index of 1.0
    
    // Test with negative values
    Vector64d v2(4);
    v2 << -1.0, -5.0, -2.0, -3.0;
    REQUIRE(argmin(v2) == 1);  // Index of -5.0
    
    // Test with all same values
    Vector64d v3(3);
    v3 << 1.0, 1.0, 1.0;
    REQUIRE(argmin(v3) == 0);  // First occurrence
}

TEST_CASE("flatnonzero works correctly", "[utilities]") {
    Vector32i v1(5);
    v1 << 0, 1, 0, 1, 1;
    
    Vector32i nz = flatnonzero(v1);
    REQUIRE(nz.size() == 3);
    REQUIRE(nz(0) == 1);
    REQUIRE(nz(1) == 3);
    REQUIRE(nz(2) == 4);
    
    // All zeros
    Vector32i v2(3);
    v2 << 0, 0, 0;
    Vector32i nz2 = flatnonzero(v2);
    REQUIRE(nz2.size() == 0);
}

TEST_CASE("gen_times generates geometric sequence", "[utilities]") {
    Vector32i times = gen_times(1.0f, 2.0f, 10.0f);
    
    REQUIRE(times.size() > 0);
    REQUIRE(times(0) == 0);  // Always starts with 0
    
    // Check that times are sorted
    for (Int32 i = 1; i < times.size(); ++i) {
        REQUIRE(times(i) >= times(i-1));
    }
}

TEST_CASE("intersect modifies array correctly", "[utilities]") {
    Array2f a;
    a << 0.2f, 0.8f;
    
    // Intersect with [0.3, 0.7]
    intersect(a, 0.3f, 0.7f);
    REQUIRE(a(0) == Catch::Approx(0.3f));
    REQUIRE(a(1) == Catch::Approx(0.7f));
    
    // Intersect with wider range
    Array2f b;
    b << 0.4f, 0.6f;
    intersect(b, 0.0f, 1.0f);
    REQUIRE(b(0) == Catch::Approx(0.4f));
    REQUIRE(b(1) == Catch::Approx(0.6f));
}
