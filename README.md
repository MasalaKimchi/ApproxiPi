# SATO-X Benchmark MVP

SATO-X is a local benchmark harness for comparing pi algorithms under one
reproducible C++17 + GMP + MPFR environment. The MVP treats Chudnovsky binary
splitting as the baseline to beat and rejects speed claims for candidate
formulas until they pass the same verification pipeline.

## Build

```sh
make
```

## Test

```sh
make test
```

## Smoke Benchmark

```sh
make smoke
```

Outputs are written to:

- `results/benchmark.csv`
- `results/benchmark.json`
- `results/summary.md`

## Full Default Benchmark

```sh
./bin/satox-bench --digits 1000,10000,100000,1000000 --guard 25 --out results
```

Ramanujan and AGM intentionally cap at `100000` digits in v1. Chudnovsky runs
through `1000000` digits and serves as the baseline.

Benchmark rows are repeated by default. Use `--trials` and `--warmups` to tune
the statistics:

```sh
./bin/satox-bench --digits 1000,10000 --trials 5 --warmups 1 --out results
```

The CSV preserves a `wall_ms` column for plotting, but it now represents median
wall time and also includes `min_wall_ms`, `max_wall_ms`, and `stddev_wall_ms`.

## Candidate Formula Metadata

Candidate formulas can be registered with:

```sh
./bin/satox-bench --digits 1000 --candidates formulas/candidates.tsv
```

The TSV schema is:

```text
id|family|recurrence|estimated_digits_per_term|discriminant|class_invariant|algebraic_height_bits|polynomial_degree|numerator_degree|denominator_degree|binary_splitting_ready|proof_status|implementation_notes|source
```

Metadata alone never creates a performance claim. A candidate must have an
implemented kernel, pass verification, and beat the Chudnovsky baseline at the
same precision before it is considered a credible faster approximation.

Candidate scoring is intentionally conservative. It rewards convergence,
binary-splitting readiness, and proof status, while penalizing algebraic height
and recurrence degree. Symbolic targets such as `CM-HX` can be represented with
non-numeric discriminants like `-d`, but they remain metadata-only until a proof
certificate and benchmark kernel exist.

Formula specs live in `candidates/*.formula`. They are key-value files that can
describe a linear-factor hypergeometric binary-splitting candidate:

```text
id=C-163
p_factors=6n-5,2n-1,6n-1
q_factors=n,n,n
q_constant=10939058860032000
linear_a=545140134
linear_b=13591409
alternating=yes
unit_first_p=yes
unit_first_q=yes
leaf_t_multiplier=p
gcd_cancellation=no
```

Running the benchmark writes `results/satox-score.md`, a ranked pre-benchmark
score report for all formula specs in the candidate directory.

## Figures

Generate optimized SVG figures from the benchmark CSV:

```sh
make figures
```

The figures are written to `docs/figures/`:

- `wall_time_log.svg`
- `relative_wall_time.svg`
- `terms_or_iterations.svg`
- `verification_matrix.svg`

## Implemented Algorithms

- `chudnovsky_bs`: Chudnovsky binary splitting baseline.
- `ramanujan_classic_bs`: classical Ramanujan series with binary splitting.
- `gauss_legendre_agm`: quadratic-convergent AGM comparator.
- `borwein_quartic`: quartic-convergent Borwein-style comparator.

SATO-X discovery is not yet implemented as a proof-producing formula search
engine. The current project is the benchmark and verification substrate needed
before new Ramanujan-Sato candidates can make credible speed claims.

AGM and Borwein attempt adaptive precision internally. If the adaptive pass
fails exact prefix verification, the implementation reruns at full MPFR
precision and labels the verification method as a fallback. This is deliberately
strict: adaptive shortcuts are allowed only when they verify.

The repo also includes a generic linear-factor hypergeometric binary-splitting
kernel in `include/satox/binary_splitting.hpp`. It is the first step toward a
candidate compiler for recurrence-defined Ramanujan-Sato formulas.

Chudnovsky and Ramanujan now both use that shared compiler internally; their
algorithm files keep only the formula constants and final normalization.

The compiler supports opt-in `gcd_cancellation=yes`, which safely divides only
common factors shared by `P`, `Q`, and `T`. It is disabled for the current
Chudnovsky/Ramanujan baselines because measured GCD overhead outweighs its
benefit at local benchmark sizes, but it is available for future candidates
whose recurrences have heavier common factors.
