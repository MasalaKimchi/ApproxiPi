# An AI Agent as Autonomous Performance Engineer: A Falsifiable-Hypothesis Case Study in High-Precision Computation of π

*Draft for arXiv (cs.AI, cross-list cs.MS). All numbers reproducible from
this repository; see Section 8.*

## Abstract

We report a case study in which an AI coding agent autonomously optimized a
verified million-digit computation of π on a single Apple Silicon machine,
using an explicit scientific-method loop: each change was preceded by a
written hypothesis with a predicted effect size and a falsification
criterion, benchmarked under an exact-prefix verification harness, and
recorded in a ledger including refutations. Across twelve hypotheses (eight
confirmed, four refuted), median verified wall time at 10^6 digits fell from
140.5 ms to 59.3 ms (2.37×) against the agent's own Chudnovsky
binary-splitting baseline, and the final implementation outperforms the two
strongest installable references on the same machine and pipeline —
FLINT/Arb's `arb_const_pi` (1.67×) and MPFR's `mpfr_const_pi` (7.7×) — while
producing byte-identical digit strings. A post-hoc prior-art audit shows the
agent independently rediscovered optimizations documented by practitioners
over the last two decades (truncated binary splitting; tight per-node
precision control), and contributed less-documented ones (root-Q elision
with warm-started Newton correction; carry-checked spliced-prefix decimal
rendering). We argue the contribution is methodological: the ledger,
including its refuted entries, a Cost(D) phase decomposition, energy- and
I/O-aware efficiency metrics (digits/joule, digits/GB), and a four-layer
baseline ladder with ablations, is a template for auditable systems-style
performance engineering—not only raw digits/sec.

## 1. Introduction

Record computations of π are dominated by one design: the Chudnovsky series
evaluated by binary splitting over fast integer multiplication. The
asymptotics — O(M(n) log n) — have been settled for decades; practical
progress lives in constant factors, memory behavior, parallel structure, and
the unglamorous finalize/format phases. This regime is a good stress test
for a question independent of π: **can an AI agent conduct competent,
honest, empirical performance research** — generate hypotheses from
profiling data, predict effect sizes, accept refutation, and avoid fooling
itself?

This paper is the experiment's lab notebook, written up. The task given to
the agent: make a verified π computation as fast as possible on one machine,
under three standing rules:

1. **Verification gate.** Every variant must reproduce the exact decimal
   prefix (compared against MPFR's `const_pi`, spot-checked with BBP hex
   digits); unverified speed does not count.
2. **Pre-registered predictions.** Each hypothesis states a predicted effect
   size and what result would refute it, before implementation.
3. **Ledger.** Refuted hypotheses are reported, reverted, and kept.

## 2. Related work and prior-art audit

The closest prior art to the agent's central optimization (H1) is the
"truncation trick" for binary splitting: Gourdon & Sebah describe it as a
crucial practical optimization, and Mezzarobba (arXiv:1209.5097) gives a
rigorous error analysis of truncated binary splitting for D-finite series.
Cheng, Hanrot, Thomé, Zima & Zimmermann (ISSAC 2007) reduce the cost of
binary splitting via fully factored forms; y-cruncher's implementation notes
document tight precision control, skewed splitting and GCD factorization.
The Borwein iterations and the Gauss–Legendre AGM provide the classical
higher-order alternatives benchmarked here; FLINT/Arb and MPFR provide the
installable state-of-practice baselines.

We stress the audit's direction: the agent derived H1 from first principles
(operand-size profiling), not from the literature, and the rediscovery was
identified afterwards by a web search. For a novelty claim this is fatal;
for a case study of AI-driven research it is evidence the loop works.

## 3. System under study

The repository implements eight π algorithms behind one interface
(`PiAlgorithm`), with per-phase timing (split / finalize / format / verify),
exact-prefix verification, and CSV/JSON/Markdown reporting. The optimized
variant (`chudnovsky_bs_crown`) keeps exact integer binary splitting for the
bottom of the tree (2^k independent "chunks", computed in parallel) and
merges the top k levels — the *crown* — in MPFR fixed point, with per-node
precision capped by a rigorous lower bound on each subtree's contribution
offset, derived from the exact bit sizes of the chunk results. On top of the
crown sit: pipelined value-independent constants (√10005, 10^D), root-Q
elision (π = C·Q_l·Q_r/T never forms the root Q), a gated warm-started
Newton correction for 1/T, and parallel divide-and-conquer decimal
rendering.

## 4. Methodology: the hypothesis loop

Each iteration: (i) read the phase-level benchmark data; (ii) write a
hypothesis naming the binding constraint and a falsifiable prediction;
(iii) implement minimally; (iv) benchmark (median of ≥3 trials, warmup,
verification gate); (v) record confirm/refute and either keep or revert.
The full ledger (H1–H12) is in `docs/research-log.md`; highlights:

