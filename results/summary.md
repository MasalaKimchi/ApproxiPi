# SATO-X Benchmark Summary

Guard digits: `25`

Trials per row: `3`; warmups: `0`

| Digits | Algorithm | Supported | Verified | Median wall ms | Min | Max | Stddev | Terms/iterations | GCD reductions | Cancelled bits | Relative to Chudnovsky | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `chudnovsky_bs` | yes | yes | 0.082 | 0.049 | 0.666 | 0.348 | 83 | 0 | 0.000 | 1.000 |  |
| 1000 | `ramanujan_classic_bs` | yes | yes | 0.136 | 0.073 | 0.146 | 0.039 | 153 | 0 | 0.000 | 1.672 |  |
| 1000 | `gauss_legendre_agm` | yes | yes | 0.072 | 0.070 | 0.080 | 0.006 | 13 | 0 | 0.000 | 0.881 |  |
| 1000 | `borwein_quartic` | yes | yes | 0.106 | 0.104 | 0.158 | 0.031 | 7 | 0 | 0.000 | 1.298 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 1.327 | 0.672 | 1.661 | 0.504 | 717 | 0 | 0.000 | 1.000 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 2.166 | 1.061 | 2.458 | 0.737 | 1278 | 0 | 0.000 | 1.632 |  |
| 10000 | `gauss_legendre_agm` | yes | yes | 1.863 | 1.820 | 1.994 | 0.091 | 16 | 0 | 0.000 | 1.403 |  |
| 10000 | `borwein_quartic` | yes | yes | 2.698 | 2.564 | 2.899 | 0.168 | 8 | 0 | 0.000 | 2.033 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 31.682 | 14.471 | 32.217 | 10.095 | 7064 | 0 | 0.000 | 1.000 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 23.159 | 23.005 | 49.950 | 15.513 | 12528 | 0 | 0.000 | 0.731 |  |
| 100000 | `gauss_legendre_agm` | yes | yes | 52.284 | 52.020 | 52.391 | 0.191 | 19 | 0 | 0.000 | 1.650 |  |
| 100000 | `borwein_quartic` | yes | yes | 78.443 | 78.047 | 78.607 | 0.288 | 10 | 0 | 0.000 | 2.476 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 250.228 | 248.733 | 693.194 | 256.179 | 70526 | 0 | 0.000 | 1.000 |  |
| 1000000 | `ramanujan_classic_bs` | no | no | 0.000 | 0.000 | 0.000 | 0.000 | 0 | 0 | 0.000 | 0.000 | requested precision exceeds algorithm max_digits |
| 1000000 | `gauss_legendre_agm` | no | no | 0.000 | 0.000 | 0.000 | 0.000 | 0 | 0 | 0.000 | 0.000 | requested precision exceeds algorithm max_digits |
| 1000000 | `borwein_quartic` | yes | yes | 1402.108 | 1398.826 | 1422.512 | 12.833 | 11 | 0 | 0.000 | 5.603 |  |

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
