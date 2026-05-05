# SATO-X Benchmark Summary

Guard digits: `25`

Trials per row: `3`; warmups: `0`

Optimization notes: shared binary splitting uses bounded parallel subtree evaluation, an `mpz_addmul` merge to avoid one temporary large-integer product per internal node, small 8-term leaf blocks before recursion, opt-in leaf valuation cancellation, and `log10(396^4 / 256)` for Ramanujan term-count estimation. Phase columns expose split/finalize/format/verify bottlenecks.

| Digits | Algorithm | Supported | Verified | Median wall ms | Split | Finalize | Format | Verify | Terms/iterations | Max operand bits | Parallel depth | Relative to Chudnovsky | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `chudnovsky_bs` | yes | yes | 0.048 | 0.029 | 0.006 | 0.007 | 0.006 | 83 | 5616 | 0 | 1.000 |  |
| 1000 | `chudnovsky_bs_valuation` | yes | yes | 0.043 | 0.030 | 0.004 | 0.007 | 0.005 | 83 | 5328 | 0 | 0.892 |  |
| 1000 | `ramanujan_classic_bs` | yes | yes | 0.055 | 0.045 | 0.004 | 0.005 | 0.005 | 153 | 8870 | 0 | 1.149 |  |
| 1000 | `machin_arctan` | yes | yes | 0.184 | 0.000 | 0.178 | 0.005 | 0.005 | 825 | 0 | 0 | 3.832 |  |
| 1000 | `gauss_legendre_agm` | yes | yes | 0.076 | 0.000 | 0.071 | 0.010 | 0.010 | 13 | 0 | 0 | 1.581 |  |
| 1000 | `borwein_cubic` | yes | yes | 0.098 | 0.000 | 0.092 | 0.005 | 0.005 | 13 | 0 | 0 | 2.038 |  |
| 1000 | `borwein_quartic` | yes | yes | 0.109 | 0.000 | 0.104 | 0.010 | 0.010 | 7 | 0 | 0 | 2.271 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 0.668 | 0.384 | 0.144 | 0.138 | 0.125 | 717 | 55463 | 0 | 1.000 |  |
| 10000 | `chudnovsky_bs_valuation` | yes | yes | 0.684 | 0.428 | 0.128 | 0.119 | 0.113 | 717 | 52905 | 0 | 1.024 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 0.935 | 0.726 | 0.105 | 0.114 | 0.106 | 1280 | 89681 | 0 | 1.399 |  |
| 10000 | `machin_arctan` | yes | yes | 8.151 | 0.000 | 8.025 | 0.126 | 0.104 | 7263 | 0 | 0 | 12.196 |  |
| 10000 | `gauss_legendre_agm` | yes | yes | 1.984 | 0.000 | 1.872 | 0.218 | 0.207 | 16 | 0 | 0 | 2.969 |  |
| 10000 | `borwein_cubic` | yes | yes | 2.884 | 0.000 | 2.768 | 0.119 | 0.106 | 15 | 0 | 0 | 4.316 |  |
| 10000 | `borwein_quartic` | yes | yes | 2.900 | 0.000 | 2.781 | 0.234 | 0.262 | 8 | 0 | 0 | 4.339 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 17.858 | 10.549 | 3.532 | 3.549 | 3.370 | 7064 | 616721 | 0 | 1.000 |  |
| 100000 | `chudnovsky_bs_valuation` | yes | yes | 16.361 | 10.280 | 3.089 | 2.987 | 3.034 | 7064 | 591473 | 0 | 0.916 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 19.733 | 12.890 | 3.765 | 3.118 | 4.210 | 12555 | 1044752 | 1 | 1.105 |  |
| 100000 | `machin_arctan` | yes | yes | 214.279 | 0.000 | 211.476 | 2.821 | 4.066 | 71644 | 0 | 0 | 11.999 |  |
| 100000 | `gauss_legendre_agm` | yes | yes | 63.550 | 0.000 | 59.452 | 6.593 | 7.174 | 19 | 0 | 0 | 3.559 |  |
| 100000 | `borwein_cubic` | yes | yes | 86.573 | 0.000 | 83.595 | 3.301 | 3.376 | 17 | 0 | 0 | 4.848 |  |
| 100000 | `borwein_quartic` | yes | yes | 96.679 | 0.000 | 92.970 | 6.559 | 6.567 | 10 | 0 | 0 | 5.414 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 171.897 | 67.275 | 45.569 | 59.925 | 59.705 | 70526 | 6860005 | 4 | 1.000 |  |
| 1000000 | `chudnovsky_bs_valuation` | yes | yes | 170.637 | 64.377 | 45.474 | 59.421 | 60.188 | 70526 | 6607956 | 4 | 0.993 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 259.269 | 151.025 | 47.386 | 61.315 | 58.681 | 125301 | 12089942 | 4 | 1.508 |  |
| 1000000 | `machin_arctan` | no | no | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0 | 0 | 0 | 0.000 | requested precision exceeds algorithm max_digits |
| 1000000 | `gauss_legendre_agm` | no | no | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0 | 0 | 0 | 0.000 | requested precision exceeds algorithm max_digits |
| 1000000 | `borwein_cubic` | yes | yes | 1312.290 | 0.000 | 1255.353 | 56.609 | 56.238 | 19 | 0 | 0 | 7.634 |  |
| 1000000 | `borwein_quartic` | yes | yes | 1641.934 | 0.000 | 1583.047 | 118.083 | 118.401 | 11 | 0 | 0 | 9.552 |  |

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
