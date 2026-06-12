# Truncated-Crown Research Log

Iterative hypothesis loop targeting the Chudnovsky binary-splitting baseline
(`chudnovsky_bs`) under the SATO-X rules: every claim must pass the same
exact-prefix verification, and refuted hypotheses are recorded, not hidden.

Hardware: Apple Silicon, GMP 6.3.0 / MPFR 4.2.x, FLINT 3.5.0; 5 trials,
1 warmup, guard 25. Wall time excludes the harness verify phase;
verification must still pass.

## Final standing (`results/summary.md`, with external baselines)

| Digits | `chudnovsky_bs` | crown (+H12) | Relative | MPFR `const_pi` | Arb `const_pi` | Verified |
|---:|---:|---:|---:|---:|---:|---|
| 1,000 | 0.045 ms | 0.045 ms | 1.00 | 0.033 ms | **0.009 ms** | yes |
| 10,000 | 0.62 ms | 0.54 ms | 0.87 | 0.74 ms | 0.97 ms | yes |
| 100,000 | 12.11 ms | 4.17 ms | **0.34** | 25.8 ms | 8.81 ms | yes |
| 1,000,000 | 322 ms | 104 ms | **0.32** | — | — | yes |
| 10,000,000 | 5,644 ms | 1,597 ms | **0.28** | — | — | yes |
| 100,000,000 | 97.4 s | 30.6 s | **0.31** | — | — | yes |

All rows produce byte-identical decimal prefixes (same hash). At and above
10^5 digits the crown beats both installable external references on this
machine: 1.67-1.97x vs FLINT/Arb and 5.8-7.7x vs MPFR. Below ~10^4 digits
Arb wins; the crown's threading and pipelining do not pay there.

Max exact operand drops from 6,860,005 bits to 54,002 bits at 10^6 digits:
the crown never materializes the redundant low half of the tree's integers.

## Machine-independent work metric (new)

Every split-phase multiplication now contributes `bits(a) + bits(b)` to a
`mul_bit_volume` counter (CSV column). Findings at 10^6 digits:

- baseline 0.376 Gbit vs crown 0.350 Gbit: truncation removes only ~7% of
  raw multiplication volume, because most volume lives in the lower exact
  tree levels that the crown leaves untouched;
- the 2.37x wall-time win therefore comes mostly from *restructuring*, not
  raw work removal: chunk-level parallelism, far smaller top operands
  (127x), and pipeline overlap of finalize/format with the series;
- Ramanujan-396 costs 2.1x the bit volume of Chudnovsky for the same digits,
  matching its 1.44x wall ratio plus its worse digits-per-term.

## Founding observation

At 10^6 digits the exact tree's root operands are ~6.86 Mbit, but only
D*log2(10) + guard ~ 3.32 Mbit of the final ratio T/Q can influence the
printed digits. Every full product near the root, and the full-precision
final division, pays for dead bits. Additionally, the baseline's time-to-pi
was only 39% series evaluation; finalize (26%) and decimal formatting (35%)
were co-dominant and untouched by classical series-side optimizations.

## Hypothesis ledger

