# Benchmark Suite Documentation

## Overview

The benchmark suite measures performance of the C++20 betting-by-time implementation using Google Benchmark. It provides detailed timing information for all major components and strategies.

## Building Benchmarks

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release  # Release mode for accurate benchmarks
make betting_benchmarks
```

## Running Benchmarks

### Run All Benchmarks
```bash
./benchmarks/betting_benchmarks
```

### Run Specific Benchmark
```bash
# Run only vanilla geo benchmarks
./benchmarks/betting_benchmarks --benchmark_filter=BM_VanillaGeo

# Run only adaptive benchmarks
./benchmarks/betting_benchmarks --benchmark_filter=BM_Adaptive

# Run scaling benchmarks
./benchmarks/betting_benchmarks --benchmark_filter=BM_Scaling
```

### Custom Iterations
```bash
# Run each benchmark for at least 1 second
./benchmarks/betting_benchmarks --benchmark_min_time=1

# Run exactly 1000 iterations
./benchmarks/betting_benchmarks --benchmark_repetitions=3
```

### Output Formats
```bash
# Console output (default)
./benchmarks/betting_benchmarks

# CSV output
./benchmarks/betting_benchmarks --benchmark_format=csv > results.csv

# JSON output
./benchmarks/betting_benchmarks --benchmark_format=json > results.json
```

## Benchmark Categories

### 1. Vanilla Betting Benchmarks

Measures performance of vanilla betting strategy with different configurations:

- **BM_VanillaGeo_Small**: 100 samples, grid=100, GeoCheckingCapital
- **BM_VanillaGeo_Medium**: 500 samples, grid=500, GeoCheckingCapital
- **BM_VanillaGeo_Large**: 1000 samples, grid=1000, GeoCheckingCapital
- **BM_VanillaSeq_Small**: 100 samples, grid=100, SequenceCheckingCapital
- **BM_VanillaSeq_Medium**: 500 samples, grid=500, SequenceCheckingCapital
- **BM_VanillaSeq_Large**: 1000 samples, grid=1000, SequenceCheckingCapital

**Purpose**: Measure baseline performance and compare capital process types.

### 2. Adaptive Betting Benchmarks

Measures performance of adaptive betting strategy:

- **BM_AdaptiveGeo_Small**: 100 samples, grid=100, GeoCheckingCapital
- **BM_AdaptiveGeo_Medium**: 500 samples, grid=500, GeoCheckingCapital
- **BM_AdaptiveGeo_Large**: 1000 samples, grid=1000, GeoCheckingCapital
- **BM_AdaptiveSeq_Small**: 100 samples, grid=100, SequenceCheckingCapital
- **BM_AdaptiveSeq_Medium**: 500 samples, grid=500, SequenceCheckingCapital
- **BM_AdaptiveSeq_Large**: 1000 samples, grid=1000, SequenceCheckingCapital

**Purpose**: Compare adaptive vs vanilla performance and measure binary search overhead.

### 3. Scaling Benchmarks

Measures how performance scales with problem size:

- **BM_GridScaling**: Varies grid size from 100 to 2000 (fixed 500 samples)
  - Tests O(n·k) complexity where k = grid size
  
- **BM_SampleScaling**: Varies sample count from 100 to 2000 (fixed grid=500)
  - Tests O(n·k) complexity where n = sample count

**Purpose**: Verify theoretical complexity and identify bottlenecks.

### 4. Capital Process Operation Benchmarks

Measures low-level operations:

- **BM_GeoAddSample**: Time to add 1000 samples to GeoCheckingCapital
- **BM_SeqAddSample**: Time to add 1000 samples to SequenceCheckingCapital
- **BM_GeoAdvance**: Time to advance one step with 100 hypotheses

**Purpose**: Identify which operations dominate runtime.

### 5. Comparison Benchmarks

Direct comparison between strategies:

- **BM_Comparison_Vanilla_vs_Adaptive_Geo**: Runs both strategies in parallel threads
  - Thread 0: Vanilla betting
  - Thread 1: Adaptive betting

**Purpose**: Direct head-to-head comparison under identical conditions.

## Expected Results

### Typical Performance Characteristics

| Benchmark | Expected Time | Notes |
|-----------|--------------|-------|
| VanillaGeo_Small | ~10-50 μs | Fast, small problem |
| VanillaGeo_Medium | ~100-500 μs | Moderate size |
| VanillaGeo_Large | ~500-2000 μs | Large problem |
| AdaptiveGeo_Small | ~20-80 μs | Slightly slower due to binary search |
| AdaptiveGeo_Medium | ~150-600 μs | Binary search helps with larger grids |
| AdaptiveGeo_Large | ~400-1500 μs | Should be faster than vanilla for large grids |
| GridScaling(100) | ~50 μs | Small grid |
| GridScaling(2000) | ~2000 μs | Large grid, linear scaling expected |
| SampleScaling(100) | ~30 μs | Few samples |
| SampleScaling(2000) | ~1500 μs | Many samples, linear scaling |

### Key Metrics to Watch

1. **Vanilla vs Adaptive Ratio**
   - For small grids: Vanilla should be faster (less overhead)
   - For large grids: Adaptive should be faster (binary search advantage)
   
2. **Geo vs Seq Ratio**
   - GeoCheckingCapital typically 2-5x faster than SequenceCheckingCapital
   - Due to pre-computed capitals vs on-the-fly computation
   
3. **Scaling Behavior**
   - Grid scaling: Should be approximately linear O(k)
   - Sample scaling: Should be approximately linear O(n)
   - Deviations indicate optimization opportunities

## Interpreting Results

### Understanding Output

```
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
BM_VanillaGeo_Small           25.3 μs         25.1 μs        27834
BM_VanillaGeo_Medium         245.7 μs        244.9 μs         2856
BM_AdaptiveGeo_Medium        198.4 μs        197.8 μs         3534
```

- **Time**: Wall-clock time per iteration
- **CPU**: CPU time per iteration (excludes I/O, context switches)
- **Iterations**: Number of times the benchmark ran

### Comparing Strategies

```
Adaptive Speedup = Vanilla_Time / Adaptive_Time

