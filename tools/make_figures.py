#!/usr/bin/env python3
"""Generate optimized SVG figures from SATO-X benchmark CSV output."""

from __future__ import annotations

import argparse
import csv
import html
import json
import math
import statistics
from pathlib import Path
from typing import Callable, Iterable


# Sparse hex-digit extraction is not comparable to full-prefix pi computation.
PERFORMANCE_EXCLUDE = {"bbp_hex_extract"}

PALETTE = {
    "chudnovsky_naive": "#64748b",
    "chudnovsky_recurrence": "#94a3b8",
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
    "chudnovsky_hybrid": "#be123c",
}

ALGORITHM_GROUPS = {
    "SATO-X Chudnovsky": [
        "chudnovsky_hybrid",
        "chudnovsky_bs_crown_tuned",
        "chudnovsky_bs_crown_h15",
        "chudnovsky_bs_crown",
        "chudnovsky_bs_valuation",
        "chudnovsky_bs",
        "chudnovsky_naive",
        "chudnovsky_recurrence",
    ],
    "Other Series": [
        "ramanujan_classic_bs",
        "machin_arctan",
    ],
    "Iterative": [
        "gauss_legendre_agm",
        "borwein_cubic",
        "borwein_quartic",
    ],
    "External References": [
        "arb_const_pi",
        "mpfr_const_pi",
    ],
}

GROUP_LABELS = {algorithm: group for group, algorithms in ALGORITHM_GROUPS.items() for algorithm in algorithms}
ALGORITHM_ORDER = {
    algorithm: index
    for index, algorithm in enumerate(
        algorithm for algorithms in ALGORITHM_GROUPS.values() for algorithm in algorithms
    )
}

MARKERS = {
    "chudnovsky_hybrid": "star",
    "chudnovsky_bs_crown_tuned": "diamond",
    "chudnovsky_bs_crown_h15": "triangle-up",
    "chudnovsky_bs_crown": "circle",
    "chudnovsky_bs_valuation": "square",
    "chudnovsky_bs": "circle-open",
    "chudnovsky_naive": "x",
    "chudnovsky_recurrence": "plus",
    "ramanujan_classic_bs": "triangle-down",
    "ramanujan_classic": "triangle-down",
    "machin_arctan": "square-open",
    "gauss_legendre_agm": "diamond-open",
    "borwein_cubic": "hex",
    "borwein_quartic": "pentagon",
    "arb_const_pi": "star-open",
    "mpfr_const_pi": "cross",
}

LINE_STYLES = {
    "chudnovsky_bs_crown_tuned": "7 4",
    "chudnovsky_bs_crown_h15": "3 4",
    "chudnovsky_bs_valuation": "8 3 2 3",
    "chudnovsky_naive": "2 5",
    "chudnovsky_recurrence": "2 5",
    "machin_arctan": "5 4",
    "gauss_legendre_agm": "6 3",
    "borwein_cubic": "4 3",
    "borwein_quartic": "8 3",
    "mpfr_const_pi": "3 3",
    "arb_const_pi": "10 3",
}

LABELS = {
    "chudnovsky_naive": "Chudnovsky naive",
    "chudnovsky_recurrence": "Chudnovsky recurrence",
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
    "chudnovsky_hybrid": "Hybrid router (H22)",
}

GRID = "#d7dde8"
AXIS = "#1f2937"
TEXT = "#111827"
MUTED = "#64748b"
OK = "#15803d"
SKIP = "#94a3b8"
FAIL = "#b91c1c"
NOT_RUN = "#cbd5e1"


def safe_float(value: object, default: float = 0.0) -> float:
    text = str(value or "").strip()
    if not text:
        return default
    return float(text)


def safe_int(value: object, default: int = 0) -> int:
    text = str(value or "").strip()
    if not text:
        return default
    return int(text)


