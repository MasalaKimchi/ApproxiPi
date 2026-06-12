<!-- generated-by: gsd-doc-writer -->

# Pi Computation Methods — SATO-X Comparison

This document compares every π algorithm implemented in ApproxiPi/SATO-X: mathematical basis, implementation strategy, asymptotic behavior, and measured runtime from `results/benchmark.csv` (median of **2 trials**, 0 warmups, 25 guard digits, Apple Silicon host with GMP/MPFR/FLINT). All variants pass exact-prefix verification against MPFR `const_pi` and BBP hex spot checks.

## Summary comparison

Each timing cell is **`wall_ms` / `total_cost_ms`** (milliseconds). **`wall_ms`** is compute-only (split, finalize, format; verify timed separately). **`total_cost_ms`** is the full verified pipeline (wall + verify + I/O) and matches the “Runtime ms” column in `results/summary.md`. Relative speed uses **`total_cost_ms` @1M** vs `chudnovsky_bs`.

| Algorithm key | Category | Convergence | Max digits | 10k (wall/total) | 100k (wall/total) | 1M (wall/total) | 10M (wall/total) | 100M (wall/total) | vs `chudnovsky_bs` @1M |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `chudnovsky_bs` | Series / BS | 14.18 digits/term | 100,000,000 | 1.34 / 1.37 | 32.6 / 33.5 | 307 / 324 | 5408 / 5477 | 93442 / 94023 | 1.00x |
| `chudnovsky_bs_valuation` | Series / BS | 14.18 digits/term | 100,000,000 | 1.43 / 1.47 | 31.6 / 32.4 | 302 / 785 | 5396 / 5464 | 92287 / 92870 | 2.42x |
| `chudnovsky_bs_crown` | Series / TCBS | 14.18 digits/term | 100,000,000 | 0.98 / 1.02 | 5.74 / 6.64 | 83.7 / 103 | 1595 / 1669 | 28447 / 29574 | **0.32x** |
| `chudnovsky_bs_crown_tuned` | Series / TCBS | 14.18 digits/term | 100,000,000 | 1.12 / 1.16 | 5.84 / 6.74 | 69.2 / 86.4 | 1199 / 1268 | 23080 / 23696 | **0.27x** |
| `ramanujan_classic_bs` | Series / BS | 7.98 digits/term | 10,000,000 | 1.11 / 1.14 | 17.2 / 18.1 | 225 / 233 | 4349 / 4886 | — | 0.72x |
| `machin_arctan` | Series / arctan | ~1.4 digits/term | 100,000 | 6.32 / 6.41 | 175 / 177 | — | — | — | — |
| `gauss_legendre_agm` | Iterative / AGM | ~2x digits/iter | 100,000 | 1.75 / 1.91 | 50.9 / 54.8 | — | — | — | — |
| `borwein_cubic` | Iterative / Borwein | ~3x digits/iter | 1,000,000 | 2.55 / 2.64 | 70.8 / 72.7 | 1167 / 1671 | — | — | 5.15x |
| `borwein_quartic` | Iterative / Borwein | ~4x digits/iter | 1,000,000 | 2.70 / 2.90 | 77.2 / 81.1 | 1372 / 1442 | — | — | 4.45x |
| `mpfr_const_pi` | External / MPFR | library-internal | 100,000,000 | 0.85 / 0.88 | 25.7 / 26.6 | 444 / 461 | 8314 / 8383 | 133903 / 134484 | 1.42x |
| `arb_const_pi` | External / FLINT | library-internal | 100,000,000 | 0.89 / 0.92 | 7.56 / 8.47 | 80.3 / 98.8 | 912 / 982 | 12888 / 13940 | **0.30x** |
| `chudnovsky_hybrid` | Router | scale-aware | 100,000,000 | 0.89 / 0.92 | 5.70 / 6.61 | 78.5 / 95.8 | 968 / 1039 | 12508 / 13086 | **0.30x** |

**Reading the table:** Lower **total** time is better for end-to-end comparisons; **wall** isolates compute. After H20/H21, FLINT/Arb (`arb_const_pi`) uses the same scaled-integer verification and parallel decimal rendering as the crown path. H31-H40 retuned the hybrid router to use crown below 700,000 digits and Arb at 700,000+. Rows marked — are unsupported at that scale. Ramanujan is capped at 10⁷ after 10⁸ failed even with guard 1000.

