# SATO-X Engineering Benchmark Summary

Guard digits: `25`

Canonical data: `results/benchmark.csv` (merged from incremental runs).

Cost model: T_series + T_bigint + T_sqrt/div + T_radix + T_verify + T_I/O. Efficiency = (seconds * watts * bytes moved) / verified digits.

| Digits | Algorithm | Supported | Verified | Runtime ms | Peak RAM MiB | R/W GB | Energy J | Digits/sec | Digits/J | Relative | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `chudnovsky_bs` | yes | yes | 0.127 | 2.609 | 0.0000 | 0.000 | 7845721.728 | 0.000 | 1.000000 | no_residues |
| 1000 | `chudnovsky_naive` | yes | yes | 0.396 | 2.500 | 0.0000 | 0.000 | 2523926.826 | 0.000 | 2.936819 | no_residues |
| 1000 | `chudnovsky_recurrence` | yes | yes | 0.241 | 2.531 | 0.0000 | 0.000 | 4148671.803 | 0.000 | 1.932005 | no_residues |
| 10000 | `arb_const_pi` | yes | yes | 1.690 | 7.562 | 0.0000 | 0.000 | 5917159.763 | 0.000 | 0.920339 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 3.179 | 3.281 | 0.0000 | 0.000 | 3145148.608 | 0.000 | 1.000000 |  |
| 10000 | `chudnovsky_bs_crown` | yes | yes | 1.366 | 3.312 | 0.0000 | 0.000 | 7319529.676 | 0.000 | 0.466636 |  |
| 10000 | `chudnovsky_bs_valuation` | yes | yes | 2.750 | 3.281 | 0.0000 | 0.000 | 3635978.884 | 0.000 | 0.980682 |  |
| 10000 | `chudnovsky_naive` | yes | yes | 2.712 | 2.984 | 0.0000 | 0.000 | 3687257.171 | 0.000 | 0.831723 |  |
| 10000 | `chudnovsky_recurrence` | yes | yes | 2.664 | 3.031 | 0.0000 | 0.000 | 3753401.520 | 0.000 | 0.850503 |  |
| 10000 | `mpfr_const_pi` | yes | yes | 0.840 | 3.484 | 0.0000 | 0.000 | 11911242.188 | 0.000 | 0.493803 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 2.430 | 3.453 | 0.0000 | 0.000 | 4115930.958 | 0.000 | 0.772264 |  |
| 100000 | `arb_const_pi` | yes | yes | 35.327 | 12.828 | 0.0000 | 0.000 | 2830696.148 | 0.000 | 0.000000 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 36.820 | 5.406 | 0.0000 | 0.000 | 2715949.047 | 0.000 | 1.000000 |  |
| 100000 | `chudnovsky_bs_crown` | yes | yes | 7.283 | 6.172 | 0.0000 | 0.000 | 13730447.157 | 0.000 | 0.000000 |  |
| 100000 | `chudnovsky_bs_valuation` | yes | yes | 36.330 | 5.422 | 0.0000 | 0.000 | 2752523.982 | 0.000 | 0.700662 |  |
| 100000 | `chudnovsky_hybrid` | yes | yes | 7.256 | 13.484 | 0.0000 | 0.000 | 13781696.006 | 0.000 | 0.000000 | delegate=chudnovsky_bs_crown |
| 100000 | `chudnovsky_naive` | yes | yes | 112.135 | 9.969 | 0.0000 | 0.000 | 891782.879 | 0.000 | 2.513118 |  |
| 100000 | `chudnovsky_recurrence` | no | no | 0.000 | 9.969 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 100000 | `mpfr_const_pi` | yes | yes | 28.348 | 7.875 | 0.0000 | 0.000 | 3527554.611 | 0.000 | 0.531330 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 19.710 | 7.641 | 0.0000 | 0.000 | 5073684.614 | 0.000 | 0.376894 |  |
| 1000000 | `arb_const_pi` | yes | yes | 549.899 | 62.453 | 0.0000 | 0.000 | 1818515.489 | 0.000 | 0.000000 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 320.890 | 30.797 | 0.0000 | 0.000 | 3116334.729 | 0.000 | 1.000000 |  |
| 1000000 | `chudnovsky_bs_crown` | yes | yes | 93.433 | 34.812 | 0.0000 | 0.000 | 10702890.041 | 0.000 | 0.000000 |  |
| 1000000 | `chudnovsky_bs_crown_h15` | yes | yes | 92.310 | 42.500 | 0.0000 | 0.000 | 10833116.256 | 0.000 | 0.000000 |  |
| 1000000 | `chudnovsky_bs_valuation` | yes | yes | 318.565 | 30.984 | 0.0000 | 0.000 | 3139077.202 | 0.000 | 0.593952 |  |
| 1000000 | `chudnovsky_hybrid` | yes | yes | 90.365 | 43.656 | 0.0000 | 0.000 | 11066236.538 | 0.000 | 0.000000 | delegate=chudnovsky_bs_crown |
| 1000000 | `chudnovsky_naive` | no | no | 0.000 | 17.703 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | naive summation capped at 100000 digits |
| 1000000 | `chudnovsky_recurrence` | no | no | 0.000 | 17.703 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 1000000 | `mpfr_const_pi` | yes | yes | 501.669 | 53.953 | 0.0000 | 0.000 | 1993345.714 | 0.000 | 0.904617 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 240.279 | 52.797 | 0.0000 | 0.000 | 4161829.979 | 0.000 | 0.438198 |  |
| 10000000 | `arb_const_pi` | yes | yes | 2026.054 | 654.656 | 0.0000 | 0.000 | 4935702.095 | 0.000 | 0.000000 |  |
| 10000000 | `chudnovsky_bs` | yes | yes | 5751.458 | 276.203 | 0.0100 | 0.000 | 1738689.450 | 0.000 | 1.000000 |  |
| 10000000 | `chudnovsky_bs_crown` | yes | yes | 1626.797 | 373.172 | 0.0000 | 0.000 | 6147047.516 | 0.000 | 0.000000 |  |
| 10000000 | `chudnovsky_bs_valuation` | yes | yes | 5521.557 | 284.109 | 0.0300 | 0.000 | 1811083.509 | 0.000 | 0.971864 |  |
| 10000000 | `chudnovsky_hybrid` | no | no | 0 | 0.000 | 0.0000 |  |  |  |  |  |
| 10000000 | `mpfr_const_pi` | yes | yes | 8975.614 | 700.750 | 0.0400 | 0.000 | 1114129.908 | 0.000 | 1.565746 |  |
| 10000000 | `ramanujan_classic_bs` | yes | yes | 4399.893 | 684.922 | 0.0400 | 0.000 | 2272782.500 | 0.000 | 0.758104 |  |
| 100000000 | `chudnovsky_bs` | yes | yes | 93100.155 | 1753.812 | 0.2400 | 0.000 | 1074112.066 | 0.000 |  |  |
| 100000000 | `chudnovsky_bs_crown` | yes | no | 156367.382 | 1851.656 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 |  |

See [`results/efficiency.md`](efficiency.md) for digits/sec, digits/joule, peak RAM, and I/O columns across all methods.

## BBP Verification Spots

| Hex offset | 8 hex digits |
|---:|---|
| 0 | `243f6a88` |
| 10 | `a308d313` |
| 100 | `29b7c97c` |

## Formula Spec Score Report

Wrote `results/satox-score.md` from `candidates/*.formula`.

No SATO-X candidate is considered faster unless it is benchmarked, verified, and compared against the same Chudnovsky baseline.
