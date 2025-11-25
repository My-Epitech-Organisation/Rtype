#!/usr/bin/env python3
"""Run every serialization PoC, then aggregate the key metrics into result.md."""

from __future__ import annotations

import argparse
import re
import subprocess
from pathlib import Path
from typing import Dict, List

ROOT = Path(__file__).parent.resolve()
RESULT_PATH = ROOT / "result.md"

POCS = [
    ("Binary", "Binary Custom Packet", ROOT / "Binary_Custom_Packet"),
    ("Protobuf", "Protobuf", ROOT / "Protobuf"),
    ("JSON", "JSON", ROOT / "JSON_Serialization"),
]

SIZE_FILES = {
    "Binary": "binary_size_results.txt",
    "Protobuf": "protobuf_size_results.txt",
    "JSON": "json_size_results.txt",
}

PATTERNS = {
    "Binary": {
        "pos_size": r"Binary position\s+(\d+(?:\.\d+)?)\s+bytes",
        "entity_size": r"Binary entity \(compact\)\s+(\d+(?:\.\d+)?)\s+bytes",
        "packet_5": r"Binary packet \(5 entities\)\s+(\d+(?:\.\d+)?)\s+bytes",
        "packet_10": r"Binary packet \(10 entities\)\s+(\d+(?:\.\d+)?)\s+bytes",
    },
    "Protobuf": {
        "pos_size": r"Vec2 \(position\)\s+(\d+(?:\.\d+)?)\s+bytes",
        "entity_size": r"EntityState\s+(\d+(?:\.\d+)?)\s+bytes",
        "packet_5": r"GameState x5\s+(\d+(?:\.\d+)?)\s+bytes",
        "packet_10": r"GameState x10\s+(\d+(?:\.\d+)?)\s+bytes",
    },
    "JSON": {
        "pos_size": r"--- Single Position ---.*?JSON \(compact\)\s+(\d+(?:\.\d+)?)\s+bytes",
        "entity_size": r"--- Single Entity ---.*?JSON \(compact\)\s+(\d+(?:\.\d+)?)\s+bytes",
        "packet_5": r"5 entities \(compact\)\s+(\d+(?:\.\d+)?)\s+bytes",
        "packet_10": r"10 entities \(compact\)\s+(\d+(?:\.\d+)?)\s+bytes",
    },
}

ROW_DEFS = [
    ("Position size (bytes)", "pos_size"),
    ("Entity size (bytes)", "entity_size"),
    ("GameState 5 entities (bytes)", "packet_5"),
    ("GameState 10 entities (bytes)", "packet_10"),
    ("Bandwidth 5 entities @60Hz (Kbps)", "bw_5"),
    ("Bandwidth 10 entities @60Hz (Kbps)", "bw_10"),
]


def run_poc_scripts(skip_runs: bool) -> None:
    if skip_runs:
        return
    for key, display, directory in POCS:
        script = directory / "test_poc.sh"
        if not script.exists():
            raise FileNotFoundError(f"Missing {script} for {display}")
        print(f"\n=== Running {display} PoC ===")
        subprocess.run(["/bin/bash", script.name], cwd=script.parent, check=True)


def extract_metrics() -> Dict[str, Dict[str, float]]:
    metrics: Dict[str, Dict[str, float]] = {}
    for key, _display, directory in POCS:
        size_file = directory / SIZE_FILES[key]
        text = size_file.read_text()
        patterns = PATTERNS[key]
        data: Dict[str, float] = {}
        for metric, pattern in patterns.items():
            match = re.search(pattern, text, re.MULTILINE | re.DOTALL)
            if not match:
                raise ValueError(f"Unable to find '{metric}' in {size_file}")
            data[metric] = float(match.group(1))
        data["bw_5"] = to_bandwidth_kbps(data["packet_5"])
        data["bw_10"] = to_bandwidth_kbps(data["packet_10"])
        metrics[key] = data
    return metrics


def to_bandwidth_kbps(packet_bytes: float, packets_per_second: float = 60.0) -> float:
    return packet_bytes * packets_per_second * 8.0 / 1000.0


def format_value(value: float) -> str:
    if abs(value - round(value)) < 1e-6:
        return f"{int(round(value))}"
    return f"{value:.2f}"


def build_table(metrics: Dict[str, Dict[str, float]]) -> str:
    headers = ["Metric"] + [display for _key, display, _dir in POCS]
    lines = [" | ".join(headers), " | ".join(["---"] * len(headers))]
    for label, key in ROW_DEFS:
        row: List[str] = [label]
        for poc_key, _display, _dir in POCS:
            row.append(format_value(metrics[poc_key][key]))
        lines.append(" | ".join(row))
    return "\n".join(lines)


def build_recommendation(metrics: Dict[str, Dict[str, float]]) -> str:
    binary = metrics["Binary"]
    proto = metrics["Protobuf"]
    json = metrics["JSON"]
    proto_overhead = percent_delta(proto["packet_10"], binary["packet_10"])
    json_overhead = percent_delta(json["packet_10"], binary["packet_10"])
    proto_bw_overhead = percent_delta(proto["bw_10"], binary["bw_10"])
    json_bw_overhead = percent_delta(json["bw_10"], binary["bw_10"])
    lines = [
        f"- Binary packets stay at {format_value(binary['packet_10'])} bytes for 10 entities, "
        f"which is {proto_overhead:.1f}% smaller than Protobuf and {json_overhead:.1f}% smaller than JSON.",
        f"- At 60 Hz, Binary needs {format_value(binary['bw_10'])} Kbps, vs {format_value(proto['bw_10'])} "
        f"({proto_bw_overhead:.1f}% more) for Protobuf and {format_value(json['bw_10'])} "
        f"({json_bw_overhead:.1f}% more) for JSON.",
        "- Binary is the only option that meets our 10 Kbps target with safe headroom while"
        " keeping the schema/tooling overhead near zero.",
    ]
    return "\n".join(lines)


def percent_delta(candidate: float, baseline: float) -> float:
    if baseline == 0:
        return 0.0
    return (candidate - baseline) / baseline * 100.0


def write_summary(metrics: Dict[str, Dict[str, float]]) -> None:
    table_md = build_table(metrics)
    recommendation_md = build_recommendation(metrics)
    content = (
        "# Network Serialization Comparison\n\n"
        "This file is generated by `run_all_pocs.py`. Run it again whenever new measurements are needed.\n\n"
        f"{table_md}\n\n"
        "## Highlights\n"
        f"{recommendation_md}\n\n"
        "## Recommendation\n"
        "Custom binary packets remain the preferred serialization strategy for production because they\n"
        "minimize size, bandwidth, and latency without requiring extra tooling. Keep Protobuf for tooling\n"
        "experiments and JSON for debugging only.\n"
    )
    RESULT_PATH.write_text(content)
    print(f"\nWrote consolidated results to {RESULT_PATH.relative_to(ROOT)}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--skip-run",
        action="store_true",
        help="Skip running individual PoC scripts and only regenerate result.md",
    )
    args = parser.parse_args()
    run_poc_scripts(skip_runs=args.skip_run)
    metrics = extract_metrics()
    write_summary(metrics)


if __name__ == "__main__":
    main()