**Timing notes (H13/H20/H21/H23/H25/H31-H40):** Before H13, verification used post-format MPFR prefix checks (expensive at 10^6+ digits). H13 compares scaled integers in MPFR *before* decimal rendering (`V = min(D, 10^6)`). H20 applies that same path and the parallel decimal renderer to MPFR/Arb baselines; H21 caches the reference scaled integer per precision. H23 normalized merged `total_cost_ms = wall_ms + verify_ms + io_ms`; H25 aligned timer boundaries so `wall_ms` excludes verification for SATO-X and external rows. H31-H40 lower the hybrid Arb crossover from 10^6 to 700,000 digits after focused boundary probes.

---

## Innovations: crown truncation and valuation cancellation

SATO-X adds two engineering layers on top of standard binary splitting. They are independent: valuation cancellation operates in the exact integer kernel; the crown operates at merge time.

### Leaf valuation cancellation (`chudnovsky_bs_valuation`)

At each leaf of the binary-splitting tree, the hypergeometric term contributes integers $(P, Q, T)$. Before propagating upward, `reduce_common_pq_leaf` computes $g = \gcd(P, Q)$ and, when $g > 1$, divides $P$, $Q$, and $T$ by $g$. This removes shared prime-power factors that would otherwise inflate operand sizes through the merge without changing the rational sum.

The valuation variant enables `leaf_pq_cancellation` on the Chudnovsky spec. Benchmarks show a modest operand-bit reduction (e.g. max operand bits 591,473 vs 616,721 at 100k digits) with near-parity wall time — the GCD work roughly offsets the savings from smaller integers at most precisions.

### Truncated-crown binary splitting (TCBS) (`chudnovsky_bs_crown`)

The crown kernel splits the evaluation tree into two regions:

1. **Bottom (exact chunks):** The series range $[0, N)$ is partitioned into $2^k$ independent chunks, each evaluated with the existing exact GMP binary-splitting kernel (parallel when depth permits).
2. **Top (the crown):** The upper $k$ merge levels run in MPFR fixed-point arithmetic instead of exact integers.

Three mechanisms make the crown fast:

- **Per-node precision capping:** Each subtree's contribution has a rigorously bounded binary *shift* (how many low-order bits it can affect). Values are truncated to `needed_precision = target + 64 guard bits − shift`, so root merges use short operands instead of full-width integers (max operand bits drops from 6,860,005 to 54,002 at 10⁶ digits).
- **Dead P elision:** On the rightmost spine, $P$ products are skipped when provably never consumed in the final sum form $\pi = C \cdot Q / T$.
- **Root-Q elision:** Since $\pi = 426880\sqrt{10005} \cdot Q_l Q_r / T$, the root $Q$ product is never formed; numerator assembly and a warm-started Newton reciprocal for $1/T$ overlap the root merge.

At large precisions (≥20,000 digits), additional pipelining overlaps constant precomputation ($10^D$, $426880\sqrt{10005}$), parallel decimal rendering, and carry-checked spliced-prefix formatting (H12). The autotuned variant (`chudnovsky_bs_crown_tuned`) loads knob settings from `results/tuning.json` produced by `satox-bench --tune`; gains over the hand-tuned crown are marginal (~2% in the H11 experiment).

---

## Series methods

### Chudnovsky binary splitting — `chudnovsky_bs`

**Full name:** Chudnovsky series with exact integer binary splitting.

**Formula.** The implementation evaluates the hypergeometric series in $(P, Q, T)$ form and finishes with:

$$\pi = \frac{426880\sqrt{10005}\,Q}{T}$$

where the sum satisfies the Chudnovsky recurrence. Per term $k$:

- $P_k = (6k-5)(2k-1)(6k-1)$ (with $P_0 = 1$)
- $Q_k = k^3 \cdot C_q$ with $C_q = 10{,}939{,}058{,}860{,}032{,}000$
- $T_k = (-1)^k(13{,}591{,}409 + 545{,}140{,}134\,k)$

Equivalently, the classical closed form:

$$\frac{1}{\pi} = 12\sum_{k=0}^{\infty}\frac{(-1)^k(6k)!\,(13{,}591{,}409 + 545{,}140{,}134k)}{(3k)!\,(k!)^3\,640{,}320^{3k+3/2}}$$

**Convergence:** ~**14.18 decimal digits per term** (`kChudnovskyDigitsPerTerm`).

**Implementation:** Exact integer binary splitting with bounded parallel subtree evaluation, 8-term iterative leaf blocks, `mpz_addmul` merge optimization, and optional parallel depth up to 4. Split / finalize / format / verify phases are timed separately.