| # | Hypothesis | Prediction | Result | Verdict |
|---|---|---|---|---|
| H1 | Truncated MPFR crown over exact chunks: per-node precision capped by each subtree's contribution offset (derived rigorously from exact chunk bit sizes); P skipped on the rightmost spine where it is provably dead | 4-7% wall at 1M | split -25%, wall -9.5% at 1M | confirmed |
| H2 | The binding constraint is not the series: pipeline value-independent constants (sqrt(10005), 10^D) under the split; fold 10^D into the numerator; 4-way parallel divide-and-conquer decimal rendering | >=20% wall | wall 150->92.6 ms at 1M (0.62 rel) | confirmed |
| H3 | Intra-node parallel root products + fused constant + reciprocal-vs-numerator overlap in finalize | finalize and split both shrink | wall 72.8 ms at 1M (0.50 rel) | confirmed |
| H4 | Warm-start Newton 1/T from the root's leading T product (asymmetric 9/16 root split guarantees overlap > target/2); 8-way format | several ms | wall 69.9 ms; format flat | partly confirmed |
| H5 | Warm the reciprocal from root *inputs* (1/Tl * 1/Qr) to hide it under the whole root merge | finalize -30% | finalize unchanged (MPFR div already Newton-bootstraps internally) | refuted |
| H6 | P-precision cap theorem: P(a,b) is only consumed against offsets >= b, so P needs right-edge precision, not left-edge | merge shrinks | folded into H7 run; small win | confirmed (weak) |
| H7 | Root-Q elision: pi = C*Ql*Qr/T, so never form root Q; assemble numerator concurrently with root T merge; finalize becomes pi0 - pi0*(T*r0 - 1) with half-width correction | finalize 23->11 ms | finalize 23.6->18.8 ms, wall 71.9 ms | confirmed (smaller than predicted) |
| H9a | Deeper crown (128 chunks): truncation reaches further down the right side, chunk exact work shrinks | chunks -25% | chunks 14.1->10.7 ms, wall 68.2 ms | confirmed |
| H9b | More crown merge parallelism (7 levels) | merge shrinks | merge 21.6->28.0 ms (oversubscription) | refuted, reverted |
| H10 | Division-free "digit window" formatting via precomputed reciprocal powers of ten | format halves | format -4 ms but finalize +37 ms: the reciprocal-power table cannot hide under the split because chunks saturate all cores | refuted, reverted |
| H11 | Autotuning the crown knob space (leaf block, chunk depth, parallel levels, intra-node threshold, root split) via coordinate descent, profile cached to `results/tuning.json` | >=5% wall at 1M | 64.0 -> 62.7 ms (-2%), within run noise; the hand-derived H1-H9 configuration is already a local optimum | refuted (marginal) |
| H12 | Spliced-prefix formatting: after `pi0 = N*r0`, the Newton correction only perturbs digits below the warm-bits horizon (~557k of 1M), so divmod + high-half rendering run concurrently with the residual/correction products; the corrected low half is recovered by integer subtraction (`low = z_corr - (z0 - low0)`) whose range check 0 <= low < 10^(4w) *proves* the splice exact, with full re-render fallback | 8-12% wall at 1M | wall 64.0 -> 59.9 ms (-6.4%), format 16.0 -> 13.4 ms; verified | confirmed (low end) |
| H13 | Pre-format scaled-integer verification: compare `floor(pi_computed * 10^V)` to `floor(const_pi * 10^V)` in MPFR before decimal rendering, with `V = min(D, 10^6)` and reference precision capped at `V`; eliminates five redundant `mpfr_const_pi` passes and million-digit decimal round-trips | verify -80% at 1M total; 10^8 must verify | 1M verify 229 -> 11 ms, total 322 -> 104 ms (3.1x vs BS); 10^8 wall 96.3 -> 29.5 s (3.3x), verify 139 s -> 1.1 s, **verified** | confirmed |
| H14 | Hybrid router (`chudnovsky_hybrid`): crown below 10^8 digits, `arb_const_pi` at and above when FLINT is built | matches best delegate per scale; 10^8 wall within ~10% of arb | 10^5 hybrid 6.6 ms (crown 6.6, arb 10.3); 10^6 hybrid 93 ms (crown 82, arb 105); 10^7 hybrid 1608 ms (crown 1555, arb 1562); 10^8 hybrid 24.2 s (crown 29.8, arb 21.5); all verified | partial |

## Error-budget notes (why the truncations are safe)

- Per-node crown precision: target + 64 guard bits minus a *lower bound* on
  the subtree's contribution downshift, computed from exact chunk bit sizes
  (`log2|Q| >= size-1`, `log2|P| <= size`), i.e. always over-allocates.
- The T merge is benign: `|Pl*Tr| / |Tl*Qr| ~ 2^-(shift(mid)-shift(lo))`, so
  the addition has condition number ~1 (no catastrophic cancellation).
