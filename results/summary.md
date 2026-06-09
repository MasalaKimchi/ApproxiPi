# SATO-X Benchmark Summary

Guard digits: `25`

Trials per row: `5`; warmups: `1`

Optimization notes: shared binary splitting uses bounded parallel subtree evaluation, an `mpz_addmul` merge to avoid one temporary large-integer product per internal node, small 8-term leaf blocks before recursion, opt-in leaf valuation cancellation, and `log10(396^4 / 256)` for Ramanujan term-count estimation. Phase columns expose split/finalize/format/verify bottlenecks.

| Digits | Algorithm | Supported | Verified | Median wall ms | Split | Finalize | Format | Verify | Terms/iterations | Max operand bits | Mul Gbit | Parallel depth | Relative to Chudnovsky | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `chudnovsky_bs` | yes | yes | 0.045 | 0.031 | 0.005 | 0.008 | 0.007 | 83 | 5616 | 0.000 | 0 | 1.000 |  |
| 1000 | `chudnovsky_bs_valuation` | yes | yes | 0.048 | 0.035 | 0.005 | 0.007 | 0.007 | 83 | 5328 | 0.000 | 0 | 1.075 |  |
| 1000 | `chudnovsky_bs_crown` | yes | yes | 0.048 | 0.035 | 0.005 | 0.007 | 0.007 | 83 | 5328 | 0.000 | 0 | 1.061 |  |
| 1000 | `chudnovsky_bs_crown_tuned` | yes | yes | 0.045 | 0.033 | 0.005 | 0.006 | 0.006 | 83 | 5328 | 0.000 | 0 | 1.004 |  |
| 1000 | `ramanujan_classic_bs` | yes | yes | 0.067 | 0.054 | 0.005 | 0.006 | 0.006 | 153 | 8870 | 0.000 | 0 | 1.479 |  |
| 1000 | `machin_arctan` | yes | yes | 0.262 | 0.000 | 0.254 | 0.008 | 0.007 | 825 | 0 | 0.000 | 0 | 5.811 |  |
| 1000 | `gauss_legendre_agm` | yes | yes | 0.095 | 0.000 | 0.088 | 0.013 | 0.013 | 13 | 0 | 0.000 | 0 | 2.107 |  |
| 1000 | `borwein_cubic` | yes | yes | 0.120 | 0.000 | 0.113 | 0.007 | 0.007 | 13 | 0 | 0.000 | 0 | 2.666 |  |
| 1000 | `borwein_quartic` | yes | yes | 0.137 | 0.000 | 0.129 | 0.014 | 0.013 | 7 | 0 | 0.000 | 0 | 3.028 |  |
| 1000 | `mpfr_const_pi` | yes | yes | 0.033 | 0.000 | 0.026 | 0.006 | 0.007 | 1 | 0 | 0.000 | 0 | 0.727 |  |
| 1000 | `arb_const_pi` | yes | yes | 0.009 | 0.000 | 0.001 | 0.007 | 0.033 | 1 | 0 | 0.000 | 0 | 0.192 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 0.621 | 0.381 | 0.117 | 0.121 | 0.124 | 717 | 55463 | 0.002 | 0 | 1.000 |  |
| 10000 | `chudnovsky_bs_valuation` | yes | yes | 0.642 | 0.415 | 0.111 | 0.114 | 0.115 | 717 | 52905 | 0.002 | 0 | 1.034 |  |
| 10000 | `chudnovsky_bs_crown` | yes | yes | 0.568 | 0.349 | 0.108 | 0.109 | 0.109 | 717 | 27585 | 0.002 | 1 | 0.913 |  |
| 10000 | `chudnovsky_bs_crown_tuned` | yes | yes | 0.538 | 0.330 | 0.102 | 0.105 | 0.105 | 717 | 27585 | 0.002 | 1 | 0.865 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 0.963 | 0.752 | 0.103 | 0.105 | 0.105 | 1280 | 89681 | 0.003 | 0 | 1.550 |  |
| 10000 | `machin_arctan` | yes | yes | 5.554 | 0.000 | 5.457 | 0.097 | 0.096 | 7263 | 0 | 0.000 | 0 | 8.937 |  |
| 10000 | `gauss_legendre_agm` | yes | yes | 1.724 | 0.000 | 1.628 | 0.191 | 0.189 | 16 | 0 | 0.000 | 0 | 2.774 |  |
| 10000 | `borwein_cubic` | yes | yes | 2.415 | 0.000 | 2.320 | 0.096 | 0.096 | 15 | 0 | 0.000 | 0 | 3.887 |  |
| 10000 | `borwein_quartic` | yes | yes | 2.418 | 0.000 | 2.322 | 0.188 | 0.190 | 8 | 0 | 0.000 | 0 | 3.892 |  |
| 10000 | `mpfr_const_pi` | yes | yes | 0.740 | 0.000 | 0.645 | 0.095 | 0.095 | 1 | 0 | 0.000 | 0 | 1.191 |  |
| 10000 | `arb_const_pi` | yes | yes | 0.966 | 0.000 | 0.874 | 0.092 | 0.698 | 1 | 0 | 0.000 | 0 | 1.554 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 12.106 | 7.154 | 2.515 | 2.604 | 2.558 | 7064 | 616721 | 0.027 | 0 | 1.000 |  |
| 100000 | `chudnovsky_bs_valuation` | yes | yes | 12.566 | 7.326 | 2.626 | 2.587 | 2.536 | 7064 | 591473 | 0.025 | 0 | 1.038 |  |
| 100000 | `chudnovsky_bs_crown` | yes | yes | 4.467 | 2.820 | 0.886 | 0.748 | 2.416 | 7064 | 38872 | 0.025 | 4 | 0.369 |  |
| 100000 | `chudnovsky_bs_crown_tuned` | yes | yes | 4.170 | 2.320 | 1.067 | 0.763 | 2.470 | 7064 | 38872 | 0.025 | 4 | 0.344 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 15.086 | 9.890 | 2.579 | 2.544 | 2.515 | 12555 | 1044752 | 0.053 | 1 | 1.246 |  |
| 100000 | `machin_arctan` | yes | yes | 171.937 | 0.000 | 169.308 | 2.574 | 2.445 | 71644 | 0 | 0.000 | 0 | 14.202 |  |
| 100000 | `gauss_legendre_agm` | yes | yes | 50.252 | 0.000 | 47.802 | 5.050 | 5.030 | 19 | 0 | 0.000 | 0 | 4.151 |  |
| 100000 | `borwein_cubic` | yes | yes | 68.971 | 0.000 | 66.505 | 2.593 | 2.526 | 17 | 0 | 0.000 | 0 | 5.697 |  |
| 100000 | `borwein_quartic` | yes | yes | 76.286 | 0.000 | 73.675 | 5.048 | 5.064 | 10 | 0 | 0.000 | 0 | 6.301 |  |
| 100000 | `mpfr_const_pi` | yes | yes | 25.809 | 0.000 | 23.252 | 2.557 | 2.533 | 1 | 0 | 0.000 | 0 | 2.132 |  |
| 100000 | `arb_const_pi` | yes | yes | 8.810 | 0.000 | 6.405 | 2.403 | 24.924 | 1 | 0 | 0.000 | 0 | 0.728 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 140.500 | 53.451 | 37.670 | 49.558 | 48.593 | 70526 | 6860005 | 0.376 | 4 | 1.000 |  |
| 1000000 | `chudnovsky_bs_valuation` | yes | yes | 139.206 | 51.927 | 38.281 | 49.089 | 48.724 | 70526 | 6607956 | 0.359 | 4 | 0.991 |  |
| 1000000 | `chudnovsky_bs_crown` | yes | yes | 59.306 | 29.462 | 16.206 | 13.348 | 49.366 | 70526 | 54002 | 0.350 | 7 | 0.422 |  |
| 1000000 | `chudnovsky_bs_crown_tuned` | yes | yes | 59.253 | 29.542 | 16.117 | 13.326 | 49.753 | 70526 | 27048 | 0.350 | 8 | 0.422 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 202.400 | 116.382 | 37.847 | 48.819 | 49.356 | 125301 | 12089942 | 0.794 | 4 | 1.441 |  |
| 1000000 | `machin_arctan` | no | no | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0 | 0 | 0.000 | 0 | 0.000 | requested precision exceeds algorithm max_digits |
| 1000000 | `gauss_legendre_agm` | no | no | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0 | 0 | 0.000 | 0 | 0.000 | requested precision exceeds algorithm max_digits |
| 1000000 | `borwein_cubic` | yes | yes | 1135.204 | 0.000 | 1086.223 | 49.226 | 49.025 | 19 | 0 | 0.000 | 0 | 8.080 |  |
| 1000000 | `borwein_quartic` | yes | yes | 1365.186 | 0.000 | 1316.370 | 98.370 | 98.366 | 11 | 0 | 0.000 | 0 | 9.717 |  |
| 1000000 | `mpfr_const_pi` | yes | yes | 458.248 | 0.000 | 409.377 | 48.867 | 48.885 | 1 | 0 | 0.000 | 0 | 3.262 |  |
| 1000000 | `arb_const_pi` | yes | yes | 99.042 | 0.000 | 49.807 | 49.276 | 462.044 | 1 | 0 | 0.000 | 0 | 0.705 |  |

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
