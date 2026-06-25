import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))

import betting_by_time_cxx as cxx_betting

from samplers.betting_by_time.geo_checking import GeoCheckingCapital
from samplers.betting_by_time.sequence_checking import SequenceCheckingCapital
from samplers.betting_by_time.betting_strategies import vanilla_betting_factory, adaptive_betting_factory

NUM_SAMPLES = 10000
PRIOR_MEAN = 0.5
DELTA = 0.01
GRID_NUM = 1000

SELECTIVITIES = [0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5]


def generate_binary_samples(true_mean, n, seed=0):
    np.random.seed(seed)
    return np.random.binomial(1, true_mean, n).astype(np.float32)


def approx_equal(a, b, tol=1e-5):
    return abs(a - b) <= tol


def test_vanilla_geo():
    print("=" * 60)
    print("Testing vanilla_geo equivalence")
    print("=" * 60)
    
    strategy_fn = vanilla_betting_factory(GeoCheckingCapital)
    all_passed = True
    
    for test_id, selectivity in enumerate(SELECTIVITIES):
        samples = generate_binary_samples(selectivity, NUM_SAMPLES, seed=test_id)
        times = np.arange(len(samples) + 1, dtype=np.int32)
        
        py_est, py_lb, py_ub, py_used = strategy_fn(
            samples, times, PRIOR_MEAN, DELTA, GRID_NUM,
            GeoCheckingCapital(0.05, 0.5, GRID_NUM)
        )
        
        cxx_est, cxx_lb, cxx_ub, cxx_used = cxx_betting.vanilla_betting_geo(
            samples, PRIOR_MEAN, DELTA, GRID_NUM,
            breakpoints=times.tolist()
        )
        
        est_ok = approx_equal(py_est, cxx_est, 1e-5)
        lb_ok = approx_equal(py_lb, cxx_lb, 1e-5)
        ub_ok = approx_equal(py_ub, cxx_ub, 1e-5)
        used_ok = py_used == cxx_used
        
        status = "PASS" if (est_ok and lb_ok and ub_ok and used_ok) else "FAIL"
        all_passed = all_passed and (est_ok and lb_ok and ub_ok and used_ok)
        
        print(f"  test_id={test_id}, selectivity={selectivity}")
        print(f"    Python: est={py_est:.6f}, lb={py_lb:.6f}, ub={py_ub:.6f}, used={py_used}")
        print(f"    C++   : est={cxx_est:.6f}, lb={cxx_lb:.6f}, ub={cxx_ub:.6f}, used={cxx_used}")
        print(f"    Status: {status}")
        if not est_ok:
            print(f"      est diff: {abs(py_est - cxx_est):.10f}")
        if not lb_ok:
            print(f"      lb diff: {abs(py_lb - cxx_lb):.10f}")
        if not ub_ok:
            print(f"      ub diff: {abs(py_ub - cxx_ub):.10f}")
        if not used_ok:
            print(f"      used diff: py={py_used}, cxx={cxx_used}")
    
    print(f"\n  Overall: {'ALL PASSED' if all_passed else 'SOME FAILED'}")
    return all_passed


def test_vanilla_seq():
    print("\n" + "=" * 60)
    print("Testing vanilla_seq equivalence")
    print("=" * 60)
    
    strategy_fn = vanilla_betting_factory(SequenceCheckingCapital)
    all_passed = True
    
    for test_id, selectivity in enumerate(SELECTIVITIES):
        samples = generate_binary_samples(selectivity, NUM_SAMPLES, seed=test_id)
        times = np.arange(len(samples) + 1, dtype=np.int32)
        
        py_est, py_lb, py_ub, py_used = strategy_fn(
            samples, times, PRIOR_MEAN, DELTA, GRID_NUM,
            SequenceCheckingCapital(0.05, 0.5, GRID_NUM)
        )
        
        cxx_est, cxx_lb, cxx_ub, cxx_used = cxx_betting.vanilla_betting_seq(
            samples, PRIOR_MEAN, DELTA, GRID_NUM,
            breakpoints=times.tolist()
        )
        
        est_ok = approx_equal(py_est, cxx_est, 1e-5)
        lb_ok = approx_equal(py_lb, cxx_lb, 1e-5)
        ub_ok = approx_equal(py_ub, cxx_ub, 1e-5)
        used_ok = py_used == cxx_used
        
        status = "PASS" if (est_ok and lb_ok and ub_ok and used_ok) else "FAIL"
        all_passed = all_passed and (est_ok and lb_ok and ub_ok and used_ok)
        
        print(f"  test_id={test_id}, selectivity={selectivity}")
        print(f"    Python: est={py_est:.6f}, lb={py_lb:.6f}, ub={py_ub:.6f}, used={py_used}")
        print(f"    C++   : est={cxx_est:.6f}, lb={cxx_lb:.6f}, ub={cxx_ub:.6f}, used={cxx_used}")
        print(f"    Status: {status}")
        if not est_ok:
            print(f"      est diff: {abs(py_est - cxx_est):.10f}")
        if not lb_ok:
            print(f"      lb diff: {abs(py_lb - cxx_lb):.10f}")
        if not ub_ok:
            print(f"      ub diff: {abs(py_ub - cxx_ub):.10f}")
        if not used_ok:
            print(f"      used diff: py={py_used}, cxx={cxx_used}")
    
    print(f"\n  Overall: {'ALL PASSED' if all_passed else 'SOME FAILED'}")
    return all_passed


