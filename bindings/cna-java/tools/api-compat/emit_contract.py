#!/usr/bin/env python3
"""Prints the exact expected Java contract for one or more mapped XNA types.

This is the implementation aid for the strict projection: it applies the same
mapping the verifier applies and prints Java-shaped declarations, so a new type
is written against the measured contract instead of against a guess.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("verify", Path(__file__).with_name("verify.py"))
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("cannot load the verifier module")
VERIFY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VERIFY)


def simple(name: str | None) -> str:
    if not name:
        return "void"
    suffix = ""
    while name.endswith("[]"):
        suffix += "[]"
        name = name[:-2]
    if "<" in name:
        base, arguments = name.split("<", 1)
        arguments = arguments[:-1]
        pieces = []
        depth = 0
        start = 0
        for index, character in enumerate(arguments):
            if character == "<":
                depth += 1
            elif character == ">":
                depth -= 1
            elif character == "," and depth == 0:
                pieces.append(arguments[start:index])
                start = index + 1
        pieces.append(arguments[start:])
        return simple(base) + "<" + ",".join(simple(value) for value in pieces) + ">" + suffix
    return name.rsplit(".", 1)[-1].replace("$", ".") + suffix


def render(type_contract: dict, rules: dict, java_name: str) -> str:
    lines: list[str] = []
    kind = type_contract["kind"]
    lines.append(f"// {java_name}  [{kind}]"
                 f"{'  abstract' if type_contract.get('abstract') else ''}"
                 f"{'  sealed' if type_contract.get('sealed') else ''}")
    base = type_contract.get("baseType")
    if base:
        lines.append(f"//   extends {simple(VERIFY.map_type(base) or base)}")
    for value in type_contract.get("interfaces", []):
        lines.append(f"//   implements {simple(VERIFY.map_type(value) or value)}")
    if kind == "enum":
        for member in VERIFY.mapped_members(type_contract, rules, []):
            if member["kind"] == "field":
                lines.append(f"    {member['name']} = {member.get('constant')};")
            else:
                lines.append(f"    {member['access']} {simple(member.get('returnType'))} "
                             f"{member['name']}();")
        return "\n".join(lines)
    for member in VERIFY.mapped_members(type_contract, rules, []):
        modifiers = [member.get("access", "public")]
        if member.get("static"):
            modifiers.append("static")
        if member.get("abstract"):
            modifiers.append("abstract")
        elif member.get("final"):
            modifiers.append("final")
        prefix = " ".join(modifiers)
        if member["kind"] == "field":
            constant = "" if member.get("constant") is None else f" = {member['constant']}"
            lines.append(f"    {prefix} {simple(member.get('type'))} {member['name']}{constant};")
            continue
        parameters = ", ".join(f"{simple(value['type'])} {value['name']}"
                               for value in member.get("parameters", []))
        generic = "<T> " if member.get("genericArity") else ""
        if member["kind"] == "constructor":
            lines.append(f"    {prefix} {generic}{java_name.rsplit('.', 1)[-1]}({parameters});")
        else:
            lines.append(f"    {prefix} {generic}{simple(member.get('returnType'))} "
                         f"{member['name']}({parameters});")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference-dir", required=True)
    parser.add_argument("--profile", default=str(ROOT / "tools/api-compat/profiles/xna40-full-runtime.json"))
    parser.add_argument("--mapping-rules", default=str(ROOT / "tools/api-compat/mapping-rules.json"))
    parser.add_argument("--namespace", help="only types in this namespace")
    parser.add_argument("--type", action="append", default=[], help="CLR or Java type name")
    parser.add_argument("--cache", help="reuse an extracted reference JSON")
    arguments = parser.parse_args()

    rules = json.loads(Path(arguments.mapping_rules).read_text(encoding="utf-8"))
    VERIFY.GENERIC_RENAMES.update(rules.get("genericTypeRenames", {}))
    VERIFY.TYPE_RENAMES.update(rules.get("frameworkTypeMappings", {}))
    profile = json.loads(Path(arguments.profile).read_text(encoding="utf-8"))

    if arguments.cache and Path(arguments.cache).is_file():
        reference = json.loads(Path(arguments.cache).read_text(encoding="utf-8"))
    else:
        with tempfile.TemporaryDirectory(prefix="cna-java-emit-") as directory:
            reference = VERIFY.read_reference(arguments.reference_dir, profile, Path(directory))
        if arguments.cache:
            Path(arguments.cache).write_text(json.dumps(reference), encoding="utf-8")

    selected = []
    for type_contract in reference["types"]:
        java_name = VERIFY.map_type(type_contract["name"], identity=True) or type_contract["name"]
        if arguments.type:
            if type_contract["name"] not in arguments.type and java_name not in arguments.type:
                continue
        elif arguments.namespace and not java_name.startswith(arguments.namespace):
            continue
        selected.append((java_name, type_contract))

    # Sort by the Java name alone. Two reference types can map to one Java name -- a CLR
    # generic and its non-generic companion do -- and comparing the contracts behind them is
    # comparing two dictionaries, which Python refuses.
    for java_name, type_contract in sorted(selected, key=lambda pair: pair[0]):
        print(render(type_contract, rules, java_name))
        print()
    print(f"// TYPES={len(selected)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
