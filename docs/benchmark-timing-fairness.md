# Benchmark timing fairness

**Status:** resolved on 2026-06-11  
**Related:** H13 pre-format verification, H20 external baseline parity, H23/H25 measurement hygiene

## Contract

All supported benchmark rows now use the same timing ledger:

```text
total_cost_ms = wall_ms + verify_ms + io_ms
```

`wall_ms` excludes the harness verification phase. It covers the compute path
through decimal rendering: split/series work, finalize, sqrt/div, and format.
`verify_ms` is timed separately and counted exactly once in `total_cost_ms`.
`io_ms` is explicit checkpoint or artifact I/O.

Use `wall_ms` for compute-path comparisons and `total_cost_ms` for verified
end-to-end rankings.

## What was fixed

The audit found that external baselines stopped their wall timer before
verification, while several SATO-X paths stopped after verification:

- `chudnovsky_bs`
- `chudnovsky_bs_valuation`
- `chudnovsky_bs_crown`
- `chudnovsky_bs_crown_h15`
- `chudnovsky_bs_crown_tuned`
- `chudnovsky_naive`
- `chudnovsky_recurrence`
- `chudnovsky_hybrid` when delegated to Crown
- `ramanujan_classic_bs`
- `bbp_hex_extract`

That made `total_cost_ms = wall_ms + verify_ms` double-count verification for
those rows. The implementations now subtract their measured verification block
from the elapsed wall timer before writing `wall_ms`, and the benchmark helpers
fallback to `wall_ms + verify_ms + io_ms`.

## Verification

The current `results/benchmark.csv` has 90 rows. A CSV invariant pass found zero
supported-row violations for:

```text
abs(total_cost_ms - (wall_ms + verify_ms + io_ms)) <= 0.002
```

The summary tables and SVG figures were regenerated after normalization.

## Current 1M reference points

| Algorithm | wall_ms | verify_ms | total_cost_ms | relative_wall_time |
|---|---:|---:|---:|---:|
| `chudnovsky_bs` | 307.415 | 16.892 | 324.307 | 1.000 |
| `chudnovsky_bs_crown` | 83.714 | 18.822 | 102.536 | 0.272 |
| `chudnovsky_bs_crown_tuned` | 69.191 | 17.216 | 86.407 | 0.225 |
| `arb_const_pi` | 80.294 | 18.529 | 98.823 | 0.261 |
| `chudnovsky_hybrid` | 78.500 | 17.313 | 95.813 | 0.255 |
| `mpfr_const_pi` | 443.928 | 17.094 | 461.022 | 1.444 |

## Files touched

| Area | Files |
|---|---|
| Timer boundaries | `src/chudnovsky.cpp`, `src/chudnovsky_baselines.cpp`, `src/chudnovsky_crown.cpp`, `src/ramanujan.cpp`, `src/bbp_algorithm.cpp` |
| Derived metrics | `src/benchmark.cpp`, `tools/merge_benchmark_csv.py` |
| Data and docs | `results/benchmark.csv`, `results/summary.md`, `results/efficiency.md`, `docs/methods-comparison.md`, `docs/research-log.md` |
| Figures | `docs/figures/*.svg`, `docs/figures/index.md` |
