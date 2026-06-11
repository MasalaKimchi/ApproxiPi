# High-Priority Objectives

This is the active queue after the H20-H24 refresh. Priority is based on
correctness risk first, then measurement quality, then wall-time upside.

## P0 - Correctness and Claim Hygiene

1. Audit `ramanujan_classic_bs` at 10^8.
   - Status: capped at 10^7.
   - Evidence: 10^8 fails with guard 25, 100, and 1000 with the same prefix hash.
   - Next experiment: derive an independent term-count/error bound and inspect
     the binary-splitting spec/final normalization against a high-precision
     small-window oracle.

2. Keep benchmark invariants enforced.
   - `total_cost_ms` must be `wall_ms + verify_ms + io_ms`.
   - `relative_wall_time` must be recomputed after every merge.
   - Verification matrix should have no missing cells; unsupported caps are
     explicit `skip` rows.

## P1 - Performance Frontier

3. Promote tuned crown into routing policy only if it beats Arb on a target
   host or FLINT is absent.
   - Current Apple Silicon result: Arb-backed hybrid wins from 10^6 upward.
   - Tuned crown remains valuable as the best in-repo path at 10^7-10^8.

4. Revisit H19 entropy-weighted/nonuniform crown segmentation.
   - Target: reduce crown merge bit-volume or wall by >=8% at 10^7/10^8.
   - Risk: incorrect precision accounting or worse scheduling locality.

5. Try H16 independent BBP spot checks off the critical path.
   - Goal: add an orthogonal correctness signal with <1% wall impact at 10^6.
   - Keep it reported separately from full-prefix verification.

## P2 - Measurement Breadth

6. Validate H18 energy metrics on Linux with Intel RAPL.
   - Code now records RAPL deltas instead of absolute counter values.
   - macOS remains `energy_joules=0` unless an external `powermetrics` harness is added.

7. Re-run full 3-trial ladders on a quiet machine.
   - Several canonical rows are still from focused 1- or 2-trial refreshes.
   - Publishable claims should use one consistent trial policy and host manifest.
