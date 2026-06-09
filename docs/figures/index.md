# SATO-X Benchmark Figures

Generated from `results/benchmark.csv` with `tools/make_figures.py`.

For method descriptions and formulae, see [Methods comparison](../methods-comparison.md).

## Performance

### Wall time by precision

Median verified wall time across decimal digit targets (log–log scale). Lower is better.

![Wall time by precision](wall_time_log.svg)

### Relative wall time vs. Chudnovsky baseline

Values below the dashed line (1.0) are faster than standard Chudnovsky binary splitting at the same precision.

![Relative wall time](relative_wall_time.svg)

### Multiplication bit volume

Machine-independent work metric for binary-splitting series evaluation: sum of operand bit-lengths over every split-phase multiplication.

![Multiplication bit volume](bit_volume.svg)

## Work breakdown

### Phase timing at maximum precision

Where each algorithm spends time: series/split, finalize, format, and overhead.

![Phase breakdown](phase_breakdown.svg)

### Terms or iterations

Series methods report term counts; AGM reports iterations.

![Terms or iterations](terms_or_iterations.svg)

## Research progression

### Hypothesis ledger at 1M digits

Wall-time improvements from the optimization hypothesis sequence. Numbered stages are explained in the legend below the chart.

![Hypothesis progression](hypothesis_progression.svg)

## Correctness

### Verification matrix

Green cells passed full-prefix verification; gray cells exceed the algorithm's precision cap.

![Verification matrix](verification_matrix.svg)
