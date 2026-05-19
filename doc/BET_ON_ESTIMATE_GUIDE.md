# bet_on and estimate Functions Documentation

## Overview

The `bet_on` and `estimate` functions provide incremental hypothesis testing and mean estimation capabilities for the betting-by-time framework. These are essential building blocks for adaptive betting strategies.

## Function Signatures

### bet_on

```cpp
template<typename CapitalProcess>
bool bet_on(CapitalProcess& gambler, Float32 m, Int32 mi, Int32 which = 2);
```

**Purpose**: Updates cumulative capitals for a specific hypothesis and checks if it should be rejected.

**Parameters**:
- `gambler`: Reference to capital process (GeoCheckingCapital or SequenceCheckingCapital)
- `m`: Hypothesized mean value to test
- `mi`: Index of the hypothesis in the grid (mi = m * grid_num)
- `which`: Which twin(s) to update
  - `0`: Update only positive twin (bet on mean > m)
  - `1`: Update only negative twin (bet on mean < m)
  - `2`: Update both twins (default)

**Returns**: `true` if capital exceeds threshold (hypothesis rejected), `false` otherwise

**Key Features**:
- **Incremental**: Tracks position to avoid reprocessing samples
- **Efficient**: Early exit when threshold exceeded
- **Flexible**: Can update one or both twins independently

### estimate

```cpp
template<typename CapitalProcess>
Float32 estimate(CapitalProcess& gambler, Float32 l, Float32 u);
```

**Purpose**: Estimates the true mean by finding the hypothesis with minimum capital in a range.

**Parameters**:
- `gambler`: Reference to capital process
- `l`: Lower bound of search range
- `u`: Upper bound of search range

**Returns**: Estimated mean value (grid point with minimum capital sum)

**Key Features**:
- **Batch Processing**: Tests all hypotheses in range [l, u]
- **Optimal Selection**: Returns hypothesis with minimum capital
- **Range Clamping**: Automatically clamps to [0, 1]

## How They Work

### bet_on Algorithm

1. **Position Tracking**: Each hypothesis maintains `cum_cap_pos[mi]` tracking which samples have been processed
2. **Catch-up Logic**: If twins are at different positions, advance the lagging one first
3. **Joint Advancement**: Process remaining samples for both twins
4. **Early Exit**: Stop processing if capital exceeds threshold
5. **State Update**: Update positions and return rejection status

```
Sample Timeline:
|--processed--|--unprocessed-->
              ^
         old_pos[mi]
         
After bet_on:
|--all processed-->
                  ^
            new pos = s_ptr
```

### estimate Algorithm

1. **Range Validation**: Clamp l and u to [0, 1], check validity
2. **Index Conversion**: Convert bounds to grid indices
3. **Batch bet_on**: Call bet_on for all hypotheses in range
4. **Capital Summation**: Sum positive and negative twins for each hypothesis
5. **Minimum Finding**: Return hypothesis index with minimum sum

```
Grid:     0    0.1   0.2   0.3   ...  0.9   1.0
          |-----|-----|-----|-----|-----|
Range:          [l============u]
                 ^           ^
                li          ui
                
Find min capital sum in [li, ui]
```

## Usage Examples

### Basic bet_on Usage

```cpp
#include "betting_by_time/bet_on_estimate.hpp"

GeoCheckingCapital gambler(0.05f, 0.5f, 1000);

// Add some samples
for (int i = 0; i < 100; ++i) {
    gambler.add_sample(sample[i]);
}

// Test hypothesis m = 0.5
Float32 m = 0.5f;
Int32 mi = static_cast<Int32>(m * 1000);

bool rejected = bet_on(gambler, m, mi, 2);

if (rejected) {
    std::cout << "Hypothesis m=0.5 rejected!" << std::endl;
} else {
    std::cout << "Hypothesis m=0.5 still plausible" << std::endl;
}
```

### Basic estimate Usage

```cpp
// Estimate mean in range [0.4, 0.8]
Float32 estimated = estimate(gambler, 0.4f, 0.8f);

std::cout << "Estimated mean: " << estimated << std::endl;
```

### Incremental Testing

```cpp
// Test multiple hypotheses incrementally
std::vector<Float32> hypotheses = {0.3f, 0.5f, 0.7f};

for (Float32 m : hypotheses) {
    Int32 mi = static_cast<Int32>(m * 1000);
    
    // Reset position tracking for fresh test
    gambler.reset();
    
    // Add samples and test
    for (int i = 0; i < 200; ++i) {
        gambler.add_sample(samples[i]);
        
        // Test every 50 samples
        if ((i + 1) % 50 == 0) {
            bool rejected = bet_on(gambler, m, mi, 2);
            std::cout << "m=" << m << " at sample " << (i+1) 
                      << ": " << (rejected ? "REJECTED" : "OK") << std::endl;
        }
    }
}
```

### Binary Search for Bounds

```cpp
// Find lower bound using binary search
Float32 l = 0.0f, u = 0.5f;
Int32 li = 0, ui = 500;

while (li + 1 < ui) {
    Int32 mid = (li + ui) / 2;
    Float32 m = mid / 1000.0f;
    
    if (bet_on(gambler, m, mid, 0)) {
        li = mid;  // Lower bound is higher
    } else {
        ui = mid;  // Lower bound is lower
    }
}

Float32 lower_bound = ui / 1000.0f;
```

