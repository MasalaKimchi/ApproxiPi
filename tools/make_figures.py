#!/usr/bin/env python3
"""Generate optimized SVG figures from SATO-X benchmark CSV output."""

from __future__ import annotations

import argparse
import csv
import html
import math
from pathlib import Path
from typing import Callable, Iterable


PALETTE = {
    "chudnovsky_bs": "#2155d9",
    "chudnovsky_bs_valuation": "#0891b2",
    "chudnovsky_bs_crown": "#dc2626",
    "chudnovsky_bs_crown_tuned": "#f59e0b",
    "ramanujan_classic_bs": "#c2410c",
    "ramanujan_classic": "#c2410c",
    "machin_arctan": "#b45309",
    "gauss_legendre_agm": "#15803d",
    "borwein_cubic": "#9333ea",
    "borwein_quartic": "#7c3aed",
    "mpfr_const_pi": "#475569",
    "arb_const_pi": "#0f766e",
}

LABELS = {
    "chudnovsky_bs": "Chudnovsky BS",
    "chudnovsky_bs_valuation": "Chudnovsky valuation",
    "chudnovsky_bs_crown": "Truncated crown (ours)",
    "chudnovsky_bs_crown_tuned": "Crown + autotune",
    "ramanujan_classic_bs": "Ramanujan BS",
    "ramanujan_classic": "Ramanujan",
    "machin_arctan": "Machin arctan",
    "gauss_legendre_agm": "AGM",
    "borwein_cubic": "Borwein cubic",
    "borwein_quartic": "Borwein quartic",
    "mpfr_const_pi": "MPFR const_pi",
    "arb_const_pi": "FLINT/Arb const_pi",
}

GRID = "#d7dde8"
AXIS = "#1f2937"
TEXT = "#111827"
MUTED = "#64748b"
OK = "#15803d"
SKIP = "#94a3b8"
FAIL = "#b91c1c"


def load_rows(path: Path) -> list[dict[str, object]]:
    with path.open(newline="", encoding="utf-8") as handle:
        rows: list[dict[str, object]] = []
        for raw in csv.DictReader(handle):
            row: dict[str, object] = dict(raw)
            row["digits"] = int(str(raw["digits"]))
            row["guard_digits"] = int(str(raw["guard_digits"]))
            row["supported"] = str(raw["supported"]).lower() == "true"
            row["verified"] = str(raw["verified"]).lower() == "true"
            row["wall_ms"] = float(str(raw["wall_ms"]))
            row["cpu_ms"] = float(str(raw["cpu_ms"]))
            row["terms_or_iterations"] = int(str(raw["terms_or_iterations"]))
            row["estimated_digits_per_term"] = float(str(raw["estimated_digits_per_term"]))
            row["relative_wall_time"] = float(str(raw["relative_wall_time"]))
            row["split_ms"] = float(str(raw.get("split_ms", 0) or 0))
            row["finalize_ms"] = float(str(raw.get("finalize_ms", 0) or 0))
            row["format_ms"] = float(str(raw.get("format_ms", 0) or 0))
            row["mul_bit_volume"] = float(str(raw.get("mul_bit_volume", 0) or 0))
            rows.append(row)
    return rows


def write(path: Path, svg: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(svg, encoding="utf-8")


def fmt(value: float) -> str:
    return f"{value:.2f}".rstrip("0").rstrip(".")


def svg_document(width: int, height: int, title: str, desc: str, body: str) -> str:
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" '
        f'role="img" aria-labelledby="title desc">'
        f"<title id=\"title\">{html.escape(title)}</title>"
        f"<desc id=\"desc\">{html.escape(desc)}</desc>"
        "<style>"
        "text{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif}"
        ".title{font-size:22px;font-weight:700;fill:#111827}"
        ".subtitle{font-size:12px;fill:#64748b}"
        ".axis{stroke:#1f2937;stroke-width:1.4}"
        ".grid{stroke:#d7dde8;stroke-width:1}"
        ".tick{font-size:11px;fill:#64748b}"
        ".label{font-size:12px;fill:#111827}"
        ".legend{font-size:12px;fill:#111827}"
        "</style>"
        f"{body}</svg>"
    )