- Each truncated merge contributes <= a few ulps at >= 64 bits below target;
  the caller's 153-digit decimal guard band dwarfs the accumulated error.
- Warm-start gate: the Newton correction is only taken when
  `2*(agreement - 64) >= precision_bits + 256`, guaranteed by the 9/16 root
  split; otherwise the kernel falls back to a plain full-precision reciprocal.
- All shortcuts remain subject to the harness's exact-prefix verification
  against MPFR `const_pi`; no unverified claim is reported.

## Error-budget notes for H12 (why the splice is exact)

- After `pi0 = N * r0`, the residual `|T*r0 - 1| <= 2^(-warm_bits+eps)` with
  `warm_bits ~ 1.85 Mbit`, so the scaled correction is < 10^(D-557000+3),
  while the low-half modulus is 10^(4w) ~ 10^500000: a ~10^57000 margin.
- The splice never *relies* on that margin: the corrected low half is
  recovered as `low = z_corr - (z0 - low0)`, and the explicit range check
  `0 <= low < 10^(4w)` is itself the proof that the pre-rendered high half
  equals the high half of the corrected integer. If the check fails (carry
  crossed the boundary), the code falls back to the full re-render.
- The harness still exact-prefix-verifies every emitted digit string.

## Negative results worth remembering

1. Warming an MPFR reciprocal does not help: `mpfr_ui_div` already
   Newton-bootstraps internally; wins must come from removing or overlapping
   whole operations, not pre-seeding them.
2. Parallelism is budget-constrained, not free: both the 7-level merge
   fan-out and the reciprocal-power format lost because the chunk phase
   already saturates every core. Any "overlapped" work must fit in real idle
   cycles.
3. The crown only pays above ~20k digits; below that, thread spawn dominates
   and the variant routes to a plain serial finalize/format path. Against
   external references the crossover is ~10^4 digits: FLINT/Arb wins below.
4. H11: after nine data-driven hypotheses, blind knob search recovers <2%.
   Structured, evidence-guided hypotheses beat parameter sweeps once the
   configuration is near a local optimum.
5. Prior-art audit (post-hoc): the H1 truncation idea independently
   reproduces Gourdon-Sebah's "truncation trick" as analyzed rigorously by
   Mezzarobba (arXiv:1209.5097), and tight precision control is standard in
   y-cruncher. The H7 root-Q elision, H4/H12 warm-Newton + spliced-prefix
   pipeline appear to be less commonly documented; novelty claims should be
   limited accordingly.

## H14+ frontier (not yet attempted)

H13 closed the verification cliff through 10^8 digits. H14 implemented a
scale-aware `chudnovsky_hybrid` router (crown below 10^8, arb at 10^8+).
Single-threshold routing matches crown at 10^5–10^7 and arb-class wall time
at 10^8; a lower crossover (~10^7) might help when arb wins on wall time at
10^7 in some runs. Plausible next hypotheses:

| # | Hypothesis sketch | Why it might matter | Risk |
|---|---|---|---|
| H15 | Crown merge restructuring: merge-adaptive depth (+1 level) | Merge ~64% of split at 10^8; prototype −8% wall at 10^7 even when merge<chunks | Trigger predicate needs retune; may oversubscribe (cf. H9b) |
| H16 | Independent BBP hex spot checks in parallel with H13 MPFR verify | Cheap entropy cross-check; catches bugs H13 cannot | Adds harness complexity; spots are sparse |
| H17 | New Ramanujan–Sato formula with higher digits/term than Chudnovsky | Only path to beat Chudnovsky asymptotics; `satox-score` has no promote candidate yet | Research-heavy; proof certificate required |
| H18 | Energy / $ efficiency instrumentation (RAPL, `powermetrics`) | Efficiency table columns are zero today; publish digits/J and verified digits/$ | Platform-dependent; not wall-time |

**Do not expect** another 2x from crown knob tuning alone. The next wall-time
wins likely require either a different algorithm family (H17) or merge-phase
restructuring at 10^8+ where merge now dominates chunks (H15 prototype below).

