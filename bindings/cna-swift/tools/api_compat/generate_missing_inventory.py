#!/usr/bin/env python3
"""Render the current strict scoreboard as retained Markdown evidence."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    report = json.loads(args.report.read_text(encoding="utf-8"))
    summary = report["summary"]
    lines = [
        "# XNA to Swift missing-type inventory",
        "",
        "Generated from the compiler Symbol Graph and the pinned XNA 4.0 Windows runtime contract.",
        "Normal strict status is intentionally red.",
        "",
        "```text",
    ]
    for key, value in summary.items():
        lines.append(f"{key}={value}")
    lines += ["```", "", "## Complete types", ""]
    lines += [f"- `{name}`" for name in report["completeTypes"]]
    lines += ["", "## Partial types and exact diagnostics", ""]
    for item in report["typeScoreboard"]:
        if item["status"] != "PARTIAL":
            continue
        lines += [f"### `{item['type']}`", "", f"Expected members: {item['expectedMembers']}; emitted members: {item['targetMembers']}.", ""]
        lines += [f"- `{entry['category']}` — `{entry['subject']}`: {entry['detail']}" for entry in item["diagnostics"]]
        lines.append("")
    lines += ["## Missing types", ""]
    lines += [f"- `{name}`" for name in report["missingTypes"]]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