def load_rows(path: Path) -> list[dict[str, object]]:
    with path.open(newline="", encoding="utf-8") as handle:
        rows: list[dict[str, object]] = []
        for raw in csv.DictReader(handle):
            row: dict[str, object] = dict(raw)
            row["digits"] = safe_int(raw["digits"])
            row["guard_digits"] = safe_int(raw["guard_digits"])
            row["supported"] = str(raw.get("supported", "")).lower() == "true"
            row["verified"] = str(raw.get("verified", "")).lower() == "true"
            row["wall_ms"] = safe_float(raw.get("wall_ms"))
            row["cpu_ms"] = safe_float(raw.get("cpu_ms"))
            row["total_cost_ms"] = safe_float(raw.get("total_cost_ms"))
            row["terms_or_iterations"] = safe_int(raw.get("terms_or_iterations"))
            row["estimated_digits_per_term"] = safe_float(raw.get("estimated_digits_per_term"))
            row["relative_wall_time"] = safe_float(raw.get("relative_wall_time"))
            row["split_ms"] = safe_float(raw.get("split_ms"))
            row["finalize_ms"] = safe_float(raw.get("finalize_ms"))
            row["format_ms"] = safe_float(raw.get("format_ms"))
            row["series_ms"] = safe_float(raw.get("series_ms"))
            row["verify_ms"] = safe_float(raw.get("verify_ms"))
            row["mul_bit_volume"] = safe_float(raw.get("mul_bit_volume"))
            row["digits_per_sec"] = safe_float(raw.get("digits_per_sec"))
            row["digits_per_joule"] = safe_float(raw.get("digits_per_joule"))
            if row["algorithm"] not in PERFORMANCE_EXCLUDE:
                rows.append(row)
    return rows


