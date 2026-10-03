#!/usr/bin/env python3
"""Audit pinned Microsoft XNA assemblies against the retained contract.

The retained contract in `reference/xna40-windows-runtime-contract.json` is the
public-shape authority, and a hash-matched assembly is the behavior authority
for the types it declares. Before an assembly may be used as a behavior
authority it must be *registered*: its SHA-256 recorded, and its public
metadata machine-compared against every contract entry it owns.

This tool performs that comparison. It disassembles each assembly with
`ikdasm`, reconstructs the public type/member shape in the contract's own
schema, and diffs it against the contract.

Correctness of the reconstruction is not asserted, it is calibrated: the two
assemblies that were already registered before this tool existed must
reproduce their contract entries exactly. `--require-exact NAME` makes that a
hard gate, and the audit refuses to report on any other assembly unless every
calibration assembly passes.

    python3 tools/api_compat/pinned_assembly_audit.py \
        --assembly-dir ~/Downloads/win \
        --require-exact Microsoft.Xna.Framework.dll \
        --require-exact Microsoft.Xna.Framework.Graphics.dll \
        --output docs/generated/pinned-assembly-audit.json
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
REFERENCE = ROOT / "tools/api_compat/reference/xna40-windows-runtime-contract.json"
REGISTRY = ROOT / "tools/api_compat/registered-assemblies.json"
RESOURCE_REFERENCE = (
    ROOT / "tools/api_compat/reference/xna40-selected-resource-strings.json")

PRIMITIVES = {
    "void": "System.Void",
    "bool": "System.Boolean",
    "char": "System.Char",
    "int8": "System.SByte",
    "unsigned int8": "System.Byte",
    "int16": "System.Int16",
    "unsigned int16": "System.UInt16",
    "int32": "System.Int32",
    "unsigned int32": "System.UInt32",
    "int64": "System.Int64",
    "unsigned int64": "System.UInt64",
    "float32": "System.Single",
    "float64": "System.Double",
    "string": "System.String",
    "object": "System.Object",
    "native int": "System.IntPtr",
    "native unsigned int": "System.UIntPtr",
    "typedref": "System.TypedReference",
    # ikdasm also emits the short unsigned spellings.
    "uint8": "System.Byte",
    "uint16": "System.UInt16",
    "uint32": "System.UInt32",
    "uint64": "System.UInt64",
}

ACCESS = {
    "public": "public",
    "family": "protected",
    "famorassem": "protected-internal",
}
# Only these reach the public contract surface.
VISIBLE_ACCESS = set(ACCESS)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    digest.update(path.read_bytes())
    return digest.hexdigest()


def disassemble(path: Path, cache: Path | None) -> str:
    if cache is not None and cache.exists():
        return cache.read_text(encoding="utf-8", errors="replace")
    completed = subprocess.run(
        ["ikdasm", str(path)], capture_output=True, text=True, errors="replace",
    )
    if completed.returncode != 0:
        raise SystemExit(f"ikdasm failed for {path}: {completed.stderr[:400]}")
    if cache is not None:
        cache.parent.mkdir(parents=True, exist_ok=True)
        cache.write_text(completed.stdout, encoding="utf-8")
    return completed.stdout


def strip_assembly_ref(text: str) -> str:
    return re.sub(r"\[[A-Za-z0-9_.]+\]", "", text)


def normalize_type(text: str) -> str:
    """Map an IL type expression onto the contract's CLR type spelling."""
    text = text.strip()
    if not text:
        return ""
    # Drop custom modifiers and marshalling directives, which the contract
    # does not record.
    text = re.sub(r"modopt\([^)]*\)|modreq\([^)]*\)", "", text)
    text = re.sub(r"\bmarshal\s*\([^)]*\)", "", text).strip()
    # Trailing by-ref marker is carried as a separate flag by the caller.
    text = text.rstrip()
    suffix = ""
    if text.endswith("&"):
        # The contract keeps the by-ref marker in the type spelling and
        # additionally records ref/out flags, so preserve it here.
        return normalize_type(text[:-1]) + "&"
    while text.endswith("[]") or text.endswith("*"):
        if text.endswith("[]"):
            suffix = "[]" + suffix
            text = text[:-2].rstrip()
        else:
            suffix = "*" + suffix
            text = text[:-1].rstrip()
    for keyword in ("class ", "valuetype ", "instance "):
        while text.startswith(keyword):
            text = text[len(keyword):].strip()
    if text in PRIMITIVES:
        return PRIMITIVES[text] + suffix
    # Generic instantiation: Foo`1<class Bar> -> Foo`1[Bar]
    match = re.match(r"^(.*?)<(.*)>$", text, re.S)
    if match:
        base = normalize_type(match.group(1))
        arguments = [normalize_type(item) for item in split_top(match.group(2))]
        # The retained contract separates generic arguments with a bare comma.
        return f"{base}[{','.join(arguments)}]" + suffix
    text = strip_assembly_ref(text).strip()
    if text in PRIMITIVES:
        return PRIMITIVES[text] + suffix
    text = text.replace("/", "+")
    text = text.strip("'")
    return text + suffix


def split_top(text: str) -> list[str]:
    parts: list[str] = []
    depth = 0
    start = 0
    for index, char in enumerate(text):
        if char in "<[(":
            depth += 1
        elif char in ">])":
            depth -= 1
        elif char == "," and depth == 0:
            parts.append(text[start:index])
            start = index + 1
    parts.append(text[start:])
    return [item.strip() for item in parts if item.strip()]


def positional_generics(
    text: str,
    type_parameters: tuple[str, ...],
    method_parameters: tuple[str, ...],
) -> str:
    """Rewrite named IL generic parameters to the contract's positional form.

    IL spells a generic parameter by name (`!!T`, `!TPacked`); the retained
    contract spells it by position (`!!0`, `!0`).
    """
    for index, name in enumerate(method_parameters):
        text = re.sub(rf"!!{re.escape(name)}\b", f"!!{index}", text)
    for index, name in enumerate(type_parameters):
        text = re.sub(rf"(?<!!)!{re.escape(name)}\b", f"!{index}", text)
    return text