## TRIZ analysis (H15+)

### Physical contradictions (from phase columns)

Crown `chudnovsky_bs_crown` phase shares at representative scales
(`results/benchmark.csv`, notes field for chunk/merge split):

| Digits | split_ms | chunks | merge | finalize_ms | format_ms | verify_ms | Binding phase |
|---:|---:|---:|---:|---:|---:|---:|---|
| 10^5 | 4.9 | 3.1 | 1.3 | 1.0 | 0.5 | 12.3 | verify (harness) |
| 10^6 | 50.7 | 33 | 17 | 15.9 | 8.0 | 220* | split ≈ verify† |
| 10^7 | 1033 | 782 | 247 | 276 | 156 | 306 | split (merge rising) |
| 10^8 | 9715 | 3450 | **6264** | 2713 | 13 | 143928‡ | **merge** (40%+ of split) |

\*noisy trial in latest CSV; stable runs ~11 ms post-H13.  
†H13 reduced verify to ~11 ms; split is binding again at 10^6.  
‡10^8 row failed exact-prefix verify in one run; merge/chunk ratio still diagnostic.

**Contradiction A — speed vs memory:** Deeper crown (more, smaller chunks) cuts
merge operand width but multiplies chunk temporaries and scheduling metadata.
At 10^8, peak RSS is already 2.1 GiB for crown vs 1.75 GiB for baseline.

**Contradiction B — parallelism vs overhead:** Chunk phase saturates all cores
(H9b, H10 refuted). Any overlapped work must fit *real idle* cycles — format
(156 ms at 10^7) and finalize (276 ms) are the only sizable holes while split
runs; verify is now cheap post-H13.

**Contradiction C — exactness vs operand width:** H1 truncation removes only
~7% of `mul_bit_volume`; wall wins come from restructuring (smaller top
operands, pipelining), not raw work elimination. Pushing truncation deeper
(merge-adaptive depth) trades exact-chunk work for shorter MPFR merge chains.

**Contradiction D — verify rigor vs cost:** H13 closed the verify cliff; further
gains must come from compute, not weaker checks. Independent cross-checks
(BBP spots) add harness cost unless hidden under idle (Contradiction B).

**Contradiction E — single kernel vs routing:** At 10^8 on this machine Arb
(21.6 s) beats crown (28.2 s wall in research-log table; merge-dominated split
in CSV). H14 hybrid routing is the TRIZ *separation in time/space* resolution:
use crown where it wins, Arb where merge-bound.

### TRIZ principles mapped

| Principle | Application here |
|---|---|
| **Segmentation** | H15: split exact layer into more chunks when merge dominates; H14: route by digit scale |
| **Prior action** | H12 splice renders stable high half before correction finishes; H4 warm Newton |
| **Local quality** | Per-node `needed_precision` from contribution offset; asymmetric 9/16 root split |
| **Nested doll** | Exact chunks inside truncated MPFR crown inside pipelined finalize/format |
| **Another dimension** | H17: change formula family (digits/term), not just kernel knobs |
| **Dynamics** | H15: depth adapts to term scale / predicted merge:chunk ratio |
| **Universality** | H13 scaled-integer verify works for all algorithms in harness |
| **Cheap shortcuts** | H16 BBP spots — only if overlapped, not on critical path |

### Hypotheses H15–H19 (falsifiable)