**Strengths:**
- State-of-practice asymptotics: $O(\mathcal{M}(n)\log n)$ with excellent constant factors for a pure-integer path
- Highest digits-per-term among implemented series methods
- Serves as the benchmark baseline (relative wall time = 1.0)

**Weaknesses:**
- Operand sizes grow to millions of bits at 10⁶ digits (6.86M max operand bits)
- Finalize phase (MPFR scaling, $\sqrt{10005}$, division) becomes significant at high precision
- No crown truncation — full-width integer merges throughout

**Benchmark highlights:** 3.2 ms total @10k · 36.8 ms total @100k · **320.9 ms total @1M** (531.5 ms wall)

---

### Chudnovsky with valuation cancellation — `chudnovsky_bs_valuation`

**Full name:** Chudnovsky binary splitting with leaf $\gcd(P,Q)$ cancellation.

**Formula:** Identical to `chudnovsky_bs`.

**Convergence:** 14.18 digits/term.

**Implementation:** Same as baseline, plus `leaf_pq_cancellation = true` — GCD reduction at every leaf before tree merge.

**Strengths:**
- Slightly smaller intermediate integers (measurable `cancelled_bits` and lower `max_operand_bits`)
- Mathematically sound simplification with zero risk to correctness

**Weaknesses:**
- Per-leaf GCD cost largely cancels savings at tested precisions
- Does not address crown-level operand blow-up

**Benchmark highlights:** 0.048 ms @1k · 0.642 ms @10k · 12.6 ms @100k · 139.2 ms @1M (≈1.0× baseline)

---

### Chudnovsky truncated crown — `chudnovsky_bs_crown` / `chudnovsky_bs_crown_tuned`

**Full name:** Chudnovsky series with truncated-crown binary splitting (TCBS).

**Formula:** Identical Chudnovsky series; only the evaluation kernel differs.

**Convergence:** 14.18 digits/term (same term count as baseline).

**Implementation:** TCBS via `binary_split_crown()` — exact parallel chunks + MPFR crown merge with per-node truncation, dead-P skipping, root-Q elision, warm-started Newton $1/T$, and pipelined format at ≥20k digits. Tuned variant reads `CrownTuning` from `results/tuning.json`.

**Strengths:**
- **3.6× faster** than baseline at 10⁶ digits on total cost (89.2 vs 320.9 ms; 83.0 vs 531.5 ms wall)
- 127× smaller max operand bits at 10⁶ digits
- Outperforms MPFR (7.7×) and FLINT/Arb (1.67×) at 10⁶ on this host
- Chunk-level parallelism (depth 4–8 depending on precision)

**Weaknesses:**
- More complex code path; crown rounding requires ≥64 MPFR guard bits (satisfied by +128 decimal guard band)
- Small-precision overhead: Arb wins below ~10⁴ digits
- Autotuning (H11) yields <2% after hand tuning — knob space is already near-optimal

**Benchmark highlights:** 1.37 ms total @10k · **8.00 ms total @100k** · **89.2 ms total @1M** (83.0 ms wall)

---

### Ramanujan classical binary splitting — `ramanujan_classic_bs`

**Full name:** Ramanujan's 1914 modular equation series with binary splitting.

**Formula:**

$$\frac{1}{\pi} = \frac{2\sqrt{2}}{9801}\sum_{k=0}^{\infty}\frac{(4k)!\,(1103 + 26390k)}{(k!)^4\,396^{4k}}$$

Implemented as hypergeometric BS with $P_k = (4k+1)(4k+2)(4k+3)(4k+4)$, four $(k+1)$ factors in $Q$, $C_q = 396^4$, and $T_k = (1103 + 26390k)$ (with $T$ using $Q$ at leaves). Finalize:

$$\pi = \frac{9801\,Q}{2\sqrt{2}\,T}$$

**Convergence:** ~**7.98 digits/term** — roughly half the Chudnovsky rate. Term count uses $\lceil D / 7.98 \rceil + 8$.

**Implementation:** Exact integer binary splitting (shared kernel), parallel depth as recommended.

**Strengths:**
- Faster per-term convergence than Machin or classical arctan series
- Well-studied modular-origin series; good cross-check against Chudnovsky

**Weaknesses:**
- ~1.8× more terms than Chudnovsky at equal precision (e.g. 125,301 vs 70,526 at 10⁶)
- ~1.44× slower than crown Chudnovsky at 10⁶ digits
- Four $P$ factors per leaf vs Chudnovsky's three

**Benchmark highlights:** 0.067 ms @1k · 0.963 ms @10k · 15.1 ms @100k · 202.4 ms @1M

