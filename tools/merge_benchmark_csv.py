#!/usr/bin/env python3
"""Merge benchmark CSV fragments and optionally regenerate summary artifacts."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

BBP_CHECKS = [
    (0, "243f6a88"),
    (10, "a308d313"),
    (100, "29b7c97c"),
]

# Sparse hex-digit extraction is not comparable to full-prefix pi computation.
PERFORMANCE_EXCLUDE = {"bbp_hex_extract"}


def row_key(row: dict[str, str]) -> tuple[str, str, str]:
    return (row["algorithm"], row["digits"], row.get("notes") or "")


def merge(inputs: list[Path], output: Path) -> list[dict[str, str]]:
    rows_by_key: dict[tuple[str, str, str], dict[str, str]] = {}
    header: list[str] | None = None

    for path in inputs:
        if not path.exists() or path.stat().st_size == 0:
            continue
        with path.open(newline="") as handle:
            reader = csv.DictReader(handle)
            if reader.fieldnames is None:
                continue
            if header is None:
                header = reader.fieldnames
            for row in reader:
                rows_by_key[row_key(row)] = row

    if not header:
        raise SystemExit("no benchmark rows to merge")

    rows = sorted(rows_by_key.values(), key=lambda r: (int(r["digits"]), r["algorithm"], r.get("notes") or ""))
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=header)
        writer.writeheader()
        writer.writerows(rows)
    print(f"Merged {len(rows)} rows -> {output}")
    return rows


def write_summary(rows: list[dict[str, str]], output: Path, guard_digits: str = "25") -> None:
    lines = [
        "# SATO-X Engineering Benchmark Summary",
        "",
        f"Guard digits: `{guard_digits}`",
        "",
        "Canonical data: `results/benchmark.csv` (merged from incremental runs).",
        "",
        "Cost model: T_series + T_bigint + T_sqrt/div + T_radix + T_verify + T_I/O. "
        "Efficiency = (seconds * watts * bytes moved) / verified digits.",
        "",
        "| Digits | Algorithm | Supported | Verified | Runtime ms | Peak RAM MiB | R/W GB | "
        "Energy J | Digits/sec | Digits/J | Relative | Notes |",
        "|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|",
    ]

    for row in rows:
        if row["algorithm"] in PERFORMANCE_EXCLUDE:
            continue
        peak_mib = float(row.get("peak_rss_bytes") or 0) / (1024 * 1024)
        rw_gb = (float(row.get("bytes_read") or 0) + float(row.get("bytes_written") or 0)) / 1e9
        notes = row.get("notes") or row.get("error") or ""
        runtime = row.get("total_cost_ms") or row.get("wall_ms") or "0"
        lines.append(
            f"| {row['digits']} | `{row['algorithm']}` | "
            f"{'yes' if row.get('supported') == 'true' else 'no'} | "
            f"{'yes' if row.get('verified') == 'true' else 'no'} | "
            f"{runtime} | {peak_mib:.3f} | {rw_gb:.4f} | "
            f"{row.get('energy_joules', '0')} | {row.get('digits_per_sec', '0')} | "
            f"{row.get('digits_per_joule', '0')} | {row.get('relative_wall_time', '0')} | "
            f"{notes} |"
        )

    lines.extend(
        [
            "",
            "See [`results/efficiency.md`](efficiency.md) for digits/sec, digits/joule, peak RAM, and I/O columns across all methods.",
            "",
            "## BBP Verification Spots",
            "",
            "| Hex offset | 8 hex digits |",
            "|---:|---|",
        ]
    )
    for offset, digits in BBP_CHECKS:
        lines.append(f"| {offset} | `{digits}` |")

    lines.extend(
        [
            "",
            "## Formula Spec Score Report",
            "",
            "Wrote `results/satox-score.md` from `candidates/*.formula`.",
            "",
            "No SATO-X candidate is considered faster unless it is benchmarked, verified, and "
            "compared against the same Chudnovsky baseline.",
            "",
        ]
    )
    output.write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote {output}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="results/benchmark.csv")
    parser.add_argument("--summary", default="")
    parser.add_argument("inputs", nargs="+")
    args = parser.parse_args()

    rows = merge([Path(p) for p in args.inputs], Path(args.output))
    if args.summary:
        write_summary(rows, Path(args.summary))


if __name__ == "__main__":
    main()