def write(path: Path, svg: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(svg, encoding="utf-8")


def fmt(value: float) -> str:
    return f"{value:.2f}".rstrip("0").rstrip(".")


def format_digit_label(digits: int) -> str:
    if digits >= 1_000_000:
        exp = int(round(math.log10(digits)))
        if abs(10**exp - digits) < max(1, digits // 100):
            return f"10^{exp}"
    return f"{digits:,}"


def algorithm_sort_key(algorithm: str) -> tuple[int, str]:
    return (ALGORITHM_ORDER.get(algorithm, 10_000), LABELS.get(algorithm, algorithm))


def ordered_algorithms(algorithms: Iterable[str]) -> list[str]:
    return sorted(algorithms, key=algorithm_sort_key)


def marker_svg(
    shape: str,
    x: float,
    y: float,
    color: str,
    size: float = 6.0,
    stroke_width: float = 1.8,
) -> str:
    x_s, y_s = fmt(x), fmt(y)
    fill = color
    stroke = "#fff"
    sw = stroke_width
    if shape.endswith("-open"):
        fill = "#fff"
        stroke = color
        sw = 2.2
        shape = shape[:-5]
    if shape == "circle":
        return (
            f'<circle cx="{x_s}" cy="{y_s}" r="{fmt(size)}" fill="{fill}" '
            f'stroke="{stroke}" stroke-width="{fmt(sw)}"/>'
        )
    if shape == "square":
        s = size * 1.45
        return (
            f'<rect x="{fmt(x - s / 2)}" y="{fmt(y - s / 2)}" width="{fmt(s)}" height="{fmt(s)}" '
            f'rx="1.5" fill="{fill}" stroke="{stroke}" stroke-width="{fmt(sw)}"/>'
        )
    if shape == "diamond":
        s = size * 1.45
        points = [(x, y - s / 2), (x + s / 2, y), (x, y + s / 2), (x - s / 2, y)]
    elif shape == "triangle-up":
        s = size * 1.8
        points = [(x, y - s / 2), (x + s / 2, y + s / 2), (x - s / 2, y + s / 2)]
    elif shape == "triangle-down":
        s = size * 1.8
        points = [(x - s / 2, y - s / 2), (x + s / 2, y - s / 2), (x, y + s / 2)]
    elif shape == "pentagon":
        points = [
            (x + math.cos(-math.pi / 2 + i * 2 * math.pi / 5) * size,
             y + math.sin(-math.pi / 2 + i * 2 * math.pi / 5) * size)
            for i in range(5)
        ]
    elif shape == "hex":
        points = [
            (x + math.cos(math.pi / 6 + i * math.pi / 3) * size,
             y + math.sin(math.pi / 6 + i * math.pi / 3) * size)
            for i in range(6)
        ]
    elif shape in {"star", "star-open"}:
        points = [
            (
                x + math.cos(-math.pi / 2 + i * math.pi / 5) * (size if i % 2 == 0 else size * 0.45),
                y + math.sin(-math.pi / 2 + i * math.pi / 5) * (size if i % 2 == 0 else size * 0.45),
            )
            for i in range(10)
        ]
    elif shape == "x":
        s = size
        return (
            f'<path d="M{fmt(x - s)} {fmt(y - s)} L{fmt(x + s)} {fmt(y + s)} '
            f'M{fmt(x + s)} {fmt(y - s)} L{fmt(x - s)} {fmt(y + s)}" '
            f'stroke="{color}" stroke-width="{fmt(sw + 0.4)}" stroke-linecap="round"/>'
        )
    elif shape in {"plus", "cross"}:
        s = size
        rotate = f' transform="rotate(45 {x_s} {y_s})"' if shape == "cross" else ""
        return (
            f'<path d="M{fmt(x - s)} {y_s} L{fmt(x + s)} {y_s} M{x_s} {fmt(y - s)} L{x_s} {fmt(y + s)}"'
            f'{rotate} stroke="{color}" stroke-width="{fmt(sw + 0.4)}" stroke-linecap="round"/>'
        )
    else:
        return marker_svg("circle", x, y, color, size, stroke_width)
    point_text = " ".join(f"{fmt(px)},{fmt(py)}" for px, py in points)
    return (
        f'<polygon points="{point_text}" fill="{fill}" stroke="{stroke}" '
        f'stroke-width="{fmt(sw)}" stroke-linejoin="round"/>'
    )


def line_dash_attribute(algorithm: str) -> str:
    dash = LINE_STYLES.get(algorithm)
    return f' stroke-dasharray="{dash}"' if dash else ""


def largest_verified_digits(rows: list[dict[str, object]]) -> int:
    verified = [
        int(r["digits"])
        for r in rows
        if r["supported"] and r["verified"] and float(r["wall_ms"]) > 0
    ]
    if verified:
        return max(verified)
    return max(int(r["digits"]) for r in rows)


def svg_document(width: int, height: int, title: str, desc: str, body: str) -> str:
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" '
        f'role="img" aria-labelledby="title desc">'
        f"<title id=\"title\">{html.escape(title)}</title>"
        f"<desc id=\"desc\">{html.escape(desc)}</desc>"
        "<style>"
        "text{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif}"
        ".title{font-size:22px;font-weight:700;fill:#111827}"
        ".subtitle{font-size:13px;fill:#64748b}"
        ".axis{stroke:#1f2937;stroke-width:1.5}"
        ".grid{stroke:#e2e8f0;stroke-width:1}"
        ".tick{font-size:12px;fill:#64748b}"
        ".label{font-size:13px;font-weight:600;fill:#111827}"
        ".legend{font-size:13px;fill:#111827}"
        ".legend-box{fill:#f8fafc;stroke:#e2e8f0;stroke-width:1}"
        ".refline{stroke:#94a3b8;stroke-width:1.5;stroke-dasharray:6 4}"
        "</style>"
        f"{body}</svg>"
    )


LEGEND_COLS = 3
LEGEND_ROW_HEIGHT = 24
LEGEND_PAD = 20
LABEL_W = 248


def legend_height(count: int, cols: int = LEGEND_COLS) -> int:
    if count <= 0:
        return 0
    rows = math.ceil(count / cols)
    return LEGEND_PAD * 2 + rows * LEGEND_ROW_HEIGHT


def render_legend(
    algorithms: list[str],
    x0: float,
    y0: float,
    width: float,
    cols: int = LEGEND_COLS,
) -> list[str]:
    """Grouped multi-column legend below the plot area."""
    if not algorithms:
        return []
    grouped: list[tuple[str, list[str]]] = []
    seen = set(algorithms)
    for group, group_algorithms in ALGORITHM_GROUPS.items():
        present = [algorithm for algorithm in group_algorithms if algorithm in seen]
        if present:
            grouped.append((group, present))
    leftovers = [algorithm for algorithm in algorithms if algorithm not in GROUP_LABELS]
    if leftovers:
        grouped.append(("Other", leftovers))

    entries: list[tuple[str, str]] = []
    for group, group_algorithms in grouped:
        entries.append(("__group__", group))
        entries.extend(("algorithm", algorithm) for algorithm in group_algorithms)

    rows = math.ceil(len(entries) / cols)
    box_h = LEGEND_PAD * 2 + rows * LEGEND_ROW_HEIGHT
    col_w = width / cols
    parts = [
        f'<rect class="legend-box" x="{fmt(x0)}" y="{fmt(y0)}" '
        f'width="{fmt(width)}" height="{fmt(box_h)}" rx="8"/>',
    ]
    for index, (entry_type, value) in enumerate(entries):
        col = index % cols
        row = index // cols
        x = x0 + 16 + col * col_w
        y = y0 + LEGEND_PAD + row * LEGEND_ROW_HEIGHT + 4
        if entry_type == "__group__":
            parts.append(
                f'<text class="legend" x="{fmt(x)}" y="{fmt(y + 4)}" '
                f'font-weight="700" fill="{MUTED}">{html.escape(value)}</text>'
            )
            continue
        algorithm = value
        color = PALETTE.get(algorithm, "#6d28d9")
        dash = line_dash_attribute(algorithm)
        parts.append(
            f'<line x1="{fmt(x)}" y1="{fmt(y - 1)}" x2="{fmt(x + 24)}" y2="{fmt(y - 1)}" '
            f'stroke="{color}" stroke-width="3" stroke-linecap="round"{dash}/>'
        )
        parts.append(marker_svg(MARKERS.get(algorithm, "circle"), x + 12, y - 1, color, 5.2))
        parts.append(
            f'<text class="legend" x="{fmt(x + 32)}" y="{fmt(y + 4)}">'
            f"{html.escape(LABELS.get(algorithm, algorithm))}</text>"
        )
    return parts


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
    reference_y: float | None = None,
) -> None:
    margin = {"left": 82, "right": 40, "top": 74, "bottom": 56}
    plot_w = 880
    plot_h = 380
    plot = {
        "x0": margin["left"],
        "y0": margin["top"],
        "x1": margin["left"] + plot_w,
        "y1": margin["top"] + plot_h,
    }
    usable = [
        r
        for r in rows
        if r["supported"] and r["verified"] and float(r[metric]) > 0 and int(r["digits"]) > 0
    ]
    algorithms = ordered_algorithms({str(r["algorithm"]) for r in usable})
    legend_y = plot["y1"] + margin["bottom"] + 8
    legend_w = plot_w
    total_legend_h = legend_height(len(algorithms))
    width = margin["left"] + plot_w + margin["right"]
    height = int(legend_y + total_legend_h + 16)

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
        parts.append(
            f'<line class="grid" x1="{fmt(x)}" y1="{plot["y0"]}" x2="{fmt(x)}" y2="{plot["y1"]}"/>'
        )
        label = f"10^{power}" if power >= 4 else (f"{10**power:,}" if power >= 3 else str(10**power))
        parts.append(
            f'<text class="tick" x="{fmt(x)}" y="{fmt(plot["y1"] + 22)}" text-anchor="middle">{label}</text>'
        )

    y_lo, y_hi = y_forward.domain  # type: ignore[attr-defined]
    if y_log:
        y_ticks = [10**p for p in range(int(y_lo), int(y_hi) + 1)]
    else:
        step = y_hi / 5
        y_ticks = [step * i for i in range(0, 6)]
    for value in y_ticks:
        y = y_forward(value if value > 0 else 0)
        label = f"1e{int(math.log10(value))}" if y_log and value > 0 else fmt(value)
        parts.append(
            f'<line class="grid" x1="{plot["x0"]}" y1="{fmt(y)}" x2="{plot["x1"]}" y2="{fmt(y)}"/>'
        )
        parts.append(f'<text class="tick" x="{plot["x0"] - 10}" y="{fmt(y + 4)}" text-anchor="end">{label}</text>')

    if reference_y is not None and not y_log:
        ref_y = y_forward(reference_y)
        parts.append(
            f'<line class="refline" x1="{plot["x0"]}" y1="{fmt(ref_y)}" '
            f'x2="{plot["x1"]}" y2="{fmt(ref_y)}"/>'
        )
        parts.append(
            f'<text class="tick" x="{plot["x1"] + 6}" y="{fmt(ref_y + 4)}" '
            f'text-anchor="start">baseline (1.0)</text>'
        )

    parts.append(
        f'<line class="axis" x1="{plot["x0"]}" y1="{plot["y1"]}" x2="{plot["x1"]}" y2="{plot["y1"]}"/>'
    )
    parts.append(
        f'<line class="axis" x1="{plot["x0"]}" y1="{plot["y0"]}" x2="{plot["x0"]}" y2="{plot["y1"]}"/>'
    )
    parts.append(
        f'<text class="label" x="{fmt(plot["x0"] + plot_w / 2)}" y="{fmt(plot["y1"] + 44)}" '
        f'text-anchor="middle">Decimal digits</text>'
    )
    parts.append(
        f'<text class="label" transform="translate(24 {fmt(plot["y0"] + plot_h / 2)}) rotate(-90)" '
        f'text-anchor="middle">{html.escape(ylabel)}</text>'
    )

    for algorithm in algorithms:
        series = sorted([r for r in usable if r["algorithm"] == algorithm], key=lambda r: int(r["digits"]))
        color = PALETTE.get(algorithm, "#6d28d9")
        points = [(xscale(float(r["digits"])), y_forward(float(r[metric]))) for r in series]
        if len(points) > 1:
            path_data = " ".join(
                ("M" if i == 0 else "L") + f"{fmt(x)} {fmt(y)}"
                for i, (x, y) in enumerate(points)
            )
            parts.append(
                f'<path d="{path_data}" fill="none" stroke="{color}" stroke-width="3" '
                f'stroke-linejoin="round" stroke-linecap="round"{line_dash_attribute(algorithm)}/>'
            )
        for x, y in points:
            parts.append(marker_svg(MARKERS.get(algorithm, "circle"), x, y, color, 6.0))

    parts.extend(render_legend(algorithms, plot["x0"], legend_y, legend_w))
    write(output, svg_document(width, height, title, subtitle, parts_to_string(parts)))