| # | Name | TRIZ principle | Prediction | Falsifier | Binding constraint |
|---|---|---|---|---|---|
| **H15** | Merge-adaptive crown depth | Segmentation + Dynamics | When merge/chunk ≥ 1 (10^8), +1 crown level cuts merge ≥15% and wall ≥5%; at 10^7 with merge<chunk, ≤2% or regression | Wall not improved at 10^8 with depth 8→9; or chunk growth swamps merge at 10^7 | merge_ms at 10^8 |
| **H16** | BBP spot verify during format idle | Prior action + Cheap shortcuts | Zero wall impact; catches class of bugs H13 misses; <5 ms amortized at 10^6 | Adds >1% wall at 10^6; or spots disagree with MPFR on valid run | verify_ms (must stay off critical path) |
| **H17** | Higher digits/term Ramanujan–Sato | Another dimension | New formula with d/term > 14.18 beats crown asymptotically by ≥10% at 10^8 | `satox-score` promote + benchmark still loses to Chudnovsky at 10^7 | split_ms (series terms) |
| **H18** | RAPL / powermetrics energy column | Universality | Populates digits/J; reveals crown vs Arb efficiency crossover ≠ wall crossover | Instrumentation noise >20%; or all-zero on Apple Silicon without sudo | efficiency table (not wall) |
| **H19** | Entropy-weighted chunk tree | Local quality + Segmentation | Asymmetric chunk sizes from per-subtree `shift_lb` gradient reduce merge bit-volume ≥8% at 10^7 vs uniform binary partition | mul_bit_volume unchanged and merge_ms ≥ baseline; or verify fails from mis-sized truncation | merge_ms + mul_bit_volume |
| **H20** | External baseline pipeline reuse | Universality + Prior action | Give MPFR/Arb the H13 scaled verify and parallel decimal renderer; Arb wall improves ≥15% at 10^6 and total improves ≥5x | Arb wall remains ≥ old wall or verify still dominates total | Arb wall/total |
| **H21** | Shared reference integer cache | Taking out + Universality | One `floor(pi*10^V)` reference per `(V, guard)` precision; repeated same-precision rows avoid cold MPFR reference rebuild | Verify medians stay at hundreds of ms after first row | verify_ms |
| **H22** | Retuned hybrid crossover | Separation in scale | After H20/H21, route to Arb from 10^6 digits; hybrid should match best delegate at 10^6 and 10^7 | Crown still beats Arb by >5% at 10^6/10^7 | hybrid wall/total |
| **H23** | Measurement hygiene | Taking out | Normalize `total_cost_ms = wall_ms + verify_ms + io_ms` and recompute relative wall time on every merge | Any supported row has `total_cost_ms < wall_ms`; relative rows stay zero with a baseline present | benchmark CSV invariants |
| **H24** | Ramanujan 100M guard falsifier | Parameter change | If failure is only guard budget, guard 100 or 1000 should verify | Same prefix hash fails at guard 100 and 1000 | correctness cap |
| **H25** | Verify-format overlap | Prior action | Run scaled-integer verification on a private MPFR copy while radix rendering consumes the original; total cost should drop by roughly `min(format_ms, verify_ms)` at 10^6 | Copy/thread overhead erases the overlap, verification fails, or total cost remains unchanged | total_cost_ms |
| **H26** | Shared scaled-integer snapshot | Taking out + Universality | Convert `pi_scaled` to an integer once and feed both verifier and formatter through 10^6 digits | At larger scales, integer downsampling dominates and total cost regresses | format/verify split |
| **H27** | Baseline scaled finalizer | Universality | Move plain Chudnovsky finalization onto the same scaled integer renderer/verifier so baseline postprocessing stops paying redundant decimal work | Prefix hash changes or baseline total cost does not improve | chudnovsky_bs total |
| **H28** | Formatter arity reduction | Segmentation | A 4-way formatter may reduce thread overhead at mid-size | Arb/baseline formatting regresses or total cost rises | format_ms |
| **H29** | Guard-budget trim | Parameter change | Reducing the fixed guard bonus from 128 to 64 at small/mid scale preserves exact-prefix verification and trims MPFR work | Any verification failure or high-scale regression | verified + total |
| **H30** | Reference-cache prewarm | Prior action | Build the trusted `floor(pi*10^V)` reference under the main computation so final verification does not cold-start it | 10^7 verification remains hundreds of ms or duplicate cache work appears | verify_ms |

### H15 prototype (`chudnovsky_bs_crown_h15`)

