#!/usr/bin/env python3
"""Emit publishable engineering efficiency table from benchmark CSV."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


CORE_METHODS = [
    "chudnovsky_naive",
    "chudnovsky_recurrence",
    "chudnovsky_bs",
    "chudnovsky_bs_valuation",
    "chudnovsky_bs_crown",
    "chudnovsky_bs_crown_tuned",
    "ramanujan_classic_bs",
    "bbp_hex_extract",
    "mpfr_const_pi",
    "arb_const_pi",
]


def load_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="") as handle:
        return list(csv.DictReader(handle))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", default="results/benchmark.csv")
    parser.add_argument("--output", default="results/efficiency.md")
    args = parser.parse_args()

    rows = load_rows(Path(args.input))
    by_key = {(r["algorithm"], r["digits"]): r for r in rows}

    lines = [
        "# Engineering Efficiency Table",
        "",
        "| Method | Digits | Runtime ms | Peak RAM MiB | R/W GB | Energy J | Verified? | Digits/sec | Digits/J |",
        "|---|---:|---:|---:|---:|---:|---|---:|---:|",
    ]

    for method in CORE_METHODS:
        for digits in sorted({r["digits"] for r in rows}, key=int):
            row = by_key.get((method, digits))
            if not row:
                continue
            peak_mib = float(row.get("peak_rss_bytes") or 0) / (1024 * 1024)
            rw_gb = (float(row.get("bytes_read") or 0) + float(row.get("bytes_written") or 0)) / 1e9
            lines.append(
                f"| `{method}` | {digits} | {row.get('total_cost_ms', row.get('wall_ms', '0'))} | "
                f"{peak_mib:.2f} | {rw_gb:.4f} | {row.get('energy_joules', '0')} | "
                f"{row.get('verified', 'false')} | {row.get('digits_per_sec', '0')} | "
                f"{row.get('digits_per_joule', '0')} |"
            )

    Path(args.output).write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {args.output}")


if __name__ == "__main__":
    main()
