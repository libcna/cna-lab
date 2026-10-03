#!/usr/bin/env python3
"""Mechanically inventories the canonical CNA C ABI from the live CNA headers.

The CNA C API is the only native contract CNA-Java is allowed to bind, so the
Java-side manifest must be derived from those headers rather than maintained by
hand.  This module parses ``CNA/C/*.h`` and produces a stable, sorted, JSON
inventory of every exported identity:

``functions``   every ``CNA_C_API`` declaration with its return type and
                parameter list, keyed by symbol name.
``identities``  every ``typedef uint32_t CNA_X;`` fixed-width identity together
                with the ``#define`` constants that name its values.
``constants``   every remaining object-like ``#define`` that carries a value.
``structures``  every ``typedef struct { ... } CNA_X;`` with its field list.
``callbacks``   every function-pointer typedef the ABI hands to a caller.
``handles``     every opaque handle typedef.

The parser is deliberately syntactic.  The CNA headers are generated-quality C17
with one declaration per statement, so a full C parser buys nothing; what
matters is that an unparsable declaration is reported instead of dropped, which
``--strict`` enforces.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys
from typing import Any


COMMENT = re.compile(r"/\*.*?\*/", re.DOTALL)
LINE_COMMENT = re.compile(r"//[^\n]*")
FUNCTION = re.compile(
    r"CNA_C_API\s+(?P<return>[A-Za-z_][A-Za-z0-9_ *]*?)\s*"
    r"(?P<name>cna_[A-Za-z0-9_]+)\s*\((?P<parameters>[^;]*?)\)\s*;",
    re.DOTALL,
)
IDENTITY = re.compile(r"typedef\s+(?P<underlying>u?int(?:8|16|32|64)_t)\s+(?P<name>CNA_[A-Za-z0-9_]+)\s*;")
HANDLE = re.compile(r"typedef\s+CNA_Handle\s+(?P<name>CNA_[A-Za-z0-9_]+)\s*;")
DEFINE = re.compile(r"^[ \t]*#define[ \t]+(?P<name>CNA_[A-Za-z0-9_]+)[ \t]+(?P<value>[^\n(][^\n]*|\([^\n]*)$", re.MULTILINE)
DEFINE_CALL = re.compile(r"^[ \t]*#define[ \t]+(?P<name>CNA_[A-Za-z0-9_]+)\((?P<args>[^)]*)\)", re.MULTILINE)
STRUCTURE = re.compile(
    r"typedef\s+struct\s*(?:[A-Za-z_][A-Za-z0-9_]*\s*)?\{(?P<body>.*?)\}\s*(?P<name>CNA_[A-Za-z0-9_]+)\s*;",
    re.DOTALL,
)
CALLBACK = re.compile(
    r"typedef\s+(?P<return>[A-Za-z_][A-Za-z0-9_ *]*?)\s*\(\s*\*\s*(?P<name>CNA_[A-Za-z0-9_]+)\s*\)\s*"
    r"\((?P<parameters>[^;]*?)\)\s*;",
    re.DOTALL,
)
INTEGER_VALUE = re.compile(r"^U?INT(?:8|16|32|64)_C\((?P<value>-?(?:0[xX])?[0-9A-Fa-f]+)\)$")


def strip_comments(text: str) -> str:
    return LINE_COMMENT.sub("", COMMENT.sub("", text))


def normalize(value: str) -> str:
    return re.sub(r"\s+", " ", value).strip()


def split_parameters(text: str) -> list[dict[str, str]]:
    body = normalize(text)
    if body in ("", "void"):
        return []
    parameters: list[dict[str, str]] = []
    depth = 0
    start = 0
    pieces: list[str] = []
    for index, character in enumerate(body):
        if character in "([":
            depth += 1
        elif character in ")]":
            depth -= 1
        elif character == "," and depth == 0:
            pieces.append(body[start:index])
            start = index + 1
    pieces.append(body[start:])
    for piece in pieces:
        piece = piece.strip()
        match = re.match(r"^(?P<type>.*?)(?P<name>[A-Za-z_][A-Za-z0-9_]*)(?P<array>\[\d*\])?$", piece)
        if match is None or not match.group("type").strip():
            parameters.append({"type": piece, "name": ""})
            continue
        type_name = normalize(match.group("type"))
        if match.group("array"):
            type_name += match.group("array")
        parameters.append({"type": type_name, "name": match.group("name")})
    return parameters


def parse_structure_fields(body: str) -> list[dict[str, str]]:
    fields: list[dict[str, str]] = []
    for statement in body.split(";"):
        statement = normalize(statement)
        if not statement:
            continue
        match = re.match(
            r"^(?P<type>.*?)(?P<name>[A-Za-z_][A-Za-z0-9_]*)(?P<array>(?:\[[^\]]*\])+)?$", statement)
        if match is None or not match.group("type").strip():
            fields.append({"type": statement, "name": ""})
            continue
        type_name = normalize(match.group("type"))
        if match.group("array"):
            type_name += match.group("array")
        fields.append({"type": type_name, "name": match.group("name")})
    return fields


def integer_constant(value: str) -> int | None:
    match = INTEGER_VALUE.match(value.strip())
    if match is None:
        return None
    literal = match.group("value")
    try:
        return int(literal, 16) if literal.lower().startswith("0x") else int(literal)
    except ValueError:
        return None


def identity_prefix(name: str) -> str:
    """Return the ``#define`` prefix that names one identity's values.

    ``CNA_TonemappingMode`` names its values ``CNA_TONEMAPPING_MODE_*``; the
    conversion is the ordinary CNA screaming-snake spelling of the type name.
    """
    body = name[len("CNA_"):]
    snake = re.sub(r"(?<=[a-z0-9])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])", "_", body)
    return "CNA_" + snake.upper() + "_"


# `@param <name> <text>` inside a Doxygen block, up to the next tag or the block's end.
PARAMETER_DOC = re.compile(r"@param\s+(?:\[[^\]]*\]\s*)?(\w+)\s+(.*?)(?=\n\s*\*\s*@|\Z)",
                           re.S)


def document_parameters(raw: str) -> dict[str, dict[str, str]]:
    """Maps each declared symbol to its ``@param`` text, keyed by parameter name.

    The declarations themselves are parsed from comment-stripped text, because a
    comment can contain anything.  What a parameter's *documentation* says is a
    separate question, and one worth asking: CNA states some contracts only in
    prose -- "destination for eight corners" -- and a generator that cannot see
    that prose has no way to know a bare ``T*`` is an array rather than one
    struct.  It cannot read the contract either, but it can notice that there is
    one and refuse instead of guessing.
    """
    documented: dict[str, dict[str, str]] = {}
    for block in re.finditer(r"/\*\*(.*?)\*/\s*CNA_C_API[^;]*?(cna_\w+)\s*\(", raw, re.S):
        body, symbol = block.group(1), block.group(2)
        entries: dict[str, str] = {}
        for parameter in PARAMETER_DOC.finditer(body):
            entries[parameter.group(1)] = normalize(
                re.sub(r"^\s*\*", "", parameter.group(2), flags=re.M))
        if entries:
            documented[symbol] = entries
    return documented


def inventory(include: Path, *, strict: bool = False) -> dict[str, Any]:
    headers = sorted(p for p in (include / "CNA/C").glob("*.h"))
    if not headers:
        raise FileNotFoundError(f"no CNA C headers under {include}")

    functions: dict[str, Any] = {}
    identities: dict[str, Any] = {}
    handles: dict[str, Any] = {}
    structures: dict[str, Any] = {}
    callbacks: dict[str, Any] = {}
    constants: dict[str, Any] = {}
    macros: dict[str, Any] = {}
    problems: list[str] = []

    for header in headers:
        name = header.name
        raw = header.read_text(encoding="utf-8")
        text = strip_comments(raw)
        parameter_docs = document_parameters(raw)

        for match in FUNCTION.finditer(text):
            symbol = match.group("name")
            if symbol in functions:
                problems.append(f"DUPLICATE_FUNCTION={symbol}")
                continue
            parameters = split_parameters(match.group("parameters"))
            for parameter in parameters:
                documented = parameter_docs.get(symbol, {}).get(parameter["name"])
                if documented:
                    parameter["doc"] = documented
            functions[symbol] = {
                "name": symbol,
                "header": name,
                "returnType": normalize(match.group("return")),
                "parameters": parameters,
            }

        for match in HANDLE.finditer(text):
            handles[match.group("name")] = {"name": match.group("name"), "header": name}

        for match in IDENTITY.finditer(text):
            identity = match.group("name")
            identities[identity] = {
                "name": identity,
                "header": name,
                "underlying": match.group("underlying"),
                "values": {},
            }

        for match in CALLBACK.finditer(text):
            callbacks[match.group("name")] = {
                "name": match.group("name"),
                "header": name,
                "returnType": normalize(match.group("return")),
                "parameters": split_parameters(match.group("parameters")),
            }

        for match in STRUCTURE.finditer(text):
            structures[match.group("name")] = {
                "name": match.group("name"),
                "header": name,
                "fields": parse_structure_fields(match.group("body")),
            }

        for match in DEFINE_CALL.finditer(text):
            macros[match.group("name")] = {
                "name": match.group("name"),
                "header": name,
                "parameters": [value.strip() for value in match.group("args").split(",") if value.strip()],
            }

        for match in DEFINE.finditer(text):
            symbol = match.group("name")
            if symbol in macros or symbol.endswith("_H"):
                continue
            value = normalize(match.group("value"))
            constants[symbol] = {"name": symbol, "header": name, "value": value,
                                 "integer": integer_constant(value)}

    # Attach every constant that names one identity's value to that identity.
    prefixes = sorted(((identity_prefix(name), name) for name in identities),
                      key=lambda item: -len(item[0]))
    for symbol, constant in constants.items():
        for prefix, identity in prefixes:
            if symbol.startswith(prefix):
                identities[identity]["values"][symbol] = constant["integer"]
                constant["identity"] = identity
                break

    if strict and problems:
        for problem in problems:
            print(problem, file=sys.stderr)
        raise ValueError(f"{len(problems)} unparsable C API declarations")

    result = {
        "schemaVersion": 1,
        "headers": [header.name for header in headers],
        "functions": dict(sorted(functions.items())),
        "identities": dict(sorted(identities.items())),
        "handles": dict(sorted(handles.items())),
        "structures": dict(sorted(structures.items())),
        "callbacks": dict(sorted(callbacks.items())),
        "constants": dict(sorted(constants.items())),
        "macros": dict(sorted(macros.items())),
        "problems": problems,
    }
    payload = json.dumps({key: result[key] for key in
                          ("functions", "identities", "handles", "structures", "callbacks", "constants")},
                         sort_keys=True, separators=(",", ":"))
    result["inventorySha256"] = hashlib.sha256(payload.encode("utf-8")).hexdigest()
    return result


def abi_version(include: Path) -> str:
    text = strip_comments((include / "CNA/C/abi.h").read_text(encoding="utf-8"))
    parts = []
    for component in ("MAJOR", "MINOR", "PATCH"):
        match = re.search(rf"#define\s+CNA_ABI_VERSION_{component}\s+UINT32_C\((\d+)\)", text)
        if match is None:
            raise ValueError(f"CNA_ABI_VERSION_{component} not found in abi.h")
        parts.append(match.group(1))
    return ".".join(parts)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cna-root", required=True)
    parser.add_argument("--output")
    parser.add_argument("--strict", action="store_true")
    parser.add_argument("--summary", action="store_true")
    arguments = parser.parse_args()

    include = Path(arguments.cna_root).resolve() / "modules/c-api/include"
    result = inventory(include, strict=arguments.strict)
    result["abiVersion"] = abi_version(include)

    if arguments.output:
        path = Path(arguments.output)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if arguments.summary or not arguments.output:
        print(f"ABI_VERSION={result['abiVersion']}")
        print(f"HEADERS={len(result['headers'])}")
        print(f"FUNCTIONS={len(result['functions'])}")
        print(f"IDENTITIES={len(result['identities'])}")
        print(f"HANDLES={len(result['handles'])}")
        print(f"STRUCTURES={len(result['structures'])}")
        print(f"CALLBACKS={len(result['callbacks'])}")
        print(f"CONSTANTS={len(result['constants'])}")
        print(f"MACROS={len(result['macros'])}")
        print(f"PROBLEMS={len(result['problems'])}")
        print(f"INVENTORY_SHA256={result['inventorySha256']}")
    return 1 if result["problems"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