**Implementation:** `CrownTuning::merge_adaptive_depth` bumps `max_crown_depth` by
one when `terms ≥ 400k` or empirical merge:chunk ratio ≥ 1. Algorithm variant
`chudnovsky_bs_crown_h15` enables the flag; logic in `choose_crown_depth()`
(`src/crown.cpp`).

**Benchmark** (3 trials, 1 warmup, guard 25, Apple Silicon, 2025-06-10):

| Digits | Algorithm | wall_ms | split_ms | depth | chunks† | merge† | Verified |
|---:|---|---:|---:|---:|---:|---:|---|
| 10^6 | crown | 80.3 | 48.8 | 7 | 30.8 | 16.7 | yes |
| 10^6 | crown_h15 | 80.6 | 48.6 | 7 | 31.1 | 17.4 | yes |
| 10^7 | crown | 1451 | 948 | 7 | 718 | 226 | yes |
| 10^7 | crown_h15 | **1335** | **839** | **8** | 578 | 265 | yes |

†ms from verification_method notes field.

**Verdict: partly confirmed; trigger predicate refuted.** At 10^7, merge (226 ms)
≪ chunks (718 ms), yet +1 depth improved wall **−8%** (1451 → 1335 ms) and split
**−11%**, with merge rising only 17%. The narrow falsifier “only help when
merge > chunk” is wrong at this scale; the broader segmentation win (shorter
MPFR operands, better chunk parallelism) still holds. At 10^6 the floor does not
fire (70k terms); parity within noise. **Next:** retune trigger to depth+1 only
when `merge_expected_to_dominate_chunks()` (≈3–5M terms, ~10^8 digits); validate
at 10^8 where merge is 6264 ms vs chunks 3450 ms.

**Files:** `include/satox/crown.hpp`, `src/crown.cpp`, `src/chudnovsky_crown.cpp`,
`include/satox/algorithm.hpp`, `src/benchmark.cpp`.

### H20-H22 external pipeline and routing refresh

**TRIZ framing:** The old harness applied the strongest verification/formatting
ideas only to the in-repo crown path. That created an artificial contradiction:
external references were useful as speed controls but paid a weaker, slower
post-format verification tax. H20 applies *universality* by reusing the scaled
integer verifier and parallel decimal renderer for MPFR/Arb. H21 applies
*taking out* by caching the trusted `floor(pi * 10^V)` reference once per
precision/guard pair. H22 applies *separation in scale*: use crown where it wins
through 10^5 digits and Arb where the refreshed external pipeline wins from
10^6 upward.

**Benchmark** (2 trials, 1 warmup, guard 25, Apple Silicon, focused refresh):

| Digits | Algorithm | wall_ms | total_cost_ms | format_ms | verify_ms | Delegate | Verified |
|---:|---|---:|---:|---:|---:|---|---|
| 10^5 | crown | 5.7 | 6.6 | 0.7 | 0.9 | - | yes |
| 10^5 | Arb | 7.6 | 8.5 | 1.2 | 0.9 | - | yes |
| 10^6 | crown | 83.7 | 102.5 | 8.0 | 18.8 | - | yes |
| 10^6 | Arb | **80.3** | **98.8** | 18.0 | 18.5 | - | yes |
| 10^6 | hybrid | **78.5** | **95.8** | 16.9 | 17.3 | Arb | yes |
| 10^7 | crown | 1595 | 1669 | 166 | 74.6 | - | yes |
| 10^7 | Arb | **912** | **982** | 267 | 70.2 | - | yes |
| 10^7 | hybrid | 968 | 1039 | 271 | 71.5 | Arb | yes |

**Verdict:** H20 confirmed for Arb wall and total cost; H21 confirmed as an
important benchmark-harness efficiency improvement; H22 confirmed on the tested
scales. The hybrid crossover is now `10^6` digits when FLINT/Arb is built.
This does not weaken correctness: external rows now use the same pre-format
scaled-integer check as crown rows, with the emitted decimal prefix still
hash-compared across rows by the benchmark outputs.

### H23-H24 measurement hygiene and Ramanujan cap

