# High-Priority Objectives

This is the active queue after the H31-H40 final-algorithm pass. Priority is
based on correctness risk first, then measurement quality, then remaining
performance upside.

## P0 - Correctness and Claim Hygiene

1. Audit `ramanujan_classic_bs` before raising its cap.
   - Status: capped at 10^7.
   - Evidence: 10^8 failed with guard 25, 100, and 1000 with the same prefix
     hash; H38 confirmed 20M and 100M now return explicit unsupported rows.
   - Next experiment: derive an independent term-count/error bound and inspect
     the binary-splitting spec/final normalization against a high-precision
     small-window oracle.

2. Keep benchmark invariants enforced.
   - `total_cost_ms` must be measured end-to-end trial cost, including any
     verification/format overlap and `io_ms`.
   - `relative_wall_time` must be recomputed after every merge.
   - Verification matrix should have no missing cells; unsupported caps are
     explicit skip rows.
   - H37 found one prefix hash per `(digits, guard)` across verified H31-H36
     scratch rows; keep that check in future refreshes.

## P1 - Final Algorithm Policy

3. Use `chudnovsky_hybrid` as the recommended v1 algorithm when FLINT/Arb is
   built.
   - Final policy: route to `chudnovsky_bs_crown` below 700,000 digits and
     `arb_const_pi` at or above 700,000 digits.
   - Evidence: H31-H36 crossover probes found crown winning through 650k, the
     700k boundary within noise, and Arb-backed hybrid winning from 750k/1M
     through 10M.

4. Keep `chudnovsky_bs_crown_tuned` as a benchmark/fallback algorithm, not the
   public router delegate on this host.
   - Evidence: H39 found tuned crown beats plain crown at 1M+ but still loses
     to Arb-backed hybrid from the measured crossover upward.
   - Reconsider only on hosts without FLINT/Arb or after a full-ladder rerun
     shows tuned crown beating Arb by more than noise.

## P2 - Performance Frontier

5. Revisit H19 entropy-weighted/nonuniform crown segmentation.
   - Target: reduce crown merge bit-volume or wall by >=8% at 10^7/10^8.
   - Risk: incorrect precision accounting or worse scheduling locality.

6. Try H16 independent BBP spot checks off the critical path.
   - Goal: add an orthogonal correctness signal with <1% wall impact at 10^6.
   - Keep it reported separately from full-prefix verification.

## P3 - Measurement Breadth

7. Validate H18 energy metrics on Linux with Intel RAPL.
   - Code now records RAPL deltas instead of absolute counter values.
   - macOS remains `energy_joules=0` unless an external `powermetrics` harness
     is added.

8. Re-run full 3-trial ladders on a quiet machine.
   - Several canonical rows are still from focused 1- or 2-trial refreshes.
   - Publishable claims should use one consistent trial policy and host
     manifest.
