# SATO-X Engineering Benchmark Summary

Guard digits: `25`

Trials per row: `2`; warmups: `0`

Cost model: T_series + T_bigint + T_sqrt/div + T_radix + T_verify + T_I/O. Efficiency = (seconds * watts * bytes moved) / verified digits.

| Digits | Algorithm | Supported | Verified | Runtime ms | Peak RAM MiB | R/W GB | Energy J | Digits/sec | Digits/J | Relative | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 100000 | `chudnovsky_bs` | yes | yes | 36.820 | 5.406 | 0.000 | 0.000 | 2715949.047 | 0.000 | 1.000 |  |
| 100000 | `chudnovsky_bs_valuation` | yes | yes | 36.330 | 5.422 | 0.000 | 0.000 | 2752523.982 | 0.000 | 0.701 |  |
| 100000 | `chudnovsky_bs_crown` | yes | yes | 7.997 | 7.062 | 0.000 | 0.000 | 12504232.683 | 0.000 | 0.142 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 19.710 | 7.641 | 0.000 | 0.000 | 5073684.614 | 0.000 | 0.377 |  |
| 100000 | `mpfr_const_pi` | yes | yes | 28.348 | 7.875 | 0.000 | 0.000 | 3527554.611 | 0.000 | 0.531 |  |
| 100000 | `arb_const_pi` | yes | yes | 35.013 | 13.297 | 0.000 | 0.000 | 2856085.453 | 0.000 | 0.207 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 320.890 | 30.797 | 0.000 | 0.000 | 3116334.729 | 0.000 | 1.000 |  |
| 1000000 | `chudnovsky_bs_valuation` | yes | yes | 318.565 | 30.984 | 0.000 | 0.000 | 3139077.202 | 0.000 | 0.594 |  |
| 1000000 | `chudnovsky_bs_crown` | yes | yes | 89.218 | 46.141 | 0.000 | 0.000 | 11208500.527 | 0.000 | 0.156 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 240.279 | 52.797 | 0.000 | 0.000 | 4161829.979 | 0.000 | 0.438 |  |
| 1000000 | `mpfr_const_pi` | yes | yes | 501.669 | 53.953 | 0.000 | 0.000 | 1993345.714 | 0.000 | 0.905 |  |
| 1000000 | `arb_const_pi` | yes | yes | 562.791 | 73.359 | 0.000 | 0.000 | 1776858.026 | 0.000 | 0.196 |  |
| 10000000 | `chudnovsky_bs` | yes | yes | 5751.458 | 276.203 | 0.010 | 0.000 | 1738689.450 | 0.000 | 1.000 |  |
| 10000000 | `chudnovsky_bs_valuation` | yes | yes | 5521.557 | 284.109 | 0.030 | 0.000 | 1811083.509 | 0.000 | 0.972 |  |
| 10000000 | `chudnovsky_bs_crown` | yes | yes | 1610.022 | 431.734 | 0.040 | 0.000 | 6211095.896 | 0.000 | 0.301 |  |
| 10000000 | `ramanujan_classic_bs` | yes | yes | 4399.893 | 684.922 | 0.040 | 0.000 | 2272782.500 | 0.000 | 0.758 |  |
| 10000000 | `mpfr_const_pi` | yes | yes | 8975.614 | 700.750 | 0.040 | 0.000 | 1114129.908 | 0.000 | 1.566 |  |
| 10000000 | `arb_const_pi` | yes | yes | 1911.424 | 678.359 | 0.040 | 0.000 | 5231701.943 | 0.000 | 0.256 |  |
| 100000000 | `chudnovsky_bs` | yes | yes | 93100.155 | 1753.812 | 0.240 | 0.000 | 1074112.066 | 0.000 | 1.000 |  |
| 100000000 | `chudnovsky_bs_valuation` | yes | yes | 93039.940 | 1753.812 | 0.340 | 0.000 | 1074807.231 | 0.000 | 0.985 |  |
| 100000000 | `chudnovsky_bs_crown` | yes | yes | 28239.463 | 2181.031 | 0.440 | 0.000 | 3541143.798 | 0.000 | 0.296 |  |
| 100000000 | `ramanujan_classic_bs` | yes | no | 40486.626 | 3257.766 | 0.440 | 0.000 | 0.000 | 0.000 | 0.418 |  |
| 100000000 | `mpfr_const_pi` | yes | yes | 141005.699 | 1390.203 | 0.440 | 0.000 | 709191.194 | 0.000 | 1.517 |  |
| 100000000 | `arb_const_pi` | yes | yes | 21589.327 | 3575.750 | 0.440 | 0.000 | 4631918.301 | 0.000 | 0.227 |  |

## BBP Verification Spots

| Hex offset | 8 hex digits |
|---:|---|
| 0 | `243f6a88` |
| 10 | `a308d313` |
| 100 | `29b7c97c` |

## Formula Spec Score Report

Wrote `results/satox-score.md` from `candidates/*.formula`.

No SATO-X candidate is considered faster unless it is benchmarked, verified, and compared against the same Chudnovsky baseline.
 | 0.000 | 1993345.714 | 0.000 | 0.904617 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 240.279 | 52.797 | 0.0000 | 0.000 | 4161829.979 | 0.000 | 0.438198 |  |
| 10000000 | `arb_const_pi` | yes | yes | 1911.424 | 678.359 | 0.0400 | 0.000 | 5231701.943 | 0.000 | 0.256097 |  |
| 10000000 | `chudnovsky_bs` | yes | yes | 5751.458 | 276.203 | 0.0100 | 0.000 | 1738689.450 | 0.000 | 1.000000 |  |
| 10000000 | `chudnovsky_bs_crown` | yes | yes | 1610.022 | 431.734 | 0.0400 | 0.000 | 6211095.896 | 0.000 | 0.301265 |  |
| 10000000 | `chudnovsky_bs_valuation` | yes | yes | 5521.557 | 284.109 | 0.0300 | 0.000 | 1811083.509 | 0.000 | 0.971864 |  |
| 10000000 | `mpfr_const_pi` | yes | yes | 8975.614 | 700.750 | 0.0400 | 0.000 | 1114129.908 | 0.000 | 1.565746 |  |
| 10000000 | `ramanujan_classic_bs` | yes | yes | 4399.893 | 684.922 | 0.0400 | 0.000 | 2272782.500 | 0.000 | 0.758104 |  |
| 100000000 | `chudnovsky_bs` | yes | yes | 93100.155 | 1753.812 | 0.2400 | 0.000 | 1074112.066 | 0.000 | None |  |
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