def log_scale(values: Iterable[float], lo_px: float, hi_px: float) -> Callable[[float], float]:
    vals = [v for v in values if v > 0]
    lo = math.floor(math.log10(min(vals)))
    hi = math.ceil(math.log10(max(vals)))
    if lo == hi:
        hi += 1

    def scale(value: float) -> float:
        return lo_px + (math.log10(value) - lo) / (hi - lo) * (hi_px - lo_px)

    scale.domain = (lo, hi)  # type: ignore[attr-defined]
    return scale


def linear_scale(values: Iterable[float], lo_px: float, hi_px: float) -> Callable[[float], float]:
    vals = list(values)
    lo = 0.0
    hi = max(vals) if vals else 1.0
    if hi <= 0:
        hi = 1.0
    hi *= 1.12

    def scale(value: float) -> float:
        return lo_px + (value - lo) / (hi - lo) * (hi_px - lo_px)

    scale.domain = (lo, hi)  # type: ignore[attr-defined]
    return scale


def line_chart(
    rows: list[dict[str, object]],
    metric: str,
    title: str,
    subtitle: str,
    ylabel: str,
    output: Path,
    y_log: bool,
) -> None:
    plot = {"x0": 82, "y0": 74, "x1": 850, "y1": 430}
    width, height = 960, 540
    usable = [
        r
        for r in rows
        if r["supported"] and r["verified"] and float(r[metric]) > 0 and int(r["digits"]) > 0
    ]
    xscale = log_scale([float(r["digits"]) for r in usable], plot["x0"], plot["x1"])
    if y_log:
        y_forward = log_scale([float(r[metric]) for r in usable], plot["y1"], plot["y0"])
    else:
        y_forward = linear_scale([float(r[metric]) for r in usable], plot["y1"], plot["y0"])

    parts = [
        f'<rect width="{width}" height="{height}" fill="#fff"/>',
        f'<text class="title" x="40" y="38">{html.escape(title)}</text>',
        f'<text class="subtitle" x="40" y="58">{html.escape(subtitle)}</text>',
    ]

    x_lo, x_hi = xscale.domain  # type: ignore[attr-defined]
    for power in range(int(x_lo), int(x_hi) + 1):
        value = 10**power
        x = xscale(value)
        parts.append(f'<line class="grid" x1="{fmt(x)}" y1="{plot["y0"]}" x2="{fmt(x)}" y2="{plot["y1"]}"/>')
        parts.append(f'<text class="tick" x="{fmt(x)}" y="454" text-anchor="middle">1e{power}</text>')

    y_lo, y_hi = y_forward.domain  # type: ignore[attr-defined]
    if y_log:
        y_ticks = [10**p for p in range(int(y_lo), int(y_hi) + 1)]
    else:
        step = y_hi / 5
        y_ticks = [step * i for i in range(0, 6)]
    for value in y_ticks:
        y = y_forward(value if value > 0 else 0)
        label = f"1e{int(math.log10(value))}" if y_log and value > 0 else fmt(value)
        parts.append(f'<line class="grid" x1="{plot["x0"]}" y1="{fmt(y)}" x2="{plot["x1"]}" y2="{fmt(y)}"/>')
        parts.append(f'<text class="tick" x="72" y="{fmt(y + 4)}" text-anchor="end">{label}</text>')

    parts.append(f'<line class="axis" x1="{plot["x0"]}" y1="{plot["y1"]}" x2="{plot["x1"]}" y2="{plot["y1"]}"/>')
    parts.append(f'<line class="axis" x1="{plot["x0"]}" y1="{plot["y0"]}" x2="{plot["x0"]}" y2="{plot["y1"]}"/>')
    parts.append('<text class="label" x="466" y="494" text-anchor="middle">Decimal digits</text>')
    parts.append(f'<text class="label" transform="translate(22 254) rotate(-90)" text-anchor="middle">{html.escape(ylabel)}</text>')

    algorithms = sorted({str(r["algorithm"]) for r in usable})
    for index, algorithm in enumerate(algorithms):
        series = sorted([r for r in usable if r["algorithm"] == algorithm], key=lambda r: int(r["digits"]))
        color = PALETTE.get(algorithm, "#6d28d9")
        points = [(xscale(float(r["digits"])), y_forward(float(r[metric]))) for r in series]
        if len(points) > 1:
            path_data = " ".join(
                ("M" if i == 0 else "L") + f"{fmt(x)} {fmt(y)}"
                for i, (x, y) in enumerate(points)
            )
            parts.append(f'<path d="{path_data}" fill="none" stroke="{color}" stroke-width="2.8" stroke-linejoin="round" stroke-linecap="round"/>')
        for x, y in points:
            parts.append(f'<circle cx="{fmt(x)}" cy="{fmt(y)}" r="4.2" fill="{color}" stroke="#fff" stroke-width="1.4"/>')
        lx = 680
        ly = 96 + index * 22
        parts.append(f'<circle cx="{lx}" cy="{ly - 4}" r="4" fill="{color}"/>')
        parts.append(f'<text class="legend" x="{lx + 12}" y="{ly}">{html.escape(LABELS.get(algorithm, algorithm))}</text>')

    write(output, svg_document(width, height, title, subtitle, parts_to_string(parts)))


