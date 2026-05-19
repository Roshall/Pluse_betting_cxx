#!/bin/bash
# Quick benchmark runner script

set -e

BUILD_DIR="build"
BENCHMARK_BIN="$BUILD_DIR/benchmarks/betting_benchmarks"

echo "=========================================="
echo "  Betting-by-Time Benchmark Suite"
echo "=========================================="
echo

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory not found. Building first..."
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make betting_benchmarks -j$(nproc)
    cd ..
fi

# Check if benchmark binary exists
if [ ! -f "$BENCHMARK_BIN" ]; then
    echo "Benchmark binary not found. Building..."
    cd $BUILD_DIR
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make betting_benchmarks -j$(nproc)
    cd ..
fi

echo "Running benchmarks..."
echo

# Run all benchmarks with default settings
$BENCHMARK_BIN \
    --benchmark_min_time=0.5 \
    --benchmark_repetitions=3 \
    --benchmark_report_aggregates_only=true \
    --benchmark_format=console

echo
echo "=========================================="
echo "  Benchmarks Complete!"
echo "=========================================="
echo
echo "To save results to JSON:"
echo "  $BENCHMARK_BIN --benchmark_out=results.json --benchmark_out_format=json"
echo
echo "To run specific benchmarks:"
echo "  $BENCHMARK_BIN --benchmark_filter=BM_VanillaGeo"
echo "  $BENCHMARK_BIN --benchmark_filter=BM_Adaptive"
echo "  $BENCHMARK_BIN --benchmark_filter=BM_Scaling"
echo
