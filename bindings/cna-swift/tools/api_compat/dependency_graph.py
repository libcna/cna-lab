#!/usr/bin/env python3
"""Public-signature dependency graph over the pinned XNA contract.

An edge `A -> B` exists when the pinned public signature of XNA type `A`
mentions XNA type `B`: its base type, a direct interface, a member return type,
a property or field type, or a parameter type. Generic arguments and array or
by-reference decorations are unwrapped, and self edges are dropped. Only types
declared in the pinned contract are nodes; BCL types are not.

Node names are mapped through `mapping-rules.json` exactly as the strict
verifier maps them, so a CLR nested name (`A+B`) and a generic collision name
such as ContentTypeReader with arity 1 resolve to the same identity the strict
report uses.
Without that mapping a nested or generic type is looked up under a name the
graph does not contain, silently reports zero dependencies, and is ranked as
trivially dependency-complete when it is not.

`--type NAME` reports one type's reverse dependents. Without it the tool ranks
every still-missing type whose XNA dependencies are all strict-complete, which
is the selection input for the next milestone. Ranking is by transitive missing
reverse reach, then by direct missing reverse consumers, then by name.

Run from the repository root after regenerating the strict report:

    python3 tools/api_compat/dependency_graph.py \
        --report docs/generated/api-compat-report.json \
        --output docs/generated/dependency-graph.json
"""

from __future__ import annotations

import argparse
import collections
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
REFERENCE = ROOT / "tools/api_compat/reference/xna40-windows-runtime-contract.json"
RULES = ROOT / "tools/api_compat/mapping-rules.json"


def map_type_name(name: str, rules: dict[str, Any]) -> str:
    collision = rules["genericCollisionTypeNames"].get(name)
    if collision:
        return collision
    return re.sub(r"`\d+", "", name.replace("+", "."))


def build_graph(
    contract: dict[str, Any],
    rules: dict[str, Any],
) -> tuple[dict[str, set[str]], dict[str, set[str]]]:
    names = {item["name"] for item in contract["types"]}

    def referenced(text: str | None) -> list[str]:
        if not text:
            return []
        found: list[str] = []
        for fragment in re.split(r"[\[\],<>&*]+", text):
            fragment = re.sub(r"`\d+$", "", fragment.strip().rstrip("&*"))
            for candidate in (fragment, fragment + "`1", fragment + "`2"):
                if candidate in names:
                    found.append(map_type_name(candidate, rules))
                    break
        return found

    forward: dict[str, set[str]] = {}
    for item in contract["types"]:
        refs: set[str] = set()
        refs.update(referenced(item.get("baseType")))
        for interface in item.get("directInterfaces") or []:
            refs.update(referenced(interface))
        for member in item["members"]:
            refs.update(referenced(member.get("returnType")))
            refs.update(referenced(member.get("type")))
            for parameter in member.get("parameters") or []:
                refs.update(referenced(parameter.get("type")))
        owner = map_type_name(item["name"], rules)
        refs.discard(owner)
        forward[owner] = refs

    reverse: dict[str, set[str]] = collections.defaultdict(set)
    for owner, refs in forward.items():
        for ref in refs:
            reverse[ref].add(owner)
    return forward, reverse


def reverse_closure(name: str, reverse: dict[str, set[str]]) -> set[str]:
    seen: set[str] = set()
    stack = [name]
    while stack:
        current = stack.pop()
        for consumer in reverse.get(current, ()):
            if consumer not in seen:
                seen.add(consumer)
                stack.append(consumer)
    seen.discard(name)
    return seen


def describe(
    name: str,
    forward: dict[str, set[str]],
    reverse: dict[str, set[str]],
    complete: set[str],
    partial: set[str],
    missing: set[str],
) -> dict[str, Any]:
    direct = reverse.get(name, set())
    closure = reverse_closure(name, reverse)
    return {
        "type": name,
        "dependencies": sorted(forward.get(name, set())),
        "dependencyComplete": all(
            item in complete for item in forward.get(name, set())
        ),
        "directMissingReverseConsumers": sorted(direct & missing),
        "directPartialReverseConsumers": sorted(direct & partial),
        "transitiveMissingReverseReach": len(closure & missing),
        "transitivePartialReverseReach": sorted(closure & partial),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--report", type=Path,
        default=ROOT / "docs/generated/api-compat-report.json",
    )
    parser.add_argument("--type")
    parser.add_argument("--output", type=Path)
    parser.add_argument("--limit", type=int, default=10)
    args = parser.parse_args()

    contract = json.loads(REFERENCE.read_text(encoding="utf-8"))
    rules = json.loads(RULES.read_text(encoding="utf-8"))
    report = json.loads(args.report.read_text(encoding="utf-8"))
    complete = set(report["completeTypes"])
    partial = set(report["partialTypes"])
    missing = set(report["missingTypes"])
    forward, reverse = build_graph(contract, rules)

    if args.type:
        entry = describe(args.type, forward, reverse, complete, partial, missing)
        print(json.dumps(entry, indent=2))
        return 0

    candidates = [
        describe(name, forward, reverse, complete, partial, missing)
        for name in sorted(missing)
    ]
    eligible = [item for item in candidates if item["dependencyComplete"]]
    eligible.sort(key=lambda item: (
        -item["transitiveMissingReverseReach"],
        -len(item["directMissingReverseConsumers"]),
        item["type"],
    ))
    payload = {
        "schemaVersion": 1,
        "reference": {
            "path": str(REFERENCE.relative_to(ROOT)),
            "rules": str(RULES.relative_to(ROOT)),
        },
        "edgeRule":
            "A -> B when A's pinned public signature (base, direct interface, "
            "return, property/field type, or parameter type) mentions XNA type B",
        "DEPENDENCY_COMPLETE_MISSING_TYPES": len(eligible),
        "ranked": eligible[:args.limit],
    }
    text = json.dumps(payload, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    print(f"DEPENDENCY_COMPLETE_MISSING_TYPES={len(eligible)}")
    for item in eligible[:args.limit]:
        print(
            f"  reach={item['transitiveMissingReverseReach']:3d}"
            f" directMissing={len(item['directMissingReverseConsumers']):2d}"
            f" directPartial={len(item['directPartialReverseConsumers'])}"
            f" {item['type']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
