# SATO-X Engineering Benchmark Summary

Guard digits: `25`

Canonical data: `results/benchmark.csv` (merged from incremental runs).

Cost model: T_series + T_bigint + T_sqrt/div + T_radix + T_verify + T_I/O. Efficiency = (seconds * watts * bytes moved) / verified digits.

| Digits | Algorithm | Supported | Verified | Runtime ms | Peak RAM MiB | R/W GB | Energy J | Digits/sec | Digits/J | Relative | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `bbp_hex_extract` | yes | yes | 0.039 | 2.609 | 0.0000 | 0.000 | 25317096.635 | 0.000 | 0.218061 |  |
| 1000 | `chudnovsky_bs` | yes | yes | 0.127 | 2.609 | 0.0000 | 0.000 | 7845721.728 | 0.000 | 1.000000 | no_residues |
| 1000 | `chudnovsky_naive` | yes | yes | 0.396 | 2.500 | 0.0000 | 0.000 | 2523926.826 | 0.000 | 2.936819 | no_residues |
| 1000 | `chudnovsky_recurrence` | yes | yes | 0.241 | 2.531 | 0.0000 | 0.000 | 4148671.803 | 0.000 | 1.932005 | no_residues |
| 10000 | `arb_const_pi` | yes | yes | 1.690 | 7.562 | 0.0000 | 0.000 | 5917159.763 | 0.000 | 0.920339 |  |
| 10000 | `bbp_hex_extract` | yes | yes | 0.130 | 3.453 | 0.0000 | 0.000 | 76701821.668 | 0.000 | 0.079827 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 3.179 | 3.281 | 0.0000 | 0.000 | 3145148.608 | 0.000 | 1.000000 |  |
| 10000 | `chudnovsky_bs_crown` | yes | yes | 1.366 | 3.312 | 0.0000 | 0.000 | 7319529.676 | 0.000 | 0.466636 |  |
| 10000 | `chudnovsky_bs_valuation` | yes | yes | 2.750 | 3.281 | 0.0000 | 0.000 | 3635978.884 | 0.000 | 0.980682 |  |
| 10000 | `chudnovsky_naive` | yes | yes | 2.712 | 2.984 | 0.0000 | 0.000 | 3687257.171 | 0.000 | 0.831723 |  |
| 10000 | `chudnovsky_recurrence` | yes | yes | 2.664 | 3.031 | 0.0000 | 0.000 | 3753401.520 | 0.000 | 0.850503 |  |
| 10000 | `mpfr_const_pi` | yes | yes | 0.840 | 3.484 | 0.0000 | 0.000 | 11911242.188 | 0.000 | 0.493803 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 2.430 | 3.453 | 0.0000 | 0.000 | 4115930.958 | 0.000 | 0.772264 |  |
| 100000 | `arb_const_pi` | yes | yes | 35.454 | 17.797 | 0.0000 | 0.000 | 2820519.778 | 0.000 | 0.300544 |  |
| 100000 | `bbp_hex_extract` | yes | yes | 1.221 | 13.078 | 0.0000 | 0.000 | 81877684.565 | 0.000 | 0.039100 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 65.658 | 10.297 | 0.0000 | 0.000 | 1523037.851 | 0.000 | 1.000000 |  |
| 100000 | `chudnovsky_bs_crown` | yes | yes | 39.368 | 12.422 | 0.0000 | 0.000 | 2540158.380 | 0.000 | 0.159349 |  |
| 100000 | `chudnovsky_bs_valuation` | yes | yes | 65.924 | 10.500 | 0.0000 | 0.000 | 1516886.765 | 0.000 | 1.007816 |  |
| 100000 | `chudnovsky_naive` | yes | yes | 112.135 | 9.969 | 0.0000 | 0.000 | 891782.879 | 0.000 | 2.513118 |  |
| 100000 | `chudnovsky_recurrence` | no | no | 0.000 | 9.969 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 100000 | `mpfr_const_pi` | yes | yes | 28.685 | 13.078 | 0.0000 | 0.000 | 3486152.792 | 0.000 | 0.840400 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 51.554 | 13.078 | 0.0000 | 0.000 | 1939723.142 | 0.000 | 0.543488 |  |
| 1000000 | `arb_const_pi` | yes | yes | 570.622 | 89.859 | 0.0000 | 0.000 | 1752474.897 | 0.000 | 0.358826 |  |
| 1000000 | `bbp_hex_extract` | yes | yes | 16.243 | 61.328 | 0.0000 | 0.000 | 61563875.106 | 0.000 | 0.057133 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 848.605 | 32.609 | 0.0000 | 0.000 | 1178404.441 | 0.000 | 1.000000 |  |
| 1000000 | `chudnovsky_bs_crown` | yes | yes | 652.389 | 40.953 | 0.0000 | 0.000 | 1532827.022 | 0.000 | 0.000000 |  |
| 1000000 | `chudnovsky_bs_valuation` | yes | yes | 848.780 | 36.219 | 0.0000 | 0.000 | 1178162.117 | 0.000 | 1.092667 |  |
| 1000000 | `chudnovsky_naive` | no | no | 0.000 | 17.703 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | naive summation capped at 100000 digits |
| 1000000 | `chudnovsky_recurrence` | no | no | 0.000 | 17.703 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 1000000 | `mpfr_const_pi` | yes | yes | 520.396 | 62.266 | 0.0000 | 0.000 | 1921615.232 | 0.000 | 1.653785 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 776.240 | 61.297 | 0.0000 | 0.000 | 1288261.085 | 0.000 | 0.740171 |  |
| 10000000 | `chudnovsky_bs` | yes | yes | 15695.870 | 262.922 | 0.0100 | 0.000 | 637110.255 | 0.000 | 1.000000 |  |
| 10000000 | `chudnovsky_bs_crown` | yes | yes | 3497.404 | 439.047 | 0.0100 | 0.000 | 2859263.682 | 0.000 | 0.235989 |  |
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
