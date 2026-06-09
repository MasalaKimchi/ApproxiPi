# SATO-X Benchmark MVP

SATO-X is a local benchmark harness for comparing pi algorithms under one
reproducible C++17 + GMP + MPFR environment. The MVP treats Chudnovsky binary
splitting as the baseline to beat and rejects speed claims for candidate
formulas until they pass the same verification pipeline.

The repository doubles as the artifact for a case study in AI-driven
performance engineering: a hypothesis ledger with pre-registered predictions
and recorded refutations (`docs/research-log.md`) and a paper-style writeup
(`docs/PAPER.md`). Headline at 10^6 digits on this machine, all rows
byte-identical and verified: our truncated-crown Chudnovsky runs in 59.3 ms
vs 99.0 ms for FLINT/Arb `arb_const_pi` and 458 ms for MPFR `mpfr_const_pi`.

## Build

```sh
make
```

FLINT is optional: when `pkg-config --exists flint` succeeds (e.g.
`brew install flint`), the `arb_const_pi` external baseline is compiled in;
otherwise it reports itself as unavailable and everything else still builds.

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

Machin and AGM intentionally cap at `100000` digits in v1. The binary-splitting
algorithms and Borwein variants run through `1000000` digits, with Chudnovsky
serving as the baseline to beat.

Benchmark rows are repeated by default. Use `--trials` and `--warmups` to tune
the statistics:

```sh
./bin/satox-bench --digits 1000,10000 --trials 5 --warmups 1 --out results
```

The CSV preserves a `wall_ms` column for plotting, but it now represents median
wall time and also includes `min_wall_ms`, `max_wall_ms`, and `stddev_wall_ms`.

It also reports a machine-independent work metric: `mul_count` and
`mul_bit_volume` accumulate `bits(a) + bits(b)` over every series-phase
multiplication, so "less work" can be distinguished from "restructured work"
independently of the host machine.

## Autotuning (H11)

```sh
./bin/satox-bench --tune --digits 1000000 --trials 3 --passes 2 --out results
```

Coordinate descent over the crown knob space (leaf block, chunk depth,
parallel levels, intra-node threshold, root split ratio). The winning profile
is cached to `results/tuning.json`; every evaluation is logged to
`results/tuning-log.csv`. When a profile exists, the benchmark adds a
`chudnovsky_bs_crown_tuned` row that loads it at run time. Measured outcome
on this machine: <2% over the hand-derived defaults (H11 refuted; see the
research log).

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
- `bit_volume.svg` (machine-independent multiplication work)
- `phase_breakdown.svg` (split/finalize/format stacked bars at the largest size)
- `hypothesis_progression.svg` (wall time at 1M digits across the ledger)
- `terms_or_iterations.svg`
- `verification_matrix.svg`

An interactive animation is available at
`docs/animations/algorithm-comparison.html`. It compares the same benchmark data
as a wall-time race, convergence-work animation, and verification matrix.

## Implemented Algorithms

- `chudnovsky_bs`: Chudnovsky binary splitting baseline.
- `chudnovsky_bs_valuation`: Chudnovsky with opt-in leaf valuation
  cancellation to reduce operand growth.
- `chudnovsky_bs_crown`: truncated-crown binary splitting (TCBS). Exact
  integer chunks at the bottom of the tree, then an MPFR fixed-point "crown"
  on top whose per-node precision is capped by each subtree's contribution
  offset (derived rigorously from the exact chunk bit sizes). The crown skips
  P products on the rightmost spine, never forms the root Q (pi only needs
  `C * Ql * Qr / T`), warm-starts the Newton reciprocal of T from the root
  children behind a precision-margin gate, pipelines the value-independent
  constants `sqrt(10005)` and `10^digits` under the series evaluation, and
  renders decimals with an 8-way parallel divide-and-conquer conversion.
  H12 additionally splices the decimal output: the high half of the digits is
  rendered concurrently with the Newton correction (which provably cannot
  change it, enforced by an integer range check with a re-render fallback).
  Verified at all benchmark sizes; ~2.4x faster than `chudnovsky_bs` at
  10^6 digits and ~2.9x at 10^5 on this machine. The hypothesis-by-hypothesis
  derivation, including refuted attempts, is in `docs/research-log.md`.
- `chudnovsky_bs_crown_tuned`: the same kernel driven by the cached autotune
  profile (only listed when `results/tuning.json` exists).
- `ramanujan_classic_bs`: classical Ramanujan series with binary splitting.
- `machin_arctan`: independent Machin arctangent identity comparator.
- `gauss_legendre_agm`: quadratic-convergent AGM comparator.
- `borwein_cubic`: cubic-convergent Borwein comparator.
- `borwein_quartic`: quartic-convergent Borwein-style comparator.
- `mpfr_const_pi`: external baseline; MPFR's own pi with a cold cache per
  trial, run through the same format/verify pipeline.
- `arb_const_pi`: external baseline; FLINT/Arb's pi with a cold cache per
  trial and all hardware threads granted (requires FLINT at build time).

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

The shared binary-splitting path now uses bounded parallel subtree evaluation,
an `mpz_addmul` merge that avoids one temporary large-integer product per
internal node, small 8-term leaf blocks before recursing, and an experimental
Chudnovsky valuation-cancellation variant that trims common factors at leaf
construction. Ramanujan uses the actual asymptotic convergence rate
`log10(396^4 / 256)` instead of the rounded `8.0`, which lets the harness
verify it at `1000000` digits without under-counting required terms. The
benchmark CSV also reports phase timings for split, finalization, decimal
formatting, and verification, plus max operand bits and selected parallel
depth. Current regenerated results show the valuation-cancelled Chudnovsky
variant slightly ahead at `1000000` digits on this machine, while the other
verified methods remain important correctness and convergence comparators.

The compiler supports opt-in `gcd_cancellation=yes`, which safely divides only
common factors shared by `P`, `Q`, and `T`. It is disabled for the current
Chudnovsky/Ramanujan baselines because measured GCD overhead outweighs its
benefit at local benchmark sizes, but it is available for future candidates
whose recurrences have heavier common factors.
