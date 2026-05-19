/**
 * @file test_types.cpp
 * @brief Unit tests for core type definitions
 */

#include <catch2/catch_all.hpp>
#include "betting_by_time/core/types.hpp"

using namespace betting;

TEST_CASE("Type aliases are correctly defined", "[types]") {
    // Test Float32
    STATIC_REQUIRE(std::is_same_v<Float32, float>);
    
    // Test Float64
    STATIC_REQUIRE(std::is_same_v<Float64, double>);
    
    // Test Int32
    STATIC_REQUIRE(std::is_same_v<Int32, int32_t>);
}

TEST_CASE("Eigen vector types work correctly", "[types][eigen]") {
    // Test Vector32f
    Vector32f v32(5);
    v32 << 1.0f, 2.0f, 3.0f, 4.0f, 5.0f;
    REQUIRE(v32.size() == 5);
    REQUIRE(v32(0) == Catch::Approx(1.0f));
    REQUIRE(v32(4) == Catch::Approx(5.0f));
    
    // Test Vector64d
    Vector64d v64(3);
    v64 << 1.5, 2.5, 3.5;
    REQUIRE(v64.size() == 3);
    REQUIRE(v64(0) == Catch::Approx(1.5));
    
    // Test Vector32i
    Vector32i vi(4);
    vi << 1, 2, 3, 4;
    REQUIRE(vi.size() == 4);
    REQUIRE(vi(2) == 3);
}

TEST_CASE("Eigen matrix types work correctly", "[types][eigen]") {
    // Test Matrix64d
    Matrix64d m(2, 3);
    m << 1.0, 2.0, 3.0,
         4.0, 5.0, 6.0;
    REQUIRE(m.rows() == 2);
    REQUIRE(m.cols() == 3);
    REQUIRE(m(0, 0) == Catch::Approx(1.0));
    REQUIRE(m(1, 2) == Catch::Approx(6.0));
    
    // Test Matrix32i
    Matrix32i mi(2, 2);
    mi << 1, 2,
          3, 4;
    REQUIRE(mi(0, 1) == 2);
    REQUIRE(mi(1, 0) == 3);
}

TEST_CASE("Array types work correctly", "[types][eigen]") {
    // Test Array2f
    Array2f a2f;
    a2f << 1.5f, 2.5f;
    REQUIRE(a2f.size() == 2);
    REQUIRE(a2f(0) == Catch::Approx(1.5f));
    REQUIRE(a2f(1) == Catch::Approx(2.5f));
    
    // Test Array2i
    Array2i a2i;
    a2i << 10, 20;
    REQUIRE(a2i(0) == 10);
    REQUIRE(a2i(1) == 20);
}

TEST_CASE("Type conversions work correctly", "[types]") {
    // Float32 to Float64
    Float32 f32 = 3.14f;
    Float64 f64 = static_cast<Float64>(f32);
    REQUIRE(f64 == Catch::Approx(3.14).epsilon(1e-5));
    
    // Int32 to Float32
    Int32 i32 = 42;
    Float32 f = static_cast<Float32>(i32);
    REQUIRE(f == Catch::Approx(42.0f));
}

TEST_CASE("Eigen operations work with custom types", "[types][eigen]") {
    Vector32f v1(3), v2(3);
    v1 << 1.0f, 2.0f, 3.0f;
    v2 << 4.0f, 5.0f, 6.0f;
    
    // Addition
    Vector32f sum = v1 + v2;
    REQUIRE(sum(0) == Catch::Approx(5.0f));
    REQUIRE(sum(2) == Catch::Approx(9.0f));
    
    // Dot product
    Float32 dot = v1.dot(v2);
    REQUIRE(dot == Catch::Approx(32.0f));  // 1*4 + 2*5 + 3*6
    
    // Norm
    Float32 norm = v1.norm();
    REQUIRE(norm == Catch::Approx(std::sqrt(14.0f)).margin(1e-5));
}