def split_parameters(
    text: str,
    type_parameters: tuple[str, ...] = (),
    method_parameters: tuple[str, ...] = (),
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for fragment in split_top(text):
        if not fragment or fragment == "...":
            continue
        is_out = bool(re.match(r"^\[out\]", fragment.strip()))
        fragment = re.sub(r"^\[(in|out|opt)\]\s*", "", fragment.strip())
        tokens = fragment.rsplit(" ", 1)
        if len(tokens) == 2 and re.match(r"^'?[A-Za-z_][\w]*'?$", tokens[1]):
            type_text, name = tokens[0], tokens[1].strip("'")
        else:
            type_text, name = fragment, ""
        mapped = normalize_type(
            positional_generics(type_text, type_parameters, method_parameters))
        result.append({
            "name": name,
            "type": mapped,
            "ref": mapped.endswith("&"),
            "out": is_out,
            "in": False,
            "optional": False,
        })
    return result


DIRECTIVE = re.compile(
    r"\b(?:marshal|modopt|modreq|pinvokeimpl)\s*\([^()]*(?:\([^()]*\)[^()]*)*\)")


def strip_directives(text: str) -> str:
    """Remove marshalling and custom-modifier directives.

    These carry their own parentheses, so they must be removed before the
    parameter list is located or `split_call` locks on to the wrong `(`.
    """
    previous = None
    while previous != text:
        previous = text
        text = DIRECTIVE.sub(" ", text)
    return re.sub(r"\s+", " ", text).strip()


def split_call(text: str) -> tuple[str, str, str] | None:
    """Split `head(parameters)tail` at the top-level parameter list."""
    depth = 0
    start = None
    for index, char in enumerate(text):
        if char == "<":
            depth += 1
        elif char == ">":
            depth -= 1
        elif char == "(" and depth == 0:
            start = index
            break
    if start is None:
        return None
    depth = 0
    for index in range(start, len(text)):
        if text[index] == "(":
            depth += 1
        elif text[index] == ")":
            depth -= 1
            if depth == 0:
                return text[:start], text[start + 1:index], text[index + 1:]
    return None


def split_generic_suffix(head: str) -> tuple[str, tuple[str, ...]]:
    """Split `Name<a, b>` into its name and its generic parameter names."""
    head = head.strip()
    if not head.endswith(">"):
        return head, ()
    depth = 0
    for index in range(len(head) - 1, -1, -1):
        if head[index] == ">":
            depth += 1
        elif head[index] == "<":
            depth -= 1
            if depth == 0:
                names = tuple(
                    item.split()[-1].strip("'")
                    for item in split_top(head[index + 1:-1]) if item.split()
                )
                return head[:index].strip(), names
    return head, ()


INTEGER_LITERAL = re.compile(
    r"^(int8|int16|int32|int64|uint8|uint16|uint32|uint64|"
    r"unsigned int8|unsigned int16|unsigned int32|unsigned int64|char|bool)"
    r"\((.*)\)$")
FLOAT_LITERAL = re.compile(r"^(float32|float64)\((.*)\)$")

# The retained contract records literal values in their decimal C# spelling,
# so a hexadecimal IL literal is normalised rather than compared verbatim.
LITERAL_WIDTH = {
    "int8": 8, "uint8": 8, "unsigned int8": 8, "bool": 8,
    "int16": 16, "uint16": 16, "unsigned int16": 16, "char": 16,
    "int32": 32, "uint32": 32, "unsigned int32": 32,
    "int64": 64, "uint64": 64, "unsigned int64": 64,
}
SIGNED_LITERAL = {"int8", "int16", "int32", "int64"}


def parse_literal(text: str) -> str | None:
    match = re.search(r"=\s*(.*)$", text)
    if not match:
        return None
    value = match.group(1).strip()
    found = INTEGER_LITERAL.match(value)
    if found:
        kind, raw = found.group(1), found.group(2).strip()
        try:
            number = int(raw, 0)
        except ValueError:
            return raw
        width = LITERAL_WIDTH.get(kind)
        if width and kind in SIGNED_LITERAL and number >= (1 << (width - 1)):
            number -= 1 << width
        return str(number)
    found = FLOAT_LITERAL.match(value)
    if found:
        raw = found.group(2).strip()
        try:
            number = float.fromhex(raw) if raw.lower().startswith("0x") else float(raw)
        except ValueError:
            return raw
        # The retained contract stores a Single constant in the CLR's default
        # `Single.ToString()` rendering, which is seven significant digits.
        # ikdasm prints the full decimal expansion of the same bits.
        digits = 7 if found.group(1) == "float32" else 15
        return f"{number:.{digits}G}"
    return value or None


# ----------------------------------------------------------------------------
# Embedded managed resources
#
# A .NET assembly's user-visible messages are not IL literals: the code loads a
# resource KEY and the runtime resolves it against an embedded string table. A
# message this binding reproduces is therefore a fact about that table, and it
# is read out of the binary here rather than transcribed. Both the XNA audit
# and the BCL authority audit use these readers, so the two agree by
# construction about what an assembly says.
# ----------------------------------------------------------------------------

RESOURCE_MAGIC = 0xBEEFCACE


def _rva_to_offset(data: bytes, sections: list[tuple[int, int, int]], rva: int) -> int:
    for virtual_address, virtual_size, raw_pointer in sections:
        if virtual_address <= rva < virtual_address + virtual_size:
            return raw_pointer + (rva - virtual_address)
    raise ValueError(f"RVA 0x{rva:08x} is in no section")


def embedded_resource(data: bytes, name: str) -> bytes | None:
    """One embedded managed resource, located through the PE and CLI headers.

    Nothing is written to disk and no external tool is invoked: the resource is
    reached by walking the PE optional header's CLI data directory to the CLI
    header's own `Resources` directory, which is exactly how the runtime
    reaches it. The blob is length-prefixed there.

    Only `mscorlib`'s single `System.Resources.ResourceReader` set is needed,
    so the lookup is by reader identity rather than by a metadata name: the
    resource whose header names that reader IS the string table, and an
    assembly carrying none returns `None` rather than a guess.
    """
    if data[:2] != b"MZ":
        return None
    pe = int.from_bytes(data[0x3C:0x40], "little")
    if data[pe:pe + 4] != b"PE\0\0":
        return None
    coff = pe + 4
    section_count = int.from_bytes(data[coff + 2:coff + 4], "little")
    optional_size = int.from_bytes(data[coff + 16:coff + 18], "little")
    optional = coff + 20
    magic = int.from_bytes(data[optional:optional + 2], "little")
    directories = optional + (112 if magic == 0x20B else 96)
    section_table = optional + optional_size
    sections: list[tuple[int, int, int]] = []
    for index in range(section_count):
        entry = section_table + index * 40
        sections.append((
            int.from_bytes(data[entry + 12:entry + 16], "little"),
            max(
                int.from_bytes(data[entry + 8:entry + 12], "little"),
                int.from_bytes(data[entry + 16:entry + 20], "little"),
            ),
            int.from_bytes(data[entry + 20:entry + 24], "little"),
        ))
    cli_rva = int.from_bytes(
        data[directories + 14 * 8:directories + 14 * 8 + 4], "little")
    if not cli_rva:
        return None
    cli = _rva_to_offset(data, sections, cli_rva)
    resources_rva = int.from_bytes(data[cli + 24:cli + 28], "little")
    resources_size = int.from_bytes(data[cli + 28:cli + 32], "little")
    if not resources_rva or not resources_size:
        return None
    base = _rva_to_offset(data, sections, resources_rva)
    offset = 0
    while offset + 4 <= resources_size:
        length = int.from_bytes(data[base + offset:base + offset + 4], "little")
        blob = data[base + offset + 4:base + offset + 4 + length]
        if (
            len(blob) >= 4 and
            int.from_bytes(blob[:4], "little") == RESOURCE_MAGIC and
            b"System.Resources.ResourceReader" in blob[:512]
        ):
            return blob
        offset += 4 + length
    return None


def resource_strings(blob: bytes) -> dict[str, str]:
    """Every string entry of a v2 `System.Resources.ResourceReader` set.

    The format is fixed: a magic, a reader-header block, the format version,
    the entry and type counts, the type names, an eight-byte alignment, the
    name hashes, the name offsets, the data-section offset, then a name table
    of UTF-16 names each followed by its data offset. A string value is the
    7-bit-length-prefixed UTF-8 written by `BinaryWriter.Write(string)` under
    type index 1.
    """
    position = 0

    def uint32() -> int:
        nonlocal position
        value = int.from_bytes(blob[position:position + 4], "little")
        position += 4
        return value

    def seven_bit() -> int:
        nonlocal position
        value = shift = 0
        while True:
            byte = blob[position]
            position += 1
            value |= (byte & 0x7F) << shift
            if not byte & 0x80:
                return value
            shift += 7

    def prefixed() -> str:
        nonlocal position
        count = seven_bit()
        text = blob[position:position + count].decode("utf-8", "replace")
        position += count
        return text

    if uint32() != RESOURCE_MAGIC:
        return {}
    uint32()                                   # reader-header version
    header_size = uint32()
    position += header_size                    # reader and set type names
    uint32()                                   # resource-set format version
    entries = uint32()
    type_count = uint32()
    for _ in range(type_count):
        prefixed()
    while position & 7:
        position += 1
    position += 4 * entries                    # name hashes
    offsets = [uint32() for _ in range(entries)]
    data_section = uint32()
    name_section = position
    found: dict[str, str] = {}
    for offset in offsets:
        position = name_section + offset
        count = seven_bit()
        name = blob[position:position + count].decode("utf-16-le", "replace")
        position += count
        value_offset = uint32()
        position = data_section + value_offset
        if seven_bit() == 1:                   # ResourceTypeCode.String
            found[name] = prefixed()
    return found


def sentinel_checks(
    by_type: dict[str, dict[str, Any]],
) -> tuple[int, list[str]]:
    """Independently-expected facts about the selected families.

    These are written from the documented .NET Framework 4.0 contract, not read
    back from the extraction, so an extractor that produced an empty, truncated
    or systematically wrong shape fails here even though it would happily agree
    with a manifest it had itself generated. This is the check that stops the
    calibration being vacuous.
    """
    failures: list[str] = []
    checks = 0

    def require(condition: bool, message: str) -> None:
        nonlocal checks
        checks += 1
        if not condition:
            failures.append(message)

    def members_of(type_name: str, kind: str, member_name: str) -> list[dict[str, Any]]:
        record = by_type.get(type_name)
        if record is None:
            return []
        return [
            item for item in record["members"]
            if item["kind"] == kind and item["name"] == member_name
        ]

    for name in (COLLECTION, READONLY, LIST, ILIST):
        require(name in by_type, f"{name} was not extracted at all")

    collection = by_type.get(COLLECTION)
    readonly = by_type.get(READONLY)
    if collection is None or readonly is None:
        return checks, failures

    # -- shape of the two base families ------------------------------------
    require(collection["kind"] == "class", "Collection<T> is not a class")
    require(not collection["sealed"], "Collection<T> is sealed")
    require(not collection["abstract"], "Collection<T> is abstract")
    require(collection["baseType"] == "System.Object",
            "Collection<T> does not derive directly from System.Object")
    require(collection["genericArity"] == 1,
            "Collection<T> is not of generic arity 1")
    require(readonly["kind"] == "class", "ReadOnlyCollection<T> is not a class")
    require(readonly["baseType"] == "System.Object",
            "ReadOnlyCollection<T> does not derive directly from System.Object")
    require(readonly["genericArity"] == 1,
            "ReadOnlyCollection<T> is not of generic arity 1")

    # The CLR hierarchy is two siblings, NOT an inheritance chain. Getting this
    # backwards would produce a Swift hierarchy in which a mutable collection
    # inherits a read-only one.
    require(collection["baseType"] != READONLY,
            "Collection<T> was extracted as deriving from ReadOnlyCollection<T>")
    require(readonly["baseType"] != COLLECTION,
            "ReadOnlyCollection<T> was extracted as deriving from Collection<T>")

    for name, record in ((COLLECTION, collection), (READONLY, readonly)):
        for interface in (
            "System.Collections.Generic.IList`1[!T]",
            "System.Collections.Generic.ICollection`1[!T]",
            "System.Collections.Generic.IEnumerable`1[!T]",
            "System.Collections.IList",
            "System.Collections.ICollection",
            "System.Collections.IEnumerable",
        ):
            require(interface in record["directInterfaces"],
                    f"{name} does not declare {interface}")

    # -- constructors -------------------------------------------------------
    collection_ctors = members_of(COLLECTION, "constructor", ".ctor")
    require(len(collection_ctors) == 2,
            f"Collection<T> should declare exactly 2 public constructors, "
            f"found {len(collection_ctors)}")
    require(any(not item["parameters"] for item in collection_ctors),
            "Collection<T> has no parameterless constructor")
    require(any(
        len(item["parameters"]) == 1 and
        item["parameters"][0]["type"] == "System.Collections.Generic.IList`1[!0]"
        for item in collection_ctors),
        "Collection<T> has no IList<T> wrapping constructor")

    readonly_ctors = members_of(READONLY, "constructor", ".ctor")
    require(len(readonly_ctors) == 1,
            f"ReadOnlyCollection<T> should declare exactly 1 public "
            f"constructor, found {len(readonly_ctors)}")
    require(bool(readonly_ctors) and
            len(readonly_ctors[0]["parameters"]) == 1 and
            readonly_ctors[0]["parameters"][0]["type"] ==
            "System.Collections.Generic.IList`1[!0]",
            "ReadOnlyCollection<T>'s only constructor does not take IList<T>")
    # There is deliberately no parameterless constructor: a read-only view must
    # always be a view OF something.
    require(all(item["parameters"] for item in readonly_ctors),
            "ReadOnlyCollection<T> declares a parameterless constructor")

    # -- the four protected virtual hooks ----------------------------------
    for hook, arity in (("ClearItems", 0), ("InsertItem", 2),
                        ("RemoveItem", 1), ("SetItem", 2)):
        candidates = members_of(COLLECTION, "method", hook)
        require(len(candidates) == 1,
                f"Collection<T>.{hook} should be declared exactly once, "
                f"found {len(candidates)}")
        if not candidates:
            continue
        member = candidates[0]
        require(member["access"] == "protected",
                f"Collection<T>.{hook} is not protected")
        require(member["overridable"],
                f"Collection<T>.{hook} is not an overridable virtual hook")
        require(not member["abstract"],
                f"Collection<T>.{hook} is abstract")
        require(len(member["parameters"]) == arity,
                f"Collection<T>.{hook} should take {arity} parameters")
        require(member["returnType"] == "System.Void",
                f"Collection<T>.{hook} does not return void")

    # ReadOnlyCollection<T> deliberately has none of them: it is not a
    # mutation point, so there is nothing to hook.
    for hook in ("ClearItems", "InsertItem", "RemoveItem", "SetItem"):
        require(not members_of(READONLY, "method", hook),
                f"ReadOnlyCollection<T> unexpectedly declares {hook}")

    # -- the public surface is sealed against overriding --------------------
    # Every public member of Collection<T> is `virtual final` -- a sealed
    # interface implementation. A subclass changes behaviour ONLY through the
    # four hooks, and a projection that made the public methods overridable
    # would invent an extension point the CLR does not have.
    for method in ("Add", "Clear", "Contains", "CopyTo", "GetEnumerator",
                   "IndexOf", "Insert", "Remove", "RemoveAt"):
        candidates = members_of(COLLECTION, "method", method)
        require(len(candidates) == 1,
                f"Collection<T>.{method} should be declared exactly once")
        for member in candidates:
            require(member["access"] == "public",
                    f"Collection<T>.{method} is not public")
            require(not member["overridable"],
                    f"Collection<T>.{method} is overridable; the CLR seals it")

    # ReadOnlyCollection<T> exposes no mutator at all.
    for method in ("Add", "Clear", "Insert", "Remove", "RemoveAt"):
        require(not members_of(READONLY, "method", method),
                f"ReadOnlyCollection<T> unexpectedly exposes the mutator {method}")
    for method in ("Contains", "CopyTo", "GetEnumerator", "IndexOf"):
        require(len(members_of(READONLY, "method", method)) == 1,
                f"ReadOnlyCollection<T>.{method} is missing")

    # -- Count, Item and the protected Items view --------------------------
    for name, record_name in ((COLLECTION, "Collection<T>"),
                              (READONLY, "ReadOnlyCollection<T>")):
        count = members_of(name, "property", "Count")
        require(len(count) == 1, f"{record_name}.Count is missing")
        for member in count:
            require(member["type"] == "System.Int32",
                    f"{record_name}.Count is not Int32")
            require(member["getAccess"] == "public" and
                    member["setAccess"] is None,
                    f"{record_name}.Count is not a public get-only property")
        items = members_of(name, "property", "Items")
        require(len(items) == 1, f"{record_name}.Items is missing")
        for member in items:
            require(member["getAccess"] == "protected",
                    f"{record_name}.Items is not a protected getter")
            require(member["setAccess"] is None,
                    f"{record_name}.Items has a setter")
            require(member["type"] == "System.Collections.Generic.IList`1[!0]",
                    f"{record_name}.Items is not IList<T>")

    # The read/write asymmetry between the two families is the whole point of
    # having two of them.
    collection_item = members_of(COLLECTION, "property", "Item")
    require(len(collection_item) == 1, "Collection<T>.Item is missing")
    for member in collection_item:
        require(member["getAccess"] == "public" and
                member["setAccess"] == "public",
                "Collection<T>.Item is not a public read/write indexer")
        require(len(member["parameters"]) == 1 and
                member["parameters"][0]["type"] == "System.Int32",
                "Collection<T>.Item is not indexed by Int32")
        require(member["type"] == "!0",
                "Collection<T>.Item is not typed by the generic parameter")
    readonly_item = members_of(READONLY, "property", "Item")
    require(len(readonly_item) == 1, "ReadOnlyCollection<T>.Item is missing")
    for member in readonly_item:
        require(member["getAccess"] == "public",
                "ReadOnlyCollection<T>.Item has no public getter")
        require(member["setAccess"] is None,
                "ReadOnlyCollection<T>.Item has a setter; it must be read-only")

    # -- the backing store --------------------------------------------------
    backing = by_type.get(LIST)
    if backing is not None:
        require(backing["kind"] == "class", "List<T> is not a class")
        require(backing["genericArity"] == 1, "List<T> is not of generic arity 1")
        require("System.Collections.Generic.IList`1[!T]" in
                backing["directInterfaces"],
                "List<T> does not implement IList<T>, so it cannot be the "
                "backing store Collection<T>..ctor() allocates")
        require(any(
            not item["parameters"]
            for item in members_of(LIST, "constructor", ".ctor")),
            "List<T> has no parameterless constructor")
    contract = by_type.get(ILIST)
    if contract is not None:
        require(contract["kind"] == "interface", "IList<T> is not an interface")
        for method in ("IndexOf", "Insert", "RemoveAt"):
            require(len(members_of(ILIST, "method", method)) == 1,
                    f"IList<T>.{method} is missing")

    # ------------------------------------------------------------------
    # The exception families.
    #
    # These decide whether eight XNA types get the right base, the right
    # constructors and the right observable state. Every fact below is stated
    # from the documented .NET Framework 4.0 contract, independently of the
    # extractor, exactly as the collection facts above are.
    # ------------------------------------------------------------------
    exception = by_type.get(EXCEPTION)
    system_exception = by_type.get(SYSTEM_EXCEPTION)
    external = by_type.get(EXTERNAL_EXCEPTION)
    for name in (EXCEPTION, SYSTEM_EXCEPTION, EXTERNAL_EXCEPTION):
        require(name in by_type, f"{name} was not extracted at all")
    if exception is None or system_exception is None or external is None:
        return checks, failures

    for name, record in ((EXCEPTION, exception),
                         (SYSTEM_EXCEPTION, system_exception),
                         (EXTERNAL_EXCEPTION, external)):
        require(record["kind"] == "class", f"{name} is not a class")
        require(not record["sealed"], f"{name} is sealed")
        require(not record["abstract"], f"{name} is abstract")
        require(record["genericArity"] == 0, f"{name} is generic")

    # The chain is exactly three links, and the middle one is real. Collapsing
    # ExternalException straight onto Exception would drop SystemException's
    # substituted message and its HResult.
    require(exception["baseType"] == "System.Object",
            "Exception does not derive directly from System.Object")
    require(system_exception["baseType"] == EXCEPTION,
            "SystemException does not derive directly from Exception")
    require(external["baseType"] == SYSTEM_EXCEPTION,
            "ExternalException's direct base is not SystemException")
    require(external["baseType"] != EXCEPTION,
            "ExternalException was extracted as deriving from Exception "
            "directly, which would erase SystemException")

    for interface in ("System.Runtime.Serialization.ISerializable",
                      "System.Runtime.InteropServices._Exception"):
        require(interface in exception["directInterfaces"],
                f"Exception does not declare {interface}")

    # Constructors: three public plus one protected serialization constructor
    # on Exception and SystemException; ExternalException adds the errorCode
    # overload.
    for name, public_count in ((EXCEPTION, 3), (SYSTEM_EXCEPTION, 3),
                               (EXTERNAL_EXCEPTION, 4)):
        constructors = members_of(name, "constructor", ".ctor")
        public = [item for item in constructors if item["access"] == "public"]
        protected = [
            item for item in constructors if item["access"] == "protected"]
        require(len(public) == public_count,
                f"{name} should declare {public_count} public constructors, "
                f"found {len(public)}")
        require(len(protected) == 1,
                f"{name} should declare exactly one protected serialization "
                f"constructor, found {len(protected)}")
        require(any(not item["parameters"] for item in public),
                f"{name} has no public parameterless constructor")
        require(any(
            [entry["type"] for entry in item["parameters"]] ==
            ["System.String"] for item in public),
            f"{name} has no (String) constructor")
        require(any(
            [entry["type"] for entry in item["parameters"]] ==
            ["System.String", EXCEPTION] for item in public),
            f"{name} has no (String, Exception) constructor")
        require(all(
            [entry["type"] for entry in item["parameters"]] == [
                "System.Runtime.Serialization.SerializationInfo",
                "System.Runtime.Serialization.StreamingContext",
            ] for item in protected),
            f"{name}'s protected constructor is not "
            "(SerializationInfo, StreamingContext)")
    require(any(
        [entry["type"] for entry in item["parameters"]] ==
        ["System.String", "System.Int32"]
        for item in members_of(EXTERNAL_EXCEPTION, "constructor", ".ctor")),
        "ExternalException has no (String, Int32 errorCode) constructor")

    # Message is overridable; InnerException is `virtual final` and is NOT an
    # override point. Projecting either the wrong way round changes what an
    # XNA subclass can do.
    message = members_of(EXCEPTION, "property", "Message")
    require(len(message) == 1, "Exception.Message is missing")
    for member in message:
        require(member["type"] == "System.String",
                "Exception.Message is not a String")
        require(member["getAccess"] == "public" and
                member["setAccess"] is None,
                "Exception.Message is not a public get-only property")
        require(member["getOverridable"],
                "Exception.Message is not overridable")
    inner = members_of(EXCEPTION, "property", "InnerException")
    require(len(inner) == 1, "Exception.InnerException is missing")
    for member in inner:
        require(member["type"] == EXCEPTION,
                "Exception.InnerException is not an Exception")
        require(member["getAccess"] == "public" and
                member["setAccess"] is None,
                "Exception.InnerException is not a public get-only property")
        require(not member["getOverridable"],
                "Exception.InnerException is overridable; the CLR seals it")

    # HResult is the protected state SystemException and ExternalException
    # write and ErrorCode reads.
    hresult = members_of(EXCEPTION, "property", "HResult")
    require(len(hresult) == 1, "Exception.HResult is missing")
    for member in hresult:
        require(member["type"] == "System.Int32",
                "Exception.HResult is not an Int32")
        require(member["getAccess"] == "protected" and
                member["setAccess"] == "protected",
                "Exception.HResult is not a protected read/write property")

    help_link = members_of(EXCEPTION, "property", "HelpLink")
    require(len(help_link) == 1, "Exception.HelpLink is missing")
    for member in help_link:
        require(member["getAccess"] == "public" and
                member["setAccess"] == "public",
                "Exception.HelpLink is not a public read/write property")
        require(member["getOverridable"] and member["setOverridable"],
                "Exception.HelpLink is not overridable in both directions")

    base_exception = members_of(EXCEPTION, "method", "GetBaseException")
    require(len(base_exception) == 1, "Exception.GetBaseException is missing")
    for member in base_exception:
        require(member["returnType"] == EXCEPTION,
                "Exception.GetBaseException does not return an Exception")
        require(not member["parameters"],
                "Exception.GetBaseException takes parameters")

    error_code = members_of(EXTERNAL_EXCEPTION, "property", "ErrorCode")
    require(len(error_code) == 1, "ExternalException.ErrorCode is missing")
    for member in error_code:
        require(member["type"] == "System.Int32",
                "ExternalException.ErrorCode is not an Int32")
        require(member["getAccess"] == "public" and
                member["setAccess"] is None,
                "ExternalException.ErrorCode is not a public get-only property")
        require(member["getOverridable"],
                "ExternalException.ErrorCode is not overridable")

    # SystemException adds no member of its own. If it ever appears to, the
    # projection would owe a member it has no evidence for.
    require(not [
        item for item in system_exception["members"]
        if item["kind"] != "constructor"
    ], "SystemException declares a non-constructor member")

    # The CLR runtime services that are deliberately NOT projected must still
    # be present in the pinned shape, so that dropping them from the Swift
    # surface stays a recorded decision rather than an extraction accident.
    for absent in ("StackTrace", "Source", "TargetSite", "Data"):
        require(len(members_of(EXCEPTION, "property", absent)) == 1,
                f"Exception.{absent} vanished from the extraction")
    for absent in ("ToString", "GetObjectData", "GetType"):
        require(len(members_of(EXCEPTION, "method", absent)) == 1,
                f"Exception.{absent} vanished from the extraction")

    # ------------------------------------------------------------------
    # The dictionary family.
    # ------------------------------------------------------------------
    dictionary = by_type.get(DICTIONARY)
    for name in (DICTIONARY, KEY_COLLECTION, VALUE_COLLECTION,
                 DICT_ENUMERATOR, KEY_VALUE_PAIR, IEQUALITY_COMPARER,
                 IDICTIONARY):
        require(name in by_type, f"{name} was not extracted at all")
    if dictionary is None:
        return checks, failures

    require(dictionary["kind"] == "class", "Dictionary<K,V> is not a class")
    require(not dictionary["sealed"],
            "Dictionary<K,V> is sealed, so LaunchParameters could not derive "
            "from it")
    require(dictionary["baseType"] == "System.Object",
            "Dictionary<K,V> does not derive directly from System.Object")
    require(dictionary["genericArity"] == 2,
            "Dictionary<K,V> is not of generic arity 2")
    for interface in (
        "System.Collections.Generic.ICollection`1"
        "[System.Collections.Generic.KeyValuePair`2[!TKey,!TValue]]",
        "System.Collections.Generic.IDictionary`2[!TKey,!TValue]",
        "System.Collections.Generic.IEnumerable`1"
        "[System.Collections.Generic.KeyValuePair`2[!TKey,!TValue]]",
        "System.Collections.ICollection",
        "System.Collections.IDictionary",
        "System.Collections.IEnumerable",
        "System.Runtime.Serialization.IDeserializationCallback",
        "System.Runtime.Serialization.ISerializable",
    ):
        require(interface in dictionary["directInterfaces"],
                f"Dictionary<K,V> does not declare {interface}")

    # NOTHING on the public surface is an override point. `GetObjectData` and
    # `OnDeserialization` are the only overridable members, and neither is
    # projected, so a Swift projection whose methods were `open` would invent
    # extension points the CLR does not have.
    for method in ("Add", "Clear", "ContainsKey", "ContainsValue", "Remove",
                   "TryGetValue", "GetEnumerator"):
        candidates = members_of(DICTIONARY, "method", method)
        require(len(candidates) == 1,
                f"Dictionary<K,V>.{method} should be declared exactly once, "
                f"found {len(candidates)}")
        for member in candidates:
            require(member["access"] == "public",
                    f"Dictionary<K,V>.{method} is not public")
            require(not member["overridable"],
                    f"Dictionary<K,V>.{method} is overridable; the CLR does "
                    "not make it an override point")
    for method in ("GetObjectData", "OnDeserialization"):
        candidates = members_of(DICTIONARY, "method", method)
        require(len(candidates) == 1,
                f"Dictionary<K,V>.{method} is missing")
        for member in candidates:
            require(member["overridable"],
                    f"Dictionary<K,V>.{method} should be the one kind of "
                    "overridable member this class has")

    # Six public constructors plus the protected serialization one.
    constructors = members_of(DICTIONARY, "constructor", ".ctor")
    public_constructors = [
        item for item in constructors if item["access"] == "public"]
    require(len(public_constructors) == 6,
            f"Dictionary<K,V> should declare 6 public constructors, found "
            f"{len(public_constructors)}")
    require(len([
        item for item in constructors if item["access"] == "protected"]) == 1,
        "Dictionary<K,V> should declare one protected serialization "
        "constructor")
    for signature in (
        [],
        ["System.Int32"],
        [f"{IEQUALITY_COMPARER}[!0]"],
        ["System.Int32", f"{IEQUALITY_COMPARER}[!0]"],
        [f"{IDICTIONARY}[!0,!1]"],
        [f"{IDICTIONARY}[!0,!1]", f"{IEQUALITY_COMPARER}[!0]"],
    ):
        require(any(
            [entry["type"] for entry in item["parameters"]] == signature
            for item in public_constructors),
            f"Dictionary<K,V> has no constructor taking {signature}")

    # The indexer is read/write while Keys, Values, Count and Comparer are
    # get-only. Getting the indexer wrong is the difference between Add
    # refusing a duplicate and the setter overwriting it.
    indexer = members_of(DICTIONARY, "property", "Item")
    require(len(indexer) == 1, "Dictionary<K,V>.Item is missing")
    for member in indexer:
        require(member["getAccess"] == "public" and
                member["setAccess"] == "public",
                "Dictionary<K,V>.Item is not a public read/write indexer")
        require(len(member["parameters"]) == 1 and
                member["parameters"][0]["type"] == "!0",
                "Dictionary<K,V>.Item is not indexed by the key type")
        require(member["type"] == "!1",
                "Dictionary<K,V>.Item does not yield the value type")
    for name, clr_type in (
        ("Count", "System.Int32"),
        ("Comparer", f"{IEQUALITY_COMPARER}[!0]"),
        ("Keys", f"{KEY_COLLECTION}[!0,!1]"),
        ("Values", f"{VALUE_COLLECTION}[!0,!1]"),
    ):
        candidates = members_of(DICTIONARY, "property", name)
        require(len(candidates) == 1, f"Dictionary<K,V>.{name} is missing")
        for member in candidates:
            require(member["getAccess"] == "public" and
                    member["setAccess"] is None,
                    f"Dictionary<K,V>.{name} is not a public get-only property")
            require(member["type"] == clr_type,
                    f"Dictionary<K,V>.{name} is {member['type']}, not "
                    f"{clr_type}")

    # `GetEnumerator` returns the nested struct, not the interface, and that
    # struct carries nothing beyond the enumeration contract.
    for member in members_of(DICTIONARY, "method", "GetEnumerator"):
        require(member["returnType"] == f"{DICT_ENUMERATOR}[!0,!1]",
                "Dictionary<K,V>.GetEnumerator does not return its own "
                "nested Enumerator")
    enumerator = by_type.get(DICT_ENUMERATOR)
    if enumerator is not None:
        require(enumerator["kind"] == "struct",
                "Dictionary<K,V>.Enumerator is not a struct")
        require({item["name"] for item in enumerator["members"]} ==
                {"Current", "MoveNext", "Dispose"},
                "Dictionary<K,V>.Enumerator carries more than the enumeration "
                "contract, so projecting it as CNAEnumerator would drop "
                "something")

    # The two collection views are sealed live views, and neither exposes a
    # mutator or a public Contains.
    for name in (KEY_COLLECTION, VALUE_COLLECTION):
        view = by_type.get(name)
        if view is None:
            continue
        require(view["kind"] == "class", f"{name} is not a class")
        require(view["sealed"], f"{name} is not sealed")
        require(len(members_of(name, "constructor", ".ctor")) == 1,
                f"{name} should declare exactly one constructor")
        for method in ("Add", "Clear", "Remove", "Contains"):
            require(not members_of(name, "method", method),
                    f"{name} unexpectedly exposes {method}")
        require(len(members_of(name, "method", "CopyTo")) == 1,
                f"{name}.CopyTo is missing")
        require(len(members_of(name, "property", "Count")) == 1,
                f"{name}.Count is missing")

    pair = by_type.get(KEY_VALUE_PAIR)
    if pair is not None:
        require(pair["kind"] == "struct", "KeyValuePair<K,V> is not a struct")
        for name, clr_type in (("Key", "!0"), ("Value", "!1")):
            candidates = members_of(KEY_VALUE_PAIR, "property", name)
            require(len(candidates) == 1, f"KeyValuePair<K,V>.{name} is missing")
            for member in candidates:
                require(member["setAccess"] is None,
                        f"KeyValuePair<K,V>.{name} has a setter")
                require(member["type"] == clr_type,
                        f"KeyValuePair<K,V>.{name} is not {clr_type}")

    # ------------------------------------------------------------------
    # System.Attribute.
    # ------------------------------------------------------------------
    attribute = by_type.get(ATTRIBUTE)
    require(ATTRIBUTE in by_type, "System.Attribute was not extracted at all")
    if attribute is not None:
        require(attribute["kind"] == "class", "Attribute is not a class")
        require(attribute["abstract"],
                "Attribute is not abstract; the CLR forbids constructing one")
        require(not attribute["sealed"], "Attribute is sealed")
        require(attribute["baseType"] == "System.Object",
                "Attribute does not derive directly from System.Object")
        require(attribute["genericArity"] == 0, "Attribute is generic")
        constructors = members_of(ATTRIBUTE, "constructor", ".ctor")
        require(len(constructors) == 1,
                f"Attribute should declare exactly one constructor, found "
                f"{len(constructors)}")
        for member in constructors:
            require(member["access"] == "protected",
                    "Attribute's constructor is not protected")
            require(not member["parameters"],
                    "Attribute's constructor takes parameters")
        default_attribute = members_of(ATTRIBUTE, "method", "IsDefaultAttribute")
        require(len(default_attribute) == 1,
                "Attribute.IsDefaultAttribute is missing")
        for member in default_attribute:
            require(member["returnType"] == "System.Boolean",
                    "Attribute.IsDefaultAttribute does not return Boolean")
            require(member["overridable"],
                    "Attribute.IsDefaultAttribute is not overridable")
            require(not member["parameters"],
                    "Attribute.IsDefaultAttribute takes parameters")
        # The reflection surface must stay in the pinned shape, so that not
        # projecting it stays a recorded decision rather than an extraction
        # accident.
        for reflective in ("GetCustomAttribute", "GetCustomAttributes",
                           "IsDefined", "Match", "Equals", "GetHashCode"):
            require(bool(members_of(ATTRIBUTE, "method", reflective)),
                    f"Attribute.{reflective} vanished from the extraction")
        require(len(members_of(ATTRIBUTE, "property", "TypeId")) == 1,
                "Attribute.TypeId vanished from the extraction")

    comparer = by_type.get(IEQUALITY_COMPARER)
    if comparer is not None:
        require(comparer["kind"] == "interface",
                "IEqualityComparer<T> is not an interface")
        require(comparer["genericArity"] == 1,
                "IEqualityComparer<T> is not of generic arity 1")
        require({item["name"] for item in comparer["members"]} ==
                {"Equals", "GetHashCode"},
                "IEqualityComparer<T> is not exactly Equals and GetHashCode")
        for member in members_of(IEQUALITY_COMPARER, "method", "GetHashCode"):
            require(member["returnType"] == "System.Int32",
                    "IEqualityComparer<T>.GetHashCode does not return Int32")
    return checks, failures



class Parser:
    """Reconstruct public type shape from ikdasm IL output."""

    def __init__(self, il: str) -> None:
        self.lines = il.splitlines()
        self.types: dict[str, dict[str, Any]] = {}

    def parse(self) -> dict[str, dict[str, Any]]:
        stack: list[dict[str, Any]] = []
        index = 0
        while index < len(self.lines):
            line = self.lines[index]
            stripped = line.strip()
            if stripped.startswith(".class "):
                declaration, index = self.gather(index, ("{",))
                stack.append(self.begin_class(declaration, stack))
                continue
            if stripped.startswith("} // end of class"):
                stack.pop()
                index += 1
                continue
            if not stack or stack[-1] is None:
                index += 1
                continue
            owner = stack[-1]
            if stripped.startswith(".method "):
                declaration, index = self.gather(index, ("cil managed", "runtime managed", "cil runtime managed", "managed"))
                self.add_method(owner, declaration)
                index = self.skip_block(index, "} // end of method")
                continue
            if stripped.startswith(".property "):
                declaration, index = self.gather(index, ("{",))
                index = self.add_property(owner, declaration, index)
                continue
            if stripped.startswith(".event "):
                declaration, index = self.gather(index, ("{",))
                index = self.add_event(owner, declaration, index)
                continue
            if stripped.startswith(".field "):
                declaration, index = self.gather(index, None)
                self.add_field(owner, declaration)
                continue
            index += 1
        return self.types

    def gather(self, index: int, terminators: tuple[str, ...] | None) -> tuple[str, int]:
        """Collect a possibly multi-line declaration starting at `index`."""
        parts: list[str] = []
        while index < len(self.lines):
            raw = self.lines[index].strip()
            index += 1
            if terminators is None:
                parts.append(raw)
                break
            if raw.endswith("{"):
                parts.append(raw[:-1].strip())
                break
            parts.append(raw)
            joined = " ".join(parts)
            if any(joined.rstrip().endswith(item) for item in terminators):
                break
            if index < len(self.lines) and self.lines[index].strip() == "{":
                index += 1
                break
        return " ".join(part for part in parts if part), index

    def skip_block(self, index: int, end_marker: str) -> int:
        """Skip a method body, starting just after its opening brace.

        `end_marker` is the commented closing line `ikdasm` normally emits, but
        it does **not** always emit one: a `pinvokeimpl(...) ... preservesig`
        method whose body is nothing but `.custom` attributes closes with a
        bare `}`. Stopping only on the comment therefore ran past the method,
        past the *class*, and swallowed the following type's members into this
        one -- `System.Exception` absorbed all of `System.ValueType`'s. The
        brace depth is the real boundary, so a `}` at depth zero ends the body
        whether or not the comment is there. For a well-formed method the two
        agree, because that depth-zero `}` **is** the commented line, which is
        matched first; nothing about the XNA contract extraction changes.
        """
        depth = 0
        while index < len(self.lines):
            stripped = self.lines[index].strip()
            if stripped.startswith(end_marker):
                return index + 1
            if stripped == "{":
                depth += 1
            elif stripped.startswith("}"):
                if not depth:
                    return index + 1
                depth -= 1
            index += 1
        return index

    def begin_class(self, declaration: str, stack: list[dict[str, Any]]) -> dict[str, Any] | None:
        body = declaration[len(".class"):].strip()
        extends = None
        implements: list[str] = []
        match = re.search(r"\bextends\s+(.*?)(?:\s+implements\s+(.*))?$", body)
        if match:
            extends = normalize_type(match.group(1))
            if match.group(2):
                implements = [normalize_type(item) for item in split_top(match.group(2))]
            body = body[:match.start()].strip()
        else:
            match = re.search(r"\bimplements\s+(.*)$", body)
            if match:
                implements = [normalize_type(item) for item in split_top(match.group(1))]
                body = body[:match.start()].strip()
        tokens = body.split()
        name = tokens[-1] if tokens else ""
        attributes = set(tokens[:-1])
        # A generic class name carries its parameter list: `Foo`1<T>`.
        if ">" in body:
            head, _, _rest = body.partition("<")
            head_tokens = head.split()
            if head_tokens:
                name_index = body.index(head_tokens[-1])
                name, type_parameters = split_generic_suffix(body[name_index:])
                attributes = set(body[:name_index].split())
            else:
                type_parameters = ()
        else:
            name, type_parameters = split_generic_suffix(name)
        nested = "nested" in attributes
        parent = stack[-1] if stack else None
        if nested:
            visible = any(
                f"nested {item}" in body for item in ("public", "family", "famorassem")
            )
            if parent is None or not visible:
                return None
            full = f"{parent['name']}+{name}"
        else:
            if "public" not in attributes:
                return None
            full = name
        kind = "interface" if "interface" in attributes else (
            "enum" if extends == "System.Enum" else (
                "struct" if extends == "System.ValueType" else "class"
            )
        )
        record = {
            "name": full,
            "kind": kind,
            "sealed": "sealed" in attributes,
            "abstract": "abstract" in attributes,
            "baseType": extends,
            "directInterfaces": implements,
            "genericParameters": list(type_parameters),
            "members": [],
        }
        self.types[full] = record
        return record

    @staticmethod
    def access_of(attributes: set[str]) -> str | None:
        for key, value in ACCESS.items():
            if key in attributes:
                return value
        return None

    def add_method(self, owner: dict[str, Any], declaration: str) -> None:
        body = strip_directives(declaration[len(".method"):].strip())
        split = split_call(body)
        if split is None:
            return
        head, parameter_text, _tail = split
        head, method_parameters = split_generic_suffix(head)
        tokens = head.split()
        if not tokens:
            return
        name = tokens[-1].strip("'")
        rest = tokens[:-1]
        attributes: set[str] = set()
        return_tokens: list[str] = []
        known = {
            "public", "private", "family", "assembly", "famorassem", "famandassem",
            "static", "instance", "hidebysig", "specialname", "rtspecialname",
            "virtual", "abstract", "final", "newslot", "pinvokeimpl", "strict",
            "explicit", "reqsecobj", "unmanagedexp",
        }
        for token in rest:
            if token in known:
                attributes.add(token)
            else:
                return_tokens.append(token)
        access = self.access_of(attributes)
        if access is None:
            return
        if name == ".cctor":
            return
        type_parameters = tuple(owner.get("genericParameters") or ())
        return_type = normalize_type(positional_generics(
            " ".join(return_tokens), type_parameters, method_parameters))
        parameters = split_parameters(
            parameter_text, type_parameters, method_parameters)
        is_constructor = name == ".ctor"
        owner["members"].append({
            "kind": "constructor" if is_constructor else "method",
            "name": name,
            "static": "static" in attributes,
            "access": access,
            "returnType": None if is_constructor else return_type,
            "genericParameters": [
                {"name": item} for item in method_parameters
            ],
            "parameters": parameters,
            "_specialname": "specialname" in attributes,
        })

    def add_property(self, owner: dict[str, Any], declaration: str, index: int) -> int:
        body = strip_directives(declaration[len(".property"):].strip())
        split = split_call(body)
        if split is None:
            return self.skip_block(index, "} // end of property")
        head, parameter_text, _tail = split
        tokens = head.split()
        name = tokens[-1].strip("'")
        type_text = " ".join(tokens[:-1])
        type_text = re.sub(r"^\s*(instance|specialname|rtspecialname)\s+", "", type_text)
        while True:
            new = re.sub(r"^\s*(instance|specialname|rtspecialname)\s+", "", type_text)
            if new == type_text:
                break
            type_text = new
        static = "instance" not in head.split()
        get_access: str | None = None
        set_access: str | None = None
        cursor = index
        while cursor < len(self.lines):
            stripped = self.lines[cursor].strip()
            if stripped.startswith("} // end of property"):
                cursor += 1
                break
            if stripped.startswith(".get "):
                get_access = self.accessor_access(owner, stripped, f"get_{name}")
            elif stripped.startswith(".set "):
                set_access = self.accessor_access(owner, stripped, f"set_{name}")
            cursor += 1
        if get_access is None and set_access is None:
            return cursor
        type_parameters = tuple(owner.get("genericParameters") or ())
        owner["members"].append({
            "kind": "property",
            "name": name,
            "type": normalize_type(
                positional_generics(type_text, type_parameters, ())),
            "static": static,
            "get": get_access is not None,
            "set": set_access is not None,
            "getAccess": get_access,
            "setAccess": set_access,
            "parameters": split_parameters(parameter_text, type_parameters, ()),
        })
        return cursor

    @staticmethod
    def accessor_access(owner: dict[str, Any], line: str, accessor: str) -> str | None:
        for member in owner["members"]:
            if member.get("name") == accessor and member["kind"] == "method":
                return member.get("access")
        return None

    def add_event(self, owner: dict[str, Any], declaration: str, index: int) -> int:
        body = strip_directives(declaration[len(".event"):].strip())
        tokens = body.split()
        name = tokens[-1].strip("'")
        type_text = " ".join(tokens[:-1])
        type_text = re.sub(r"^\s*(specialname|rtspecialname)\s+", "", type_text).strip()
        add_access = None
        remove_access = None
        fires = False
        cursor = index
        while cursor < len(self.lines):
            stripped = self.lines[cursor].strip()
            if stripped.startswith("} // end of event"):
                cursor += 1
                break
            if stripped.startswith(".addon "):
                add_access = self.accessor_access(owner, stripped, f"add_{name}")
            elif stripped.startswith(".removeon "):
                remove_access = self.accessor_access(owner, stripped, f"remove_{name}")
            elif stripped.startswith(".fire "):
                fires = True
            cursor += 1
        if add_access is None and remove_access is None:
            return cursor
        if fires:
            owner.setdefault("_raisers", set()).add(f"raise_{name}")
        owner["members"].append({
            "kind": "event",
            "name": name,
            "type": normalize_type(positional_generics(
                type_text, tuple(owner.get("genericParameters") or ()), ())),
            "static": False,
            "add": add_access is not None,
            "remove": remove_access is not None,
        })
        return cursor

    def add_field(self, owner: dict[str, Any], declaration: str) -> None:
        body = strip_directives(declaration[len(".field"):].strip())
        value = None
        if " = " in body:
            head, _, _tail = body.partition(" = ")
            value = parse_literal(body)
            body = head.strip()
        tokens = body.split()
        if not tokens:
            return
        name = tokens[-1].strip("'")
        rest = tokens[:-1]
        attributes: set[str] = set()
        type_tokens: list[str] = []
        known = {
            "public", "private", "family", "assembly", "famorassem", "famandassem",
            "static", "literal", "initonly", "specialname", "rtspecialname",
            "notserialized", "marshal",
        }
        for token in rest:
            if token in known:
                attributes.add(token)
            else:
                type_tokens.append(token)
        access = self.access_of(attributes)
        if access is None:
            return
        owner["members"].append({
            "kind": "field",
            "name": name,
            # Recorded, not merely used to decide. `access_of` gates whether the
            # field is kept at all, and dropping the answer afterwards meant
            # `bcl_authority_audit.visible_members` -- which filters fields by
            # exactly this key -- saw None on every field and discarded all of
            # them. Every enum admitted as a BCL family therefore extracted
            # with no values at all, which is a vacuous admission of precisely
            # the kind that audit exists to refuse (Foundation 102).
            "access": access,
            "type": normalize_type(positional_generics(
                " ".join(type_tokens),
                tuple(owner.get("genericParameters") or ()), ())),
            "static": "static" in attributes or "literal" in attributes,
            "constant": "literal" in attributes,
            # `initonly` is CLR metadata (FieldAttributes.InitOnly), not a C#
            # convention, so it is part of the shape a projection must
            # reproduce: a readonly field may not become a mutable Swift `var`.
            # It is recorded separately from `constant`, because `literal` and
            # `initonly` are different attributes and a field never carries
            # both.
            "readonly": "initonly" in attributes,
            "value": value,
        })


def finalize(types: dict[str, dict[str, Any]]) -> dict[str, dict[str, Any]]:
    """Drop accessor methods that a property or event already represents."""
    for record in types.values():
        accessors: set[str] = set()
        for member in record["members"]:
            if member["kind"] == "property":
                accessors.add(f"get_{member['name']}")
                accessors.add(f"set_{member['name']}")
            elif member["kind"] == "event":
                accessors.add(f"add_{member['name']}")
                accessors.add(f"remove_{member['name']}")
                accessors.add(f"raise_{member['name']}")
        record["members"] = [
            member for member in record["members"]
            if not (
                member["kind"] == "method" and
                member.get("_specialname") and
                member["name"] in accessors
            )
        ]
        for member in record["members"]:
            member.pop("_specialname", None)
        record.pop("_raisers", None)
    return types


def member_key(member: dict[str, Any]) -> tuple:
    kind = member["kind"]
    if kind in ("method", "constructor"):
        return (
            kind, member["name"], bool(member.get("static")),
            tuple(
                (item.get("type"), bool(item.get("ref")), bool(item.get("out")))
                for item in member.get("parameters", [])
            ),
            member.get("returnType"),
        )
    if kind == "property":
        return (
            kind, member["name"], bool(member.get("static")), member.get("type"),
            bool(member.get("get")), bool(member.get("set")),
            tuple(item.get("type") for item in member.get("parameters", [])),
        )
    if kind == "event":
        return (kind, member["name"], member.get("type"))
    return (
        kind, member["name"], bool(member.get("static")), member.get("type"),
        bool(member.get("constant")), bool(member.get("readonly")),
        member.get("value"),
    )


def compare_type(expected: dict[str, Any], found: dict[str, Any]) -> list[str]:
    issues: list[str] = []
    if expected["kind"] != found["kind"]:
        issues.append(f"kind {expected['kind']} != {found['kind']}")
    if bool(expected.get("sealed")) != bool(found.get("sealed")):
        issues.append(f"sealed {expected.get('sealed')} != {found.get('sealed')}")
    if expected.get("baseType") != found.get("baseType"):
        issues.append(f"base {expected.get('baseType')} != {found.get('baseType')}")
    # The retained contract records a *reduced* direct-interface set: an
    # interface that is already a base interface of another listed entry is
    # omitted (IEnumerable behind IEnumerable`1, IGraphicsResource behind
    # IDynamicGraphicsResource, and so on). Reproducing that reduction would
    # mean modelling BCL interface hierarchies, so the audit requires the
    # weaker, sound relation instead: every interface the contract records must
    # actually be declared by the assembly. Interfaces the assembly declares
    # beyond that set are the contract's documented reduction and are reported
    # separately rather than treated as a discrepancy.
    expected_interfaces = set(expected.get("directInterfaces") or [])
    found_interfaces = set(found.get("directInterfaces") or [])
    for item in sorted(expected_interfaces - found_interfaces):
        issues.append(f"assembly does not declare contract interface {item}")
    expected_members = sorted(member_key(item) for item in expected["members"])
    found_members = sorted(member_key(item) for item in found["members"])
    missing = [item for item in expected_members if item not in found_members]
    extra = [item for item in found_members if item not in expected_members]
    for item in missing:
        issues.append(f"assembly is missing contract member {item}")
    for item in extra:
        issues.append(f"assembly declares member absent from contract {item}")
    return issues


def self_test(
    contract_types: dict[str, dict[str, Any]],
    extracted: dict[str, dict[str, Any]],
) -> tuple[int, list[str]]:
    """Prove the comparison is not vacuous.

    A comparison that always reports "exact" would pass the calibration gate
    for the wrong reason. Each mutation below perturbs one already-matching
    type and must be detected.
    """
    import copy as copy_module

    failures: list[str] = []
    checks = 0
    subjects = [
        name for name in (
            "Microsoft.Xna.Framework.Vector2",
            "Microsoft.Xna.Framework.Graphics.PresentationParameters",
            "Microsoft.Xna.Framework.Input.MouseState",
            "Microsoft.Xna.Framework.Color",
            "Microsoft.Xna.Framework.MathHelper",
            # BlendState is here because it is the calibration subject that
            # actually carries `initonly` fields; without it the readonly
            # mutation below could only ever flip false to true.
            "Microsoft.Xna.Framework.Graphics.BlendState",
        ) if name in extracted
    ]
    if len(subjects) < 6:
        failures.append("self-test subjects are missing from the extraction")
    checks += 1

    def drop_first(kind: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            for index, member in enumerate(record["members"]):
                if member["kind"] == kind:
                    del record["members"][index]
                    return True
            return False
        return mutate

    def retype_first(kind: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if member["kind"] == kind:
                    key = "returnType" if kind in ("method",) else "type"
                    if member.get(key) is None:
                        continue
                    member[key] = "Microsoft.Xna.Framework.Point"
                    return True
            return False
        return mutate

    def revalue_first(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member["kind"] == "field" and member.get("constant"):
                member["value"] = "999"
                return True
        return False

    def flip_sealed(record: dict[str, Any]) -> bool:
        record["sealed"] = not record["sealed"]
        return True

    def rebase(record: dict[str, Any]) -> bool:
        record["baseType"] = "Microsoft.Xna.Framework.Point"
        return True

    def drop_interface(record: dict[str, Any]) -> bool:
        if not record.get("directInterfaces"):
            return False
        record["directInterfaces"] = record["directInterfaces"][1:]
        return True

    def add_member(record: dict[str, Any]) -> bool:
        record["members"].append({
            "kind": "method", "name": "InventedMember", "static": False,
            "access": "public", "returnType": "System.Void",
            "genericParameters": [], "parameters": [],
        })
        return True

    def rename_first_method(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member["kind"] == "method":
                member["name"] = member["name"] + "Renamed"
                return True
        return False

    def flip_readonly(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member["kind"] == "field":
                member["readonly"] = not member.get("readonly")
                return True
        return False

    def flip_static(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member["kind"] in ("method", "property"):
                member["static"] = not member.get("static")
                return True
        return False

    mutations = [
        ("dropped method", drop_first("method")),
        ("dropped property", drop_first("property")),
        ("dropped field", drop_first("field")),
        ("dropped constructor", drop_first("constructor")),
        ("retyped method result", retype_first("method")),
        ("retyped property", retype_first("property")),
        ("retyped field", retype_first("field")),
        ("changed constant value", revalue_first),
        ("flipped sealed", flip_sealed),
        ("changed base type", rebase),
        ("dropped declared interface", drop_interface),
        ("invented extra member", add_member),
        ("renamed method", rename_first_method),
        ("flipped static identity", flip_static),
        ("flipped field readonly identity", flip_readonly),
    ]

    for name in subjects:
        expected = contract_types[name]
        found = extracted[name]
        if compare_type(expected, found):
            failures.append(f"{name}: unmutated comparison is not clean")
        checks += 1
        for label, mutate in mutations:
            candidate = copy_module.deepcopy(found)
            if not mutate(candidate):
                continue
            if not compare_type(expected, candidate):
                failures.append(f"{name}: {label} was not detected")
            checks += 1

    checks_made, boundary_failures = class_boundary_self_test()
    checks += checks_made
    failures.extend(boundary_failures)
    return checks, failures


# A `pinvokeimpl(...) ... preservesig` method whose body holds nothing but
# `.custom` attributes is closed by `ikdasm` with a BARE `}` -- no
# `// end of method` comment. `Parser.skip_block` used to stop only on that
# comment, so it ran past the method, past the enclosing type's own `}` and
# on into the NEXT type, silently attributing that type's members to this one.
# `System.Exception` contains exactly such a method and absorbed the whole of
# `System.ValueType`. The fixture below is that shape, minimised, and the
# assertion is the one the bug broke: two types, each with only its own
# members.
CLASS_BOUNDARY_FIXTURE = """\
.class public auto ansi beforefieldinit Fixture.Leaky
       extends System.Object
{
  .method public hidebysig instance void  Own() cil managed
  {
    IL_0000:  ret
  } // end of method Leaky::Own

  .method private hidebysig static pinvokeimpl("QCall" unicode winapi)
          void  NativeHelper(int32 kind) cil managed preservesig
  {
    .custom instance void System.Security.SecurityCriticalAttribute::.ctor() = ( 01 00 00 00 )
  }
} // end of class Fixture.Leaky

.class public auto ansi beforefieldinit Fixture.Follower
       extends System.Object
{
  .method public hidebysig instance void  Neighbour() cil managed
  {
    IL_0000:  ret
  } // end of method Follower::Neighbour
} // end of class Fixture.Follower
"""


def class_boundary_self_test() -> tuple[int, list[str]]:
    """Prove a commentless method close cannot merge two types."""
    failures: list[str] = []
    checks = 0
    parsed = Parser(CLASS_BOUNDARY_FIXTURE).parse()

    checks += 1
    if sorted(parsed) != ["Fixture.Follower", "Fixture.Leaky"]:
        failures.append(
            f"the class-boundary fixture parsed as {sorted(parsed)}")
        return checks, failures

    leaky = {item["name"] for item in parsed["Fixture.Leaky"]["members"]}
    follower = {item["name"] for item in parsed["Fixture.Follower"]["members"]}
    checks += 1
    if "Own" not in leaky:
        failures.append("the type's own member was lost across a bare `}`")
    checks += 1
    if "Neighbour" in leaky:
        failures.append(
            "a commentless method close leaked the NEXT type's members into "
            "this one")
    checks += 1
    if follower != {"Neighbour"}:
        failures.append(
            f"the following type parsed as {sorted(follower)}, not its own "
            "single member")
    return checks, failures


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--assembly-dir", type=Path, required=True)
    parser.add_argument("--require-exact", action="append", default=[])
    parser.add_argument("--il-cache", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument(
        "--write-resources", action="store_true",
        help="regenerate the pinned selected resource strings from the "
             "registered binaries")
    args = parser.parse_args()

    contract = json.loads(REFERENCE.read_text(encoding="utf-8"))
    by_name = {item["name"]: item for item in contract["types"]}

    assemblies = sorted(args.assembly_dir.glob("Microsoft.Xna.Framework*.dll"))
    if not assemblies:
        raise SystemExit(f"no XNA assemblies under {args.assembly_dir}")

    extracted: dict[str, dict[str, dict[str, Any]]] = {}
    for path in assemblies:
        cache = (args.il_cache / f"{path.stem}.il") if args.il_cache else None
        parsed = finalize(Parser(disassemble(path, cache)).parse())
        extracted[path.name] = parsed

    # Attribute each contract type to the assembly that declares it.
    owner_of: dict[str, str] = {}
    unattributed: list[str] = []
    for name in by_name:
        owners = [item for item, types in extracted.items() if name in types]
        if len(owners) == 1:
            owner_of[name] = owners[0]
        elif not owners:
            unattributed.append(name)
        else:
            owner_of[name] = sorted(owners)[0]

    results: list[dict[str, Any]] = []
    for path in assemblies:
        owned = sorted(name for name, item in owner_of.items() if item == path.name)
        if not owned:
            continue
        mismatches: list[dict[str, Any]] = []
        members = 0
        reductions = 0
        for name in owned:
            members += len(by_name[name]["members"])
            found = extracted[path.name][name]
            reductions += len(
                set(found.get("directInterfaces") or []) -
                set(by_name[name].get("directInterfaces") or []))
            issues = compare_type(by_name[name], found)
            if issues:
                mismatches.append({"type": name, "issues": issues})
        results.append({
            "assembly": path.name,
            "sha256": sha256(path),
            "contractTypes": len(owned),
            "contractMembers": members,
            "contractInterfaceReductions": reductions,
            "typesExact": len(owned) - len(mismatches),
            "typesMismatched": len(mismatches),
            "mismatches": mismatches,
        })

    # ------------------------------------------------------------------
    # The selected resource strings.
    #
    # A user-visible XNA message is a resource lookup, not an IL literal, so
    # every message this binding reproduces is read out of the registered
    # binary's own embedded string table rather than transcribed.
    # ------------------------------------------------------------------
    registry = json.loads(REGISTRY.read_text(encoding="utf-8"))
    selected_resources = registry.get("selectedResourceStrings", [])
    resource_tables: dict[str, dict[str, str]] = {}
    resource_records: list[dict[str, Any]] = []
    resource_failures: list[str] = []
    resource_checks = 0
    for entry in selected_resources:
        resource_checks += 1
        assembly = entry["assembly"]
        if assembly not in resource_tables:
            candidates = [item for item in assemblies if item.name == assembly]
            if not candidates:
                resource_failures.append(
                    f"{assembly}: not supplied, so its resource strings could "
                    "not be read")
                resource_tables[assembly] = {}
            else:
                blob = embedded_resource(candidates[0].read_bytes(), "resources")
                if blob is None:
                    resource_failures.append(
                        f"{assembly}: no embedded ResourceReader string table")
                    resource_tables[assembly] = {}
                else:
                    resource_tables[assembly] = resource_strings(blob)
        value = resource_tables[assembly].get(entry["key"])
        if value is None:
            resource_failures.append(
                f"{assembly}: resource key {entry['key']!r} is absent")
            continue
        resource_records.append({
            "assembly": assembly, "key": entry["key"], "value": value,
        })

    resource_document = {
        "schemaVersion": 1,
        "profile": "XNA 4.0 Windows runtime selected resource strings",
        "note": (
            "The user-visible messages this binding reproduces, read "
            "mechanically out of the embedded string tables of the "
            "hash-registered XNA assemblies named in "
            "registered-assemblies.json. Only keys and values are retained; "
            "no Microsoft binary is stored in the repository."),
        "resourceStrings": sorted(
            resource_records, key=lambda item: (item["assembly"], item["key"])),
    }
    if args.write_resources:
        RESOURCE_REFERENCE.parent.mkdir(parents=True, exist_ok=True)
        RESOURCE_REFERENCE.write_text(
            json.dumps(resource_document, indent=2) + "\n", encoding="utf-8")
    if RESOURCE_REFERENCE.exists():
        pinned_resources = json.loads(
            RESOURCE_REFERENCE.read_text(encoding="utf-8"))
        pinned_by_key = {
            (item["assembly"], item["key"]): item["value"]
            for item in pinned_resources.get("resourceStrings", [])
        }
        found_by_key = {
            (item["assembly"], item["key"]): item["value"]
            for item in resource_records
        }
        for key in sorted(set(pinned_by_key) | set(found_by_key)):
            resource_checks += 1
            if key not in found_by_key:
                resource_failures.append(
                    f"{key[1]}: pinned but not extracted")
            elif key not in pinned_by_key:
                resource_failures.append(
                    f"{key[1]}: extracted but not pinned")
            elif pinned_by_key[key] != found_by_key[key]:
                resource_failures.append(
                    f"{key[1]}: {found_by_key[key]!r} != pinned "
                    f"{pinned_by_key[key]!r}")
    elif not args.write_resources:
        resource_failures.append(f"{RESOURCE_REFERENCE.name} does not exist")

    merged: dict[str, dict[str, Any]] = {}
    for types in extracted.values():
        merged.update(types)
    checks, self_test_failures = self_test(by_name, merged)

    by_assembly = {item["assembly"]: item for item in results}
    calibration_failed = []
    for name in args.require_exact:
        record = by_assembly.get(name)
        if record is None:
            calibration_failed.append(f"{name}: not audited")
        elif record["typesMismatched"]:
            calibration_failed.append(
                f"{name}: {record['typesMismatched']} type(s) differ from the contract")

    report = {
        "schemaVersion": 1,
        "referenceSha256": hashlib.sha256(REFERENCE.read_bytes()).hexdigest(),
        "calibrationAssemblies": sorted(args.require_exact),
        "CALIBRATION_STATUS":
            "FAIL" if (calibration_failed or self_test_failures or
                       resource_failures) else "PASS",
        "RESOURCE_STRING_CHECKS": resource_checks,
        "RESOURCE_STRINGS_REPRODUCED": len(resource_records),
        "resourceStringFailures": resource_failures,
        "AUDIT_SELF_TESTS": checks,
        "AUDIT_SELF_TEST_STATUS": "FAIL" if self_test_failures else "PASS",
        "auditSelfTestFailures": self_test_failures,
        "CONTRACT_TYPES_REPRODUCED": sum(
            item["typesExact"] for item in results),
        "CONTRACT_MEMBERS_REPRODUCED": sum(
            item["contractMembers"] for item in results
            if not item["typesMismatched"]),
        "calibrationFailures": calibration_failed,
        "unattributedContractTypes": sorted(unattributed),
        "assemblies": results,
    }
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    for record in results:
        print(
            f"{record['assembly']:44s} types={record['contractTypes']:3d} "
            f"members={record['contractMembers']:4d} "
            f"exact={record['typesExact']:3d} mismatched={record['typesMismatched']:3d}"
        )
    print(f"UNATTRIBUTED_CONTRACT_TYPES={len(unattributed)} {unattributed}")
    print(
        f"CONTRACT_TYPES_REPRODUCED={report['CONTRACT_TYPES_REPRODUCED']} "
        f"CONTRACT_MEMBERS_REPRODUCED={report['CONTRACT_MEMBERS_REPRODUCED']}")
    print(
        f"AUDIT_SELF_TESTS={checks} "
        f"AUDIT_SELF_TEST_STATUS={report['AUDIT_SELF_TEST_STATUS']}")
    print(
        f"RESOURCE_STRING_CHECKS={resource_checks} "
        f"RESOURCE_STRINGS_REPRODUCED={len(resource_records)}")
    print(f"CALIBRATION_STATUS={report['CALIBRATION_STATUS']}")
    for line in calibration_failed + self_test_failures + resource_failures:
        print(f"  {line}", file=sys.stderr)
    return 1 if (calibration_failed or self_test_failures or
                 resource_failures) else 0


if __name__ == "__main__":
    raise SystemExit(main())
