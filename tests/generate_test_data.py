import numpy as np
import os

from samplers.betting_by_time.geo_checking import GeoCheckingCapital
from samplers.betting_by_time.sequence_checking import SequenceCheckingCapital
from samplers.betting_by_time.betting_strategies import vanilla_betting_factory, adaptive_betting_factory

NUM_SAMPLES = 10000
PRIOR_MEAN = 0.5
DELTA = 0.01
GRID_NUM = 1000

SELECTIVITIES = [0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5]


def generate_binary_samples(true_mean, n):
    return np.random.binomial(1, true_mean, n).astype(np.float32)


def write_csv(filepath, test_cases):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, 'w') as f:
        f.write("test_id,prior_mean,delta,grid_num,expected_mean,expected_lower_bound,expected_upper_bound,expected_used,num_samples,samples...\n")
        for tc in test_cases:
            row = [
                str(tc['test_id']),
                str(tc['prior_mean']),
                str(tc['delta']),
                str(tc['grid_num']),
                str(tc['expected_mean']),
                str(tc['expected_lower_bound']),
                str(tc['expected_upper_bound']),
                str(tc['expected_used']),
                str(tc['num_samples'])
            ] + [str(s) for s in tc['samples']]
            f.write(','.join(row) + '\n')


def generate_strategy_data(strategy_name, capital_cls, strategy_factory):
    test_cases = []
    strategy_fn = strategy_factory(capital_cls)
    
    for test_id, selectivity in enumerate(SELECTIVITIES):
        np.random.seed(test_id)
        samples = generate_binary_samples(selectivity, NUM_SAMPLES)
        
        gambler = capital_cls(0.05, 0.5, GRID_NUM)
        times = np.arange(len(samples) + 1, dtype=np.int32)
        
        result = strategy_fn(samples, times, PRIOR_MEAN, DELTA, GRID_NUM, gambler)
        
        if len(result) == 4:
            est, lb, ub, used = result
        else:
            est, lb, ub, used = result[0], result[1], result[2], result[3]
        
        test_cases.append({
            'test_id': test_id,
            'prior_mean': PRIOR_MEAN,
            'delta': DELTA,
            'grid_num': GRID_NUM,
            'expected_mean': est,
            'expected_lower_bound': lb,
            'expected_upper_bound': ub,
            'expected_used': used,
            'num_samples': NUM_SAMPLES,
            'samples': samples
        })
        
        print(f"[{strategy_name}] test_id={test_id}, selectivity={selectivity}, est={est:.6f}, lb={lb:.6f}, ub={ub:.6f}, used={used}")
    
    return test_cases


def main():
    output_dir = os.path.join(os.path.dirname(__file__), 'test_data')
    
    print("Generating vanilla_geo data...")
    vanilla_geo_cases = generate_strategy_data("vanilla_geo", GeoCheckingCapital, vanilla_betting_factory)
    write_csv(os.path.join(output_dir, 'test_data_vanilla_geo.csv'), vanilla_geo_cases)
    
    print("\nGenerating vanilla_seq data...")
    vanilla_seq_cases = generate_strategy_data("vanilla_seq", SequenceCheckingCapital, vanilla_betting_factory)
    write_csv(os.path.join(output_dir, 'test_data_vanilla_seq.csv'), vanilla_seq_cases)
    
    print("\nGenerating adaptive_geo data...")
    adaptive_geo_cases = generate_strategy_data("adaptive_geo", GeoCheckingCapital, lambda cls: adaptive_betting_factory(cls, cap_mtd='geo'))
    write_csv(os.path.join(output_dir, 'test_data_adaptive_geo.csv'), adaptive_geo_cases)
    
    print("\nGenerating adaptive_seq data...")
    adaptive_seq_cases = generate_strategy_data("adaptive_seq", SequenceCheckingCapital, lambda cls: adaptive_betting_factory(cls, cap_mtd='seq'))
    write_csv(os.path.join(output_dir, 'test_data_adaptive_seq.csv'), adaptive_seq_cases)
    
    print("\nAll test data generated successfully!")


if __name__ == '__main__':
    main()