Example:
Vanilla: 245.7 μs
Adaptive: 198.4 μs
Speedup: 245.7 / 198.4 = 1.24x faster
```

### Identifying Bottlenecks

If `BM_GeoAddSample` is slow:
- Sample addition is the bottleneck
- Consider optimizing memory allocation
- Check if Eigen vectorization is enabled

If `BM_GeoAdvance` is slow:
- Hypothesis testing is the bottleneck
- Consider reducing grid size
- Optimize bet_on operations

## Advanced Usage

### Counter-Based Benchmarks

Add custom counters to track additional metrics:

```cpp
static void BM_Custom(benchmark::State& state) {
    // ... benchmark code ...
    
    state.counters["SamplesPerSec"] = 
        benchmark::Counter(num_samples, benchmark::Counter::kIsRate);
    state.counters["HypothesesTested"] = 
        benchmark::Counter(grid_num, benchmark::Counter::kAvgThreads);
}
```

### Memory Allocation Tracking

```bash
# Track allocations (requires malloc stats)
./benchmarks/betting_benchmarks --benchmark_counters_tabular=true
```

### Statistical Analysis

```bash
# Run with statistical tests
./benchmarks/betting_benchmarks \
    --benchmark_repetitions=10 \
    --benchmark_report_aggregates_only=true
```

This reports mean, median, stddev across 10 runs.

### Comparison Mode

Compare two builds:

```bash
# Baseline build
./build_baseline/benchmarks/betting_benchmarks \
    --benchmark_out=baseline.json \
    --benchmark_out_format=json

# Optimized build
./build_optimized/benchmarks/betting_benchmarks \
    --benchmark_out=optimized.json \
    --benchmark_out_format=json