def parts_to_string(parts: Iterable[str]) -> str:
    return "".join(parts)


def verification_matrix(rows: list[dict[str, object]], output: Path) -> None:
    algorithms = sorted({str(r["algorithm"]) for r in rows})
    digits = sorted({int(r["digits"]) for r in rows})
    lookup = {(str(r["algorithm"]), int(r["digits"])): r for r in rows}
    x0, y0 = 230, 96
    cell_w, cell_h = 128, 50
    width = 960
    height = max(460, y0 + len(algorithms) * cell_h + 88)
    parts = [
        f'<rect width="{width}" height="{height}" fill="#fff"/>',
        '<text class="title" x="40" y="38">Verification matrix</text>',
        '<text class="subtitle" x="40" y="58">Green means supported and verified; gray means outside the v1 precision cap.</text>',
    ]

    for col, digit in enumerate(digits):
        x = x0 + col * cell_w + cell_w / 2
        parts.append(f'<text class="tick" x="{fmt(x)}" y="84" text-anchor="middle">{digit:,}</text>')

    for row_index, algorithm in enumerate(algorithms):
        y = y0 + row_index * cell_h
        parts.append(f'<text class="label" x="40" y="{fmt(y + 32)}">{html.escape(LABELS.get(algorithm, algorithm))}</text>')
        for col, digit in enumerate(digits):
            r = lookup[(algorithm, digit)]
            x = x0 + col * cell_w
            if r["supported"] and r["verified"]:
                fill, label = OK, "verified"
            elif not r["supported"]:
                fill, label = SKIP, "skipped"
            else:
                fill, label = FAIL, "failed"
            parts.append(f'<rect x="{fmt(x)}" y="{fmt(y)}" width="{cell_w - 10}" height="{cell_h - 10}" rx="5" fill="{fill}"/>')
            parts.append(f'<text x="{fmt(x + (cell_w - 10) / 2)}" y="{fmt(y + 28)}" text-anchor="middle" font-size="12" font-weight="700" fill="#fff">{label}</text>')

    legend_y = y0 + len(algorithms) * cell_h + 38
    for i, (fill, label) in enumerate(((OK, "verified"), (SKIP, "unsupported"), (FAIL, "failed"))):
        x = 40 + i * 150
        parts.append(f'<rect x="{x}" y="{legend_y}" width="16" height="16" rx="3" fill="{fill}"/>')
        parts.append(f'<text class="legend" x="{x + 24}" y="{legend_y + 13}">{label}</text>')

    write(
        output,
        svg_document(
            width,
            height,
            "Verification matrix",
            "Supported and verified benchmark runs by algorithm and precision.",
            parts_to_string(parts),
        ),
    )


