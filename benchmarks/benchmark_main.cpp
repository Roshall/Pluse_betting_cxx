/**
 * @file benchmark_main.cpp
 * @brief Main entry point for benchmark suite
 */

#include <benchmark/benchmark.h>
#include "betting_by_time/core/types.hpp"
#include "betting_by_time/strategies/vanilla_betting.hpp"
#include "betting_by_time/strategies/adaptive_betting.hpp"
#include "betting_by_time/capital/geo_checking.hpp"
#include "betting_by_time/capital/sequence_checking.hpp"
#include <random>
#include <vector>

using namespace betting;

/**
 * @brief Generate binomial samples with fixed seed for reproducibility
 */
Vector32f generate_samples(Float32 p, Int32 n, unsigned int seed = 42) {
    std::mt19937 gen(seed);
    std::bernoulli_distribution d(p);
    
    Vector32f samples(n);
    for (Int32 i = 0; i < n; ++i) {
        samples(i) = d(gen) ? 1.0f : 0.0f;
    }
    
    return samples;
}

// ============================================================================
// Vanilla Betting Benchmarks
// ============================================================================

static void BM_VanillaGeo_Small(benchmark::State& state) {
    const Int32 num_samples = 100;
    const Int32 grid_num = 100;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = vanilla_betting(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VanillaGeo_Small);

static void BM_VanillaGeo_Medium(benchmark::State& state) {
    const Int32 num_samples = 500;
    const Int32 grid_num = 500;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = vanilla_betting(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VanillaGeo_Medium);

static void BM_VanillaGeo_Large(benchmark::State& state) {
    const Int32 num_samples = 1000;
    const Int32 grid_num = 1000;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = vanilla_betting(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VanillaGeo_Large);

static void BM_VanillaSeq_Small(benchmark::State& state) {
    const Int32 num_samples = 100;
    const Int32 grid_num = 100;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = vanilla_betting_sequence(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VanillaSeq_Small);

static void BM_VanillaSeq_Medium(benchmark::State& state) {
    const Int32 num_samples = 500;
    const Int32 grid_num = 500;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = vanilla_betting_sequence(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VanillaSeq_Medium);

static void BM_VanillaSeq_Large(benchmark::State& state) {
    const Int32 num_samples = 1000;
    const Int32 grid_num = 1000;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = vanilla_betting_sequence(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VanillaSeq_Large);

// ============================================================================
// Adaptive Betting Benchmarks
// ============================================================================

static void BM_AdaptiveGeo_Small(benchmark::State& state) {
    const Int32 num_samples = 100;
    const Int32 grid_num = 100;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = adaptive_betting(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AdaptiveGeo_Small);

static void BM_AdaptiveGeo_Medium(benchmark::State& state) {
    const Int32 num_samples = 500;
    const Int32 grid_num = 500;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = adaptive_betting(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AdaptiveGeo_Medium);

static void BM_AdaptiveGeo_Large(benchmark::State& state) {
    const Int32 num_samples = 1000;
    const Int32 grid_num = 1000;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = adaptive_betting(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AdaptiveGeo_Large);

static void BM_AdaptiveSeq_Small(benchmark::State& state) {
    const Int32 num_samples = 100;
    const Int32 grid_num = 100;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = adaptive_betting_sequence(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AdaptiveSeq_Small);

static void BM_AdaptiveSeq_Medium(benchmark::State& state) {
    const Int32 num_samples = 500;
    const Int32 grid_num = 500;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = adaptive_betting_sequence(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AdaptiveSeq_Medium);

static void BM_AdaptiveSeq_Large(benchmark::State& state) {
    const Int32 num_samples = 1000;
    const Int32 grid_num = 1000;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = adaptive_betting_sequence(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AdaptiveSeq_Large);

// ============================================================================
// Grid Size Scaling Benchmarks
// ============================================================================

static void BM_GridScaling(benchmark::State& state) {
    const Int32 num_samples = 500;
    const Int32 grid_num = static_cast<Int32>(state.range(0));
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = vanilla_betting(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_GridScaling)->RangeMultiplier(2)->Range(100, 2000);

// ============================================================================
// Sample Size Scaling Benchmarks
// ============================================================================

static void BM_SampleScaling(benchmark::State& state) {
    const Int32 num_samples = static_cast<Int32>(state.range(0));
    const Int32 grid_num = 500;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    for (auto _ : state) {
        auto result = vanilla_betting(samples, 0.5f, 0.1f, grid_num);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SampleScaling)->RangeMultiplier(2)->Range(100, 2000);

// ============================================================================
// Capital Process Operations Benchmarks
// ============================================================================

static void BM_GeoAddSample(benchmark::State& state) {
    GeoCheckingCapital gambler(0.05f, 0.5f, 1000);
    Vector32f samples = generate_samples(0.65f, 1000);
    
    for (auto _ : state) {
        for (Int32 i = 0; i < 1000; ++i) {
            gambler.add_sample(samples(i));
        }
        gambler.reset();
    }
}
BENCHMARK(BM_GeoAddSample);

static void BM_SeqAddSample(benchmark::State& state) {
    SequenceCheckingCapital gambler(0.05f, 0.5f, 1000);
    Vector32f samples = generate_samples(0.65f, 1000);
    
    for (auto _ : state) {
        for (Int32 i = 0; i < 1000; ++i) {
            gambler.add_sample(samples(i));
        }
        gambler.reset();
    }
}
BENCHMARK(BM_SeqAddSample);

static void BM_GeoAdvance(benchmark::State& state) {
    GeoCheckingCapital gambler(0.05f, 0.5f, 100);
    Vector32f samples = generate_samples(0.65f, 100);
    Vector32f m_lst = linspace(0.0f, 1.0f, 101);
    
    for (Int32 i = 0; i < 100; ++i) {
        gambler.add_sample(samples(i));
    }
    
    for (auto _ : state) {
        gambler.advance(0.7f, m_lst);
        state.PauseTiming();
        gambler.reset();
        for (Int32 i = 0; i < 100; ++i) {
            gambler.add_sample(samples(i));
        }
        state.ResumeTiming();
    }
}
BENCHMARK(BM_GeoAdvance);

// ============================================================================
// Comparison: Vanilla vs Adaptive
// ============================================================================

static void BM_Comparison_Vanilla_vs_Adaptive_Geo(benchmark::State& state) {
    const Int32 num_samples = 500;
    const Int32 grid_num = 500;
    Vector32f samples = generate_samples(0.65f, num_samples);
    
    // Benchmark vanilla
    if (state.thread_index() == 0) {
        for (auto _ : state) {
            auto result = vanilla_betting(samples, 0.5f, 0.1f, grid_num);
            benchmark::DoNotOptimize(result);
        }
    }
    // Benchmark adaptive
    else {
        for (auto _ : state) {
            auto result = adaptive_betting(samples, 0.5f, 0.1f, grid_num);
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK(BM_Comparison_Vanilla_vs_Adaptive_Geo)->Threads(2);

// ============================================================================
// Register benchmarks
// ============================================================================

BENCHMARK_MAIN();