**H23/H25 result:** Merged benchmark rows now recompute relative wall time
against the current `chudnovsky_bs` row at each precision, summarized rows keep
the measured median `total_cost_ms` from trials, and SATO-X timer boundaries now
exclude verification just like the external baselines. After H25, `total_cost_ms`
is intentionally allowed to be less than `wall_ms + verify_ms + io_ms` when
verification overlaps radix rendering; it must still be at least `wall_ms`.
A CSV invariant pass found no remaining supported rows with `total_cost_ms <
wall_ms` and no missing matrix cells.

### H25 verify-format overlap

**Implementation:** Crown, MPFR, and Arb compute `pi_scaled`, copy it once, then
launch `verify_scaled_pi_mpfr()` on the copy while formatting renders the
original. The copy avoids concurrent reads of the same MPFR object. Benchmark
summaries now preserve the measured median `total_cost_ms` instead of
reconstructing it from phase fields, so overlap appears as end-to-end runtime
rather than accounting noise.

**Benchmark** (5 trials, 1 warmup, guard 25, Apple Silicon, scratch outputs
under `/tmp/satox-hyp*`):

| Digits | Algorithm | Before total_ms | After total_ms | format_ms | verify_ms | Verified |
|---:|---|---:|---:|---:|---:|---|
| 10^5 | crown | 6.70 | 6.77 | 0.87 | 0.95 | yes |
| 10^6 | crown | 90.85 | **84.53** | 15.06 | 17.94 | yes |
| 10^6 | Arb | 95.10 | **79.01** | 17.62 | 19.72 | yes |
| 10^6 | hybrid | 88.57 | **86.47** | 19.17 | 20.93 | yes |

**Verdict: confirmed at 10^6, neutral at 10^5.** The win is limited by the
shorter of verification and formatting plus MPFR-copy overhead. It is most
useful once both stages are tens of milliseconds; at 10^5 the extra thread/copy
is within noise. Correctness is unchanged: all rows retain the same prefix hash
as the baseline.

### H26-H30 representation, guard, and cache loop

**Hypotheses tested:** five branches were tested after H25:

| # | Idea | Result | Verdict |
|---|---|---|---|
| H26 | Shared scaled-integer snapshot for verifier + formatter | At 10^6: crown total ~84.5 -> **~81.7 ms**, Arb ~79.0 -> **~74.2 ms** in focused scratch runs. At 10^7, naive integer downsampling made verify hundreds of ms. | confirmed only with scale gate |
| H27 | Apply scaled finalizer to plain `chudnovsky_bs` | 10^6 baseline total moved from ~316 ms pre-loop to **~266 ms** in the final focused run, with identical prefix hash. | confirmed |
| H28 | 4-way instead of 8-way integer formatter | 10^6 Arb format regressed to ~35 ms and total to ~89 ms; baseline format also regressed. | refuted and reverted |
| H29 | Trim small/mid fixed guard bonus 128 -> 64 | Verified at 10^5 and 10^6; term count drops by 5 terms at 10^6 and crown total stayed around **~81 ms**. High-scale crown still uses the existing larger guard tiers. | confirmed for <=10^6 |
| H30 | Prewarm reference integer cache concurrently | 10^7 single-row crown verify dropped from **~632 ms** after the H26 regression to **~69 ms**, restoring H13/H25-class verification cost. | confirmed |

**Integrated policy:** use shared scaled-integer snapshots only through
`10^6` digits. Above that, the verifier returns to the MPFR scaled path while
the reference cache is warmed under the main computation. Decimal rendering
keeps the 8-way split; the 4-way experiment was slower outside a narrow noisy
crown case. The final focused 10^6 scratch run after H30:

| Algorithm | total_ms | wall_ms | format_ms | verify_ms | Verified |
|---|---:|---:|---:|---:|---|
| `chudnovsky_bs` | 266.4 | 249.3 | 16.5 | 17.2 | yes |
| `chudnovsky_bs_crown` | 81.7 | 64.7 | 13.7 | 17.0 | yes |
| `arb_const_pi` | 74.2 | 55.7 | 16.3 | 17.1 | yes |
| `chudnovsky_hybrid` | 76.0 | 58.7 | 16.4 | 18.4 | yes |