## Performance Characteristics

### Time Complexity

- **bet_on**: O(n) where n = number of unprocessed samples
  - Early exit can make it much faster in practice
  - Amortized O(1) per sample due to position tracking
  
- **estimate**: O(k * n) where k = number of hypotheses in range
  - Typically k << grid_num due to range restriction
  - Can be optimized with parallel processing

### Space Complexity

- **bet_on**: O(1) additional space
  - Uses existing capital process state
  
- **estimate**: O(k) for capital sums vector
  - k = number of hypotheses in range

### Optimizations

1. **Position Tracking**: Avoids reprocessing samples
2. **Early Exit**: Stops when threshold exceeded
3. **Vectorization**: Eigen operations for batch updates
4. **Cache Efficiency**: Sequential memory access patterns

## Integration with Adaptive Betting

These functions are the foundation for adaptive betting strategies:

```cpp
// Simplified adaptive betting pseudocode
Float32 l = prior - delta, u = prior + delta;

// Phase 1: Detect direction
for (sample : samples) {
    gambler.add_sample(sample);
    
    if (bet_on(gambler, l, li, 2)) touch |= 1;  // touched lower
    if (bet_on(gambler, u, ui, 2)) touch |= 2;  // touched upper
    
    if (touch != 0) break;  // Found direction
}

// Phase 2: Refine bounds
if (touch == 1) {
    // Binary search for lower bound
    while (li + 1 < ui) {
        Int32 mid = (li + ui) / 2;
        if (bet_on(gambler, mid/stride, mid, 0)) {
            li = mid;
        } else {
            ui = mid;
        }
    }
    l = ui * stride;
}

// Final estimate
Float32 estimate = estimate(gambler, l, u);
```

## Common Patterns

### Pattern 1: Hypothesis Rejection Testing

```cpp
// Test if a specific hypothesis is plausible
bool is_plausible(GeoCheckingCapital& gambler, Float32 m) {
    Int32 mi = static_cast<Int32>(m * gambler.grid_num());
    return !bet_on(gambler, m, mi, 2);
}
```

### Pattern 2: Confidence Interval Construction

```cpp
// Find confidence interval at level delta
std::pair<Float32, Float32> confidence_interval(
    GeoCheckingCapital& gambler, Float32 delta) {
    
    Int32 grid = gambler.grid_num();
    Float32 stride = 1.0f / grid;
    
    // Find lower bound
    Int32 li = 0;
    for (Int32 i = 0; i < grid; ++i) {
        if (!bet_on(gambler, i * stride, i, 0)) {
            li = i;
            break;
        }
    }
    
    // Find upper bound
    Int32 ui = grid;
    for (Int32 i = grid; i >= 0; --i) {
        if (!bet_on(gambler, i * stride, i, 1)) {
            ui = i;
            break;
        }
    }
    
    return {li * stride, ui * stride};
}
```

### Pattern 3: Sequential Estimation

```cpp
// Estimate mean as samples arrive
Float32 sequential_estimate(GeoCheckingCapital& gambler, 
                           const Vector32f& samples) {
    Float32 window = 0.1f;  // Search window size
    
    for (Float32 sample : samples) {
        gambler.add_sample(sample);
        
        // Update estimate in sliding window
        Float32 current_est = /* previous estimate */;
        Float32 l = std::max(0.0f, current_est - window);
        Float32 u = std::min(1.0f, current_est + window);
        
        current_est = estimate(gambler, l, u);
    }
    
    return current_est;
}
```

## Testing

Run the example program to see these functions in action:

```bash
cd build
make test_bet_on_estimate
./examples/test_bet_on_estimate
```

This demonstrates:
- Basic bet_on functionality
- Hypothesis rejection
- Mean estimation
- Incremental updates
- Batch processing

## Implementation Notes

### Template Design

Both functions are templates to work with any capital process type:
- `GeoCheckingCapital`: For geometric checking
- `SequenceCheckingCapital`: For sequence checking

This provides flexibility without code duplication.

### Convenience Overloads

Inline convenience functions are provided for common types:

```cpp
// These are equivalent:
bet_on<GeoCheckingCapital>(gambler, m, mi, which);
bet_on(gambler, m, mi, which);  // Type inferred
```

### Thread Safety

These functions are **not thread-safe** as they modify gambler state. For parallel hypothesis testing:
- Use separate gambler instances per thread
- Or use mutex protection around calls

### Numerical Stability

- Capitals use Float64 (double) to prevent underflow
- Epsilon checks (1e-16) for near-zero comparisons
- Clamping to valid ranges [0, 1]

## Related Functions

- `geo_single_bet_on()`: Core betting operation
- `vanilla_betting()`: Full vanilla strategy using these functions
- `advance()`: Batch hypothesis testing (alternative approach)

## References

- Python implementation: `samplers/betting_by_time/bet_once.py`
- Paper: "Testing by Betting: A Framework for Statistical Hypothesis Testing"
- Header: `include/betting_by_time/bet_on_estimate.hpp`
