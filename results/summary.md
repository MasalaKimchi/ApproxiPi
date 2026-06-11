# SATO-X Engineering Benchmark Summary

Guard digits: `25`

Canonical data: `results/benchmark.csv` (merged from incremental runs).

Cost model: T_series + T_bigint + T_sqrt/div + T_radix + T_verify + T_I/O. Efficiency = (seconds * watts * bytes moved) / verified digits.

| Digits | Algorithm | Supported | Verified | Runtime ms | Peak RAM MiB | R/W GB | Energy J | Digits/sec | Digits/J | Relative | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 1000 | `arb_const_pi` | yes | yes | 0.071 | 3.328 | 0.0000 | 0.000 | 14084507.042 | 0.000 | 0.800000 |  |
| 1000 | `borwein_cubic` | yes | yes | 0.102 | 2.859 | 0.0000 | 0.000 | 9803921.569 | 0.000 | 1.141176 |  |
| 1000 | `borwein_quartic` | yes | yes | 0.128 | 2.859 | 0.0000 | 0.000 | 7812500.000 | 0.000 | 1.376471 |  |
| 1000 | `chudnovsky_bs` | yes | yes | 0.087 | 2.688 | 0.0000 | 0.000 | 11494252.874 | 0.000 | 1.000000 |  |
| 1000 | `chudnovsky_bs_crown` | yes | yes | 0.087 | 2.719 | 0.0000 | 0.000 | 11494252.874 | 0.000 | 1.000000 |  |
| 1000 | `chudnovsky_bs_crown_h15` | yes | yes | 0.086 | 2.734 | 0.0000 | 0.000 | 11627906.977 | 0.000 | 0.988235 |  |
| 1000 | `chudnovsky_bs_crown_tuned` | yes | yes | 0.103 | 2.750 | 0.0000 | 0.000 | 9708737.864 | 0.000 | 1.176471 |  |
| 1000 | `chudnovsky_bs_valuation` | yes | yes | 0.086 | 2.688 | 0.0000 | 0.000 | 11627906.977 | 0.000 | 0.988235 |  |
| 1000 | `chudnovsky_hybrid` | yes | yes | 0.088 | 3.328 | 0.0000 | 0.000 | 11363636.364 | 0.000 | 1.011765 | delegate=chudnovsky_bs_crown |
| 1000 | `chudnovsky_naive` | yes | yes | 0.273 | 2.516 | 0.0000 | 0.000 | 3663003.663 | 0.000 | 3.176471 |  |
| 1000 | `chudnovsky_recurrence` | yes | yes | 0.189 | 2.625 | 0.0000 | 0.000 | 5291005.291 | 0.000 | 2.200000 |  |
| 1000 | `gauss_legendre_agm` | yes | yes | 0.087 | 2.859 | 0.0000 | 0.000 | 11494252.874 | 0.000 | 0.894118 |  |
| 1000 | `machin_arctan` | yes | yes | 0.196 | 2.828 | 0.0000 | 0.000 | 5102040.816 | 0.000 | 2.247059 |  |
| 1000 | `mpfr_const_pi` | yes | yes | 0.140 | 2.969 | 0.0000 | 0.000 | 7142857.143 | 0.000 | 1.600000 |  |
| 1000 | `ramanujan_classic_bs` | yes | yes | 0.096 | 2.750 | 0.0000 | 0.000 | 10416666.667 | 0.000 | 1.094118 |  |
| 10000 | `arb_const_pi` | yes | yes | 0.920 | 7.750 | 0.0000 | 0.000 | 10869565.217 | 0.000 | 0.645044 |  |
| 10000 | `borwein_cubic` | yes | yes | 2.638 | 4.203 | 0.0000 | 0.000 | 3790750.569 | 0.000 | 1.860058 |  |
| 10000 | `borwein_quartic` | yes | yes | 2.903 | 4.234 | 0.0000 | 0.000 | 3444712.367 | 0.000 | 1.968659 |  |
| 10000 | `chudnovsky_bs` | yes | yes | 1.404 | 3.266 | 0.0000 | 0.000 | 7122507.123 | 0.000 | 1.000000 |  |
| 10000 | `chudnovsky_bs_crown` | yes | yes | 1.050 | 3.375 | 0.0000 | 0.000 | 9523809.524 | 0.000 | 0.740525 |  |
| 10000 | `chudnovsky_bs_crown_h15` | yes | yes | 1.081 | 3.391 | 0.0000 | 0.000 | 9250693.802 | 0.000 | 0.761662 |  |
| 10000 | `chudnovsky_bs_crown_tuned` | yes | yes | 1.194 | 3.406 | 0.0000 | 0.000 | 8375209.380 | 0.000 | 0.844752 |  |
| 10000 | `chudnovsky_bs_valuation` | yes | yes | 1.503 | 3.312 | 0.0000 | 0.000 | 6653359.947 | 0.000 | 1.067784 |  |
| 10000 | `chudnovsky_hybrid` | yes | yes | 0.954 | 7.844 | 0.0000 | 0.000 | 10482180.294 | 0.000 | 0.672012 | delegate=chudnovsky_bs_crown |
| 10000 | `chudnovsky_naive` | yes | yes | 20.812 | 3.016 | 0.0000 | 0.000 | 480492.024 | 0.000 | 15.142128 |  |
| 10000 | `chudnovsky_recurrence` | yes | yes | 19.650 | 3.047 | 0.0000 | 0.000 | 508905.852 | 0.000 | 14.295190 |  |
| 10000 | `gauss_legendre_agm` | yes | yes | 1.914 | 4.109 | 0.0000 | 0.000 | 5224660.397 | 0.000 | 1.273324 |  |
| 10000 | `machin_arctan` | yes | yes | 6.410 | 4.078 | 0.0000 | 0.000 | 1560062.402 | 0.000 | 4.606414 |  |
| 10000 | `mpfr_const_pi` | yes | yes | 0.881 | 4.344 | 0.0000 | 0.000 | 11350737.798 | 0.000 | 0.616618 |  |
| 10000 | `ramanujan_classic_bs` | yes | yes | 1.171 | 3.516 | 0.0000 | 0.000 | 8539709.650 | 0.000 | 0.830175 |  |
| 100000 | `arb_const_pi` | yes | yes | 8.467 | 16.719 | 0.0000 | 0.000 | 11810558.639 | 0.000 | 0.225887 |  |
| 100000 | `borwein_cubic` | yes | yes | 72.733 | 14.984 | 0.0000 | 0.000 | 1374891.727 | 0.000 | 2.115752 |  |
| 100000 | `borwein_quartic` | yes | yes | 81.105 | 14.984 | 0.0000 | 0.000 | 1232969.607 | 0.000 | 2.307069 |  |
| 100000 | `chudnovsky_bs` | yes | yes | 34.371 | 5.422 | 0.0000 | 0.000 | 2909462.982 | 0.000 | 1.000000 |  |
| 100000 | `chudnovsky_bs_crown` | yes | yes | 7.555 | 11.469 | 0.0000 | 0.000 | 13236267.373 | 0.000 | 0.198548 |  |
| 100000 | `chudnovsky_bs_crown_h15` | yes | yes | 7.623 | 11.594 | 0.0000 | 0.000 | 13118194.936 | 0.000 | 0.199653 |  |
| 100000 | `chudnovsky_bs_crown_tuned` | yes | yes | 7.644 | 11.781 | 0.0000 | 0.000 | 13082155.939 | 0.000 | 0.201476 |  |
| 100000 | `chudnovsky_bs_valuation` | yes | yes | 33.332 | 10.453 | 0.0000 | 0.000 | 3000120.005 | 0.000 | 0.969433 |  |
| 100000 | `chudnovsky_hybrid` | yes | yes | 7.513 | 17.094 | 0.0000 | 0.000 | 13310262.212 | 0.000 | 0.197412 | delegate=chudnovsky_bs_crown |
| 100000 | `chudnovsky_naive` | yes | yes | 4613.790 | 10.062 | 0.0000 | 0.000 | 21674.155 | 0.000 | 137.830764 |  |
| 100000 | `chudnovsky_recurrence` | no | no | 0.000 | 10.062 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 100000 | `gauss_legendre_agm` | yes | yes | 54.773 | 14.875 | 0.0000 | 0.000 | 1825717.050 | 0.000 | 1.522260 |  |
| 100000 | `machin_arctan` | yes | yes | 176.759 | 14.656 | 0.0000 | 0.000 | 565742.056 | 0.000 | 5.222690 |  |
| 100000 | `mpfr_const_pi` | yes | yes | 26.617 | 15.297 | 0.0000 | 0.000 | 3756997.408 | 0.000 | 0.768884 |  |
| 100000 | `ramanujan_classic_bs` | yes | yes | 18.991 | 12.125 | 0.0000 | 0.000 | 5265652.151 | 0.000 | 0.541204 |  |
| 1000000 | `arb_const_pi` | yes | yes | 98.823 | 64.219 | 0.0000 | 0.000 | 10119101.829 | 0.000 | 0.247586 |  |
| 1000000 | `borwein_cubic` | yes | yes | 1670.668 | 20.547 | 0.0000 | 0.000 | 598562.970 | 0.000 | 3.598362 |  |
| 1000000 | `borwein_quartic` | yes | yes | 1442.047 | 23.047 | 0.0000 | 0.000 | 693458.674 | 0.000 | 4.231376 |  |
| 1000000 | `chudnovsky_bs` | yes | yes | 341.199 | 22.844 | 0.0000 | 0.000 | 2930839.512 | 0.000 | 1.000000 |  |
| 1000000 | `chudnovsky_bs_crown` | yes | yes | 121.358 | 44.203 | 0.0000 | 0.000 | 8240083.060 | 0.000 | 0.316170 |  |
| 1000000 | `chudnovsky_bs_crown_h15` | yes | yes | 105.068 | 35.641 | 0.0000 | 0.000 | 9517645.715 | 0.000 | 0.271986 |  |
| 1000000 | `chudnovsky_bs_crown_tuned` | yes | yes | 103.623 | 38.812 | 0.0000 | 0.000 | 9650367.196 | 0.000 | 0.266436 |  |
| 1000000 | `chudnovsky_bs_valuation` | yes | yes | 1267.269 | 21.766 | 0.0000 | 0.000 | 789098.447 | 0.000 | 2.419590 |  |
| 1000000 | `chudnovsky_hybrid` | yes | yes | 95.813 | 65.547 | 0.0000 | 0.000 | 10436997.067 | 0.000 | 0.242055 | delegate=arb_const_pi |
| 1000000 | `chudnovsky_naive` | no | no | 0.000 | 17.703 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | naive summation capped at 100000 digits |
| 1000000 | `chudnovsky_recurrence` | no | no | 0.000 | 17.703 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 1000000 | `gauss_legendre_agm` | no | no | 0.000 | 1.828 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 1000000 | `machin_arctan` | no | no | 0.000 | 1.828 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 1000000 | `mpfr_const_pi` | yes | yes | 461.022 | 39.922 | 0.0000 | 0.000 | 2169093.883 | 0.000 | 1.368851 |  |
| 1000000 | `ramanujan_classic_bs` | yes | yes | 240.880 | 52.797 | 0.0000 | 0.000 | 4151444.703 | 0.000 | 0.718190 |  |
| 10000000 | `arb_const_pi` | yes | yes | 981.872 | 563.406 | 0.0000 | 0.000 | 10184626.917 | 0.000 | 0.166448 |  |
| 10000000 | `borwein_cubic` | no | no | 0.000 | 611.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 10000000 | `borwein_quartic` | no | no | 0.000 | 611.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 10000000 | `chudnovsky_bs` | yes | yes | 5546.712 | 250.656 | 0.0200 | 0.000 | 1802870.023 | 0.000 | 1.000000 |  |
| 10000000 | `chudnovsky_bs_crown` | yes | yes | 1743.974 | 363.438 | 0.0000 | 0.000 | 5734030.439 | 0.000 | 0.304789 |  |
| 10000000 | `chudnovsky_bs_crown_h15` | yes | yes | 2385.202 | 372.625 | 0.0100 | 0.000 | 4192517.028 | 0.000 | 0.329577 |  |
| 10000000 | `chudnovsky_bs_crown_tuned` | yes | yes | 1337.952 | 413.891 | 0.0100 | 0.000 | 7474109.684 | 0.000 | 0.231584 |  |
| 10000000 | `chudnovsky_bs_valuation` | yes | yes | 5532.815 | 250.453 | 0.0100 | 0.000 | 1807398.223 | 0.000 | 0.997633 |  |
| 10000000 | `chudnovsky_hybrid` | yes | yes | 1039.064 | 589.922 | 0.0000 | 0.000 | 9624046.257 | 0.000 | 0.176644 | delegate=arb_const_pi |
| 10000000 | `chudnovsky_naive` | no | no | 0.000 | 1.828 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | naive summation capped at 100000 digits |
| 10000000 | `chudnovsky_recurrence` | no | no | 0.000 | 1.828 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 10000000 | `gauss_legendre_agm` | no | no | 0.000 | 611.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 10000000 | `machin_arctan` | no | no | 0.000 | 611.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 10000000 | `mpfr_const_pi` | yes | yes | 8383.234 | 444.500 | 0.0100 | 0.000 | 1192857.076 | 0.000 | 1.517977 |  |
| 10000000 | `ramanujan_classic_bs` | yes | yes | 5423.392 | 611.469 | 0.0000 | 0.000 | 1843864.844 | 0.000 | 0.892106 |  |
| 100000000 | `arb_const_pi` | yes | yes | 13940.233 | 2781.219 | 0.0000 | 0.000 | 7173481.247 | 0.000 | 0.137073 |  |
| 100000000 | `borwein_cubic` | no | no | 0.000 | 3449.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 100000000 | `borwein_quartic` | no | no | 0.000 | 3449.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 100000000 | `chudnovsky_bs` | yes | yes | 94602.970 | 1753.812 | 0.2400 | 0.000 | 1057049.266 | 0.000 | 1.000000 |  |
| 100000000 | `chudnovsky_bs_crown` | yes | yes | 30701.111 | 2005.781 | 0.0000 | 0.000 | 3257211.115 | 0.000 | 0.314542 |  |
| 100000000 | `chudnovsky_bs_crown_h15` | yes | yes | 26197.621 | 2301.766 | 0.1100 | 0.000 | 3817140.495 | 0.000 | 0.271751 |  |
| 100000000 | `chudnovsky_bs_crown_tuned` | yes | yes | 24311.496 | 2396.016 | 0.1100 | 0.000 | 4113280.400 | 0.000 | 0.252023 |  |
| 100000000 | `chudnovsky_bs_valuation` | yes | yes | 93453.824 | 1485.031 | 0.1100 | 0.000 | 1070047.171 | 0.000 | 0.987744 |  |
| 100000000 | `chudnovsky_hybrid` | yes | yes | 13086.162 | 2926.469 | 0.0000 | 0.000 | 7641659.946 | 0.000 | 0.133035 | delegate=arb_const_pi |
| 100000000 | `chudnovsky_naive` | no | no | 0.000 | 608.203 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | naive summation capped at 100000 digits |
| 100000000 | `chudnovsky_recurrence` | no | no | 0.000 | 608.203 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | recurrence summation capped at 10000 digits |
| 100000000 | `gauss_legendre_agm` | no | no | 0.000 | 3449.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 100000000 | `machin_arctan` | no | no | 0.000 | 3449.469 | 0.0000 | 0.000 | 0.000 | 0.000 | 0.000000 | requested precision exceeds algorithm max_digits |
| 100000000 | `mpfr_const_pi` | yes | yes | 134483.636 | 2396.016 | 0.1100 | 0.000 | 743584.892 | 0.000 | 1.424154 |  |
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