---

### Machin arctangent — `machin_arctan`

**Full name:** Machin's formula via MPFR `atan`.

**Formula:**

$$\pi = 16\arctan\!\left(\frac{1}{5}\right) - 4\arctan\!\left(\frac{1}{239}\right)$$

Each arctangent is computed by MPFR's internal series at the target precision.

**Convergence:** ~**1.4 digits per arctan term** (estimated from precision / term budget). Dominated by finalize (MPFR `atan`), not a split phase.

**Implementation:** Pure MPFR — no binary splitting. `max_digits = 100,000`.

**Strengths:**
- Elegant closed form; historically important
- Simple code path; no integer blow-up

**Weaknesses:**
- Slowest implemented series method (14.2× baseline at 100k digits)
- Unsupported above 100,000 digits in this build
- No split-phase parallelism

**Benchmark highlights:** 0.262 ms @1k · 5.55 ms @10k · 171.9 ms @100k · *unsupported @1M*

---

## Iterative methods

### Gauss–Legendre AGM — `gauss_legendre_agm`

**Full name:** Borwein–Borwein Gauss–Legendre arithmetic-geometric mean iteration.

**Formula.** Initialize $a_0 = 1$, $b_0 = 1/\sqrt{2}$, $t_0 = 1/4$, $p_0 = 1$. Iterate:

$$a_{n+1} = \frac{a_n + b_n}{2}, \quad b_{n+1} = \sqrt{a_n b_n}$$

$$t_{n+1} = t_n - p_n(a_n - a_{n+1})^2, \quad p_{n+1} = 2p_n$$

Then:

$$\pi \approx \frac{(a_n + b_n)^2}{4t_n}$$

**Convergence:** **Quadratic** — approximately **doubling correct digits per iteration** ($\sim 2\times$ digits/iter). Iteration count: $\lceil \log_2(D + \text{guard}) \rceil + 2$.

**Implementation:** Adaptive MPFR precision per iteration (precision ramps with remaining iterations); full-precision fallback if adaptive path fails verification. `max_digits = 100,000`.

**Strengths:**
- Very few iterations (13–19 across benchmark range)
- No hypergeometric term explosion
- Classical, well-understood iteration

**Weaknesses:**
- Each iteration needs a square root and several full-width MPFR ops at ramping precision
- 4.15× slower than baseline Chudnovsky at 100k; unsupported at 10⁶
- Finalize-dominated (no split phase)

**Benchmark highlights:** 0.095 ms @1k · 1.72 ms @10k · 50.3 ms @100k · *unsupported @1M*

---

### Borwein cubic — `borwein_cubic`

**Full name:** Borwein cubic-convergence iteration.

**Formula.** With $s_0 = (\sqrt{3}-1)/2$, $a_0 = 1/3$, $p_0 = 1$:

$$r_{k+1} = \frac{3}{1 + 2(1-s_k^3)^{1/3}}, \quad s_{k+1} = \frac{r_{k+1}-1}{2}$$

$$a_{k+1} = a_k r_{k+1}^2 - p_k(r_{k+1}^2 - 1), \quad p_{k+1} = 3p_k$$

$$\pi \approx \frac{1}{a_n}$$

**Convergence:** **Cubic** — ~**tripling digits per iteration**. Iterations: $\lceil \log(D)/\log 3 \rceil + 6$.

**Implementation:** Full-precision MPFR throughout (no adaptive stepping). 15–19 iterations at benchmark precisions.

**Strengths:**
- Fewer iterations than AGM for comparable precision
- Supported to 10⁶ digits

**Weaknesses:**
- Cubic root and polynomial work per step at full precision
- 8.08× slower than crown Chudnovsky at 10⁶ (1135 ms)
- No parallelism

**Benchmark highlights:** 0.120 ms @1k · 2.42 ms @10k · 69.0 ms @100k · 1135 ms @1M

---

### Borwein quartic — `borwein_quartic`

**Full name:** Borwein quartic-convergence iteration.

**Formula.** With $y_0 = \sqrt{2}-1$, $a_0 = 6 - 4\sqrt{2}$:

$$y_{k+1} = \frac{1 - (1-y_k^4)^{1/4}}{1 + (1-y_k^4)^{1/4}}$$

$$a_{k+1} = a_k(1+y_{k+1})^4 - 2^{2k+3}\,y_{k+1}(1 + y_{k+1} + y_{k+1}^2)$$

$$\pi \approx \frac{1}{a_n}$$