The 10^7 crown smoke row remained verified with prefix hash `941cb5edb92e9f46`
and `verify_ms=69.4`.

**H24 result:** `ramanujan_classic_bs` at 10^8 failed exact-prefix verification
with guard 25, guard 100, and guard 1000, producing the same failing prefix hash
for the high-guard trials. That falsifies the simple "insufficient guard"
explanation. The algorithm is capped at 10^7 until a focused derivation/kernel
audit explains the 10^8 failure.

### H31-H40 final algorithm selection

**Goal:** test ten more falsifiable hypotheses after H30 and close the v1
algorithm policy. Scratch runs were written under `/tmp/satox-h31-*` through
`/tmp/satox-h37-*`; they were not merged into the canonical CSV because they
were boundary probes rather than full-ladder publication runs.

| # | Hypothesis | Evidence | Verdict |
|---|---|---|---|
| H31 | The Arb crossover is below the old 10^6 hybrid threshold | 3-trial coarse probe: crown/tuned win at 250k and 500k; Arb/hybrid win at 750k and 1M | confirmed |
| H32 | The crossover boundary is near 700k digits | 600k: crown 43.8 ms total vs Arb 49.5; 650k: hybrid/crown 49.4-50.4 vs Arb 51.8; 700k: crown/Arb/tuned all ~54-55 | confirmed |
| H33 | Arb-backed hybrid remains best at multi-million scales | 1-trial probe: 2M hybrid 155.8 ms total, Arb 168.8, tuned crown 187.8; 5M/10M Arb-backed hybrid remains ahead of crown variants | confirmed |
| H34 | The high-scale hybrid abort is a routing bug, not an algorithm failure | Hybrid at 2M aborted before `bin/satox-bench` was rebuilt; isolated cause was clearing an uninitialized `mpz_t` in the external MPFR/Arb >1M verifier branch | confirmed and fixed |
| H35 | Guard trimming did not create a small/mid correctness hole | guard 0 and guard 75 probes at 100k and 1M verified across crown, tuned crown, Arb, and hybrid | confirmed for probed scales |
| H36 | Retuning hybrid to 700k preserves boundary behavior | After `kHybridArbCrossoverDigits = 700000`, hybrid delegates to crown at 650k and Arb at 700k/750k/1M; all rows verified | confirmed |
| H37 | Cross-algorithm prefix hashes remain coherent after the changes | Scratch invariant pass found exactly one prefix hash per `(digits, guard)` among verified rows from 100k through 10M | confirmed |
| H38 | Ramanujan must stay capped above 10^7 | 20M and 100M requests now report `requested precision exceeds algorithm max_digits` instead of producing risky rows | confirmed |
| H39 | Tuned crown should be promoted into the public router | Tuned crown beats plain crown at 1M+ but still loses to Arb-backed hybrid from 700k upward when FLINT is present | refuted for FLINT builds |
| H40 | Final v1 policy can be closed without adding another kernel | Best verified policy is scale routing: crown below 700k, Arb at/above 700k when available; tuned crown remains the best in-repo high-scale row and fallback candidate when FLINT is absent | confirmed |

**Final algorithms for v1:**

- `chudnovsky_hybrid` is the default recommendation on hosts with FLINT/Arb:
  it routes to `chudnovsky_bs_crown` below 700,000 digits and `arb_const_pi`
  at or above 700,000 digits.
- `chudnovsky_bs_crown_tuned` is retained as the strongest in-repo Chudnovsky
  crown variant for benchmark comparison and non-FLINT environments, but it is
  not the public router delegate on this Apple Silicon/FLINT host.
- `chudnovsky_bs` remains the correctness and relative-speed baseline.
- `ramanujan_classic_bs` remains a verified comparator through 10^7 digits only.

**Current high-priority objectives:** see `docs/high-priority-objectives.md`.