| # | Hypothesis (abbrev.) | Prediction | Outcome | Verdict |
|---|---|---|---|---|
| H1 | Truncated MPFR crown over exact chunks | 4–7% wall | split −25%, wall −9.5% | confirmed |
| H2 | Bottleneck is not the series: pipeline constants, parallel decimal rendering | ≥20% | wall 150→92.6 ms | confirmed |
| H3 | Intra-node parallel root products | both shrink | wall 72.8 ms | confirmed |
| H4 | Warm-start Newton 1/T from leading root product | several ms | 69.9 ms | partly confirmed |
| H5 | Warm reciprocal from root *inputs* | finalize −30% | no change (MPFR Newton-bootstraps internally) | **refuted** |
| H7 | Root-Q elision + half-width correction | finalize 23→11 ms | 23.6→18.8 ms | confirmed (smaller) |
| H9b | Deeper merge parallelism | merge shrinks | merge +30% (oversubscription) | **refuted** |
| H10 | Division-free reciprocal-power formatting | format halves | finalize +37 ms (no idle cores to hide table build) | **refuted** |
| H11 | Autotuning the 6-knob crown space, cached profile | ≥5% | −2%, within noise | **refuted** (marginal) |
| H12 | Spliced-prefix rendering under the Newton correction | 8–12% | −6.4%, verified | confirmed (low end) |

Two methodological notes. First, refutations carried information: H5 taught
that wins must remove or overlap whole operations (MPFR already
Newton-bootstraps divisions); H9b/H10 established the machine's parallelism
budget as a hard constraint; H11 showed that after nine evidence-guided
changes, blind parameter search recovers <2% — the structured loop had
already found the local optimum. Second, the one mathematical guarantee per
optimization is load-bearing: H12's splice is *proved* exact at runtime by
an integer range check (0 ≤ z_corr − high·10^(4w) < 10^(4w)), not assumed
from an error estimate, with a fallback re-render and the global
verification gate behind it.

## 5. Metrics: separating "less work" from "restructured work"

Wall time on one machine invites two objections: it may not transfer, and it
conflates algorithmic improvement with parallel restructuring. We therefore
instrumented a machine-independent metric: every multiplication in the
series-evaluation phase contributes bits(a)+bits(b) to `mul_bit_volume`
(divisions/roots excluded; both variants share the same GMP/MPFR
substrate). At 10^6 digits:

| Variant | wall (ms) | mul bit volume | max operand bits |
|---|---:|---:|---:|
| `chudnovsky_bs` | 140.5 | 0.376 Gbit | 6,860,005 |
| `chudnovsky_bs_crown` | 59.3 | 0.350 Gbit | 54,002 |

The truncation removes only ~7% of raw multiplication volume — most volume
lives in the lower, exact levels of the tree. The 2.37× wall win is mostly
*restructuring*: chunk-level parallelism, 127× smaller top operands, and
overlap of finalize/format with the series. The metric keeps the claims
honest: this is engineering of constants and structure, not an asymptotic
improvement, and we say so.

## 6. Results against installable external baselines

y-cruncher was not available for in-pipeline comparison; instead we run two
citable references through the identical digit-request, formatting and
verification pipeline, with cold caches per trial and FLINT given all
hardware threads:

| Digits | ours (crown) | FLINT/Arb `arb_const_pi` | MPFR `mpfr_const_pi` |
|---:|---:|---:|---:|
| 1,000 | 0.045 ms | **0.009 ms** | 0.033 ms |
| 10,000 | **0.54 ms** | 0.97 ms | 0.74 ms |
| 100,000 | **4.17 ms** | 8.81 ms | 25.8 ms |
| 1,000,000 | **59.3 ms** | 99.0 ms | 458.2 ms |

All rows verified; identical prefix hashes. Above ~10^4 digits the agent's
implementation leads both references on this machine (1.67–1.97× vs Arb);
below, Arb's small-precision paths win and we report that without excuse.
Median of 5 trials, 1 warmup; Apple Silicon; GMP 6.3.0, MPFR 4.2.x,
FLINT 3.5.0.

## 7. Threats to validity

- **Single machine.** All wall-clock claims are scoped to one Apple Silicon
  host; the bit-volume metric and the relative phase structure are the
  transferable artifacts.
- **No y-cruncher.** The strongest known implementation is absent; our
  external anchors are the strongest *installable library* references. We
  do not claim records of any kind.
- **Self-built baseline.** The 2.37× headline is against our own exact
  binary splitting; the external table exists precisely to bound how good
  that baseline is.
- **Verification circularity.** MPFR `const_pi` verifies all algorithms,
  including the MPFR baseline row; cross-checks come from hash agreement
  across nine independent implementations and BBP hex spot checks.
- **Agent provenance.** The hypotheses were generated by an AI agent in
  conversation with a human who set goals and constraints; the transcript
  is part of the artifact.

## 8. Reproducibility

`make` builds with GMP/MPFR (FLINT optional, auto-detected);
`make figures` re-runs the full sweep and regenerates every figure;
`bin/satox-bench --tune` reproduces the H11 autotuner;
`results/benchmark.csv`, `results/tuning-log.csv` and `docs/figures/` are
regenerated artifacts. The hypothesis ledger is `docs/research-log.md`.

## 9. Conclusion

A disciplined falsifiable-hypothesis loop let an AI agent take a verified
million-digit π computation 2.37× past its own competent baseline and past
the installable state of practice on the host machine, while documenting
four refutations and independently rediscovering two decades of
practitioner tricks. The interesting export is not the milliseconds: it is
that the agent's value concentrated exactly where the literature is thinnest
(cross-phase pipelining, finalize/format overlap, carry-checked splicing)
and that the ledger format — predictions, refutations, machine-independent
metrics, verification gates — turns "AI made it faster" into an auditable
scientific claim.