**Convergence:** **Quartic** — ~**quadrupling digits per iteration**. Iterations: $\lceil \log(D)/\log 4 \rceil + 1$ (7–11 in benchmarks).

**Implementation:** Adaptive MPFR precision per iteration (like AGM), with full-precision fallback.

**Strengths:**
- Highest-order convergence among implemented iterative methods
- Fewest iterations (7 at 1k digits, 11 at 1M)

**Weaknesses:**
- Fourth roots and larger polynomial updates per step
- Slightly slower than cubic at most precisions in this build (format/verify overhead at high precision)
- 9.72× slower than crown at 10⁶

**Benchmark highlights:** 0.137 ms @1k · 2.42 ms @10k · 76.3 ms @100k · 1365 ms @1M

---

## External baselines

These are not SATO-X algorithms; they invoke library routines for $\pi$ and pass the result through the same format and verify **correctness** gate (prefix hash). Timer boundaries are now apples-to-apples: `wall_ms` excludes verification, while `total_cost_ms` includes verification exactly once.

### MPFR `const_pi` — `mpfr_const_pi`

**Full name:** MPFR `mpfr_const_pi` (cold cache, single-threaded).

**Formula:** MPFR's internal implementation (typically AGM- or series-based; not exposed).

**Convergence:** N/A — one library call (`terms_or_iterations = 1`).

**Implementation:** `mpfr_free_cache()` before each trial for cold timing; result formatted via `mpfr_to_decimal_prefix`.

**Strengths:**
- Gold-standard reference used for verification of all other rows
- Competitive at 10⁴ digits (0.74 ms vs crown 0.54 ms)

**Weaknesses:**
- 3.26× slower than crown at 10⁶ digits (458 ms)
- Single-threaded; no SATO-X phase overlap

**Benchmark highlights:** 0.84 ms total @10k · 28.3 ms total @100k · **501.7 ms total @1M** (480.8 ms wall)

---

### FLINT/Arb `const_pi` — `arb_const_pi`

**Full name:** FLINT/Arb `arb_const_pi` (cold cache, all hardware threads).

**Formula:** Arb's internal ball-arithmetic $\pi$ (typically Chudnovsky or related series with rigorous error bounds).

**Convergence:** N/A — one library call.

**Implementation:** `flint_set_num_threads(hardware_concurrency())`, `flint_cleanup()` for cold cache; Arb result converted to MPFR, scaled once by `10^D`, verified with the shared H13 scaled-integer reference cache, and formatted with the parallel decimal renderer. Requires `SATOX_HAVE_FLINT` at build time.

**Strengths:**
- Fast from 10⁶ upward after H20/H21 (80 ms wall @1M; 912 ms @10M)
- Strong at 10⁵, though crown/hybrid still wins total cost there
- Rigorous interval arithmetic in Arb itself

**Weaknesses:**
- Decimal rendering is still heavier than crown at large sizes
- Not built if FLINT is absent

**Benchmark highlights:** 1.44 ms total @10k · 9.59 ms total @100k · **98.0 ms total @1M** (80.3 ms wall)

---

## Choosing a method

| Goal | Recommendation |
|---|---|
| Maximum speed at ≥10⁶ verified digits | `chudnovsky_hybrid` when FLINT is built |
| Maximum speed at 10⁴-10⁵ verified digits | `chudnovsky_bs_crown` |
| Simplicity / reference baseline | `chudnovsky_bs` |
| Small precision (≤10³ digits) on this host | `arb_const_pi` (if FLINT built) |
| Cross-implementation verification | Run several families; all share prefix hash + BBP spots |
| Teaching / historical interest | `machin_arctan`, `gauss_legendre_agm` |
| Avoid integer BS entirely | Borwein quartic or AGM (within their digit caps) |

The crown variant demonstrates that for Chudnovsky-class series, the dominant wins at extreme precision come from **operand-size control and parallel structure** (truncated crown, root-Q elision, pipelined finalize/format), not from asymptotically faster series. Ramanujan offers a viable alternative series but cannot match Chudnovsky's digits-per-term. Iterative methods converge in few steps but each step is expensive in MPFR at million-digit scale.

---

## Reproducibility

```bash
make                    # build bin/satox-bench, bin/satox-tests (not vendored in git)
bin/satox-bench         # regenerate results/benchmark.csv, summary.md
bin/satox-bench --tune  # optional crown autotuning → results/tuning.json
make figures            # docs/figures/*.svg from benchmark data
```

See `docs/PAPER.md` for the hypothesis-driven optimization narrative and `docs/research-log.md` for the full H1–H13 ledger.