def test_adaptive_geo():
    print("\n" + "=" * 60)
    print("Testing adaptive_geo equivalence")
    print("=" * 60)
    
    strategy_fn = adaptive_betting_factory(GeoCheckingCapital, cap_mtd='geo')
    all_passed = True
    
    for test_id, selectivity in enumerate(SELECTIVITIES):
        samples = generate_binary_samples(selectivity, NUM_SAMPLES, seed=test_id)
        times = np.arange(len(samples) + 1, dtype=np.int32)
        
        py_est, py_lb, py_ub, py_used = strategy_fn(
            samples, times, PRIOR_MEAN, DELTA, GRID_NUM,
            GeoCheckingCapital(0.05, 0.5, GRID_NUM)
        )
        
        cxx_est, cxx_lb, cxx_ub, cxx_used = cxx_betting.adaptive_betting_geo(
            samples, PRIOR_MEAN, DELTA, GRID_NUM,
            breakpoints=times.tolist()
        )
        
        grid_step = 1.0 / GRID_NUM
        est_ok = approx_equal(py_est, cxx_est, grid_step + 1e-6)
        lb_ok = approx_equal(py_lb, cxx_lb, grid_step + 1e-6)
        ub_ok = approx_equal(py_ub, cxx_ub, grid_step + 1e-6)
        used_ok = py_used == cxx_used
        
        status = "PASS" if (est_ok and lb_ok and ub_ok and used_ok) else "FAIL"
        all_passed = all_passed and (est_ok and lb_ok and ub_ok and used_ok)
        
        print(f"  test_id={test_id}, selectivity={selectivity}")
        print(f"    Python: est={py_est:.6f}, lb={py_lb:.6f}, ub={py_ub:.6f}, used={py_used}")
        print(f"    C++   : est={cxx_est:.6f}, lb={cxx_lb:.6f}, ub={cxx_ub:.6f}, used={cxx_used}")
        print(f"    Status: {status}")
        if not est_ok:
            print(f"      est diff: {abs(py_est - cxx_est):.10f}")
        if not lb_ok:
            print(f"      lb diff: {abs(py_lb - cxx_lb):.10f}")
        if not ub_ok:
            print(f"      ub diff: {abs(py_ub - cxx_ub):.10f}")
        if not used_ok:
            print(f"      used diff: py={py_used}, cxx={cxx_used}")
    
    print(f"\n  Overall: {'ALL PASSED' if all_passed else 'SOME FAILED'}")
    return all_passed


def test_adaptive_seq():
    print("\n" + "=" * 60)
    print("Testing adaptive_seq equivalence")
    print("=" * 60)
    
    strategy_fn = adaptive_betting_factory(SequenceCheckingCapital, cap_mtd='seq')
    all_passed = True
    
    for test_id, selectivity in enumerate(SELECTIVITIES):
        samples = generate_binary_samples(selectivity, NUM_SAMPLES, seed=test_id)
        times = np.arange(len(samples) + 1, dtype=np.int32)
        
        py_est, py_lb, py_ub, py_used = strategy_fn(
            samples, times, PRIOR_MEAN, DELTA, GRID_NUM,
            SequenceCheckingCapital(0.05, 0.5, GRID_NUM)
        )
        
        cxx_est, cxx_lb, cxx_ub, cxx_used = cxx_betting.adaptive_betting_seq(
            samples, PRIOR_MEAN, DELTA, GRID_NUM,
            breakpoints=times.tolist()
        )
        
        grid_step = 1.0 / GRID_NUM
        est_ok = approx_equal(py_est, cxx_est, grid_step + 1e-6)
        lb_ok = approx_equal(py_lb, cxx_lb, grid_step + 1e-6)
        ub_ok = approx_equal(py_ub, cxx_ub, grid_step + 1e-6)
        used_ok = py_used == cxx_used
        
        status = "PASS" if (est_ok and lb_ok and ub_ok and used_ok) else "FAIL"
        all_passed = all_passed and (est_ok and lb_ok and ub_ok and used_ok)
        
        print(f"  test_id={test_id}, selectivity={selectivity}")
        print(f"    Python: est={py_est:.6f}, lb={py_lb:.6f}, ub={py_ub:.6f}, used={py_used}")
        print(f"    C++   : est={cxx_est:.6f}, lb={cxx_lb:.6f}, ub={cxx_ub:.6f}, used={cxx_used}")
        print(f"    Status: {status}")
        if not est_ok:
            print(f"      est diff: {abs(py_est - cxx_est):.10f}")
        if not lb_ok:
            print(f"      lb diff: {abs(py_lb - cxx_lb):.10f}")
        if not ub_ok:
            print(f"      ub diff: {abs(py_ub - cxx_ub):.10f}")
        if not used_ok:
            print(f"      used diff: py={py_used}, cxx={cxx_used}")
    
    print(f"\n  Overall: {'ALL PASSED' if all_passed else 'SOME FAILED'}")
    return all_passed


def main():
    print("Betting-by-Time Equivalence Tests")
    print(f"  Num samples: {NUM_SAMPLES}")
    print(f"  Prior mean: {PRIOR_MEAN}")
    print(f"  Delta: {DELTA}")
    print(f"  Grid num: {GRID_NUM}")
    print(f"  Selectivities: {SELECTIVITIES}")
    print()
    
    results = {}
    results['vanilla_geo'] = test_vanilla_geo()
    results['vanilla_seq'] = test_vanilla_seq()
    results['adaptive_geo'] = test_adaptive_geo()
    results['adaptive_seq'] = test_adaptive_seq()
    
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    for name, passed in results.items():
        print(f"  {name:20s}: {'PASS' if passed else 'FAIL'}")
    
    all_passed = all(results.values())
    print(f"\n  Overall: {'ALL TESTS PASSED' if all_passed else 'SOME TESTS FAILED'}")
    
    return 0 if all_passed else 1


if __name__ == '__main__':
    sys.exit(main())