def phase_breakdown(rows: list[dict[str, object]], digits: int, output: Path) -> None:
    """Stacked horizontal bars of split/finalize/format wall time at one size."""
    usable = [
        r
        for r in rows
        if int(r["digits"]) == digits and r["supported"] and r["verified"]
        and float(r["wall_ms"]) > 0
    ]
    usable.sort(key=lambda r: float(r["wall_ms"]))
    if not usable:
        return
    phases = [("split_ms", "#2155d9"), ("finalize_ms", "#dc2626"), ("format_ms", "#f59e0b")]
    phase_names = ["series/split", "finalize", "format"]
    x0, y0 = 230, 110
    bar_h, gap = 30, 18
    plot_w = 600
    width = 960
    height = y0 + len(usable) * (bar_h + gap) + 70
    max_wall = max(float(r["wall_ms"]) for r in usable)

    parts = [
        f'<rect width="{width}" height="{height}" fill="#fff"/>',
        f'<text class="title" x="40" y="38">Phase breakdown at {digits:,} digits</text>',
        '<text class="subtitle" x="40" y="58">Median wall time per phase; the unshaded '
        "remainder is scheduling/conversion overhead.</text>",
    ]
    for i, (name, color) in enumerate(zip(phase_names, [c for _, c in phases])):
        lx = 40 + i * 170
        parts.append(f'<rect x="{lx}" y="74" width="14" height="14" rx="3" fill="{color}"/>')
        parts.append(f'<text class="legend" x="{lx + 22}" y="86">{name}</text>')

    for index, r in enumerate(usable):
        y = y0 + index * (bar_h + gap)
        total = float(r["wall_ms"])
        label = LABELS.get(str(r["algorithm"]), str(r["algorithm"]))
        parts.append(f'<text class="label" x="40" y="{fmt(y + bar_h / 2 + 4)}">{html.escape(label)}</text>')
        total_w = total / max_wall * plot_w
        parts.append(
            f'<rect x="{x0}" y="{fmt(y)}" width="{fmt(total_w)}" height="{bar_h}" rx="4" fill="#e2e8f0"/>'
        )
        x = float(x0)
        for metric, color in phases:
            value = float(r[metric])
            if value <= 0:
                continue
            seg = value / max_wall * plot_w
            parts.append(
                f'<rect x="{fmt(x)}" y="{fmt(y)}" width="{fmt(seg)}" height="{bar_h}" fill="{color}"/>'
            )
            x += seg
        parts.append(
            f'<text class="tick" x="{fmt(x0 + total_w + 8)}" y="{fmt(y + bar_h / 2 + 4)}">{fmt(total)} ms</text>'
        )

    write(
        output,
        svg_document(
            width,
            height,
            f"Phase breakdown at {digits} digits",
            "Median split/finalize/format wall time per verified algorithm.",
            parts_to_string(parts),
        ),
    )


HYPOTHESIS_TIMELINE = [
    ("H0 baseline", 144.2, "exact binary splitting"),
    ("H1 crown", 130.5, "truncated MPFR crown"),
    ("H2 pipeline", 92.6, "overlap constants + parallel format"),
    ("H3 intra-node", 72.8, "parallel root products"),
    ("H4 warm Newton", 69.9, "warm-start 1/T"),
    ("H7 root-Q elision", 71.9, "never form root Q"),
    ("H9a deeper crown", 68.2, "more chunks"),
    ("H1-H9 final", 64.0, "hand-tuned configuration"),
    ("H11 autotune", 62.7, "knob search (refuted: <2%)"),
    ("H12 spliced prefix", 60.0, "render high half under Newton correction"),
]


