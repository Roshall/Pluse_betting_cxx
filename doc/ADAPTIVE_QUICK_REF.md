# Adaptive Betting - Quick Reference

## One-Liner Usage

```cpp
auto [make, fn] = adaptive_geo_factory();
auto gambler = make(0.05f, 0.5f, 1000);
auto [est, used] = fn(samples, prior, delta, 1000, gambler);
```

## API Signature

```cpp
std::pair<Float32, Int32> adaptive_betting(
    const Vector32f& samples,   // Input data
    Float32 prior_mean,          // Prior estimate
    Float32 delta,               // Confidence half-width
    Int32 grid_num,              // Grid resolution
    CapitalProcess& gambler      // Capital process
);
```

## Factory Functions

```cpp
// GeoCheckingCapital + Adaptive
auto [make, fn] = adaptive_geo_factory();

// SequenceCheckingCapital + Adaptive
auto [make, fn] = adaptive_seq_factory();

// Generic factory
auto [make, fn] = betting_factory(BetStrategy::Ada, CapitalType::Geo);
```

## Parameters Guide

| Parameter | Typical Values | Effect |
|-----------|---------------|--------|
| `alpha` | 0.01 - 0.1 | Confidence level (smaller = more confident) |
| `trunc_scale` | 0.1 - 1.0 | Betting aggressiveness |
| `grid_num` | 100 - 5000 | Precision (larger = more precise) |
| `prior_mean` | 0.0 - 1.0 | Initial guess |
| `delta` | 0.05 - 0.5 | Initial interval width |

## Comparison: Vanilla vs Adaptive

| Feature | Vanilla | Adaptive |
|---------|---------|----------|
| Sample efficiency | Baseline | **+10-30%** |
| Speed (small grid) | Faster | Slower |
| Speed (large grid) | Slow | **Faster** |
| Complexity | Simple | Moderate |
| Best for | Small grids | Large grids |

## Common Patterns

### Pattern 1: Basic Estimation
```cpp
auto [make, fn] = adaptive_geo_factory();
auto gambler = make(0.05f, 0.5f, 1000);
auto [est, _] = fn(samples, 0.5f, 0.1f, 1000, gambler);
```

### Pattern 2: With Custom Parameters
```cpp
Float32 alpha = 0.01f;        // High confidence
Float32 trunc = 0.8f;         // Aggressive betting
Int32 grid = 2000;            // High precision

auto [make, fn] = adaptive_geo_factory();
auto gambler = make(alpha, trunc, grid);
auto [est, used] = fn(samples, prior, delta, grid, gambler);
```

### Pattern 3: Sequential Updates
```cpp
Float32 current_est = 0.5f;
for (const auto& batch : batches) {
    auto [make, fn] = adaptive_geo_factory();
    auto gambler = make(0.05f, 0.5f, 1000);
    
    auto [new_est, _] = fn(batch, current_est, 0.05f, 1000, gambler);
    current_est = new_est;
}
```

### Pattern 4: Strategy Comparison
```cpp
// Test both strategies
auto [v_make, v_fn] = vanilla_geo_factory();
auto [a_make, a_fn] = adaptive_geo_factory();

auto v_gambler = v_make(0.05f, 0.5f, 1000);
auto a_gambler = a_make(0.05f, 0.5f, 1000);

auto [v_est, v_used] = v_fn(samples, 0.5f, 0.1f, 1000, v_gambler);
auto [a_est, a_used] = a_fn(samples, 0.5f, 0.1f, 1000, a_gambler);

std::cout << "Vanilla: " << v_used << " samples\n";
std::cout << "Adaptive: " << a_used << " samples\n";
```

## Algorithm Phases

```
Phase 1: Detect which bound is rejected first
         ↓
Phase 2: Binary search to refine that bound
         ↓
Phase 3: Estimate mean in refined interval
```

## Performance Tips

✅ **Do:**
- Use larger grids (≥1000) for best results
- Provide reasonable priors when available
- Enable compiler optimizations (-O3)
- Use GeoCheckingCapital for speed

❌ **Don't:**
- Use with very small grids (<100)
- Expect miracles with terrible priors
- Forget to check return values
- Mix capital processes between calls

## Debugging Checklist

- [ ] Samples in range [0, 1]?
- [ ] Grid size reasonable (100-5000)?
- [ ] Delta not too small (>0.01)?
- [ ] Prior in valid range [0, 1]?
- [ ] Enough samples provided?
- [ ] Using correct capital type?

## Build Commands

```bash
# Build
cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make

# Run example
./examples/test_adaptive_betting

# Run all examples
./examples/basic_usage
./examples/test_bet_on_estimate
./examples/test_adaptive_betting
```

## Files Reference

| File | Purpose |
|------|---------|
| `strategies/adaptive_betting.hpp` | Core implementation |
| `framework.hpp` | Factory functions |
| `examples/test_adaptive_betting.cpp` | Example program |
| `ADAPTIVE_BETTING_GUIDE.md` | Full documentation |
| `ADAPTIVE_IMPLEMENTATION_SUMMARY.md` | Implementation details |

## Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| "Estimate < 0" | Ran out of samples | Provide more samples |
| "Large error" | Poor prior or few samples | Improve prior or add samples |
| "Slow execution" | Large grid | Reduce grid_num or use -O3 |
| "NaN result" | Invalid parameters | Check input ranges |

## Quick Troubleshooting

**Problem**: Estimate inaccurate
```cpp
// Try:
- Increase grid_num (e.g., 1000 → 2000)
- Provide better prior
- Add more samples
- Reduce delta
```

**Problem**: Too slow
```cpp
// Try:
- Reduce grid_num (e.g., 2000 → 1000)
- Use GeoCheckingCapital instead of Seq
- Compile with -O3 -march=native
```

**Problem**: Uses too many samples
```cpp
// This is normal - adaptive may use similar samples
// but provides better accuracy for same sample count
// Focus on accuracy, not sample count
```

## Key Insights

💡 **Adaptive shines when:**
- Grid is large (≥1000 points)
- Prior is biased but direction detectable
- Sample collection is expensive
- Need tight confidence intervals

💡 **Stick with vanilla when:**
- Grid is small (<100 points)
- Simplicity is preferred
- Debugging/testing
- Prior is already accurate

## Statistical Guarantees

- ✅ Type I error controlled at level α
- ✅ Coverage probability ≥ 1 - δ
- ✅ Valid confidence intervals
- ✅ Same guarantees as vanilla betting

## Next Steps

After mastering adaptive betting:
1. Try different capital processes
2. Experiment with grid sizes
3. Compare with vanilla strategy
4. Implement custom refinements
5. Apply to your specific use case

---

**Remember**: Adaptive betting is about **efficiency**, not magic. It uses statistical evidence to focus computational effort where it matters most!
