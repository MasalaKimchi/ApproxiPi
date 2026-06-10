# SATO-X Engineering Benchmark Protocol

## Objective

Minimize the full pipeline cost:

```
Cost(D) = T_series + T_bigint + T_sqrt/div + T_radix + T_verify + T_I/O
```

Report engineering efficiency:

```
efficiency = (seconds × watts × bytes moved) / verified digits
```

Also publish: digits/sec, digits/joule, digits/GB moved, verified digits/$.

## Digit ladder

Publishable runs use `10^5, 10^6, 10^7, 10^8` decimal digits. Smoke tests may use `10^3` or `10^4`.

## Baseline layers

| Layer | Algorithm key | Role |
|---|---|---|
| 1 | `chudnovsky_naive` | Term-by-term summation cliff (capped at 10^5) |
| 2 | `chudnovsky_recurrence` | Blocked recurrence without binary tree (capped at 10^4) |
| 2 | `chudnovsky_bs` | Serious Chudnovsky binary-splitting baseline |
| 3 | `ramanujan_classic_bs` | Convergence comparison |

SATO-X stack: `chudnovsky_bs_valuation`, `chudnovsky_bs_crown`, `chudnovsky_bs_crown_tuned`.

## BBP verification (not benchmarked)

The Bailey–Borwein–Plouffe hex-digit extractor (`satox/bbp.hpp`) publishes known spot
checks in `results/summary.md`. It is **not** included in default benchmark runs:
sparse hex extraction is not comparable to full-prefix π computation on digits/sec or
relative wall time.

## Per-run logging

`bin/satox-bench` writes under `results/` (single canonical directory):

- `results/benchmark.csv` — median aggregates + derived metrics
- `results/trials.csv` — per-trial raw rows (appended when using `--merge`)
- `results/run-manifest.json` — host metadata
- `results/summary.md` — human table
- `results/efficiency.md` — publishable efficiency table (`make figures`)

Incremental runs (e.g. scale-out to 10^7/10^8 or ablations) use `--merge` to fold
new rows into the existing `benchmark.csv` instead of writing separate subfolders:

```bash
bin/satox-bench --digits 10000000 --algorithms chudnovsky_bs,chudnovsky_bs_crown \
  --out results --merge --skip-memory-guard
python3 tools/merge_benchmark_csv.py results/benchmark.csv fragment.csv \
  --output results/benchmark.csv --summary results/summary.md
```

Logged fields include wall/CPU time, Cost(D) sub-phases, peak RSS, bytes read/written, energy (when available), and verification outcome.

## Trials and timeouts

Default: `--trials 3`, `--warmups 0`. Naive/recurrence use `--timeout-sec` to avoid hung runs.

## Memory guard

By default, runs abort when estimated peak RAM exceeds 80% of system memory. Override with `--skip-memory-guard` for controlled large-scale experiments.

## Energy

Linux: Intel RAPL via `/sys/class/powercap` when present. macOS: document `powermetrics` (sudo) for external sampling; in-process fallback reports `energy_joules=0`.

Configure cost model:

```bash
bin/satox-bench --electricity-usd-per-kwh 0.12 --instance-usd-per-hour 0.05
```

## Ablations

```bash
bin/satox-bench --ablation no_binary_split --out results --merge   # blocked-leaf only
bin/satox-bench --ablation no_gcd --out results --merge
bin/satox-bench --ablation no_checkpoint
bin/satox-bench --ablation no_residues
bin/satox-bench --ablation leaf_block_16
bin/satox-bench --ablation threads_4
bin/satox-bench --ablation storage_file
make ablation
```

## Expected runtime (Apple Silicon, crown)

| Digits | Crown (approx.) |
|---:|---|
| 10^5 | sub-second |
| 10^6 | ~1 minute |
| 10^7 | tens of minutes |
| 10^8 | ~2.6 min crown compute on Apple Silicon; sampled 1M-digit prefix verify |

## Reproduce

Build artifacts (`bin/`, `build/`) are gitignored; always compile before running.

```bash
make
make test
bin/satox-bench --digits 100000,1000000,10000000,100000000 --guard 25 --trials 2 --skip-memory-guard
make figures
```
