# SATO-X Benchmark Summary

Guard digits: `25`

Trials per row: `3`; warmups: `0`

Optimization notes: shared binary splitting uses bounded parallel subtree evaluation, an `mpz_addmul` merge to avoid one temporary large-integer product per internal node, small 8-term leaf blocks before recursion, and `log10(396^4 / 256)` for Ramanujan term-count estimation. Phase columns expose split/finalize/format/verify bottlenecks.

| Digits | Algorithm | Supported | Verified | Median wall ms | Split | Finalize | Format | Verify | Terms/iterations | Max operand bits | Parallel depth | Relative to Chudnovsky | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `chudnovsky_bs` | yes | yes | 0.059 | 0.040 | 0.008 | 0.010 | 0.008 | 83 | 5616 | 0 | 1.000 |  |
| 1000 | `ramanujan_classic_bs` | yes | yes | 0.079 | 0.062 | 0.006 | 0.009 | 0.007 | 153 | 8870 | 0 | 1.328 |  |
| 1000 | `gauss_legendre_agm` | yes | yes | 0.102 | 0.000 | 0.095 | 0.013 | 0.013 | 13 | 0 | 0 | 1.716 |  |
| 1000 | `borwein_quartic` | yes | yes | 0.138 | 0.000 | 0.131 | 0.013 | 0.013 | 7 | 0 | 0 | 2.320 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 0.713 | 0.445 | 0.131 | 0.132 | 0.130 | 717 | 55463 | 0 | 1.000 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 1.162 | 0.901 | 0.128 | 0.126 | 0.124 | 1280 | 89681 | 0 | 1.629 |  |
| 10000 | `gauss_legendre_agm` | yes | yes | 2.271 | 0.000 | 2.143 | 0.248 | 0.240 | 16 | 0 | 0 | 3.185 |  |
| 10000 | `borwein_quartic` | yes | yes | 2.946 | 0.000 | 2.832 | 0.235 | 0.227 | 8 | 0 | 0 | 4.133 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 13.352 | 7.714 | 2.848 | 2.810 | 2.807 | 7064 | 616721 | 0 | 1.000 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 16.285 | 10.938 | 2.756 | 2.681 | 2.629 | 12555 | 1044752 | 1 | 1.220 |  |
| 100000 | `gauss_legendre_agm` | yes | yes | 56.177 | 0.000 | 53.492 | 5.496 | 5.662 | 19 | 0 | 0 | 4.207 |  |
| 100000 | `borwein_quartic` | yes | yes | 83.308 | 0.000 | 80.622 | 5.553 | 5.413 | 10 | 0 | 0 | 6.239 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 145.798 | 55.345 | 38.918 | 51.525 | 50.687 | 70526 | 6860005 | 4 | 1.000 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 210.999 | 120.850 | 38.848 | 50.871 | 50.663 | 125301 | 12089942 | 4 | 1.447 |  |
| 1000000 | `gauss_legendre_agm` | no | no | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0 | 0 | 0 | 0.000 | requested precision exceeds algorithm max_digits |
| 1000000 | `borwein_quartic` | yes | yes | 1408.448 | 0.000 | 1357.757 | 101.529 | 101.435 | 11 | 0 | 0 | 9.660 |  |

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