# Compare
tools/compare.py baseline.json optimized.json
```

## Optimization Tips

### Compiler Flags

For best benchmark results:

```cmake
# In CMakeLists.txt
target_compile_options(betting_by_time PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native -flto>
)
```

Key flags:
- `-O3`: Maximum optimization
- `-march=native`: Use CPU-specific instructions (AVX, AVX2, etc.)
- `-flto`: Link-time optimization

### Eigen Optimization

Ensure Eigen is optimized:

```cmake
target_compile_definitions(betting_by_time PRIVATE
    $<$<CONFIG:Release>:EIGEN_NO_DEBUG EIGEN_DONT_VECTORIZE=0>
)
```

### System Preparation

Before running benchmarks:

```bash
# Disable CPU frequency scaling
sudo cpufreq-set -g performance

# Close other applications
# Clear disk cache
sudo sync && sudo sysctl -w vm.drop_caches=3

# Pin to specific CPU core
taskset -c 0 ./benchmarks/betting_benchmarks
```

## Troubleshooting

### Problem: High variance in results

**Solution**:
- Increase `--benchmark_min_time`
- Run more repetitions: `--benchmark_repetitions=10`
- Disable CPU throttling
- Close background processes

### Problem: Benchmarks too fast (< 1 μs)

**Solution**:
- Increase problem size
- Use `--benchmark_min_time=5` for longer runs
- Add more work inside loop

### Problem: Benchmarks too slow (> 10 s)

**Solution**:
- Reduce problem size
- Use `--benchmark_min_time=0.1` for shorter runs
- Profile to find bottlenecks

### Problem: Different results on different runs

**Solution**:
- Use fixed random seed (already done)
- Control system state (CPU freq, caches)
- Run multiple times and average

## Integration with CI/CD

### GitHub Actions Example

```yaml
name: Benchmarks
on: [push]

jobs:
  benchmark:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Build benchmarks
        run: |
          mkdir build && cd build
          cmake .. -DCMAKE_BUILD_TYPE=Release
          make betting_benchmarks
      
      - name: Run benchmarks
        run: |
          cd build
          ./benchmarks/betting_benchmarks \
            --benchmark_out=results.json \
            --benchmark_out_format=json
      
      - name: Upload results
        uses: actions/upload-artifact@v2
        with:
          name: benchmark-results
          path: build/results.json
```

### Regression Detection

```python
#!/usr/bin/env python3
"""Detect performance regressions"""
import json
import sys

def check_regression(baseline_file, current_file, threshold=0.1):
    with open(baseline_file) as f:
        baseline = json.load(f)
    with open(current_file) as f:
        current = json.load(f)
    
    regressions = []
    for bench in current['benchmarks']:
        name = bench['name']
        current_time = bench['real_time']
        
        # Find matching baseline
        base_bench = next(
            (b for b in baseline['benchmarks'] if b['name'] == name),
            None
        )
        
        if base_bench:
            base_time = base_bench['real_time']
            slowdown = current_time / base_time
            
            if slowdown > (1 + threshold):
                regressions.append({
                    'name': name,
                    'baseline': base_time,
                    'current': current_time,
                    'slowdown': slowdown
                })
    
    if regressions:
        print("REGRESSIONS DETECTED:")
        for r in regressions:
            print(f"  {r['name']}: {r['slowdown']:.2f}x slower")
        sys.exit(1)
    else:
        print("No regressions detected ✓")

if __name__ == '__main__':
    check_regression(sys.argv[1], sys.argv[2])
```

## Best Practices

1. **Always use Release mode** for benchmarks
2. **Run multiple times** and check consistency
3. **Control system state** (CPU freq, thermal throttling)
4. **Use fixed seeds** for reproducibility
5. **Document hardware** when reporting results
6. **Compare relative performance**, not absolute times
7. **Watch for outliers** and investigate causes
8. **Profile before optimizing** to focus on bottlenecks

## Related Files

- `/benchmarks/benchmark_main.cpp` - Main benchmark file
- `/benchmarks/CMakeLists.txt` - Build configuration
- `/BENCHMARK_GUIDE.md` - This file
- `/CMakeLists.txt` - Main build file (enables benchmarks)

## References

- [Google Benchmark Documentation](https://github.com/google/benchmark)
- [Eigen Performance Guide](https://eigen.tuxfamily.org/dox/TopicPerformance.html)
- [Agner Fog's Optimization Manual](https://www.agner.org/optimize/)