def parts_to_string(parts: Iterable[str]) -> str:
    return "".join(parts)


def verification_matrix(rows: list[dict[str, object]], output: Path) -> None:
    algorithms = ordered_algorithms({str(r["algorithm"]) for r in rows})
    digits = sorted({int(r["digits"]) for r in rows})
    lookup = {(str(r["algorithm"]), int(r["digits"])): r for r in rows}
    cell_w = max(96, min(128, int(720 / max(1, len(digits)))))
    cell_h = 44
    x0 = LABEL_W + 24
    y0 = 96
    plot_w = len(digits) * cell_w
    width = x0 + plot_w + 48
    legend_h = 52
    height = y0 + len(algorithms) * cell_h + legend_h + 48
    label_x = LABEL_W - 8
    parts = [
        f'<rect width="{width}" height="{height}" fill="#fff"/>',
        '<text class="title" x="40" y="38">Verification matrix</text>',
        '<text class="subtitle" x="40" y="58">Green = verified; gray = unsupported cap; slate = not run; red = failed.</text>',
        f'<text class="label" x="{fmt(x0 + plot_w / 2)}" y="84" text-anchor="middle">Decimal digits</text>',
    ]

    for col, digit in enumerate(digits):
        x = x0 + col * cell_w + cell_w / 2
        parts.append(
            f'<text class="tick" x="{fmt(x)}" y="104" text-anchor="middle">'
            f"{html.escape(format_digit_label(digit))}</text>"
        )

    for row_index, algorithm in enumerate(algorithms):
        y = y0 + row_index * cell_h + 8
        parts.append(
            f'<text class="label" x="{label_x}" y="{fmt(y + cell_h / 2 + 4)}" '
            f'text-anchor="end">{html.escape(LABELS.get(algorithm, algorithm))}</text>'
        )
        for col, digit in enumerate(digits):
            r = lookup.get((algorithm, digit))
            x = x0 + col * cell_w + 6
            if r is None:
                fill, label = NOT_RUN, "n/r"
            else:
                supported = r["supported"] in ("true", True)
                verified = r["verified"] in ("true", True)
                if supported and verified:
                    fill, label = OK, "OK"
                elif not supported:
                    fill, label = SKIP, "skip"
                else:
                    fill, label = FAIL, "fail"
            parts.append(
                f'<rect x="{fmt(x)}" y="{fmt(y)}" width="{cell_w - 12}" height="{cell_h - 8}" '
                f'rx="6" fill="{fill}"/>'
            )
            parts.append(
                f'<text x="{fmt(x + (cell_w - 12) / 2)}" y="{fmt(y + cell_h / 2 + 1)}" '
                f'text-anchor="middle" font-size="13" font-weight="700" fill="#fff">{label}</text>'
            )

    legend_y = y0 + len(algorithms) * cell_h + 24
    parts.append(f'<rect class="legend-box" x="36" y="{legend_y}" width="{width - 72}" height="36" rx="8"/>')
    for i, (fill, label) in enumerate(
        ((OK, "verified"), (SKIP, "unsupported"), (NOT_RUN, "not run"), (FAIL, "failed"))
    ):
        x = 56 + i * 160
        parts.append(f'<rect x="{x}" y="{legend_y + 10}" width="18" height="18" rx="4" fill="{fill}"/>')
        parts.append(f'<text class="legend" x="{x + 28}" y="{legend_y + 24}">{label}</text>')

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
    usable.sort(key=lambda r: (GROUP_LABELS.get(str(r["algorithm"]), "Other"), float(r["wall_ms"])))
    if not usable:
        return
    phases = [("split_ms", "#2155d9"), ("finalize_ms", "#dc2626"), ("format_ms", "#f59e0b")]
    phase_names = ["series / split", "finalize", "format"]
    x0 = LABEL_W + 24
    bar_h, gap = 32, 14
    plot_w = 620
    legend_h = 48
    right_pad = 140
    width = x0 + plot_w + right_pad
    height = 110 + len(usable) * (bar_h + gap) + legend_h + 32
    max_wall = max(float(r["wall_ms"]) for r in usable)
    label_x = LABEL_W - 8

    parts = [
        f'<rect width="{width}" height="{height}" fill="#fff"/>',
        f'<text class="title" x="40" y="38">Phase breakdown at {format_digit_label(digits)} digits</text>',
        '<text class="subtitle" x="40" y="58">Stacked bars show split, finalize, and format; '
        "light gray is remaining overhead.</text>",
    ]

    legend_y = height - legend_h - 12
    parts.append(f'<rect class="legend-box" x="36" y="{legend_y}" width="{width - 72}" height="36" rx="8"/>')
    for i, (name, color) in enumerate(zip(phase_names, [c for _, c in phases])):
        lx = 56 + i * 200
        parts.append(f'<rect x="{lx}" y="{legend_y + 10}" width="16" height="16" rx="3" fill="{color}"/>')
        parts.append(f'<text class="legend" x="{lx + 24}" y="{legend_y + 24}">{name}</text>')
    lx = 56 + 3 * 200
    parts.append(f'<rect x="{lx}" y="{legend_y + 10}" width="16" height="16" rx="3" fill="#e2e8f0"/>')
    parts.append(f'<text class="legend" x="{lx + 24}" y="{legend_y + 24}">overhead</text>')

    y0 = 96
    for index, r in enumerate(usable):
        y = y0 + index * (bar_h + gap)
        total = float(r["wall_ms"])
        label = LABELS.get(str(r["algorithm"]), str(r["algorithm"]))
        parts.append(
            f'<text class="label" x="{label_x}" y="{fmt(y + bar_h / 2 + 4)}" text-anchor="end">{html.escape(label)}</text>'
        )
        total_w = total / max_wall * plot_w
        parts.append(
            f'<rect x="{x0}" y="{fmt(y)}" width="{fmt(total_w)}" height="{bar_h}" rx="5" fill="#e2e8f0"/>'
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
            f'<text class="tick" x="{fmt(x0 + total_w + 10)}" y="{fmt(y + bar_h / 2 + 4)}">{fmt(total)} ms</text>'
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


HYPOTHESIS_LEDGER_PATH = Path("data/hypothesis_progression.json")
# Index after H12 in the ledger (0-based); stages H13+ use remeasured benchmark compute_wall.
HYPOTHESIS_REGIME_BREAK_AFTER = 9


def compute_wall_ms(row: dict[str, object]) -> float:
    return float(row["split_ms"]) + float(row["finalize_ms"]) + float(row["format_ms"])


def median_compute_wall(
    rows: list[dict[str, object]], algorithm: str, digits: int = 1_000_000
) -> float | None:
    values = [
        compute_wall_ms(row)
        for row in rows
        if str(row["algorithm"]) == algorithm
        and int(row["digits"]) == digits
        and row["supported"]
        and row["verified"]
        and float(row["wall_ms"]) > 0
    ]
    if not values:
        return None
    return float(statistics.median(values))


def load_hypothesis_timeline(rows: list[dict[str, object]]) -> list[tuple[str, float, str]]:
    ledger_path = HYPOTHESIS_LEDGER_PATH
    if not ledger_path.exists():
        raise FileNotFoundError(f"missing hypothesis ledger: {ledger_path}")
    ledger = json.loads(ledger_path.read_text(encoding="utf-8"))
    digits = int(ledger.get("digits", 1_000_000))
    timeline: list[tuple[str, float, str]] = []
    for stage in ledger["stages"]:
        wall = float(stage["compute_wall_ms"]) if "compute_wall_ms" in stage else None
        algorithm = stage.get("algorithm")
        if algorithm:
            measured = median_compute_wall(rows, str(algorithm), digits)
            if measured is not None:
                wall = measured
        if wall is None:
            raise ValueError(f"no compute_wall for hypothesis stage {stage.get('id')}")
        timeline.append((str(stage["name"]), wall, str(stage["description"])))
    return timeline


def hypothesis_progression(rows: list[dict[str, object]], output: Path) -> None:
    """Compute wall (split+finalize+format) at 1M digits across the hypothesis ledger."""
    timeline = load_hypothesis_timeline(rows)
    x0, y0, x1, y1 = 90, 90, 830, 380
    width = 1002
    n = len(timeline)
    max_wall = max(w for _, w, _ in timeline) * 1.18
    legend_y = y1 + 56
    legend_h = legend_height(n, cols=2)
    height = int(legend_y + legend_h + 16)

    def x_at(i: int) -> float:
        return x0 + i / (n - 1) * (x1 - x0)

    def y_at(value: float) -> float:
        return y1 - value / max_wall * (y1 - y0)

    parts = [
        f'<rect width="{width}" height="{height}" fill="#fff"/>',
        '<text class="title" x="40" y="38">Compute wall at 1,000,000 digits across the hypothesis ledger</text>',
        '<text class="subtitle" x="40" y="58">'
        "Metric: split + finalize + format (verify excluded). H0–H12: research-log dev ledger. "
        "H13+ remeasured or refreshed on current harness. Refuted: H5, H9b, H10.</text>",
    ]
    for value in range(0, int(max_wall) + 1, 25):
        y = y_at(value)
        parts.append(f'<line class="grid" x1="{x0}" y1="{fmt(y)}" x2="{x1}" y2="{fmt(y)}"/>')
        parts.append(f'<text class="tick" x="{x0 - 10}" y="{fmt(y + 4)}" text-anchor="end">{value}</text>')
    parts.append(f'<line class="axis" x1="{x0}" y1="{y1}" x2="{x1}" y2="{y1}"/>')
    parts.append(f'<line class="axis" x1="{x0}" y1="{y0}" x2="{x0}" y2="{y1}"/>')
    parts.append(
        f'<text class="label" transform="translate(28 {fmt(y0 + (y1 - y0) / 2)}) rotate(-90)" '
        f'text-anchor="middle">Compute wall (ms)</text>'
    )
    parts.append(f'<text class="label" x="{fmt(x0 + (x1 - x0) / 2)}" y="{fmt(y1 + 36)}" text-anchor="middle">Hypothesis stage</text>')

    if HYPOTHESIS_REGIME_BREAK_AFTER < n - 1:
        break_x = x_at(HYPOTHESIS_REGIME_BREAK_AFTER) + (x_at(HYPOTHESIS_REGIME_BREAK_AFTER + 1) - x_at(HYPOTHESIS_REGIME_BREAK_AFTER)) / 2
        parts.append(
            f'<line x1="{fmt(break_x)}" y1="{fmt(y0)}" x2="{fmt(break_x)}" y2="{fmt(y1)}" '
            f'stroke="#94a3b8" stroke-width="1.5" stroke-dasharray="6 4"/>'
        )
        parts.append(
            f'<text class="tick" x="{fmt(break_x)}" y="{fmt(y0 - 8)}" text-anchor="middle" fill="{MUTED}">remeasured</text>'
        )

    points = [(x_at(i), y_at(w)) for i, (_, w, _) in enumerate(timeline)]
    path_data = " ".join(("M" if i == 0 else "L") + f"{fmt(x)} {fmt(y)}" for i, (x, y) in enumerate(points))
    parts.append(
        f'<path d="{path_data}" fill="none" stroke="#dc2626" stroke-width="3" stroke-linejoin="round"/>'
    )
    for i, ((x, y), (name, wall, desc)) in enumerate(zip(points, timeline)):
        parts.append(
            f'<circle cx="{fmt(x)}" cy="{fmt(y)}" r="5" fill="#dc2626" stroke="#fff" stroke-width="1.8"/>'
        )
        parts.append(
            f'<text class="tick" x="{fmt(x)}" y="{fmt(y - 14)}" text-anchor="middle" font-weight="700">{fmt(wall)}</text>'
        )
        parts.append(f'<text class="tick" x="{fmt(x)}" y="{fmt(y1 + 18)}" text-anchor="middle">{i + 1}</text>')

    # Numbered legend below with full hypothesis names and descriptions.
    rows = math.ceil(n / 2)
    box_h = LEGEND_PAD * 2 + rows * LEGEND_ROW_HEIGHT
    legend_box_w = width - 72
    parts.append(f'<rect class="legend-box" x="36" y="{legend_y}" width="{legend_box_w}" height="{box_h}" rx="8"/>')
    col_w = (legend_box_w - 32) / 2
    for i, (name, wall, desc) in enumerate(timeline):
        col = i // rows
        row = i % rows
        x = 52 + col * (col_w + 16)
        y = legend_y + LEGEND_PAD + row * LEGEND_ROW_HEIGHT + 4
        short_desc = desc if len(desc) <= 34 else desc[:31] + "..."
        parts.append(
            f'<text class="legend" x="{fmt(x)}" y="{fmt(y)}">'
            f'<tspan font-weight="700">{i + 1}.</tspan> {html.escape(name)} '
            f"({fmt(wall)} ms) — {html.escape(short_desc)}</text>"
        )

    write(
        output,
        svg_document(
            width,
            height,
            "Hypothesis ledger progression",
            "Compute wall (split+finalize+format) at one million digits after each hypothesis stage.",
            parts_to_string(parts),
        ),
    )


def index_markdown(output_dir: Path) -> None:
    content = """# SATO-X Benchmark Figures

Generated from `results/benchmark.csv` with `tools/make_figures.py`.

For method descriptions and formulae, see [Methods comparison](../methods-comparison.md).

## Performance

### Wall time by precision

Median verified wall time across decimal digit targets (log–log scale). Lower is better.

![Wall time by precision](wall_time_log.svg)

### Relative wall time vs. Chudnovsky baseline

Values below the dashed line (1.0) are faster than standard Chudnovsky binary splitting at the same precision.

![Relative wall time](relative_wall_time.svg)

### Multiplication bit volume

Machine-independent work metric for binary-splitting series evaluation: sum of operand bit-lengths over every split-phase multiplication.

![Multiplication bit volume](bit_volume.svg)

## Work breakdown

### Phase timing at maximum precision

Where each algorithm spends time: series/split, finalize, format, and overhead.

![Phase breakdown](phase_breakdown.svg)

### Terms or iterations

Series methods report term counts; AGM reports iterations.

![Terms or iterations](terms_or_iterations.svg)

## Research progression

### Hypothesis ledger at 1M digits

Compute wall (split + finalize + format; verify excluded) at 1M digits. H0–H12 from the research-log development ledger; H13+ is remeasured or refreshed on the current harness (dashed line marks the regime change). H20 gives Arb the same scaled-verify/parallel-format pipeline; H22 retunes hybrid routing to use Arb from 10^6 digits.

![Hypothesis progression](hypothesis_progression.svg)

## Correctness

### Verification matrix

Green cells passed full-prefix verification; gray cells exceed the algorithm's precision cap.

![Verification matrix](verification_matrix.svg)

### Engineering efficiency

Verified digits per second and (when energy sampling is available) digits per joule.

![Digits per second](digits_per_sec.svg)

![Cost breakdown](cost_breakdown_stacked.svg)
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
        reference_y=1.0,
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
    breakdown_digits = largest_verified_digits(rows)
    phase_breakdown(rows, breakdown_digits, output_dir / "phase_breakdown.svg")
    line_chart(
        rows,
        "digits_per_sec",
        "Verified digits per second",
        "Total cost includes verification; higher is better.",
        "Digits / second (log)",
        output_dir / "digits_per_sec.svg",
        y_log=True,
    )
    joule_rows = [r for r in rows if float(r.get("digits_per_joule") or 0) > 0]
    if joule_rows:
        line_chart(
            joule_rows,
            "digits_per_joule",
            "Verified digits per joule",
            "Requires platform energy sampling (RAPL on Linux); omitted when unavailable.",
            "Digits / joule (log)",
            output_dir / "digits_per_joule.svg",
            y_log=True,
        )
    cost_rows = [r for r in rows if float(r.get("series_ms") or 0) > 0 or float(r.get("split_ms") or 0) > 0]
    if cost_rows:
        phase_breakdown(cost_rows, breakdown_digits, output_dir / "cost_breakdown_stacked.svg")
    hypothesis_progression(rows, output_dir / "hypothesis_progression.svg")
    verification_matrix(rows, output_dir / "verification_matrix.svg")
    index_markdown(output_dir)
    print(f"Wrote optimized SVG figures to {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
