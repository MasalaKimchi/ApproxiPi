# Benchmark timing fairness (open issue)

**Status:** open — must be fixed before publishing cross-family wall-time claims  
**Tracked:** 2026-06-10  
**Related:** H13 pre-format verification (`docs/research-log.md`)

## Summary

`wall_ms` and `total_cost_ms` do not measure the same pipeline boundary across
algorithms. SATO-X implementations (Chudnovsky, Ramanujan, Crown) stop the outer
timer **after** verification; external baselines (MPFR `const_pi`, FLINT/Arb) stop
**before** verification. `total_cost_ms` then **double-counts** verify for SATO-X
because it is defined as `wall_ms + verify_ms` everywhere.

Cross-family charts (`wall_time_log.svg`, `relative_wall_time.svg`,
`hypothesis_progression.svg`) and prose in `docs/methods-comparison.md` that
compare crown against Arb/MPFR on `wall_ms` are therefore skewed in favor of
external baselines.

## Evidence

### Asymmetric timer boundaries

SATO-X (`chudnovsky_common.cpp`): verify runs inside the outer timer; `wall_ms`
is read after format.

```cpp
out.verified = verify_unscaled_pi_mpfr(pi, ...);
// format ...
result.wall_ms = timer.wall_ms();          // includes verify
result.total_cost_ms = result.wall_ms + result.verify_ms;  // verify counted twice
```

External baselines (`external_baselines.cpp`): outer timer stops after format;
verify is timed separately.

```cpp
result.wall_ms = timer.wall_ms();          // excludes verify
result.verify_ms = verify_timer.wall_ms();
// total_cost_ms filled later as wall_ms + verify_ms (correct for this path)
```

### Measured impact (Apple Silicon, H13 pre-format verify, 100k digits, 1 trial)

| Algorithm        | `wall_ms` | `verify_ms` | `total_cost_ms` | Verify in `wall_ms`? | `total_cost` correct? |
|------------------|----------:|------------:|----------------:|----------------------|-----------------------|
| `chudnovsky_bs`  |      64.7 |        28.8 |            93.5 | yes                  | no (double-count)     |
| `arb_const_pi`   |      12.5 |        29.1 |            41.6 | no                   | yes                   |

At 1M digits in `results/benchmark.csv` (pre-H13 post-format verify), Arb
`wall_ms` ≈ 102 ms while `verify_ms` ≈ 471 ms — verify is invisible on wall-time
charts but dominates true end-to-end cost.

### Within-family comparisons

Crown vs `chudnovsky_bs` vs Ramanujan on `wall_ms` is **mostly fair** when all
use the same pre-format scaled-integer gate (H13): same timer boundary and same
`kSampleVerifyCap` (1M digits).

## What is *not* the problem

- **Verification rigor:** All rows still pass the same prefix hash; correctness
  is not in question.
- **H13 pre-format verify:** Making verify cheaper is a legitimate optimization;
  the bug is stopwatch placement, not the verify algorithm itself.
- **Separate `verify_ms` column:** Reporting verify as its own phase is correct;
  the bug is inconsistent inclusion in `wall_ms` and double-counting in
  `total_cost_ms`.

## Recommended fix

Pick one contract and apply it to **every** `PiAlgorithm::compute()`:

### Option A — verified pipeline (preferred for Cost(D))

1. Outer timer: split → finalize → format → **stop**.
2. Verify in a separate timed block → `verify_ms`.
3. `wall_ms` = compute + format only (excludes verify).
4. `total_cost_ms` = `wall_ms + verify_ms` (verify counted once).

### Option B — compute race

1. Same as A, but charts and `relative_wall_time` use `wall_ms` only.
2. Document that verify is mandatory but excluded from race metrics.
3. Publish `total_cost_ms` alongside for full-pipeline readers.

### Additional normalization

- [ ] Use the **same verify entry point** for all algorithms (pre-format
      scaled-integer with `kSampleVerifyCap`, or post-format decimal — but not
      both).
- [ ] Add a unit test that asserts `wall_ms + verify_ms ≈ total_cost_ms` and
      `wall_ms` does not include verify (within tolerance).
- [ ] Re-run the publish ladder and regenerate figures after the fix.

## Files to touch

| Area | Files |
|------|-------|
| Timer boundaries | `src/chudnovsky_common.cpp`, `src/chudnovsky.cpp`, `src/chudnovsky_crown.cpp`, `src/ramanujan.cpp`, `src/chudnovsky_baselines.cpp` |
| Already correct pattern | `src/external_baselines.cpp`, `src/machin.cpp`, `src/agm.cpp`, `src/borwein.cpp` |
| Derived metrics | `src/benchmark.cpp` (`fill_derived_metrics`, `stats_to_csv`) |
| Docs | `docs/benchmark-protocol.md`, `docs/methods-comparison.md`, `docs/research-log.md` |
| Figures | `tools/make_figures.py` (if switching default column to `total_cost_ms`) |

## Acceptance checklist

Use this list before closing the issue:

- [ ] Every algorithm stops the outer timer at the same pipeline point.
- [ ] `total_cost_ms = wall_ms + verify_ms` with verify counted **once**.
- [ ] `fill_derived_metrics` does not add verify when it is already in `wall_ms`.
- [ ] Test asserts timer-boundary invariant on at least one SATO-X and one
      external row.
- [ ] Full benchmark ladder re-run; `results/benchmark.csv` regenerated.
- [ ] `make figures` regenerated; cross-family chart captions updated.
- [ ] `docs/methods-comparison.md` no longer claims external baselines use the
      "same" timed verify pipeline unless that is literally true.
- [ ] This file's **Status** updated to `resolved` with the commit hash.

## References

- Benchmark protocol Cost(D): `docs/benchmark-protocol.md`
- H13 hypothesis (pre-format verify): `docs/research-log.md`
- Verify implementation: `src/verification.cpp` (`kSampleVerifyCap = 1_000_000`)
