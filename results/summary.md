# SATO-X Engineering Benchmark Summary

Guard digits: `25`

Canonical data: `results/benchmark.csv` (merged from incremental runs).

Cost model: T_series + T_bigint + T_sqrt/div + T_radix + T_verify + T_I/O. Efficiency = (seconds * watts * bytes moved) / verified digits.

| Digits | Algorithm | Supported | Verified | Runtime ms | Peak RAM MiB | R/W GB | Energy J | Digits/sec | Digits/J | Relative | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `arb_const_pi` | yes | yes | 0.071 | 3.328 | 0.0000 | 0.000 | 14084507.042 | 0.000 | 0.819277 |  |
| 1000 | `borwein_cubic` | yes | yes | 0.102 | 2.859 | 0.0000 | 0.000 | 9803921.569 | 0.000 | 1.168675 |  |
| 1000 | `borwein_quartic` | yes | yes | 0.128 | 2.859 | 0.0000 | 0.000 | 7812500.000 | 0.000 | 1.409639 |  |
| 1000 | `chudnovsky_bs` | yes | yes | 0.085 | 2.688 | 0.0000 | 0.000 | 11764705.882353 | 0.000 | 1.000000 |  |
| 1000 | `chudnovsky_bs_crown` | yes | yes | 0.085 | 2.719 | 0.0000 | 0.000 | 11764705.882353 | 0.000 | 1.000000 |  |
| 1000 | `chudnovsky_bs_crown_h15` | yes | yes | 0.084 | 2.734 | 0.0000 | 0.000 | 11904761.904762 | 0.000 | 0.987952 |  |
| 1000 | `chudnovsky_bs_crown_tuned` | yes | yes | 0.100 | 2.750 | 0.0000 | 0.000 | 10000000.000000 | 0.000 | 1.168675 |  |
| 1000 | `chudnovsky_bs_valuation` | yes | yes | 0.084 | 2.688 | 0.0000 | 0.000 | 11904761.904762 | 0.000 | 0.987952 |  |
| 1000 | `chudnovsky_hybrid` | yes | yes | 0.164 | 2.500 | 0.0000 | 0.000 | 6082873.063 | 0.000 | 1.915663 | delegate=chudnovsky_bs_crown |
| 1000 | `chudnovsky_naive` | yes | yes | 0.270 | 2.516 | 0.0000 | 0.000 | 3703703.703704 | 0.000 | 3.216867 |  |
| 1000 | `chudnovsky_recurrence` | yes | yes | 0.187 | 2.625 | 0.0000 | 0.000 | 5347593.582888 | 0.000 | 2.228916 |  |
| 1000 | `gauss_legendre_agm` | yes | yes | 0.087 | 2.859 | 0.0000 | 0.000 | 11494252.874 | 0.000 | 0.915663 |  |
| 1000 | `machin_arctan` | yes | yes | 0.196 | 2.828 | 0.0000 | 0.000 | 5102040.816 | 0.000 | 2.301205 |  |
| 1000 | `mpfr_const_pi` | yes | yes | 0.140 | 2.969 | 0.0000 | 0.000 | 7142857.143 | 0.000 | 1.638554 |  |
| 1000 | `ramanujan_classic_bs` | yes | yes | 0.093 | 2.750 | 0.0000 | 0.000 | 10752688.172043 | 0.000 | 1.084337 |  |
| 10000 | `arb_const_pi` | yes | yes | 0.920 | 7.750 | 0.0000 | 0.000 | 10869565.217 | 0.000 | 0.660448 |  |
| 10000 | `borwein_cubic` | yes | yes | 2.638 | 4.203 | 0.0000 | 0.000 | 3790750.569 | 0.000 | 1.904478 |  |
| 10000 | `borwein_quartic` | yes | yes | 2.903 | 4.234 | 0.0000 | 0.000 | 3444712.367 | 0.000 | 2.015672 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 1.372 | 3.266 | 0.0000 | 0.000 | 7288629.737609 | 0.000 | 1.000000 |  |
| 10000 | `chudnovsky_bs_crown` | yes | yes | 1.016 | 3.375 | 0.0000 | 0.000 | 9842519.685039 | 0.000 | 0.732836 |  |
| 10000 | `chudnovsky_bs_crown_h15` | yes | yes | 1.045 | 3.391 | 0.0000 | 0.000 | 9569377.990431 | 0.000 | 0.752985 |  |
| 10000 | `chudnovsky_bs_crown_tuned` | yes | yes | 1.159 | 3.406 | 0.0000 | 0.000 | 8628127.696290 | 0.000 | 0.838806 |  |
| 10000 | `chudnovsky_bs_valuation` | yes | yes | 1.465 | 3.312 | 0.0000 | 0.000 | 6825938.566553 | 0.000 | 1.064925 |  |
| 10000 | `chudnovsky_hybrid` | yes | yes | 1.193 | 3.078 | 0.0000 | 0.000 | 8381351.493 | 0.000 | 0.853731 | delegate=chudnovsky_bs_crown |
| 10000 | `chudnovsky_naive` | yes | yes | 20.775 | 3.016 | 0.0000 | 0.000 | 481347.773767 | 0.000 | 15.476119 |  |
| 10000 | `chudnovsky_recurrence` | yes | yes | 19.613 | 3.047 | 0.0000 | 0.000 | 509865.905267 | 0.000 | 14.608955 |  |
| 10000 | `gauss_legendre_agm` | yes | yes | 1.914 | 4.109 | 0.0000 | 0.000 | 5224660.397 | 0.000 | 1.303731 |  |
| 10000 | `machin_arctan` | yes | yes | 6.410 | 4.078 | 0.0000 | 0.000 | 1560062.402 | 0.000 | 4.716418 |  |
| 10000 | `mpfr_const_pi` | yes | yes | 0.881 | 4.344 | 0.0000 | 0.000 | 11350737.798 | 0.000 | 0.631343 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 1.139 | 3.516 | 0.0000 | 0.000 | 8779631.255487 | 0.000 | 0.826119 |  |
| 100000 | `arb_const_pi` | yes | yes | 8.467 | 16.719 | 0.0000 | 0.000 | 11810558.639 | 0.000 | 0.232151 |  |
| 100000 | `borwein_cubic` | yes | yes | 72.733 | 14.984 | 0.0000 | 0.000 | 1374891.727 | 0.000 | 2.174420 |  |
| 100000 | `borwein_quartic` | yes | yes | 81.105 | 14.984 | 0.0000 | 0.000 | 1232969.607 | 0.000 | 2.371043 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 33.468 | 5.422 | 0.0000 | 0.000 | 2987928.767778 | 0.000 | 1.000000 |  |
| 100000 | `chudnovsky_bs_crown` | yes | yes | 6.645 | 11.469 | 0.0000 | 0.000 | 15048908.954101 | 0.000 | 0.176109 |  |
| 100000 | `chudnovsky_bs_crown_h15` | yes | yes | 6.682 | 11.594 | 0.0000 | 0.000 | 14965579.167914 | 0.000 | 0.176294 |  |
| 100000 | `chudnovsky_bs_crown_tuned` | yes | yes | 6.743 | 11.781 | 0.0000 | 0.000 | 14830194.275545 | 0.000 | 0.179395 |  |
| 100000 | `chudnovsky_bs_valuation` | yes | yes | 32.445 | 10.453 | 0.0000 | 0.000 | 3082139.004469 | 0.000 | 0.969077 |  |
| 100000 | `chudnovsky_hybrid` | yes | yes | 6.826 | 7.250 | 0.0000 | 0.000 | 14649778.012 | 0.000 | 0.181452 | delegate=chudnovsky_bs_crown |
| 100000 | `chudnovsky_naive` | yes | yes | 4612.920 | 10.062 | 0.0000 | 0.000 | 21678.242848 | 0.000 | 141.625979 |  |
| 100000 | `chudnovsky_recurrence` | no | no | 0.000 | 10.062 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 100000 | `gauss_legendre_agm` | yes | yes | 54.773 | 14.875 | 0.0000 | 0.000 | 1825717.050 | 0.000 | 1.564471 |  |
| 100000 | `machin_arctan` | yes | yes | 176.759 | 14.656 | 0.0000 | 0.000 | 565742.056 | 0.000 | 5.367511 |  |
| 100000 | `mpfr_const_pi` | yes | yes | 26.617 | 15.297 | 0.0000 | 0.000 | 3756997.408 | 0.000 | 0.790204 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 18.113 | 12.125 | 0.0000 | 0.000 | 5520896.593607 | 0.000 | 0.529249 |  |
| 1000000 | `arb_const_pi` | yes | yes | 98.823 | 64.219 | 0.0000 | 0.000 | 10119101.829 | 0.000 | 0.261191 |  |
| 1000000 | `borwein_cubic` | yes | yes | 1670.668 | 20.547 | 0.0000 | 0.000 | 598562.970 | 0.000 | 3.796087 |  |
| 1000000 | `borwein_quartic` | yes | yes | 1442.047 | 23.047 | 0.0000 | 0.000 | 693458.674 | 0.000 | 4.463884 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 324.307 | 22.844 | 0.0000 | 0.000 | 3083498.043520 | 0.000 | 1.000000 |  |
| 1000000 | `chudnovsky_bs_crown` | yes | yes | 102.536 | 44.203 | 0.0000 | 0.000 | 9752672.232192 | 0.000 | 0.272316 |  |
| 1000000 | `chudnovsky_bs_crown_h15` | yes | yes | 88.207 | 35.641 | 0.0000 | 0.000 | 11336968.721303 | 0.000 | 0.232084 |  |
| 1000000 | `chudnovsky_bs_crown_tuned` | yes | yes | 86.407 | 38.812 | 0.0000 | 0.000 | 11573136.435705 | 0.000 | 0.225074 |  |
| 1000000 | `chudnovsky_bs_valuation` | yes | yes | 784.690 | 21.766 | 0.0000 | 0.000 | 1274388.612063 | 0.000 | 0.982746 |  |
| 1000000 | `chudnovsky_hybrid` | yes | yes | 73.337 | 50.469 | 0.0000 | 0.000 | 13635708.998 | 0.000 | 0.181969 | delegate=arb_const_pi |
| 1000000 | `chudnovsky_naive` | no | no | 0.000 | 17.703 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | naive summation capped at 100000 digits |
| 1000000 | `chudnovsky_recurrence` | no | no | 0.000 | 17.703 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 1000000 | `gauss_legendre_agm` | no | no | 0.000 | 1.828 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 1000000 | `machin_arctan` | no | no | 0.000 | 1.828 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 1000000 | `mpfr_const_pi` | yes | yes | 461.022 | 39.922 | 0.0000 | 0.000 | 2169093.883 | 0.000 | 1.444067 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 232.914 | 52.797 | 0.0000 | 0.000 | 4293430.193118 | 0.000 | 0.731740 |  |
| 10000000 | `arb_const_pi` | yes | yes | 981.872 | 563.406 | 0.0000 | 0.000 | 10184626.917 | 0.000 | 0.168586 |  |
| 10000000 | `borwein_cubic` | no | no | 0.000 | 611.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 10000000 | `borwein_quartic` | no | no | 0.000 | 611.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 10000000 | `chudnovsky_bs` | yes | yes | 5477.254 | 250.656 | 0.0200 | 0.000 | 1825732.383417 | 0.000 | 1.000000 |  |
| 10000000 | `chudnovsky_bs_crown` | yes | yes | 1669.409 | 363.438 | 0.0000 | 0.000 | 5990143.817363 | 0.000 | 0.294916 |  |
| 10000000 | `chudnovsky_bs_crown_h15` | yes | yes | 1805.179 | 372.625 | 0.0100 | 0.000 | 5539616.846861 | 0.000 | 0.226554 |  |
| 10000000 | `chudnovsky_bs_crown_tuned` | yes | yes | 1268.447 | 413.891 | 0.0100 | 0.000 | 7883656.155913 | 0.000 | 0.221706 |  |
| 10000000 | `chudnovsky_bs_valuation` | yes | yes | 5464.287 | 250.453 | 0.0100 | 0.000 | 1830064.928874 | 0.000 | 0.997774 |  |
| 10000000 | `chudnovsky_hybrid` | yes | yes | 1000.807 | 438.984 | 0.0000 | 0.000 | 9991936.298 | 0.000 | 0.171886 | delegate=arb_const_pi |
| 10000000 | `chudnovsky_naive` | no | no | 0.000 | 1.828 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | naive summation capped at 100000 digits |
| 10000000 | `chudnovsky_recurrence` | no | no | 0.000 | 1.828 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 10000000 | `gauss_legendre_agm` | no | no | 0.000 | 611.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 10000000 | `machin_arctan` | no | no | 0.000 | 611.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 10000000 | `mpfr_const_pi` | yes | yes | 8383.234 | 444.500 | 0.0100 | 0.000 | 1192857.076 | 0.000 | 1.537474 |  |
| 10000000 | `ramanujan_classic_bs` | yes | yes | 4886.292 | 611.469 | 0.0000 | 0.000 | 2046541.631159 | 0.000 | 0.804245 |  |
| 100000000 | `arb_const_pi` | yes | yes | 13940.233 | 2781.219 | 0.0000 | 0.000 | 7173481.247 | 0.000 | 0.137925 |  |
| 100000000 | `borwein_cubic` | no | no | 0.000 | 3449.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 100000000 | `borwein_quartic` | no | no | 0.000 | 3449.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 100000000 | `chudnovsky_bs` | yes | yes | 94022.572 | 1753.812 | 0.2400 | 0.000 | 1063574.393604 | 0.000 | 1.000000 |  |
| 100000000 | `chudnovsky_bs_crown` | yes | yes | 29574.056 | 2005.781 | 0.0000 | 0.000 | 3381342.079017 | 0.000 | 0.304434 |  |
| 100000000 | `chudnovsky_bs_crown_h15` | yes | yes | 25550.768 | 2301.766 | 0.1100 | 0.000 | 3913776.681781 | 0.000 | 0.266517 |  |
| 100000000 | `chudnovsky_bs_crown_tuned` | yes | yes | 23695.833 | 2396.016 | 0.1100 | 0.000 | 4220151.281451 | 0.000 | 0.246999 |  |
| 100000000 | `chudnovsky_bs_valuation` | yes | yes | 92870.219 | 1485.031 | 0.1100 | 0.000 | 1076771.445968 | 0.000 | 0.987633 |  |
| 100000000 | `chudnovsky_hybrid` | yes | yes | 13086.162 | 2926.469 | 0.0000 | 0.000 | 7641659.946 | 0.000 | 0.133861 | delegate=arb_const_pi |
| 100000000 | `chudnovsky_naive` | no | no | 0.000 | 608.203 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | naive summation capped at 100000 digits |
| 100000000 | `chudnovsky_recurrence` | no | no | 0.000 | 608.203 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 100000000 | `gauss_legendre_agm` | no | no | 0.000 | 3449.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 100000000 | `machin_arctan` | no | no | 0.000 | 3449.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 100000000 | `mpfr_const_pi` | yes | yes | 134483.636 | 2396.016 | 0.1100 | 0.000 | 743584.892 | 0.000 | 1.433000 |  |
| 100000000 | `ramanujan_classic_bs` | no | no | 0.000 | 1.844 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |

See [`results/efficiency.md`](efficiency.md) for digits/sec, digits/joule, peak RAM, and I/O columns across all methods.

## BBP Verification Spots

| Hex offset | 8 hex digits |
|---:|---|
| 0 | `243f6a88` |
| 10 | `a308d313` |
| 100 | `29b7c97c` |

## Formula Spec Score Report

Wrote `results/satox-score.md` from `formulas/specs/*.formula`.

No SATO-X candidate is considered faster unless it is benchmarked, verified, and compared against the same Chudnovsky baseline.