def hypothesis_progression(output: Path) -> None:
    """Wall time at 1M digits across the hypothesis ledger (refutations omitted
    where they were reverted)."""
    x0, y0, x1, y1 = 90, 90, 830, 420
    width, height = 960, 540
    n = len(HYPOTHESIS_TIMELINE)
    max_wall = max(w for _, w, _ in HYPOTHESIS_TIMELINE) * 1.18

    def x_at(i: int) -> float:
        return x0 + i / (n - 1) * (x1 - x0)

    def y_at(value: float) -> float:
        return y1 - value / max_wall * (y1 - y0)

    parts = [
        f'<rect width="{width}" height="{height}" fill="#fff"/>',
        '<text class="title" x="40" y="38">Wall time at 1,000,000 digits across the hypothesis ledger</text>',
        '<text class="subtitle" x="40" y="58">Verified medians; refuted hypotheses (H5, H9b, H10) were reverted and do not appear as stages.</text>',
    ]
    for value in range(0, int(max_wall) + 1, 25):
        y = y_at(value)
        parts.append(f'<line class="grid" x1="{x0}" y1="{fmt(y)}" x2="{x1}" y2="{fmt(y)}"/>')
        parts.append(f'<text class="tick" x="{x0 - 10}" y="{fmt(y + 4)}" text-anchor="end">{value}</text>')
    parts.append(f'<line class="axis" x1="{x0}" y1="{y1}" x2="{x1}" y2="{y1}"/>')
    parts.append(f'<line class="axis" x1="{x0}" y1="{y0}" x2="{x0}" y2="{y1}"/>')
    parts.append(f'<text class="label" transform="translate(28 255) rotate(-90)" text-anchor="middle">Median wall (ms)</text>')

    points = [(x_at(i), y_at(w)) for i, (_, w, _) in enumerate(HYPOTHESIS_TIMELINE)]
    path_data = " ".join(("M" if i == 0 else "L") + f"{fmt(x)} {fmt(y)}" for i, (x, y) in enumerate(points))
    parts.append(f'<path d="{path_data}" fill="none" stroke="#dc2626" stroke-width="2.8" stroke-linejoin="round"/>')
    for i, ((x, y), (name, wall, _)) in enumerate(zip(points, HYPOTHESIS_TIMELINE)):
        parts.append(f'<circle cx="{fmt(x)}" cy="{fmt(y)}" r="4.5" fill="#dc2626" stroke="#fff" stroke-width="1.4"/>')
        parts.append(
            f'<text class="tick" x="{fmt(x)}" y="{fmt(y - 12)}" text-anchor="middle" font-weight="700">{fmt(wall)}</text>'
        )
        parts.append(
            f'<text class="tick" transform="translate({fmt(x)} {y1 + 14}) rotate(35)" text-anchor="start">{html.escape(name)}</text>'
        )

    write(
        output,
        svg_document(
            width,
            height,
            "Hypothesis ledger progression",
            "Median verified wall time at one million digits after each confirmed hypothesis.",
            parts_to_string(parts),
        ),
    )


def index_markdown(output_dir: Path) -> None:
    content = """# SATO-X Figures

Generated from `results/benchmark.csv` with `tools/make_figures.py`.

![Wall time](wall_time_log.svg)

![Relative wall time](relative_wall_time.svg)

![Multiplication bit volume](bit_volume.svg)

![Phase breakdown](phase_breakdown.svg)

![Hypothesis progression](hypothesis_progression.svg)

![Terms or iterations](terms_or_iterations.svg)

![Verification matrix](verification_matrix.svg)
"""
    (output_dir / "index.md").write_text(content, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", default="results/benchmark.csv")
    parser.add_argument("--output", default="docs/figures")
    args = parser.parse_args()

    rows = load_rows(Path(args.input))
    output_dir = Path(args.output)
    output_dir.mkdir(parents=True, exist_ok=True)

    line_chart(
        rows,
        "wall_ms",
        "Wall time by precision",
        "Verified full-prefix runs only; logarithmic axes expose growth across scales.",
        "Wall time (ms, log)",
        output_dir / "wall_time_log.svg",
        y_log=True,
    )
    line_chart(
        rows,
        "relative_wall_time",
        "Relative wall time vs. Chudnovsky",
        "Values below 1 are faster than the Chudnovsky run at the same precision.",
        "Relative wall time",
        output_dir / "relative_wall_time.svg",
        y_log=False,
    )
    line_chart(
        rows,
        "terms_or_iterations",
        "Terms or iterations by precision",
        "Series methods report terms; AGM reports iterations.",
        "Terms / iterations (log)",
        output_dir / "terms_or_iterations.svg",
        y_log=True,
    )
    bit_rows = [r for r in rows if float(r["mul_bit_volume"]) > 0]
    if bit_rows:
        line_chart(
            bit_rows,
            "mul_bit_volume",
            "Series-evaluation multiplication bit volume",
            "Machine-independent work metric: sum of operand bits over every split-phase multiplication.",
            "Bit volume (bits, log)",
            output_dir / "bit_volume.svg",
            y_log=True,
        )
    largest = max(int(r["digits"]) for r in rows)
    phase_breakdown(rows, largest, output_dir / "phase_breakdown.svg")
    hypothesis_progression(output_dir / "hypothesis_progression.svg")
    verification_matrix(rows, output_dir / "verification_matrix.svg")
    index_markdown(output_dir)
    print(f"Wrote optimized SVG figures to {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
