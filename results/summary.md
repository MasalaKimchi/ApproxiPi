# SATO-X Benchmark Summary

Guard digits: `25`

Trials per row: `3`; warmups: `0`

Optimization notes: shared binary splitting uses an `mpz_addmul` merge to avoid one temporary large-integer product per internal node, folds small 8-term leaf blocks before recursion, and Ramanujan uses `log10(396^4 / 256)` for term-count estimation.

| Digits | Algorithm | Supported | Verified | Median wall ms | Min | Max | Stddev | Terms/iterations | GCD reductions | Cancelled bits | Relative to Chudnovsky | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `chudnovsky_bs` | yes | yes | 0.058 | 0.040 | 1.778 | 0.998 | 83 | 0 | 0.000 | 1.000 |  |
| 1000 | `ramanujan_classic_bs` | yes | yes | 0.057 | 0.055 | 0.073 | 0.010 | 153 | 0 | 0.000 | 0.976 |  |
| 1000 | `gauss_legendre_agm` | yes | yes | 0.212 | 0.095 | 0.221 | 0.071 | 13 | 0 | 0.000 | 3.634 |  |
| 1000 | `borwein_quartic` | yes | yes | 0.305 | 0.257 | 0.317 | 0.031 | 7 | 0 | 0.000 | 5.219 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 0.608 | 0.562 | 0.971 | 0.224 | 717 | 0 | 0.000 | 1.000 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 1.023 | 0.991 | 1.053 | 0.031 | 1280 | 0 | 0.000 | 1.683 |  |
| 10000 | `gauss_legendre_agm` | yes | yes | 2.544 | 2.518 | 2.717 | 0.108 | 16 | 0 | 0.000 | 4.185 |  |
| 10000 | `borwein_quartic` | yes | yes | 3.241 | 3.214 | 3.362 | 0.079 | 8 | 0 | 0.000 | 5.331 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 12.678 | 12.668 | 14.578 | 1.100 | 7064 | 0 | 0.000 | 1.000 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 22.261 | 21.937 | 22.560 | 0.311 | 12555 | 0 | 0.000 | 1.756 |  |
| 100000 | `gauss_legendre_agm` | yes | yes | 59.377 | 57.148 | 62.166 | 2.514 | 19 | 0 | 0.000 | 4.684 |  |
| 100000 | `borwein_quartic` | yes | yes | 81.829 | 79.713 | 82.020 | 1.280 | 10 | 0 | 0.000 | 6.454 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 241.620 | 239.712 | 243.385 | 1.837 | 70526 | 0 | 0.000 | 1.000 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 433.407 | 428.925 | 445.212 | 8.413 | 125301 | 0 | 0.000 | 1.794 |  |
| 1000000 | `gauss_legendre_agm` | no | no | 0.000 | 0.000 | 0.000 | 0.000 | 0 | 0 | 0.000 | 0.000 | requested precision exceeds algorithm max_digits |
| 1000000 | `borwein_quartic` | yes | yes | 1441.578 | 1428.481 | 1497.352 | 36.573 | 11 | 0 | 0.000 | 5.966 |  |

## BBP Verification Spots

| Hex offset | 8 hex digits |
|---:|---|
| 0 | `243f6a88` |
| 10 | `a308d313` |
| 100 | `29b7c97c` |

## Candidate Formula Metadata

| Candidate | Family | D | Invariant | Proof | Digits/term | Score | Estimated multiplies @100k | Decision |
|---|---|---:|---|---|---:|---:|---:|---|
| `C-163` | Chudnovsky baseline | -163 | `j(-163)` | symbolic_certified | 14.182 | 0.857 | 63468 | hold: score below Chudnovsky replacement threshold |
| `L17-Q` | Ramanujan-Sato metadata seed | -67 | `level-17 eta/class invariant` | metadata_only | 15 | -0.610 | 99345 | hold: not binary-splitting ready |
| `CM-HX` | CM hyperdescent target | -d | `low-height CM class invariant` | metadata_only | 16 | 0.464 | 74076 | hold: needs proof certificate |

## Formula Spec Score Report

Wrote `results/satox-score.md` from `candidates/*.formula`.

No SATO-X candidate is considered faster unless it is benchmarked, verified, and compared against the same Chudnovsky baseline.
