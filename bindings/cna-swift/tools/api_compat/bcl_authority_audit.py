#!/usr/bin/env python3
"""Admit a Microsoft .NET Framework BCL assembly as a behaviour authority.

`pinned_assembly_audit.py` earns authority for an XNA assembly by
machine-comparing its public metadata against every entry it owns in the pinned
257-type / 2,964-member contract. A BCL assembly declares **zero** XNA contract
types, so that gate would pass with nothing compared -- it would calibrate
nothing at all. This tool is therefore a separate mechanism with its own,
non-vacuous calibration rule, and it deliberately shares no registry, no
counter and no scoreboard with the XNA audit.

Authority is earned here by five independent things, all of which must pass:

1. **Exact identity.** The AssemblyName, assembly version, public key and
   strong-name token, file size and full SHA-256 must equal the registered
   values. The token is *recomputed* from the assembly's own public key rather
   than read from a string, so a mismatched key cannot be asserted away.
2. **Microsoft provenance.** The PE version resource, the strong-name key file
   attribute, the required native module imports and the absence of every
   registered reimplementation marker are all checked against the binary.
3. **A pinned selected-shape manifest.** For every admitted family the full CLR
   identity, kind, base, generic arity, constructors, public members, relevant
   protected virtual members, static/instance, visibility, virtual/final/
   abstract flags, return and parameter CLR types and interface implementations
   are reconstructed from the binary and diffed against
   `reference/bcl40-selected-shape.json`. The manifest regenerates
   byte-identically from the admitted binary alone.
4. **Sentinel calibration.** Facts about the selected families that are stated
   here independently of the extractor -- arity, base, hook names, exact
   overridability, exact constructor count, the read-only asymmetry -- must
   hold. A parser that silently produced an empty or wrong shape fails these
   even if it agreed with a manifest it had itself written.
5. **Mutation self-tests.** Removing or altering a type, a member, a generic
   arity, a visibility, a virtual flag, a parameter type, a return type, a base
   or an interface must be *detected*. A comparison that always reported
   "exact" would pass steps 1-4 for the wrong reason.

Optionally, `--cross-check` re-reads the same binary with `monodis`, an
independent metadata reader, and requires it to agree about the critical shape
facts. The exact Microsoft binary remains the authority; the second tool only
validates this file's parsing of it.

    python3 tools/api_compat/bcl_authority_audit.py \
        --assembly mscorlib.dll=/path/to/mscorlib.dll \
        --cross-check \
        --output docs/generated/bcl-authority-audit.json

`--write-manifest` regenerates the pinned manifest from the admitted binaries.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

from pinned_assembly_audit import (  # noqa: E402
    Parser,
    embedded_resource,
    finalize,
    resource_strings,
    split_call,
    split_generic_suffix,
    strip_directives,
)

ROOT = Path(__file__).resolve().parents[2]
REGISTRY = ROOT / "tools/api_compat/bcl-authorities.json"
MANIFEST = ROOT / "tools/api_compat/reference/bcl40-selected-shape.json"

# The method attribute tokens ikdasm emits. Kept identical to the set
# `pinned_assembly_audit.Parser.add_method` recognises, so that what this file
# treats as a flag and what that one treats as a non-return token agree. A
# self-test holds the two in step.
METHOD_ATTRIBUTES = {
    "public", "private", "family", "assembly", "famorassem", "famandassem",
    "static", "instance", "hidebysig", "specialname", "rtspecialname",
    "virtual", "abstract", "final", "newslot", "pinvokeimpl", "strict",
    "explicit", "reqsecobj", "unmanagedexp",
}

# Visibility is recorded for protected members too: a protected virtual hook is
# part of the derived type's contract, so dropping it would silently delete the
# override point an XNA subclass depends on. These are `Parser`'s already-mapped
# spellings, not the raw IL tokens (`family` arrives here as `protected`).
RECORDED_ACCESS = ("public", "protected", "protected-internal")


def public_key_token(public_key_hex: str) -> str:
    """The strong-name token of a public key: last 8 bytes of its SHA-1, reversed.

    Computed rather than read, so a binary whose key does not actually hash to
    the registered token cannot be admitted by asserting the token.
    """
    return hashlib.sha1(bytes.fromhex(public_key_hex)).digest()[-8:][::-1].hex()


def version_resource(data: bytes) -> dict[str, str]:
    """Read the PE VS_VERSION_INFO string table.

    Anchored at the VS_VERSION_INFO key so that an unrelated UTF-16 occurrence
    of a field name elsewhere in a five-megabyte image cannot be mistaken for
    the version resource.
    """
    anchor = data.find("VS_VERSION_INFO".encode("utf-16-le"))
    if anchor < 0:
        return {}
    window = data[anchor:anchor + 8192]
    found: dict[str, str] = {}
    for name in (
        "CompanyName", "FileDescription", "FileVersion", "ProductName",
        "ProductVersion", "OriginalFilename", "InternalName", "LegalCopyright",
    ):
        index = window.find(name.encode("utf-16-le"))
        if index < 0:
            continue
        tail = window[index + len(name) * 2:]
        start = 0
        while start + 1 < len(tail) and tail[start:start + 2] == b"\x00\x00":
            start += 2
        end = start
        while end + 1 < len(tail) and tail[end:end + 2] != b"\x00\x00":
            end += 2
        found[name] = tail[start:end].decode("utf-16-le", "replace")
    return found


def disassemble(path: Path, cache_dir: Path | None, digest: str) -> str:
    """Disassemble `path`, optionally through a shared on-disk cache.

    The cache is keyed by the binary's own SHA-256, never by its file name. Two
    different `mscorlib.dll` files -- Microsoft's and a reimplementation's --
    have the same stem, so a name-keyed cache would silently hand one binary's
    IL back for the other. That is not a hypothetical: it made this tool's own
    negative controls read the admitted assembly's disassembly.
    """
    cache = (cache_dir / f"{path.stem}-{digest[:16]}.il") if cache_dir else None
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


# ----------------------------------------------------------------------------
# Extraction
# ----------------------------------------------------------------------------


def method_attributes(declaration: str) -> set[str]:
    """The CLR attribute tokens of one `.method` declaration."""
    body = strip_directives(declaration[len(".method"):].strip())
    split = split_call(body)
    if split is None:
        return set()
    head, _parameters, _tail = split
    head, _generics = split_generic_suffix(head)
    tokens = head.split()
    return {token for token in tokens[:-1] if token in METHOD_ATTRIBUTES}


class BclParser(Parser):
    """`Parser`, plus the CLR flags a BCL base-class contract depends on.

    The XNA contract never needed virtual/final/abstract, because an XNA type's
    overridability is recorded in the pinned contract itself. A BCL *base*
    class is different: whether `SetItem` is an overridable hook or a sealed
    implementation is the whole difference between a faithful projection and a
    broken one, so it is extracted rather than assumed.
    """

    def add_method(self, owner: dict[str, Any], declaration: str) -> None:
        before = len(owner["members"])
        super().add_method(owner, declaration)
        if len(owner["members"]) == before:
            return
        attributes = method_attributes(declaration)
        member = owner["members"][-1]
        member["virtual"] = "virtual" in attributes
        member["abstract"] = "abstract" in attributes
        member["final"] = "final" in attributes
        member["newslot"] = "newslot" in attributes
        # An overridable member is virtual and not sealed. `virtual final` is a
        # sealed interface implementation and is NOT an override point.
        member["overridable"] = member["virtual"] and not member["final"]
        owner.setdefault("_accessorFlags", {})[member["name"]] = {
            "virtual": member["virtual"],
            "abstract": member["abstract"],
            "final": member["final"],
            "overridable": member["overridable"],
        }

    def add_property(
        self, owner: dict[str, Any], declaration: str, index: int,
    ) -> int:
        before = len(owner["members"])
        result = super().add_property(owner, declaration, index)
        if len(owner["members"]) == before:
            return result
        member = owner["members"][-1]
        flags = owner.get("_accessorFlags", {})
        for role, accessor in (("get", f"get_{member['name']}"),
                               ("set", f"set_{member['name']}")):
            entry = flags.get(accessor)
            if entry is not None:
                member[f"{role}Overridable"] = entry["overridable"]
                member[f"{role}Abstract"] = entry["abstract"]
        return result


def visible_members(record: dict[str, Any]) -> list[dict[str, Any]]:
    """The members a derived type or a consumer can actually see.

    Explicit interface implementations are private in CLR metadata and carry a
    dotted name; they are neither callable nor overridable by name and are
    excluded. Everything public or protected is kept, including the protected
    hooks.
    """
    kept: list[dict[str, Any]] = []
    for member in record["members"]:
        if member["kind"] in ("method", "constructor"):
            if member.get("access") not in RECORDED_ACCESS:
                continue
        elif member["kind"] == "property":
            accesses = {member.get("getAccess"), member.get("setAccess")} - {None}
            if not accesses & set(RECORDED_ACCESS):
                continue
        elif member["kind"] == "field":
            if member.get("access") not in RECORDED_ACCESS:
                continue
        if "." in member["name"] and member["name"] not in (".ctor", ".cctor"):
            continue
        kept.append(member)
    return kept


def member_record(member: dict[str, Any]) -> dict[str, Any]:
    """One member of the selected-shape manifest, in a stable field order."""
    kind = member["kind"]
    record: dict[str, Any] = {"kind": kind, "name": member["name"]}
    if kind in ("method", "constructor"):
        record.update({
            "access": member.get("access"),
            "static": bool(member.get("static")),
            "virtual": bool(member.get("virtual")),
            "abstract": bool(member.get("abstract")),
            "final": bool(member.get("final")),
            "overridable": bool(member.get("overridable")),
            "returnType": member.get("returnType"),
            "genericParameters": [
                item["name"] for item in member.get("genericParameters", [])
            ],
            "parameters": [
                {
                    "name": item.get("name", ""),
                    "type": item.get("type"),
                    "ref": bool(item.get("ref")),
                    "out": bool(item.get("out")),
                }
                for item in member.get("parameters", [])
            ],
        })
    elif kind == "property":
        record.update({
            "type": member.get("type"),
            "static": bool(member.get("static")),
            "getAccess": member.get("getAccess"),
            "setAccess": member.get("setAccess"),
            "getOverridable": bool(member.get("getOverridable")),
            "setOverridable": bool(member.get("setOverridable")),
            # `overridable` cannot tell an abstract accessor from a virtual
            # one, and PropertyDescriptor's contract is precisely that eight
            # members are abstract -- a derived descriptor MUST supply them.
            # The extractor already reads the flag; it was simply dropped
            # here (Foundation 100).
            "getAbstract": bool(member.get("getAbstract")),
            "setAbstract": bool(member.get("setAbstract")),
            "parameters": [
                {"name": item.get("name", ""), "type": item.get("type")}
                for item in member.get("parameters", [])
            ],
        })
    elif kind == "event":
        record.update({"type": member.get("type")})
    else:
        record.update({
            "type": member.get("type"),
            "access": member.get("access"),
            "static": bool(member.get("static")),
            "constant": bool(member.get("constant")),
            "value": member.get("value"),
        })
    return record


def member_sort_key(record: dict[str, Any]) -> tuple:
    return (
        record["kind"], record["name"],
        json.dumps(record.get("parameters", []), sort_keys=True),
        str(record.get("returnType")), str(record.get("type")),
    )


def type_record(
    assembly: str, name: str, record: dict[str, Any],
) -> dict[str, Any]:
    members = [member_record(item) for item in visible_members(record)]
    members.sort(key=member_sort_key)
    return {
        "assembly": assembly,
        "type": name,
        "kind": record["kind"],
        "sealed": bool(record.get("sealed")),
        "abstract": bool(record.get("abstract")),
        "baseType": record.get("baseType"),
        "genericArity": len(record.get("genericParameters") or []),
        "genericParameters": list(record.get("genericParameters") or []),
        "directInterfaces": sorted(record.get("directInterfaces") or []),
        "members": members,
    }


def extract(
    assembly: str, il: str, selected: list[str],
) -> tuple[list[dict[str, Any]], list[str]]:
    types = BclParser(il).parse()
    # `finalize` drops the accessor methods a property already represents. It
    # is applied here through the same code path the XNA audit uses, so the BCL
    # manifest and that audit agree about what a property is.
    finalize(types)
    records: list[dict[str, Any]] = []
    missing: list[str] = []
    for name in selected:
        found = types.get(name)
        if found is None:
            missing.append(name)
            continue
        records.append(type_record(assembly, name, found))
    return records, missing


# ----------------------------------------------------------------------------
# Comparison
# ----------------------------------------------------------------------------


def compare_records(
    expected: dict[str, Any], found: dict[str, Any],
) -> list[str]:
    issues: list[str] = []
    for field in ("kind", "sealed", "abstract", "baseType", "genericArity",
                  "genericParameters", "directInterfaces"):
        if expected.get(field) != found.get(field):
            issues.append(
                f"{field} {expected.get(field)!r} != {found.get(field)!r}")
    expected_members = {
        json.dumps(item, sort_keys=True) for item in expected["members"]}
    found_members = {
        json.dumps(item, sort_keys=True) for item in found["members"]}
    for item in sorted(expected_members - found_members):
        issues.append(f"pinned member absent from the assembly: {item}")
    for item in sorted(found_members - expected_members):
        issues.append(f"assembly declares a member absent from the manifest: {item}")
    return issues


# ----------------------------------------------------------------------------
# Sentinels: facts stated here, independently of the extractor
# ----------------------------------------------------------------------------

COLLECTION = "System.Collections.ObjectModel.Collection`1"
READONLY = "System.Collections.ObjectModel.ReadOnlyCollection`1"
LIST = "System.Collections.Generic.List`1"
ILIST = "System.Collections.Generic.IList`1"
DICTIONARY = "System.Collections.Generic.Dictionary`2"
KEY_COLLECTION = "System.Collections.Generic.Dictionary`2+KeyCollection"
VALUE_COLLECTION = "System.Collections.Generic.Dictionary`2+ValueCollection"
DICT_ENUMERATOR = "System.Collections.Generic.Dictionary`2+Enumerator"
KEY_VALUE_PAIR = "System.Collections.Generic.KeyValuePair`2"
IEQUALITY_COMPARER = "System.Collections.Generic.IEqualityComparer`1"
IDICTIONARY = "System.Collections.Generic.IDictionary`2"
ATTRIBUTE = "System.Attribute"
ISERVICE_PROVIDER = "System.IServiceProvider"
EXCEPTION = "System.Exception"
SYSTEM_EXCEPTION = "System.SystemException"
EXTERNAL_EXCEPTION = "System.Runtime.InteropServices.ExternalException"
ARGUMENT_EXCEPTION = "System.ArgumentException"
ARGUMENT_NULL_EXCEPTION = "System.ArgumentNullException"
ARGUMENT_RANGE_EXCEPTION = "System.ArgumentOutOfRangeException"
NOT_SUPPORTED_EXCEPTION = "System.NotSupportedException"
INVALID_OPERATION_EXCEPTION = "System.InvalidOperationException"
OBJECT_DISPOSED_EXCEPTION = "System.ObjectDisposedException"
KEY_NOT_FOUND_EXCEPTION = "System.Collections.Generic.KeyNotFoundException"
ASYNC_RESULT = "System.IAsyncResult"
ASYNC_CALLBACK = "System.AsyncCallback"
FILE_MODE = "System.IO.FileMode"
FILE_ACCESS = "System.IO.FileAccess"
FILE_SHARE = "System.IO.FileShare"
TYPE_CONVERTER = "System.ComponentModel.TypeConverter"
EXPANDABLE_CONVERTER = "System.ComponentModel.ExpandableObjectConverter"
TYPE_DESCRIPTOR_CONTEXT = "System.ComponentModel.ITypeDescriptorContext"
PROPERTY_DESCRIPTOR = "System.ComponentModel.PropertyDescriptor"
PROPERTY_DESCRIPTOR_COLLECTION = "System.ComponentModel.PropertyDescriptorCollection"
CULTURE_INFO = "System.Globalization.CultureInfo"
NONGENERIC_IDICTIONARY = "System.Collections.IDictionary"


# ----------------------------------------------------------------------------
# Embedded managed resources
#
# `System.Exception.get_Message`, `System.SystemException..ctor()` and
# `System.Runtime.InteropServices.ExternalException..ctor()` do not carry their
# default messages as IL string literals: each loads a RESOURCE KEY and calls
# `Environment.GetResourceString`. The message a projected XNA exception
# reports is therefore a fact about the assembly's embedded
# `mscorlib.resources`, not about its code, and it is read from the binary here
# rather than transcribed from documentation or memory.
# ----------------------------------------------------------------------------

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
    # The six raised exception families.
    #
    # These are the classes the support layer actually THROWS, so what matters
    # is the base each sits on, the constructor overloads a raise site selects,
    # and the extra observable state two of them add. Every fact is stated from
    # the documented .NET Framework 4.0 contract, not read back from the
    # extraction.
    # ------------------------------------------------------------------
    for name in (ARGUMENT_EXCEPTION, ARGUMENT_NULL_EXCEPTION,
                 ARGUMENT_RANGE_EXCEPTION, NOT_SUPPORTED_EXCEPTION,
                 INVALID_OPERATION_EXCEPTION, KEY_NOT_FOUND_EXCEPTION):
        require(name in by_type, f"{name} was not extracted at all")
    raised = {name: by_type.get(name) for name in (
        ARGUMENT_EXCEPTION, ARGUMENT_NULL_EXCEPTION, ARGUMENT_RANGE_EXCEPTION,
        NOT_SUPPORTED_EXCEPTION, INVALID_OPERATION_EXCEPTION,
        KEY_NOT_FOUND_EXCEPTION)}
    if any(record is None for record in raised.values()):
        return checks, failures

    for name, record in raised.items():
        require(record["kind"] == "class", f"{name} is not a class")
        require(not record["sealed"], f"{name} is sealed")
        require(not record["abstract"], f"{name} is abstract")
        require(record["genericArity"] == 0, f"{name} is generic")

    # The bases decide which `catch` clauses see which failure. Three of the
    # six sit on SystemException; the two argument specializations sit on
    # ArgumentException, which is what makes `catch (ArgumentException)` see a
    # null argument and an out-of-range one alike.
    require(raised[ARGUMENT_EXCEPTION]["baseType"] == SYSTEM_EXCEPTION,
            "ArgumentException does not derive from SystemException")
    require(raised[ARGUMENT_NULL_EXCEPTION]["baseType"] == ARGUMENT_EXCEPTION,
            "ArgumentNullException does not derive from ArgumentException")
    require(raised[ARGUMENT_RANGE_EXCEPTION]["baseType"] == ARGUMENT_EXCEPTION,
            "ArgumentOutOfRangeException does not derive from ArgumentException")
    for name in (NOT_SUPPORTED_EXCEPTION, INVALID_OPERATION_EXCEPTION,
                 KEY_NOT_FOUND_EXCEPTION):
        require(raised[name]["baseType"] == SYSTEM_EXCEPTION,
                f"{name} does not derive directly from SystemException")

    # ArgumentException carries the parameter name and overrides Message to
    # append it; that override is the whole reason the payload differs from
    # the constructor argument.
    param_name = members_of(ARGUMENT_EXCEPTION, "property", "ParamName")
    require(len(param_name) == 1, "ArgumentException.ParamName is missing")
    for member in param_name:
        require(member["type"] == "System.String",
                "ArgumentException.ParamName is not a String")
        require(member["getAccess"] == "public" and member["setAccess"] is None,
                "ArgumentException.ParamName is not a public get-only property")
        require(member["getOverridable"],
                "ArgumentException.ParamName is not overridable")
    for name in (ARGUMENT_EXCEPTION, ARGUMENT_RANGE_EXCEPTION):
        overridden = members_of(name, "property", "Message")
        require(len(overridden) == 1, f"{name} does not override Message")
        for member in overridden:
            require(member["type"] == "System.String",
                    f"{name}.Message is not a String")
            require(member["getAccess"] == "public" and
                    member["setAccess"] is None,
                    f"{name}.Message is not a public get-only property")
            require(member["getOverridable"],
                    f"{name}.Message is not overridable")
    for name in (ARGUMENT_NULL_EXCEPTION, NOT_SUPPORTED_EXCEPTION,
                 INVALID_OPERATION_EXCEPTION, KEY_NOT_FOUND_EXCEPTION):
        require(not members_of(name, "property", "Message"),
                f"{name} overrides Message; it should inherit one")

    actual_value = members_of(ARGUMENT_RANGE_EXCEPTION, "property", "ActualValue")
    require(len(actual_value) == 1,
            "ArgumentOutOfRangeException.ActualValue is missing")
    for member in actual_value:
        require(member["type"] == "System.Object",
                "ArgumentOutOfRangeException.ActualValue is not an Object")
        require(member["getAccess"] == "public" and member["setAccess"] is None,
                "ArgumentOutOfRangeException.ActualValue is not public get-only")
        require(member["getOverridable"],
                "ArgumentOutOfRangeException.ActualValue is not overridable")

    # NotSupportedException, InvalidOperationException and
    # KeyNotFoundException add no member at all: their entire contribution is a
    # distinct class identity and a distinct HResult.
    for name in (NOT_SUPPORTED_EXCEPTION, INVALID_OPERATION_EXCEPTION,
                 KEY_NOT_FOUND_EXCEPTION, ARGUMENT_NULL_EXCEPTION):
        require(not [
            item for item in raised[name]["members"]
            if item["kind"] != "constructor"
        ], f"{name} declares a non-constructor member")

    # The constructor overloads each raise site selects. A missing overload
    # here means a projected payload that could not have been built.
    def signature_present(name: str, types: list[str]) -> bool:
        return any(
            [entry["type"] for entry in item["parameters"]] == types
            for item in members_of(name, "constructor", ".ctor")
            if item["access"] == "public")

    # The exact public constructor count of each family, so dropping any one
    # of them is a failure and not merely a missing overload nobody named.
    for name, public_count in ((ARGUMENT_EXCEPTION, 5),
                               (ARGUMENT_NULL_EXCEPTION, 4),
                               (ARGUMENT_RANGE_EXCEPTION, 5),
                               (NOT_SUPPORTED_EXCEPTION, 3),
                               (INVALID_OPERATION_EXCEPTION, 3),
                               (KEY_NOT_FOUND_EXCEPTION, 3)):
        constructors = members_of(name, "constructor", ".ctor")
        public = [item for item in constructors if item["access"] == "public"]
        protected = [item for item in constructors if item["access"] == "protected"]
        require(len(public) == public_count,
                f"{name} should declare {public_count} public constructors, "
                f"found {len(public)}")
        require(len(protected) == 1,
                f"{name} should declare exactly one protected serialization "
                f"constructor, found {len(protected)}")
        require(signature_present(name, []),
                f"{name} has no public parameterless constructor")
    for name in (ARGUMENT_EXCEPTION, ARGUMENT_NULL_EXCEPTION,
                 ARGUMENT_RANGE_EXCEPTION):
        require(signature_present(name, ["System.String"]),
                f"{name} has no single-String constructor")
        require(signature_present(name, ["System.String", EXCEPTION]),
                f"{name} has no (String, Exception) constructor")

    require(signature_present(ARGUMENT_EXCEPTION, ["System.String", "System.String"]),
            "ArgumentException has no (message, paramName) constructor")
    require(signature_present(
        ARGUMENT_EXCEPTION, ["System.String", "System.String", EXCEPTION]),
        "ArgumentException has no (message, paramName, innerException) constructor")
    require(signature_present(ARGUMENT_NULL_EXCEPTION, ["System.String", "System.String"]),
            "ArgumentNullException has no (paramName, message) constructor")
    require(signature_present(ARGUMENT_RANGE_EXCEPTION, ["System.String", "System.String"]),
            "ArgumentOutOfRangeException has no (paramName, message) constructor")
    require(signature_present(
        ARGUMENT_RANGE_EXCEPTION, ["System.String", "System.Object", "System.String"]),
        "ArgumentOutOfRangeException has no (paramName, actualValue, message) constructor")
    for name in (NOT_SUPPORTED_EXCEPTION, INVALID_OPERATION_EXCEPTION,
                 KEY_NOT_FOUND_EXCEPTION):
        require(signature_present(name, []), f"{name} has no parameterless constructor")
        require(signature_present(name, ["System.String"]),
                f"{name} has no (message) constructor")
        require(signature_present(name, ["System.String", EXCEPTION]),
                f"{name} has no (message, innerException) constructor")

    # ArgumentNullException's two two-argument overloads are transposed with
    # respect to each other, and confusing them would silently swap a message
    # with a parameter name at every raise site.
    require(signature_present(ARGUMENT_NULL_EXCEPTION, ["System.String", EXCEPTION]),
            "ArgumentNullException has no (message, innerException) constructor")

    # ------------------------------------------------------------------
    # ObjectDisposedException, the seventh raised family.
    #
    # It is the one that is NOT a SystemException specialization: it derives
    # from InvalidOperationException, so `catch (InvalidOperationException)`
    # sees a use-after-dispose and a bound-state-object write alike. Its
    # payload composes the same way ArgumentException's does, with two
    # differences that a template-driven projection would get wrong:
    # `ObjectName` is NOT virtual where `ParamName` is, and its getter
    # substitutes String.Empty for a null field where `ParamName` returns the
    # field as it stands.
    # ------------------------------------------------------------------
    disposed = by_type.get(OBJECT_DISPOSED_EXCEPTION)
    require(OBJECT_DISPOSED_EXCEPTION in by_type,
            f"{OBJECT_DISPOSED_EXCEPTION} was not extracted at all")
    if disposed is not None:
        require(disposed["kind"] == "class",
                "ObjectDisposedException is not a class")
        require(not disposed["sealed"], "ObjectDisposedException is sealed")
        require(not disposed["abstract"], "ObjectDisposedException is abstract")
        require(disposed["genericArity"] == 0,
                "ObjectDisposedException is generic")
        require(disposed["baseType"] == INVALID_OPERATION_EXCEPTION,
                "ObjectDisposedException does not derive from "
                "InvalidOperationException")

        object_name = members_of(OBJECT_DISPOSED_EXCEPTION, "property", "ObjectName")
        require(len(object_name) == 1,
                "ObjectDisposedException.ObjectName is missing")
        for member in object_name:
            require(member["type"] == "System.String",
                    "ObjectDisposedException.ObjectName is not a String")
            require(member["getAccess"] == "public" and
                    member["setAccess"] is None,
                    "ObjectDisposedException.ObjectName is not public get-only")
            require(not member["getOverridable"],
                    "ObjectDisposedException.ObjectName is overridable; "
                    "unlike ArgumentException.ParamName it is not virtual")

        disposed_message = members_of(OBJECT_DISPOSED_EXCEPTION, "property", "Message")
        require(len(disposed_message) == 1,
                "ObjectDisposedException does not override Message")
        for member in disposed_message:
            require(member["type"] == "System.String",
                    "ObjectDisposedException.Message is not a String")
            require(member["getAccess"] == "public" and
                    member["setAccess"] is None,
                    "ObjectDisposedException.Message is not public get-only")
            require(member["getOverridable"],
                    "ObjectDisposedException.Message is not overridable")

        # Three public constructors and one protected serialization one. The
        # single-String overload takes an OBJECT NAME, not a message, which is
        # the opposite of every other family here; requiring both two-argument
        # spellings separately is what stops a transposition being silent.
        constructors = members_of(OBJECT_DISPOSED_EXCEPTION, "constructor", ".ctor")
        public = [item for item in constructors if item["access"] == "public"]
        protected = [item for item in constructors if item["access"] == "protected"]
        require(len(public) == 3,
                "ObjectDisposedException should declare 3 public "
                f"constructors, found {len(public)}")
        require(len(protected) == 1,
                "ObjectDisposedException should declare exactly one protected "
                f"serialization constructor, found {len(protected)}")
        require(not signature_present(OBJECT_DISPOSED_EXCEPTION, []),
                "ObjectDisposedException has a parameterless constructor; it "
                "has none, because an object name is always required")
        require(signature_present(OBJECT_DISPOSED_EXCEPTION, ["System.String"]),
                "ObjectDisposedException has no single-String (objectName) "
                "constructor")
        require(signature_present(
            OBJECT_DISPOSED_EXCEPTION, ["System.String", "System.String"]),
            "ObjectDisposedException has no (objectName, message) constructor")
        require(signature_present(
            OBJECT_DISPOSED_EXCEPTION, ["System.String", EXCEPTION]),
            "ObjectDisposedException has no (message, innerException) "
            "constructor")

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
    # System.IServiceProvider.
    #
    # The smallest admitted family: one method, and the reason it is admitted
    # at all is that ContentManager's constructors declare it. Stating the
    # arity here is the point -- a projection over the concrete
    # GameServiceContainer would look identical to the extractor and would
    # still be a narrowing of XNA's contract.
    # ------------------------------------------------------------------
    require(ISERVICE_PROVIDER in by_type,
            "System.IServiceProvider was not extracted at all")
    service_provider = by_type.get(ISERVICE_PROVIDER)
    if service_provider is not None:
        require(service_provider["kind"] == "interface",
                "IServiceProvider is not an interface")
        require(service_provider["genericArity"] == 0,
                "IServiceProvider is generic")
        require(not service_provider["directInterfaces"],
                "IServiceProvider extends another interface")
        require(len(service_provider["members"]) == 1,
                "IServiceProvider declares more than GetService")
        get_service = members_of(ISERVICE_PROVIDER, "method", "GetService")
        require(len(get_service) == 1, "IServiceProvider.GetService is missing")
        for member in get_service:
            require(member["returnType"] == "System.Object",
                    "GetService does not return System.Object")
            require([item["type"] for item in member["parameters"]]
                    == ["System.Type"],
                    "GetService does not take exactly one System.Type")
            require(member["abstract"],
                    "GetService is not abstract; an interface method must be")

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

    # ------------------------------------------------------------------
    # System.ComponentModel.TypeConverter (Foundation 100).
    #
    # The transitive base of all thirteen XNA Design converters. Everything
    # below is written from the documented .NET Framework 4.0 contract; where
    # a claim turned out to be wrong the correction is noted AT the claim,
    # because a sentinel silently relaxed to match the extraction is not a
    # sentinel any more.
    # ------------------------------------------------------------------
    require(TYPE_CONVERTER in by_type,
            "System.ComponentModel.TypeConverter was not extracted at all")
    converter = by_type.get(TYPE_CONVERTER)
    if converter is not None:
        require(converter["kind"] == "class", "TypeConverter is not a class")
        # NOT abstract. This is the claim most likely to be got wrong from
        # memory -- `new TypeConverter()` is legal and returns a converter that
        # refuses every conversion, and the thirteen XNA converters rely on
        # that base behaviour for the pairs they do not override.
        require(not converter["abstract"], "TypeConverter is abstract")
        require(not converter["sealed"], "TypeConverter is sealed")
        require(converter["baseType"] == "System.Object",
                "TypeConverter does not derive directly from System.Object")
        require(converter["genericArity"] == 0, "TypeConverter is generic")

        # The four capability/conversion pairs. Each exists twice: a
        # context-free convenience overload and the virtual one that takes the
        # context first. Only the longer overload is virtual -- a projection
        # that made the short one virtual would invite an override the CLR
        # would never call.
        for name, short_params, long_params, ret in (
            ("CanConvertFrom", ["System.Type"],
             [TYPE_DESCRIPTOR_CONTEXT, "System.Type"], "System.Boolean"),
            ("CanConvertTo", ["System.Type"],
             [TYPE_DESCRIPTOR_CONTEXT, "System.Type"], "System.Boolean"),
            ("ConvertFrom", ["System.Object"],
             [TYPE_DESCRIPTOR_CONTEXT, CULTURE_INFO, "System.Object"],
             "System.Object"),
            ("ConvertTo", ["System.Object", "System.Type"],
             [TYPE_DESCRIPTOR_CONTEXT, CULTURE_INFO, "System.Object",
              "System.Type"], "System.Object"),
        ):
            overloads = members_of(TYPE_CONVERTER, "method", name)
            require(len(overloads) == 2,
                    f"TypeConverter.{name} should have exactly 2 overloads, "
                    f"found {len(overloads)}")
            by_arity = {
                tuple(item["type"] for item in member["parameters"]): member
                for member in overloads
            }
            require(tuple(short_params) in by_arity,
                    f"TypeConverter.{name} has no ({', '.join(short_params)}) "
                    "overload")
            require(tuple(long_params) in by_arity,
                    f"TypeConverter.{name} has no context overload taking "
                    f"({', '.join(long_params)})")
            long_member = by_arity.get(tuple(long_params))
            short_member = by_arity.get(tuple(short_params))
            if long_member is not None:
                # `overridable`, not merely `virtual`: `virtual final` is
                # virtual and still cannot be overridden, and a converter that
                # could not override this pair could not convert anything.
                require(long_member["overridable"],
                        f"TypeConverter.{name}'s context overload is not an "
                        "override point")
                require(long_member["returnType"] == ret,
                        f"TypeConverter.{name} does not return {ret}")
            if short_member is not None:
                require(not short_member["overridable"],
                        f"TypeConverter.{name}'s context-free overload is an "
                        "override point; it is a forwarder and overriding it "
                        "would be silently skipped by every internal caller")

        # CreateInstance is why System.Collections.IDictionary is in the
        # closure at all: the property bag GetProperties described comes back
        # as a non-generic IDictionary.
        create = members_of(TYPE_CONVERTER, "method", "CreateInstance")
        require(len(create) == 2,
                f"TypeConverter.CreateInstance should have 2 overloads, "
                f"found {len(create)}")
        require(any(
            [item["type"] for item in member["parameters"]]
            == [TYPE_DESCRIPTOR_CONTEXT, NONGENERIC_IDICTIONARY]
            for member in create),
            "TypeConverter.CreateInstance has no "
            "(ITypeDescriptorContext, IDictionary) overload")

        properties = members_of(TYPE_CONVERTER, "method", "GetProperties")
        require(len(properties) == 3,
                f"TypeConverter.GetProperties should have 3 overloads, "
                f"found {len(properties)}")
        for member in properties:
            require(member["returnType"] == PROPERTY_DESCRIPTOR_COLLECTION,
                    "TypeConverter.GetProperties does not return a "
                    "PropertyDescriptorCollection")
        widest = max(properties, key=lambda m: len(m["parameters"])) \
            if properties else None
        if widest is not None:
            params = [item["type"] for item in widest["parameters"]]
            require(len(params) == 3,
                    "TypeConverter's widest GetProperties does not take 3 "
                    "parameters")
            require(params[:2] == [TYPE_DESCRIPTOR_CONTEXT, "System.Object"],
                    "TypeConverter.GetProperties(context, value, attributes) "
                    "does not start with (ITypeDescriptorContext, Object)")
            require(len(params) == 3 and params[2].endswith("[]")
                    and "Attribute" in params[2],
                    "TypeConverter.GetProperties' third parameter is not an "
                    "array of System.Attribute")

    # ------------------------------------------------------------------
    # System.ComponentModel.ExpandableObjectConverter.
    #
    # The DIRECT base of XNA's MathTypeConverter. It adds exactly two members
    # over TypeConverter, and those two are what the math converters override.
    # ------------------------------------------------------------------
    require(EXPANDABLE_CONVERTER in by_type,
            "System.ComponentModel.ExpandableObjectConverter was not "
            "extracted at all")
    expandable = by_type.get(EXPANDABLE_CONVERTER)
    if expandable is not None:
        require(expandable["baseType"] == TYPE_CONVERTER,
                "ExpandableObjectConverter does not derive from TypeConverter")
        require(not expandable["sealed"],
                "ExpandableObjectConverter is sealed; MathTypeConverter "
                "derives from it, so a sealed base would make XNA's own "
                "hierarchy impossible")
        require(not expandable["abstract"],
                "ExpandableObjectConverter is abstract")
        ctors = members_of(EXPANDABLE_CONVERTER, "constructor", ".ctor")
        require(len(ctors) == 1 and not ctors[0]["parameters"],
                "ExpandableObjectConverter does not declare exactly one "
                "parameterless constructor")
        for name in ("GetProperties", "GetPropertiesSupported"):
            overrides = members_of(EXPANDABLE_CONVERTER, "method", name)
            require(len(overrides) == 1,
                    f"ExpandableObjectConverter.{name} should be declared "
                    f"exactly once, found {len(overrides)}")
            for member in overrides:
                require(member["virtual"],
                        f"ExpandableObjectConverter.{name} is not virtual")

    # ------------------------------------------------------------------
    # System.ComponentModel.ITypeDescriptorContext.
    #
    # In every converter signature and read by none of XNA's code. It extends
    # the already-admitted System.IServiceProvider, which is the one place in
    # this closure where two separately-justified families meet.
    # ------------------------------------------------------------------
    require(TYPE_DESCRIPTOR_CONTEXT in by_type,
            "System.ComponentModel.ITypeDescriptorContext was not extracted "
            "at all")
    context = by_type.get(TYPE_DESCRIPTOR_CONTEXT)
    if context is not None:
        require(context["kind"] == "interface",
                "ITypeDescriptorContext is not an interface")
        require(ISERVICE_PROVIDER in context["directInterfaces"],
                "ITypeDescriptorContext does not extend System.IServiceProvider")
        require(len(context["members"]) == 5,
                f"ITypeDescriptorContext should declare exactly 5 members, "
                f"found {len(context['members'])}")
        for name, clr_type in (("Instance", "System.Object"),
                               ("PropertyDescriptor", PROPERTY_DESCRIPTOR)):
            candidates = members_of(TYPE_DESCRIPTOR_CONTEXT, "property", name)
            require(len(candidates) == 1,
                    f"ITypeDescriptorContext.{name} is missing")
            for member in candidates:
                require(member["type"] == clr_type,
                        f"ITypeDescriptorContext.{name} is not {clr_type}")
                require(member["setAccess"] is None,
                        f"ITypeDescriptorContext.{name} has a setter")
        changing = members_of(TYPE_DESCRIPTOR_CONTEXT, "method",
                              "OnComponentChanging")
        require(len(changing) == 1,
                "ITypeDescriptorContext.OnComponentChanging is missing")
        for member in changing:
            # Changing can be VETOED, Changed cannot. The asymmetric return is
            # the whole difference between the two.
            require(member["returnType"] == "System.Boolean",
                    "OnComponentChanging does not return System.Boolean")
        changed = members_of(TYPE_DESCRIPTOR_CONTEXT, "method",
                             "OnComponentChanged")
        require(len(changed) == 1,
                "ITypeDescriptorContext.OnComponentChanged is missing")
        for member in changed:
            require(member["returnType"] == "System.Void",
                    "OnComponentChanged does not return void")

    # ------------------------------------------------------------------
    # System.ComponentModel.PropertyDescriptor.
    #
    # Admitted as the element type of the collection GetProperties returns,
    # NOT to be projected whole: it derives from MemberDescriptor, which is
    # deliberately outside the closure. Asserting the base here records that
    # the boundary is a choice and not an oversight.
    # ------------------------------------------------------------------
    require(PROPERTY_DESCRIPTOR in by_type,
            "System.ComponentModel.PropertyDescriptor was not extracted at all")
    descriptor = by_type.get(PROPERTY_DESCRIPTOR)
    if descriptor is not None:
        require(descriptor["kind"] == "class",
                "PropertyDescriptor is not a class")
        require(descriptor["abstract"],
                "PropertyDescriptor is not abstract")
        require(descriptor["baseType"] == "System.ComponentModel.MemberDescriptor",
                "PropertyDescriptor does not derive from MemberDescriptor")
        require(not descriptor["sealed"], "PropertyDescriptor is sealed")
        # Every constructor is protected: the type is only ever reached
        # through a derived descriptor.
        ctors = members_of(PROPERTY_DESCRIPTOR, "constructor", ".ctor")
        require(bool(ctors), "PropertyDescriptor declares no constructor")
        require(all(member["access"] == "protected" for member in ctors),
                "PropertyDescriptor declares a non-protected constructor")
        # The eight members a concrete descriptor MUST supply.
        for name, kind, ret in (
            ("ComponentType", "property", "System.Type"),
            ("IsReadOnly", "property", "System.Boolean"),
            ("PropertyType", "property", "System.Type"),
            ("CanResetValue", "method", "System.Boolean"),
            ("GetValue", "method", "System.Object"),
            ("ResetValue", "method", "System.Void"),
            ("SetValue", "method", "System.Void"),
            ("ShouldSerializeValue", "method", "System.Boolean"),
        ):
            candidates = members_of(PROPERTY_DESCRIPTOR, kind, name)
            require(len(candidates) == 1,
                    f"PropertyDescriptor.{name} should be declared exactly "
                    f"once, found {len(candidates)}")
            for member in candidates:
                require(member["abstract"] if kind == "method"
                        else member["getAbstract"],
                        f"PropertyDescriptor.{name} is not abstract")
                actual = member["returnType"] if kind == "method" \
                    else member["type"]
                require(actual == ret,
                        f"PropertyDescriptor.{name} is not {ret}")
        converter_property = members_of(PROPERTY_DESCRIPTOR, "property",
                                        "Converter")
        require(len(converter_property) == 1,
                "PropertyDescriptor.Converter is missing")
        for member in converter_property:
            require(member["type"] == TYPE_CONVERTER,
                    "PropertyDescriptor.Converter is not a TypeConverter")

    # ------------------------------------------------------------------
    # System.ComponentModel.PropertyDescriptorCollection.
    # ------------------------------------------------------------------
    require(PROPERTY_DESCRIPTOR_COLLECTION in by_type,
            "System.ComponentModel.PropertyDescriptorCollection was not "
            "extracted at all")
    collection_pd = by_type.get(PROPERTY_DESCRIPTOR_COLLECTION)
    if collection_pd is not None:
        require(collection_pd["kind"] == "class",
                "PropertyDescriptorCollection is not a class")
        require(collection_pd["baseType"] == "System.Object",
                "PropertyDescriptorCollection does not derive directly from "
                "System.Object")
        # It is BOTH a list and a dictionary -- indexable by position and by
        # name. That is the second, independent reason IDictionary is in this
        # closure, and it is why the type has two indexers.
        for interface in ("System.Collections.IList",
                          NONGENERIC_IDICTIONARY,
                          "System.Collections.ICollection",
                          "System.Collections.IEnumerable"):
            require(interface in collection_pd["directInterfaces"],
                    f"PropertyDescriptorCollection does not declare {interface}")
        indexers = [
            member for member in collection_pd["members"]
            if member["kind"] == "property" and member["name"] == "Item"
        ]
        require(len(indexers) == 2,
                f"PropertyDescriptorCollection should declare 2 indexers "
                f"(by index and by name), found {len(indexers)}")
        require({tuple(item["type"] for item in member["parameters"])
                 for member in indexers}
                == {("System.Int32",), ("System.String",)},
                "PropertyDescriptorCollection's indexers are not one Int32 "
                "and one String")
        for member in indexers:
            require(member["type"] == PROPERTY_DESCRIPTOR,
                    "PropertyDescriptorCollection's indexer does not yield a "
                    "PropertyDescriptor")
        find = members_of(PROPERTY_DESCRIPTOR_COLLECTION, "method", "Find")
        require(len(find) == 1, "PropertyDescriptorCollection.Find is missing")
        for member in find:
            require([item["type"] for item in member["parameters"]]
                    == ["System.String", "System.Boolean"],
                    "Find does not take (String, Boolean); the Boolean is the "
                    "ignoreCase flag and dropping it would change lookups")
            require(member["returnType"] == PROPERTY_DESCRIPTOR,
                    "Find does not return a PropertyDescriptor")

    # ------------------------------------------------------------------
    # System.Globalization.CultureInfo (mscorlib).
    #
    # Foundation 74 recorded that this is NOT merely an opaque token passed
    # through: the math converters format and parse through it.
    # ------------------------------------------------------------------
    require(CULTURE_INFO in by_type,
            "System.Globalization.CultureInfo was not extracted at all")
    culture = by_type.get(CULTURE_INFO)
    if culture is not None:
        require(culture["kind"] == "class", "CultureInfo is not a class")
        require(not culture["sealed"], "CultureInfo is sealed")
        for interface in ("System.ICloneable", "System.IFormatProvider"):
            require(interface in culture["directInterfaces"],
                    f"CultureInfo does not declare {interface}")
        for name in ("InvariantCulture", "CurrentCulture", "CurrentUICulture"):
            candidates = members_of(CULTURE_INFO, "property", name)
            require(len(candidates) == 1, f"CultureInfo.{name} is missing")
            for member in candidates:
                require(member["static"], f"CultureInfo.{name} is not static")
                require(member["type"] == CULTURE_INFO,
                        f"CultureInfo.{name} is not a CultureInfo")
                # In 4.0 the ambient culture is read-only from here; the
                # public setter arrives in 4.6. A projection that offered a
                # setter would be offering a later framework's contract.
                require(member["setAccess"] is None,
                        f"CultureInfo.{name} has a public setter; that is a "
                        ".NET 4.6 addition, not 4.0")
        number_format = members_of(CULTURE_INFO, "property", "NumberFormat")
        require(len(number_format) == 1, "CultureInfo.NumberFormat is missing")
        for member in number_format:
            require(member["type"]
                    == "System.Globalization.NumberFormatInfo",
                    "CultureInfo.NumberFormat is not a NumberFormatInfo")
            require(member["setAccess"] is not None,
                    "CultureInfo.NumberFormat is read-only")
        get_format = members_of(CULTURE_INFO, "method", "GetFormat")
        require(len(get_format) == 1, "CultureInfo.GetFormat is missing")
        for member in get_format:
            require([item["type"] for item in member["parameters"]]
                    == ["System.Type"],
                    "GetFormat does not take exactly one System.Type")
            require(member["returnType"] == "System.Object",
                    "GetFormat does not return System.Object")

    # ------------------------------------------------------------------
    # System.Collections.IDictionary (mscorlib).
    # ------------------------------------------------------------------
    require(NONGENERIC_IDICTIONARY in by_type,
            "System.Collections.IDictionary was not extracted at all")
    idict = by_type.get(NONGENERIC_IDICTIONARY)
    if idict is not None:
        require(idict["kind"] == "interface",
                "System.Collections.IDictionary is not an interface")
        require(idict["genericArity"] == 0,
                "System.Collections.IDictionary is generic; the generic one is "
                "IDictionary`2 and is a different type")
        require("System.Collections.ICollection" in idict["directInterfaces"],
                "System.Collections.IDictionary does not extend ICollection")
        require(len(idict["members"]) == 10,
                f"System.Collections.IDictionary should declare exactly 10 "
                f"members, found {len(idict['members'])}")
        enumerator = members_of(NONGENERIC_IDICTIONARY, "method",
                                "GetEnumerator")
        require(len(enumerator) == 1,
                "System.Collections.IDictionary.GetEnumerator is missing")
        for member in enumerator:
            # It hides IEnumerable's method with a NARROWER return. Extracting
            # IEnumerator here would erase the entry-pair enumeration that is
            # the only reason the interface is dictionary-shaped.
            require(member["returnType"]
                    == "System.Collections.IDictionaryEnumerator",
                    "IDictionary.GetEnumerator does not return "
                    "IDictionaryEnumerator")
        for name in ("Keys", "Values"):
            candidates = members_of(NONGENERIC_IDICTIONARY, "property", name)
            require(len(candidates) == 1, f"IDictionary.{name} is missing")
            for member in candidates:
                require(member["type"] == "System.Collections.ICollection",
                        f"IDictionary.{name} is not an ICollection")


    # ------------------------------------------------------------------
    # System.IAsyncResult and the Storage closure (Foundation 102).
    #
    # Written from the documented .NET Framework 4.0 contract before reading
    # the extraction, as the ComponentModel closure was. The enums matter more
    # than their size suggests: an enum has no behaviour to get wrong, only
    # values, and a value that shifts changes which file operation a caller
    # asked for without any signature changing shape.
    # ------------------------------------------------------------------
    require(ASYNC_RESULT in by_type,
            "System.IAsyncResult was not extracted at all")
    async_result = by_type.get(ASYNC_RESULT)
    if async_result is not None:
        require(async_result["kind"] == "interface",
                "IAsyncResult is not an interface")
        require(not async_result["directInterfaces"],
                "IAsyncResult extends another interface")
        require(len(async_result["members"]) == 4,
                f"IAsyncResult should declare exactly 4 members, "
                f"found {len(async_result['members'])}")
        for name, clr_type in (
            ("IsCompleted", "System.Boolean"),
            ("CompletedSynchronously", "System.Boolean"),
            ("AsyncState", "System.Object"),
            # The one this projection must DECLARE and REFUSE: handing back a
            # WaitHandle would mean inventing a synchronisation primitive, and
            # CNA's storage routes have already finished by the time a caller
            # could wait on one.
            ("AsyncWaitHandle", "System.Threading.WaitHandle"),
        ):
            candidates = members_of(ASYNC_RESULT, "property", name)
            require(len(candidates) == 1, f"IAsyncResult.{name} is missing")
            for member in candidates:
                require(member["type"] == clr_type,
                        f"IAsyncResult.{name} is not {clr_type}")
                require(member["setAccess"] is None,
                        f"IAsyncResult.{name} has a setter")
                require(member["getAbstract"],
                        f"IAsyncResult.{name} is not abstract")

    require(ASYNC_CALLBACK in by_type,
            "System.AsyncCallback was not extracted at all")
    callback = by_type.get(ASYNC_CALLBACK)
    if callback is not None:
        require(callback["kind"] == "class",
                "AsyncCallback is not a class; a delegate is one in metadata")
        require(callback["sealed"], "AsyncCallback is not sealed")
        require(callback["baseType"] == "System.MulticastDelegate",
                "AsyncCallback does not derive from MulticastDelegate")
        invoke = members_of(ASYNC_CALLBACK, "method", "Invoke")
        require(len(invoke) == 1, "AsyncCallback.Invoke is missing")
        for member in invoke:
            require(member["returnType"] == "System.Void",
                    "AsyncCallback.Invoke does not return void")
            require([item["type"] for item in member["parameters"]]
                    == [ASYNC_RESULT],
                    "AsyncCallback.Invoke does not take exactly one "
                    "IAsyncResult")

    # -- the three file enums ---------------------------------------------
    def enum_values(type_name: str) -> dict[str, Any]:
        record = by_type.get(type_name)
        if record is None:
            return {}
        return {
            item["name"]: item.get("value")
            for item in record["members"]
            if item["kind"] not in ("method", "constructor", "property", "event")
            and item.get("constant")
        }

    for type_name, expected, flags in (
        (FILE_MODE, {"CreateNew": 1, "Create": 2, "Open": 3, "OpenOrCreate": 4,
                     "Truncate": 5, "Append": 6}, False),
        (FILE_ACCESS, {"Read": 1, "Write": 2, "ReadWrite": 3}, True),
        (FILE_SHARE, {"None": 0, "Read": 1, "Write": 2, "ReadWrite": 3,
                      "Delete": 4, "Inheritable": 16}, True),
    ):
        require(type_name in by_type, f"{type_name} was not extracted at all")
        record = by_type.get(type_name)
        if record is None:
            continue
        require(record["baseType"] == "System.Enum",
                f"{type_name} is not an enum")
        found = enum_values(type_name)
        for name, value in expected.items():
            require(name in found, f"{type_name}.{name} is missing")
            if name in found:
                # The parser records a literal in its decimal SPELLING, so the
                # comparison is between strings. Comparing to an int silently
                # fails for every value and reports "is 3, not 3".
                require(str(found[name]) == str(value),
                        f"{type_name}.{name} is {found[name]!r}, not {value}")
        require(set(found) == set(expected),
                f"{type_name} declares {sorted(set(found) - set(expected))} "
                f"beyond the contract and is missing "
                f"{sorted(set(expected) - set(found))}")

    return checks, failures


def static_int32_table(il: str, type_name: str, field_name: str) -> list[int] | None:
    """One `static readonly int[]` initialised from a static-array RVA blob.

    The C# compiler lowers such a table to a `<PrivateImplementationDetails>`
    field placed at a data address, filled by `RuntimeHelpers.InitializeArray`.
    The chain followed here is exactly that one: the type's `.cctor` names the
    initializer field, the field declaration names its data address, and the
    address names a `.data ... = bytearray (...)` blob. Reading it means a
    table this binding reproduces is a fact about the assembly rather than
    something transcribed by hand.

    `None` when any link is absent, which is reported rather than assumed away.
    """
    opening = re.search(
        rf"^\.class\s+.*?\b{re.escape(type_name)}\s*$", il, re.M)
    if opening is None:
        return None
    closing = re.search(
        rf"^\}}\s*//\s*end of class\s+{re.escape(type_name)}\s*$",
        il[opening.start():], re.M)
    if closing is None:
        return None
    block = il[opening.start():opening.start() + closing.end()]

    # The `.cctor` stores into the named field; the `ldtoken` immediately
    # before that store names the initializer field.
    store = re.search(
        rf"ldtoken\s+field[^\n]*?'(\$\$method[0-9a-zA-Z-]+)'[\s\S]{{0,400}}?"
        rf"stsfld\s+int32\[\]\s+{re.escape(type_name)}::{re.escape(field_name)}",
        block)
    if store is None:
        return None
    initializer = store.group(1)

    placement = re.search(
        rf"'{re.escape(initializer)}'\s+at\s+(I_[0-9A-Fa-f]+)", il)
    if placement is None:
        return None
    blob = re.search(
        rf"^\.data\s+cil\s+{placement.group(1)}\s*=\s*bytearray\s*\("
        rf"([\s\S]*?)\)\s*$", il, re.M)
    if blob is None:
        return None
    # `ikdasm` appends an ASCII rendering after `//` on most rows, and a
    # rendering can contain two characters that look like a hex byte. The
    # comment is removed per line before any byte is read, so nothing outside
    # the blob can be mistaken for data.
    hex_only = "\n".join(
        line.split("//", 1)[0] for line in blob.group(1).splitlines())
    payload = bytes(
        int(item, 16) for item in re.findall(r"\b([0-9A-Fa-f]{2})\b", hex_only))

    # The declared element count comes from the `newarr` the `.cctor` performs,
    # so a blob that is longer than the array cannot silently add entries.
    size = re.search(
        r"ldc\.i4(?:\.s)?\s+(\d+)\s*\n\s*IL_[0-9a-f]+:\s+newarr\s+System\.Int32",
        block)
    count = int(size.group(1)) if size else len(payload) // 4
    if len(payload) < count * 4:
        return None
    return [
        int.from_bytes(payload[index * 4:index * 4 + 4], "little", signed=True)
        for index in range(count)
    ]


def static_table_checks(
    il: str, selected: list[dict[str, Any]],
) -> tuple[int, list[str], list[dict[str, Any]]]:
    """Extract every registered static table, or say why it could not be."""
    failures: list[str] = []
    records: list[dict[str, Any]] = []
    checks = 0
    for entry in selected:
        checks += 1
        values = static_int32_table(il, entry["type"], entry["field"])
        if values is None:
            failures.append(
                f"static table {entry['type']}::{entry['field']} could not be "
                "read from the assembly")
            continue
        records.append({
            "type": entry["type"],
            "field": entry["field"],
            "swiftSymbol": entry.get("swiftSymbol"),
            "values": values,
        })
    return checks, failures, records


IL_ESCAPES = {"r": "\r", "n": "\n", "t": "\t", "0": "\0", "\\": "\\", '"': '"'}


def decode_il_string(text: str) -> str:
    """Decode the escapes ikdasm writes inside an `ldstr` operand."""
    out: list[str] = []
    index = 0
    while index < len(text):
        char = text[index]
        if char == "\\" and index + 1 < len(text):
            following = text[index + 1]
            if following in IL_ESCAPES:
                out.append(IL_ESCAPES[following])
                index += 2
                continue
        out.append(char)
        index += 1
    return "".join(out)


def il_string_literal(il: str, type_name: str, member: str) -> str | None:
    """The single `ldstr` a one-line getter returns, or None.

    A registered literal must be the method's WHOLE body: `ldstr "..."` then
    `ret`, nothing else. A getter that branched, cached, or consulted the
    platform would not be a literal fact about the assembly, and reading one
    out of a longer body would be exactly the vacuous measurement this audit
    exists to prevent.
    """
    start = il.find(f"beforefieldinit {type_name}\n")
    if start < 0:
        start = il.find(f" {type_name}\n")
    if start < 0:
        return None
    end = il.find("} // end of class " + type_name.rsplit(".", 1)[-1], start)
    body = il[start:end if end > 0 else len(il)]
    site = body.find(f" {member}() cil managed")
    if site < 0:
        return None
    close = body.find("} // end of method", site)
    method = body[site:close if close > 0 else len(body)]
    instructions = re.findall(r"IL_[0-9a-fA-F]+:\s+(\S+)(?:\s+(.*))?", method)
    if len(instructions) != 2 or instructions[0][0] != "ldstr" or instructions[1][0] != "ret":
        return None
    operand = instructions[0][1].strip()
    if not (operand.startswith('"') and operand.endswith('"')):
        return None
    return decode_il_string(operand[1:-1])


def il_literal_checks(
    il: str, selected: list[dict[str, Any]],
) -> tuple[int, list[str], list[dict[str, Any]]]:
    """Every registered IL string literal, read from the body that returns it."""
    failures: list[str] = []
    records: list[dict[str, Any]] = []
    checks = 0
    for entry in selected:
        checks += 1
        found = il_string_literal(il, entry["type"], entry["member"])
        if found is None:
            failures.append(
                f"IL literal {entry['type']}::{entry['member']} is not a "
                "single ldstr/ret body in the assembly")
            continue
        if found != entry["value"]:
            failures.append(
                f"IL literal {entry['type']}::{entry['member']} is {found!r} "
                f"in the assembly, registered as {entry['value']!r}")
            continue
        records.append({
            "type": entry["type"],
            "member": entry["member"],
            "swiftSymbol": entry.get("swiftSymbol"),
            "value": found,
        })
    return checks, failures, records


def resource_checks(
    strings: dict[str, str], selected: list[dict[str, Any]],
) -> tuple[int, list[str]]:
    """Every selected message string, read from the binary's own resources.

    `Exception.get_Message`, `SystemException..ctor()` and
    `ExternalException..ctor()` all load a resource KEY. What a projected XNA
    exception reports is therefore a fact about the embedded resource table,
    and admitting it means reproducing that table's value exactly.
    """
    failures: list[str] = []
    checks = 0
    for entry in selected:
        checks += 1
        found = strings.get(entry["key"])
        if found is None:
            failures.append(
                f"resource key {entry['key']!r} is not in the assembly's "
                "embedded string table")
        elif found != entry["value"]:
            failures.append(
                f"resource {entry['key']!r} is {found!r} in the assembly, "
                f"registered as {entry['value']!r}")
    return checks, failures


# ----------------------------------------------------------------------------
# Mutation self-tests
# ----------------------------------------------------------------------------


def mutation_self_tests(
    manifest: list[dict[str, Any]],
) -> tuple[int, list[str]]:
    """Prove the comparison and the sentinels are not vacuous.

    Every mutation below perturbs one already-matching record. A mutation that
    the comparison does not detect would mean the audit passes for the wrong
    reason.
    """
    failures: list[str] = []
    checks = 0
    by_name = {item["type"]: item for item in manifest}

    subjects = [name for name in (COLLECTION, READONLY, LIST, ILIST, EXCEPTION,
                                  SYSTEM_EXCEPTION, EXTERNAL_EXCEPTION,
                                  DICTIONARY, KEY_COLLECTION, VALUE_COLLECTION,
                                  DICT_ENUMERATOR, KEY_VALUE_PAIR,
                                  IEQUALITY_COMPARER, IDICTIONARY, ATTRIBUTE,
                                  ARGUMENT_EXCEPTION, ARGUMENT_NULL_EXCEPTION,
                                  ARGUMENT_RANGE_EXCEPTION,
                                  NOT_SUPPORTED_EXCEPTION,
                                  INVALID_OPERATION_EXCEPTION,
                                  KEY_NOT_FOUND_EXCEPTION)
                if name in by_name]
    checks += 1
    if len(subjects) < 15:
        failures.append("mutation self-test subjects are missing")

    def first_of(record: dict[str, Any], kind: str) -> dict[str, Any] | None:
        for member in record["members"]:
            if member["kind"] == kind:
                return member
        return None

    def drop_member(kind: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            for index, member in enumerate(record["members"]):
                if member["kind"] == kind:
                    del record["members"][index]
                    return True
            return False
        return mutate

    def add_member_named(member_name: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            record["members"].append(member_record({
                "kind": "method", "name": member_name, "access": "public",
                "static": False, "returnType": "System.Void",
                "genericParameters": [], "parameters": [],
            }))
            return True
        return mutate

    def add_member(record: dict[str, Any]) -> bool:
        record["members"].append(member_record({
            "kind": "method", "name": "InventedMember", "access": "public",
            "static": False, "returnType": "System.Void",
            "genericParameters": [], "parameters": [],
        }))
        return True

    def change_arity(record: dict[str, Any]) -> bool:
        record["genericArity"] += 1
        return True

    def change_generic_parameters(record: dict[str, Any]) -> bool:
        record["genericParameters"] = ["TRenamed"]
        return True

    def change_base(record: dict[str, Any]) -> bool:
        record["baseType"] = "System.Collections.ObjectModel.ReadOnlyCollection`1"
        return True

    def seal_property(property_name: str) -> Any:
        """Make one named property's getter non-overridable.

        `flip_virtual` takes whichever member happens to sort first, so it
        cannot prove that a SPECIFIC override point is protected. Message is
        overridable and InnerException is not; getting that pair backwards is
        the mistake this aims at.
        """
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if (
                    member["kind"] == "property" and
                    member["name"] == property_name and
                    member.get("getOverridable")
                ):
                    member["getOverridable"] = False
                    return True
            return False
        return mutate

    def unseal_property(property_name: str) -> Any:
        """Make one named property's getter overridable.

        The mirror of `seal_property`, and the only way to plant the specific
        confusion that ObjectDisposedException invites: `ObjectName` is NOT
        virtual, while the neighbouring `ArgumentException.ParamName` is, so a
        projection written from the ArgumentException template would make it
        `open` and nothing else would notice.
        """
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if (
                    member["kind"] == "property" and
                    member["name"] == property_name and
                    not member.get("getOverridable")
                ):
                    member["getOverridable"] = True
                    return True
            return False
        return mutate

    def seal_method(method_name: str) -> Any:
        """Make one NAMED method non-overridable.

        `flip_virtual` takes whichever member sorts first, so it cannot prove a
        specific override point is protected.
        """
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if (
                    member["kind"] == "method" and
                    member["name"] == method_name and
                    member.get("overridable")
                ):
                    member["overridable"] = False
                    member["final"] = True
                    return True
            return False
        return mutate

    def rebase_onto(base: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            if record["baseType"] == base:
                return False
            record["baseType"] = base
            return True
        return mutate

    def drop_interface(record: dict[str, Any]) -> bool:
        if not record["directInterfaces"]:
            return False
        record["directInterfaces"] = record["directInterfaces"][1:]
        return True

    def drop_ilist_interface(record: dict[str, Any]) -> bool:
        target = "System.Collections.Generic.IList`1[!T]"
        if target not in record["directInterfaces"]:
            return False
        record["directInterfaces"] = [
            item for item in record["directInterfaces"] if item != target]
        return True

    def drop_named_interface(target: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            if target not in record["directInterfaces"]:
                return False
            record["directInterfaces"] = [
                item for item in record["directInterfaces"] if item != target]
            return True
        return mutate

    def drop_named_member(kind: str, name: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            before = len(record["members"])
            record["members"] = [
                item for item in record["members"]
                if not (item["kind"] == kind and item["name"] == name)
            ]
            return len(record["members"]) != before
        return mutate

    def set_abstract(value: bool) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            if record.get("abstract") == value:
                return False
            record["abstract"] = value
            return True
        return mutate

    def set_return(name: str, clr_type: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if (member["kind"] == "method" and member["name"] == name
                        and member.get("returnType") != clr_type):
                    member["returnType"] = clr_type
                    return True
            return False
        return mutate

    def make_concrete(name: str) -> Any:
        """Turn an abstract member into a virtual one with a body.

        This is the single most plausible wrong projection of
        PropertyDescriptor: an abstract member that becomes virtual still
        compiles at every call site and silently stops requiring a derived
        descriptor to supply it.
        """
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if member["name"] != name:
                    continue
                if member["kind"] == "method" and member.get("abstract"):
                    member["abstract"] = False
                    return True
                if member["kind"] == "property" and member.get("getAbstract"):
                    member["getAbstract"] = False
                    return True
            return False
        return mutate

    def widen_shortest_overload(name: str) -> Any:
        """Make the context-free forwarder an override point."""
        def mutate(record: dict[str, Any]) -> bool:
            overloads = [
                member for member in record["members"]
                if member["kind"] == "method" and member["name"] == name
            ]
            if not overloads:
                return False
            shortest = min(overloads, key=lambda m: len(m["parameters"]))
            if shortest.get("virtual"):
                return False
            shortest["virtual"] = True
            shortest["overridable"] = True
            return True
        return mutate

    def seal_overload(name: str) -> Any:
        """Make a named method's WIDEST overload `virtual final`.

        The sneakiest wrong projection: the member is still marked virtual, so
        a reviewer scanning for the keyword sees nothing wrong, but no derived
        converter can actually replace it.
        """
        def mutate(record: dict[str, Any]) -> bool:
            overloads = [
                member for member in record["members"]
                if member["kind"] == "method" and member["name"] == name
                and member.get("overridable")
            ]
            if not overloads:
                return False
            widest = max(overloads, key=lambda m: len(m["parameters"]))
            widest["overridable"] = False
            widest["final"] = True
            return True
        return mutate

    def change_enum_value(name: str) -> Any:
        """Renumber one enum member.

        The mutation the file-enum families exist for. An enum has no
        behaviour to break, only values, and a renumbered one changes which
        file operation a caller asked for while every signature keeps its
        shape. Until Foundation 102 this could not even be attempted: field
        records carried no `access`, so the audit dropped every field and an
        enum extracted as a type with nothing in it.
        """
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if member.get("name") == name and member.get("constant"):
                    member["value"] = str(int(member["value"]) + 1)
                    return True
            return False
        return mutate

    def drop_enum_value(name: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            before = len(record["members"])
            record["members"] = [
                item for item in record["members"]
                if not (item.get("name") == name and item.get("constant"))
            ]
            return len(record["members"]) != before
        return mutate

    def flip_static_of(name: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if member["name"] == name and "static" in member:
                    member["static"] = not member["static"]
                    return True
            return False
        return mutate

    def add_setter_to(name: str) -> Any:
        def mutate(record: dict[str, Any]) -> bool:
            for member in record["members"]:
                if (member["kind"] == "property" and member["name"] == name
                        and member.get("setAccess") is None):
                    member["setAccess"] = "public"
                    return True
            return False
        return mutate

    def add_interface(record: dict[str, Any]) -> bool:
        record["directInterfaces"] = sorted(
            record["directInterfaces"] + ["System.ICloneable"])
        return True

    def change_kind(record: dict[str, Any]) -> bool:
        record["kind"] = "interface" if record["kind"] == "class" else "class"
        return True

    def flip_sealed(record: dict[str, Any]) -> bool:
        record["sealed"] = not record["sealed"]
        return True

    def change_visibility(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member.get("access") == "protected":
                member["access"] = "public"
                return True
            if member.get("getAccess") == "protected":
                member["getAccess"] = "public"
                return True
        return False

    def flip_virtual(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member.get("overridable"):
                member["overridable"] = False
                member["final"] = True
                return True
        return False

    def unseal_public_member(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member["kind"] == "method" and member.get("final"):
                member["final"] = False
                member["overridable"] = True
                return True
        return False

    def change_parameter_type(record: dict[str, Any]) -> bool:
        # Skip a parameter that is ALREADY the substitute: rewriting
        # `System.Object` to `System.Object` changes nothing, and a mutation
        # that does not mutate cannot prove the comparison detects anything.
        for member in record["members"]:
            for parameter in member.get("parameters", []):
                if parameter.get("type") != "System.Object":
                    parameter["type"] = "System.Object"
                    return True
        return False

    def change_return_type(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member["kind"] == "method" and member.get("returnType"):
                member["returnType"] = "System.Object"
                return True
        return False

    def change_property_type(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member["kind"] == "property" and member.get("type") != "System.Object":
                member["type"] = "System.Object"
                return True
        return False

    def flip_static(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if "static" in member:
                member["static"] = not member["static"]
                return True
        return False

    def rename_member(record: dict[str, Any]) -> bool:
        member = first_of(record, "method")
        if member is None:
            return False
        member["name"] += "Renamed"
        return True

    def add_setter(record: dict[str, Any]) -> bool:
        for member in record["members"]:
            if member["kind"] == "property" and member.get("setAccess") is None:
                member["setAccess"] = "public"
                return True
        return False

    mutations = [
        ("dropped method", drop_member("method")),
        ("dropped constructor", drop_member("constructor")),
        ("dropped property", drop_member("property")),
        ("invented extra member", add_member),
        ("changed generic arity", change_arity),
        ("renamed generic parameter", change_generic_parameters),
        ("changed base type", change_base),
        ("dropped declared interface", drop_interface),
        ("invented declared interface", add_interface),
        ("changed type kind", change_kind),
        ("flipped sealed", flip_sealed),
        ("widened member visibility", change_visibility),
        ("removed a virtual flag", flip_virtual),
        ("unsealed a sealed public member", unseal_public_member),
        ("changed parameter type", change_parameter_type),
        ("changed return type", change_return_type),
        ("changed property type", change_property_type),
        ("flipped static identity", flip_static),
        ("renamed member", rename_member),
        ("invented a property setter", add_setter),
    ]

    for name in subjects:
        pinned = by_name[name]
        checks += 1
        if compare_records(pinned, pinned):
            failures.append(f"{name}: the unmutated comparison is not clean")
        for label, mutate in mutations:
            candidate = copy.deepcopy(pinned)
            if not mutate(candidate):
                continue
            checks += 1
            if not compare_records(pinned, candidate):
                failures.append(f"{name}: {label} was not detected")

    # A dropped type must be caught by the audit's own missing-type path, and a
    # sentinel set that survives an empty extraction would be worthless.
    checks += 1
    if not sentinel_checks({})[1]:
        failures.append("the sentinels pass against an empty extraction")

    # Each of these single mutations must break at least one sentinel.
    sentinel_mutations: list[tuple[str, str, Any]] = [
        ("Collection<T> made sealed", COLLECTION, flip_sealed),
        ("Collection<T> rebased on ReadOnlyCollection<T>", COLLECTION, change_base),
        ("Collection<T> arity changed", COLLECTION, change_arity),
        ("a Collection<T> hook made non-overridable", COLLECTION, flip_virtual),
        ("a Collection<T> hook made public", COLLECTION, change_visibility),
        ("a Collection<T> interface dropped", COLLECTION, drop_interface),
        ("a Collection<T> member dropped", COLLECTION, drop_member("method")),
        ("a Collection<T> constructor dropped", COLLECTION, drop_member("constructor")),
        ("ReadOnlyCollection<T> given a setter", READONLY, add_setter),
        ("ReadOnlyCollection<T> constructor dropped", READONLY,
         drop_member("constructor")),
        ("IList<T> turned into a class", ILIST, change_kind),
        ("List<T>'s IList<T> implementation dropped", LIST, drop_ilist_interface),
        ("Collection<T>'s IList<T> implementation dropped", COLLECTION,
         drop_ilist_interface),
        ("ReadOnlyCollection<T>'s IList<T> implementation dropped", READONLY,
         drop_ilist_interface),
        # The exception chain. Each of these is a way a plausible but wrong
        # projection could be justified, and each must break a sentinel.
        ("Exception made sealed", EXCEPTION, flip_sealed),
        ("Exception rebased", EXCEPTION, change_base),
        ("an Exception constructor dropped", EXCEPTION,
         drop_member("constructor")),
        ("an Exception property dropped", EXCEPTION, drop_member("property")),
        ("Exception's protected HResult widened", EXCEPTION, change_visibility),
        ("Message made non-overridable", EXCEPTION, seal_property("Message")),
        ("HelpLink made non-overridable", EXCEPTION, seal_property("HelpLink")),
        ("ErrorCode made non-overridable", EXTERNAL_EXCEPTION,
         seal_property("ErrorCode")),
        ("ISerializable dropped from Exception", EXCEPTION, drop_interface),
        ("SystemException collapsed onto Object", SYSTEM_EXCEPTION,
         rebase_onto("System.Object")),
        ("SystemException given a member of its own", SYSTEM_EXCEPTION,
         add_member),
        ("a SystemException constructor dropped", SYSTEM_EXCEPTION,
         drop_member("constructor")),
        ("ExternalException collapsed straight onto Exception",
         EXTERNAL_EXCEPTION, rebase_onto(EXCEPTION)),
        ("ErrorCode dropped", EXTERNAL_EXCEPTION, drop_member("property")),
        ("an ExternalException constructor dropped", EXTERNAL_EXCEPTION,
         drop_member("constructor")),
        ("ErrorCode given a setter", EXTERNAL_EXCEPTION, add_setter),
        # The six raised families. Each mutation is a way a projected payload
        # could have been quietly wrong: the wrong base changes which `catch`
        # sees it, a dropped constructor removes the overload a raise site
        # selects, and a dropped ParamName or Message override changes the
        # composed message itself.
        ("ArgumentException collapsed onto Exception", ARGUMENT_EXCEPTION,
         rebase_onto(EXCEPTION)),
        ("ArgumentException made sealed", ARGUMENT_EXCEPTION, flip_sealed),
        ("ParamName dropped", ARGUMENT_EXCEPTION, drop_member("property")),
        ("ParamName given a setter", ARGUMENT_EXCEPTION, add_setter),
        ("an ArgumentException constructor dropped", ARGUMENT_EXCEPTION,
         drop_member("constructor")),
        ("ArgumentNullException rebased onto SystemException",
         ARGUMENT_NULL_EXCEPTION, rebase_onto(SYSTEM_EXCEPTION)),
        ("an ArgumentNullException constructor dropped",
         ARGUMENT_NULL_EXCEPTION, drop_member("constructor")),
        ("ArgumentNullException given a member of its own",
         ARGUMENT_NULL_EXCEPTION, add_member),
        ("ArgumentOutOfRangeException rebased onto SystemException",
         ARGUMENT_RANGE_EXCEPTION, rebase_onto(SYSTEM_EXCEPTION)),
        ("ActualValue dropped", ARGUMENT_RANGE_EXCEPTION,
         drop_member("property")),
        ("an ArgumentOutOfRangeException constructor dropped",
         ARGUMENT_RANGE_EXCEPTION, drop_member("constructor")),
        ("NotSupportedException rebased onto ArgumentException",
         NOT_SUPPORTED_EXCEPTION, rebase_onto(ARGUMENT_EXCEPTION)),
        ("NotSupportedException given a member of its own",
         NOT_SUPPORTED_EXCEPTION, add_member),
        ("a NotSupportedException constructor dropped",
         NOT_SUPPORTED_EXCEPTION, drop_member("constructor")),
        ("InvalidOperationException rebased onto Exception",
         INVALID_OPERATION_EXCEPTION, rebase_onto(EXCEPTION)),
        ("InvalidOperationException given a member of its own",
         INVALID_OPERATION_EXCEPTION, add_member),
        ("ObjectDisposedException rebased onto SystemException",
         OBJECT_DISPOSED_EXCEPTION, rebase_onto(SYSTEM_EXCEPTION)),
        ("ObjectDisposedException made sealed", OBJECT_DISPOSED_EXCEPTION,
         flip_sealed),
        ("ObjectName dropped", OBJECT_DISPOSED_EXCEPTION,
         drop_member("property")),
        ("ObjectName given a setter", OBJECT_DISPOSED_EXCEPTION, add_setter),
        ("ObjectName made overridable, as ParamName is",
         OBJECT_DISPOSED_EXCEPTION, unseal_property("ObjectName")),
        ("Message made non-overridable on ObjectDisposedException",
         OBJECT_DISPOSED_EXCEPTION, seal_property("Message")),
        ("an ObjectDisposedException constructor dropped",
         OBJECT_DISPOSED_EXCEPTION, drop_member("constructor")),
        ("KeyNotFoundException rebased onto ArgumentException",
         KEY_NOT_FOUND_EXCEPTION, rebase_onto(ARGUMENT_EXCEPTION)),
        ("KeyNotFoundException given a member of its own",
         KEY_NOT_FOUND_EXCEPTION, add_member),
        ("a KeyNotFoundException constructor dropped",
         KEY_NOT_FOUND_EXCEPTION, drop_member("constructor")),
        # The dictionary family. Each of these is a way the projection could
        # have been quietly wrong.
        ("Dictionary<K,V> made sealed", DICTIONARY, flip_sealed),
        ("Dictionary<K,V> arity changed", DICTIONARY, change_arity),
        ("Dictionary<K,V> rebased", DICTIONARY, rebase_onto(COLLECTION)),
        ("a Dictionary<K,V> method made an override point", DICTIONARY,
         unseal_public_member),
        ("a Dictionary<K,V> constructor dropped", DICTIONARY,
         drop_member("constructor")),
        ("a Dictionary<K,V> property dropped", DICTIONARY,
         drop_member("property")),
        ("a Dictionary<K,V> method dropped", DICTIONARY,
         drop_member("method")),
        ("a Dictionary<K,V> declared interface dropped", DICTIONARY,
         drop_interface),
        ("a Dictionary<K,V> property retyped", DICTIONARY,
         change_property_type),
        ("KeyCollection unsealed", KEY_COLLECTION, flip_sealed),
        ("KeyCollection given a mutator", KEY_COLLECTION, add_member_named("Add")),
        ("KeyCollection's CopyTo dropped", KEY_COLLECTION,
         drop_member("method")),
        ("ValueCollection given a mutator", VALUE_COLLECTION,
         add_member_named("Remove")),
        ("the nested Enumerator turned into a class", DICT_ENUMERATOR,
         change_kind),
        ("the nested Enumerator given a member beyond the contract",
         DICT_ENUMERATOR, add_member),
        ("KeyValuePair turned into a class", KEY_VALUE_PAIR, change_kind),
        ("KeyValuePair.Key given a setter", KEY_VALUE_PAIR, add_setter),
        ("IEqualityComparer<T> turned into a class", IEQUALITY_COMPARER,
         change_kind),
        ("IEqualityComparer<T>.GetHashCode dropped", IEQUALITY_COMPARER,
         drop_member("method")),
        # System.Attribute.
        ("Attribute made concrete", ATTRIBUTE,
         lambda record: (record.__setitem__("abstract", False), True)[1]),
        ("Attribute made sealed", ATTRIBUTE, flip_sealed),
        ("Attribute rebased", ATTRIBUTE, rebase_onto(EXCEPTION)),
        ("Attribute's protected constructor widened", ATTRIBUTE,
         change_visibility),
        ("Attribute's constructor dropped", ATTRIBUTE,
         drop_member("constructor")),
        ("IsDefaultAttribute made non-overridable", ATTRIBUTE,
         seal_method("IsDefaultAttribute")),
        # The ComponentModel closure (Foundation 100). Every one of these is a
        # projection someone could defend in a review, and each is wrong.
        ("TypeConverter made abstract", TYPE_CONVERTER, set_abstract(True)),
        ("TypeConverter made sealed", TYPE_CONVERTER, flip_sealed),
        ("TypeConverter rebased", TYPE_CONVERTER,
         rebase_onto(EXPANDABLE_CONVERTER)),
        ("ConvertFrom's context-free forwarder made an override point",
         TYPE_CONVERTER, widen_shortest_overload("ConvertFrom")),
        ("ConvertTo's context-free forwarder made an override point",
         TYPE_CONVERTER, widen_shortest_overload("ConvertTo")),
        ("CanConvertFrom's context overload left virtual but sealed",
         TYPE_CONVERTER, seal_overload("CanConvertFrom")),
        ("ConvertTo's context overload left virtual but sealed",
         TYPE_CONVERTER, seal_overload("ConvertTo")),
        ("a ConvertFrom overload dropped", TYPE_CONVERTER,
         drop_named_member("method", "ConvertFrom")),
        ("CreateInstance dropped, and with it the IDictionary bag",
         TYPE_CONVERTER, drop_named_member("method", "CreateInstance")),
        ("GetProperties made to return something other than a collection",
         TYPE_CONVERTER, set_return("GetProperties", "System.Object")),
        ("ExpandableObjectConverter sealed against MathTypeConverter",
         EXPANDABLE_CONVERTER, flip_sealed),
        ("ExpandableObjectConverter detached from TypeConverter",
         EXPANDABLE_CONVERTER, rebase_onto("System.Object")),
        ("ExpandableObjectConverter's GetProperties dropped",
         EXPANDABLE_CONVERTER, drop_named_member("method", "GetProperties")),
        ("ITypeDescriptorContext no longer a service provider",
         TYPE_DESCRIPTOR_CONTEXT, drop_named_interface(ISERVICE_PROVIDER)),
        ("ITypeDescriptorContext turned into a class",
         TYPE_DESCRIPTOR_CONTEXT, change_kind),
        ("OnComponentChanging robbed of its veto",
         TYPE_DESCRIPTOR_CONTEXT, set_return("OnComponentChanging",
                                             "System.Void")),
        ("ITypeDescriptorContext.PropertyDescriptor given a setter",
         TYPE_DESCRIPTOR_CONTEXT, add_setter_to("PropertyDescriptor")),
        ("PropertyDescriptor made concrete", PROPERTY_DESCRIPTOR,
         set_abstract(False)),
        ("PropertyDescriptor flattened onto Object", PROPERTY_DESCRIPTOR,
         rebase_onto("System.Object")),
        ("PropertyDescriptor's protected constructor widened",
         PROPERTY_DESCRIPTOR, change_visibility),
        ("GetValue made virtual instead of abstract", PROPERTY_DESCRIPTOR,
         make_concrete("GetValue")),
        ("PropertyType made virtual instead of abstract", PROPERTY_DESCRIPTOR,
         make_concrete("PropertyType")),
        ("PropertyDescriptor.Converter retyped", PROPERTY_DESCRIPTOR,
         change_property_type),
        ("PropertyDescriptorCollection stripped of IDictionary",
         PROPERTY_DESCRIPTOR_COLLECTION,
         drop_named_interface(NONGENERIC_IDICTIONARY)),
        ("PropertyDescriptorCollection's indexers dropped",
         PROPERTY_DESCRIPTOR_COLLECTION, drop_named_member("property", "Item")),
        ("Find robbed of its ignoreCase flag",
         PROPERTY_DESCRIPTOR_COLLECTION, drop_named_member("method", "Find")),
        ("CultureInfo made sealed", CULTURE_INFO, flip_sealed),
        ("CultureInfo no longer a format provider", CULTURE_INFO,
         drop_named_interface("System.IFormatProvider")),
        ("CurrentCulture given the setter it only gains in 4.6", CULTURE_INFO,
         add_setter_to("CurrentCulture")),
        ("InvariantCulture made an instance property", CULTURE_INFO,
         flip_static_of("InvariantCulture")),
        ("NumberFormat dropped", CULTURE_INFO,
         drop_named_member("property", "NumberFormat")),
        ("IDictionary turned into a class", NONGENERIC_IDICTIONARY,
         change_kind),
        ("IDictionary detached from ICollection", NONGENERIC_IDICTIONARY,
         drop_named_interface("System.Collections.ICollection")),
        ("IDictionary.GetEnumerator narrowed back to IEnumerator",
         NONGENERIC_IDICTIONARY,
         set_return("GetEnumerator", "System.Collections.IEnumerator")),
        ("IDictionary.Keys retyped", NONGENERIC_IDICTIONARY,
         drop_named_member("property", "Keys")),
        # The Storage closure (Foundation 102).
        ("IAsyncResult turned into a class", ASYNC_RESULT, change_kind),
        ("IAsyncResult given a fifth member", ASYNC_RESULT, add_member),
        ("IAsyncResult.IsCompleted given a setter", ASYNC_RESULT,
         add_setter_to("IsCompleted")),
        ("AsyncWaitHandle dropped, hiding the member that must be refused",
         ASYNC_RESULT, drop_named_member("property", "AsyncWaitHandle")),
        ("AsyncWaitHandle retyped away from WaitHandle", ASYNC_RESULT,
         change_property_type),
        ("AsyncCallback unsealed", ASYNC_CALLBACK, flip_sealed),
        ("AsyncCallback detached from MulticastDelegate", ASYNC_CALLBACK,
         rebase_onto("System.Object")),
        ("AsyncCallback.Invoke made to return a value", ASYNC_CALLBACK,
         set_return("Invoke", "System.Object")),
        ("FileMode.Append renumbered", FILE_MODE,
         change_enum_value("Append")),
        ("FileMode.Open renumbered", FILE_MODE, change_enum_value("Open")),
        ("FileMode.Truncate dropped", FILE_MODE, drop_enum_value("Truncate")),
        ("FileMode rebased off Enum", FILE_MODE, rebase_onto("System.Object")),
        ("FileAccess.ReadWrite renumbered, breaking the flag arithmetic",
         FILE_ACCESS, change_enum_value("ReadWrite")),
        ("FileAccess.Write dropped", FILE_ACCESS, drop_enum_value("Write")),
        ("FileShare.Inheritable renumbered", FILE_SHARE,
         change_enum_value("Inheritable")),
        ("FileShare.Delete dropped -- the value a hand-written enum forgets",
         FILE_SHARE, drop_enum_value("Delete")),
        ("FileShare.None renumbered away from zero", FILE_SHARE,
         change_enum_value("None")),
    ]
    for label, subject, mutate in sentinel_mutations:
        if subject not in by_name:
            continue
        mutated = {item["type"]: copy.deepcopy(item) for item in manifest}
        checks += 1
        if not mutate(mutated[subject]):
            failures.append(f"sentinel mutation {label!r} did not apply")
            continue
        if not sentinel_checks(mutated)[1]:
            failures.append(f"sentinel mutation {label!r} was not detected")

    # Removing a whole selected family must break the sentinels too.
    for subject in (COLLECTION, READONLY, LIST, ILIST, EXCEPTION,
                    SYSTEM_EXCEPTION, EXTERNAL_EXCEPTION, DICTIONARY,
                    KEY_COLLECTION, VALUE_COLLECTION, DICT_ENUMERATOR,
                    KEY_VALUE_PAIR, IEQUALITY_COMPARER, IDICTIONARY,
                    ATTRIBUTE, OBJECT_DISPOSED_EXCEPTION):
        if subject not in by_name:
            continue
        reduced = {
            item["type"]: copy.deepcopy(item)
            for item in manifest if item["type"] != subject
        }
        checks += 1
        if not sentinel_checks(reduced)[1]:
            failures.append(f"removing {subject} entirely was not detected")

    return checks, failures


# ----------------------------------------------------------------------------
# Independent metadata reader
# ----------------------------------------------------------------------------

# CLR TypeAttributes bits the independent reader is checked against.
TYPE_INTERFACE = 0x000020
TYPE_ABSTRACT = 0x000080
TYPE_SEALED = 0x000100


def monodis(path: Path, switch: str) -> str | None:
    try:
        completed = subprocess.run(
            ["monodis", switch, str(path)],
            capture_output=True, text=True, errors="replace", timeout=900,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired, OSError):
        return None
    if completed.returncode != 0:
        return None
    return completed.stdout


TYPEDEF_ROW = re.compile(
    r"^(\d+):\s+(\S+)\s+\(flist=(\d+),\s*mlist=(\d+),\s*flags=0x([0-9a-fA-F]+)")
METHOD_ROW = re.compile(r"^(\d+):\s+(.*?)\s*\(param:")


def monodis_method_name(signature: str) -> str | None:
    """The declared name in one `monodis --method` row.

    `instance default void ClearItems ()` -> `ClearItems`. A generic method is
    rendered by monodis with its parameter list attached to the name
    (`ConvertAll<TOutput>`), so the balanced suffix is removed before the name
    is taken -- a generic parameter list may itself contain spaces. A quoted
    name that contains a dot is an explicit interface implementation, which is
    private and unnameable, and is excluded exactly as the extraction excludes
    it.
    """
    head = signature.rsplit("(", 1)[0].strip()
    if not head:
        return None
    head, _generics = split_generic_suffix(head)
    if not head.split():
        return None
    name = head.split()[-1]
    quoted = name.startswith("'") and name.endswith("'")
    name = name.strip("'")
    if name in (".ctor", ".cctor"):
        return name
    if "." in name:
        return None
    if quoted and "." in name:
        return None
    return name


def cross_check(
    path: Path, records: list[dict[str, Any]],
) -> tuple[int, list[str], bool]:
    """Re-read the same binary with `monodis` and require agreement.

    `ikdasm` and `monodis` are separate implementations of a CLI metadata
    reader, so this is a genuine second opinion on the *parsing*, not a second
    opinion on the behaviour: the exact Microsoft binary remains the only
    authority for what the BCL does. If `monodis` is unavailable the check is
    reported as skipped, never silently passed.

    The comparison is index-sliced rather than name-grepped: `--typedef` gives
    each type's first method-table row, so the rows belonging to one type are
    exactly the half-open range up to the next type's first row. Property and
    event accessors are folded into properties by the extraction, so they are
    expanded back here -- which independently checks that the folding kept the
    accessors it claims to represent.
    """
    failures: list[str] = []
    checks = 0
    typedefs = monodis(path, "--typedef")
    methods = monodis(path, "--method")
    if typedefs is None or methods is None:
        return 0, [], False

    rows: list[tuple[int, str, int, int]] = []
    for line in typedefs.splitlines():
        found = TYPEDEF_ROW.match(line.strip())
        if found:
            rows.append((
                int(found.group(1)), found.group(2).replace("/", "+"),
                int(found.group(4)), int(found.group(5), 16),
            ))
    rows.sort()

    method_rows: dict[int, str] = {}
    for line in methods.splitlines():
        found = METHOD_ROW.match(line.strip())
        if found:
            method_rows[int(found.group(1))] = found.group(2)

    checks += 1
    if not rows or not method_rows:
        failures.append("monodis produced no type or method table")
        return checks, failures, True

    by_name = {name: (index, mlist, flags) for index, name, mlist, flags in rows}
    starts = sorted(item[2] for item in rows)

    for record in records:
        name = record["type"]
        checks += 1
        entry = by_name.get(name)
        if entry is None:
            failures.append(f"monodis does not declare {name}")
            continue
        _index, mlist, flags = entry

        checks += 1
        is_interface = bool(flags & TYPE_INTERFACE)
        if is_interface != (record["kind"] == "interface"):
            failures.append(
                f"{name}: monodis says interface={is_interface}, the "
                f"extraction says kind={record['kind']}")
        checks += 1
        if bool(flags & TYPE_SEALED) != record["sealed"]:
            failures.append(
                f"{name}: monodis says sealed={bool(flags & TYPE_SEALED)}, "
                f"the extraction says {record['sealed']}")
        checks += 1
        if bool(flags & TYPE_ABSTRACT) != record["abstract"]:
            failures.append(
                f"{name}: monodis says abstract={bool(flags & TYPE_ABSTRACT)}, "
                f"the extraction says {record['abstract']}")

        following = [item for item in starts if item > mlist]
        end = following[0] if following else max(method_rows) + 1
        observed: set[str] = set()
        for row in range(mlist, end):
            signature = method_rows.get(row)
            if signature is None:
                continue
            method_name = monodis_method_name(signature)
            if method_name is not None:
                observed.add(method_name)

        expected: set[str] = set()
        for member in record["members"]:
            if member["kind"] in ("method", "constructor"):
                expected.add(member["name"])
            elif member["kind"] == "property":
                if member.get("getAccess"):
                    expected.add(f"get_{member['name']}")
                if member.get("setAccess"):
                    expected.add(f"set_{member['name']}")
            elif member["kind"] == "event":
                expected.add(f"add_{member['name']}")
                expected.add(f"remove_{member['name']}")

        checks += 1
        unseen = sorted(expected - observed)
        if unseen:
            failures.append(
                f"{name}: the extraction claims members monodis's method table "
                f"does not carry: {unseen}")
    return checks, failures, True


# ----------------------------------------------------------------------------
# Driver
# ----------------------------------------------------------------------------


# ----------------------------------------------------------------------------
# Identity and provenance
# ----------------------------------------------------------------------------


def verify_identity(
    name: str, entry: dict[str, Any], path: Path, il_cache: Path | None,
) -> tuple[int, list[str], str | None, dict[str, str], str, str]:
    """Check one candidate binary against its registered identity.

    Returns the number of independent checks made, the failures, the
    disassembly (None when the digest already disqualified the file, in which
    case nothing further is read from it), the PE version resource, the
    recomputed strong-name token and the digest.

    This is the same code path the negative controls use, so a check that has
    stopped being load-bearing shows up as a control that no longer rejects.
    """
    failures: list[str] = []
    checks = 0
    data = path.read_bytes()

    checks += 1
    digest = hashlib.sha256(data).hexdigest()
    if digest != entry["sha256"]:
        failures.append(
            f"{name}: sha256 {digest} does not match the registered "
            f"{entry['sha256']}")

    checks += 1
    if len(data) != entry["fileSize"]:
        failures.append(
            f"{name}: size {len(data)} != registered {entry['fileSize']}")

    il = disassemble(path, il_cache, digest)

    checks += 1
    # **The assembly's OWN declaration, not the first `.assembly` line.**
    # `.assembly extern X` rows come first in every assembly that references
    # another, so making `extern` optional and taking the first match reads a
    # DEPENDENCY's name. It happened to be right for mscorlib, which references
    # nothing, and wrong for System.dll -- which is how it was found.
    declared = re.search(r"^\.assembly\s+(?!extern\b)([\w.]+)\s*$", il, re.M)
    assembly_name = declared.group(1) if declared else None
    if assembly_name != entry["assemblyName"]:
        failures.append(
            f"{name}: declares assembly {assembly_name!r}, registered "
            f"{entry['assemblyName']!r}")

    checks += 1
    version = re.search(r"^\s*\.ver\s+(\d+):(\d+):(\d+):(\d+)\s*$", il, re.M)
    version_text = ".".join(version.groups()) if version else None
    if version_text != entry["assemblyVersion"]:
        failures.append(
            f"{name}: assembly version {version_text}, registered "
            f"{entry['assemblyVersion']}")

    checks += 1
    key = re.search(r"\.publickey\s*=\s*\(([0-9A-Fa-f\s]*?)\)", il, re.S)
    key_hex = re.sub(r"\s+", "", key.group(1)).lower() if key else ""
    if key_hex != entry["publicKey"]:
        failures.append(
            f"{name}: public key {key_hex!r} != registered "
            f"{entry['publicKey']!r}")

    checks += 1
    token = public_key_token(key_hex) if key_hex else ""
    if token != entry["publicKeyToken"]:
        failures.append(
            f"{name}: public key token {token!r} recomputed from the "
            f"assembly's own key != registered {entry['publicKeyToken']!r}")

    checks += 1
    module = re.search(r"^\.module\s+(\S+)\s*$", il, re.M)
    module_name = module.group(1) if module else None
    if module_name != entry["moduleName"]:
        failures.append(
            f"{name}: module {module_name!r} != registered "
            f"{entry['moduleName']!r}")

    resource = version_resource(data)
    for field, expected in entry["versionResource"].items():
        checks += 1
        if resource.get(field) != expected:
            failures.append(
                f"{name}: version resource {field}={resource.get(field)!r} "
                f"!= registered {expected!r}")

    checks += 1
    if entry["strongNameKeyFile"].encode("ascii") not in data:
        failures.append(
            f"{name}: the registered strong-name key file "
            f"{entry['strongNameKeyFile']!r} is not present in the binary")

    for module_import in entry["requiredModuleImports"]:
        checks += 1
        if not re.search(
            rf"^\.module\s+extern\s+{re.escape(module_import)}\s*$", il, re.M,
        ):
            failures.append(
                f"{name}: required module import {module_import} is absent")

    for marker in entry["forbiddenProvenanceMarkers"]:
        checks += 1
        if marker in il:
            failures.append(
                f"{name}: reimplementation marker {marker!r} is present")

    return checks, failures, il, resource, token, digest


def negative_controls(
    authorities: dict[str, dict[str, Any]],
    controls: list[str],
    il_cache: Path | None,
) -> tuple[int, list[str], list[dict[str, Any]]]:
    """Prove the identity and provenance checks actually reject.

    A gate that admits everything would pass every positive test in this file.
    Each control names a binary that must NOT be admitted, and it is run
    through exactly the checks a real admission runs through; a control that
    produces no failure is itself a failure.
    """
    failures: list[str] = []
    checks = 0
    results: list[dict[str, Any]] = []
    for control in controls:
        name, _, raw = control.partition("=")
        entry = authorities.get(name)
        if entry is None or not raw:
            failures.append(f"negative control {control!r} names no authority")
            checks += 1
            continue
        path = Path(raw).expanduser()
        if not path.is_file():
            failures.append(
                f"negative control offered as {name} is not a file")
            checks += 1
            continue
        # Deliberately NOT cached: a control is read once and its
        # disassembly is tens of megabytes. Caching it would trade real disk
        # wear for a saving no later run collects.
        made, rejected, _il, _resource, _token, _digest = verify_identity(
            name, entry, path, None)
        checks += 1
        if not rejected:
            failures.append(
                f"negative control {path.name} was NOT rejected as {name}")
        # Only the file NAME and the digest are retained, never the path. A
        # generated report is committed, and a machine-local path in it is a
        # developer-path leak the package qualification refuses.
        results.append({
            "control": path.name,
            "controlSha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            "offeredAs": name,
            "checksRun": made,
            "rejectedBy": len(rejected),
            "reasons": rejected,
        })
    return checks, failures, results


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--assembly", action="append", default=[], metavar="NAME=PATH",
        help="a registered BCL assembly and the binary to admit for it")
    parser.add_argument("--il-cache", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--write-manifest", action="store_true")
    parser.add_argument("--cross-check", action="store_true")
    parser.add_argument(
        "--negative-control", action="append", default=[], metavar="NAME=PATH",
        help="a binary that must NOT be admitted as NAME; proves the "
             "identity and provenance checks still reject")
    args = parser.parse_args()

    registry = json.loads(REGISTRY.read_text(encoding="utf-8"))
    authorities = {item["assembly"]: item for item in registry["authorities"]}

    supplied: dict[str, Path] = {}
    for entry in args.assembly:
        name, _, raw = entry.partition("=")
        if not raw:
            raise SystemExit(f"--assembly expects NAME=PATH, got {entry!r}")
        supplied[name] = Path(raw).expanduser()

    identity_failures: list[str] = []
    identity_checks = 0
    admitted: list[dict[str, Any]] = []
    manifest_records: list[dict[str, Any]] = []
    manifest_resources: list[dict[str, Any]] = []
    manifest_literals: list[dict[str, Any]] = []
    literal_check_count = 0
    manifest_tables: list[dict[str, Any]] = []
    resource_failures: list[str] = []
    resource_check_count = 0
    table_check_count = 0
    cross_check_checks = 0
    cross_check_failures: list[str] = []
    cross_check_ran = False

    for name, entry in authorities.items():
        path = supplied.get(name)
        if path is None:
            identity_failures.append(f"{name}: no binary supplied")
            identity_checks += 1
            continue
        if not path.is_file():
            identity_failures.append(f"{name}: the supplied binary is not a file")
            identity_checks += 1
            continue

        identity_made, identity_issues, il, resource, token, digest = (
            verify_identity(name, entry, path, args.il_cache))
        identity_checks += identity_made
        identity_failures.extend(identity_issues)
        if identity_issues:
            continue

        selected = [item["type"] for item in entry["selectedFamilies"]]
        records, missing = extract(name, il, selected)
        for absent in missing:
            identity_checks += 1
            identity_failures.append(
                f"{name}: selected family {absent} is not declared")
        manifest_records.extend(records)

        # The default messages these families report are resource lookups, not
        # IL literals, so they are read out of the assembly's own embedded
        # string table by this file's own PE walk -- no external tool, and
        # nothing written to disk.
        selected_resources = entry.get("selectedResourceStrings", [])
        if selected_resources:
            blob = embedded_resource(path.read_bytes(), "resources")
            resource_checks_made = 0
            if blob is None:
                resource_failures.append(
                    f"{name}: no embedded ResourceReader string table")
                resource_checks_made = 1
            else:
                strings = resource_strings(blob)
                resource_checks_made, issues = resource_checks(
                    strings, selected_resources)
                resource_failures.extend(f"{name}: {item}" for item in issues)
                manifest_resources.extend(
                    {
                        "assembly": name,
                        "key": item["key"],
                        "value": strings.get(item["key"]),
                    }
                    for item in selected_resources
                )
            resource_check_count += resource_checks_made

        # Static data tables the projection reproduces, followed through the
        # `.cctor` -> initializer field -> `.data` blob chain.
        selected_tables = entry.get("selectedStaticTables", [])
        if selected_tables:
            made, issues, table_records = static_table_checks(il, selected_tables)
            table_check_count += made
            resource_failures.extend(f"{name}: {item}" for item in issues)
            manifest_tables.extend(
                dict(record, assembly=name) for record in table_records)

        # IL string literals the projection reproduces, each required to be
        # the whole body of the method that returns it.
        selected_literals = entry.get("selectedIlLiterals", [])
        if selected_literals:
            made, issues, literal_records = il_literal_checks(il, selected_literals)
            literal_check_count += made
            resource_failures.extend(f"{name}: {item}" for item in issues)
            manifest_literals.extend(
                dict(record, assembly=name) for record in literal_records)

        admitted.append({
            "assembly": name,
            "assemblyName": entry["assemblyName"],
            "assemblyVersion": entry["assemblyVersion"],
            "publicKeyToken": entry["publicKeyToken"],
            "publicKeyTokenRecomputed": token,
            "sha256": digest,
            "fileVersion": resource.get("FileVersion"),
            "selectedTypes": len(records),
            "selectedMembers": sum(len(item["members"]) for item in records),
        })

        if args.cross_check:
            checks, failures, ran = cross_check(path, records)
            cross_check_checks += checks
            cross_check_failures.extend(f"{name}: {item}" for item in failures)
            cross_check_ran = cross_check_ran or ran

    manifest_records.sort(key=lambda item: (item["assembly"], item["type"]))
    document = {
        "schemaVersion": 1,
        "profile": registry["profile"],
        "note": (
            "The selected BCL shape this binding's support classes are derived "
            "from, reconstructed mechanically from the hash-registered "
            "Microsoft binaries named in bcl-authorities.json. It records CLR "
            "facts only -- no Swift spelling, no mapping rule and no "
            "machine-local path -- so it is a function of those binaries "
            "alone and regenerates byte-identically. No Microsoft binary is "
            "stored in the repository."),
        "types": manifest_records,
        "resourceStrings": sorted(
            manifest_resources, key=lambda item: (item["assembly"], item["key"])),
        "ilLiterals": sorted(
            manifest_literals, key=lambda item: (item["assembly"], item["type"], item["member"])),
        "staticTables": sorted(
            manifest_tables,
            key=lambda item: (item["assembly"], item["type"], item["field"])),
    }

    if args.write_manifest:
        MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        MANIFEST.write_text(
            json.dumps(document, indent=2, sort_keys=False) + "\n",
            encoding="utf-8")

    manifest_failures: list[str] = []
    manifest_checks = 0
    if MANIFEST.exists():
        pinned = json.loads(MANIFEST.read_text(encoding="utf-8"))
        pinned_by_name = {
            (item["assembly"], item["type"]): item for item in pinned["types"]}
        found_by_name = {
            (item["assembly"], item["type"]): item for item in manifest_records}
        for key in sorted(set(pinned_by_name) | set(found_by_name)):
            manifest_checks += 1
            if key not in found_by_name:
                manifest_failures.append(
                    f"{key[1]}: pinned in the manifest but not extracted")
                continue
            if key not in pinned_by_name:
                manifest_failures.append(
                    f"{key[1]}: extracted but absent from the pinned manifest")
                continue
            for issue in compare_records(pinned_by_name[key], found_by_name[key]):
                manifest_failures.append(f"{key[1]}: {issue}")
        # The pinned resource strings are compared exactly as the shapes are:
        # a message the Swift support classes reproduce is part of the shape.
        pinned_resources = {
            (item["assembly"], item["key"]): item.get("value")
            for item in pinned.get("resourceStrings", [])
        }
        found_resources = {
            (item["assembly"], item["key"]): item.get("value")
            for item in manifest_resources
        }
        for key in sorted(set(pinned_resources) | set(found_resources)):
            manifest_checks += 1
            if key not in found_resources:
                manifest_failures.append(
                    f"{key[1]}: pinned resource string not extracted")
            elif key not in pinned_resources:
                manifest_failures.append(
                    f"{key[1]}: extracted resource string absent from the "
                    "pinned manifest")
            elif pinned_resources[key] != found_resources[key]:
                manifest_failures.append(
                    f"{key[1]}: resource string {found_resources[key]!r} != "
                    f"pinned {pinned_resources[key]!r}")
        pinned_literals = {
            (item["assembly"], item["type"], item["member"]): item.get("value")
            for item in pinned.get("ilLiterals", [])
        }
        found_literals = {
            (item["assembly"], item["type"], item["member"]): item.get("value")
            for item in manifest_literals
        }
        for key in sorted(set(pinned_literals) | set(found_literals)):
            manifest_checks += 1
            if key not in found_literals:
                manifest_failures.append(
                    f"{key[1]}::{key[2]}: pinned IL literal not extracted")
            elif key not in pinned_literals:
                manifest_failures.append(
                    f"{key[1]}::{key[2]}: extracted IL literal absent from the "
                    "pinned manifest")
            elif pinned_literals[key] != found_literals[key]:
                manifest_failures.append(
                    f"{key[1]}::{key[2]}: IL literal {found_literals[key]!r} != "
                    f"pinned {pinned_literals[key]!r}")
        pinned_tables = {
            (item["assembly"], item["type"], item["field"]): item.get("values")
            for item in pinned.get("staticTables", [])
        }
        found_tables = {
            (item["assembly"], item["type"], item["field"]): item.get("values")
            for item in manifest_tables
        }
        for key in sorted(set(pinned_tables) | set(found_tables)):
            manifest_checks += 1
            if key not in found_tables:
                manifest_failures.append(
                    f"{key[1]}::{key[2]}: pinned static table not extracted")
            elif key not in pinned_tables:
                manifest_failures.append(
                    f"{key[1]}::{key[2]}: extracted static table absent from "
                    "the pinned manifest")
            elif pinned_tables[key] != found_tables[key]:
                manifest_failures.append(
                    f"{key[1]}::{key[2]}: static table differs from the pinned "
                    f"one ({len(found_tables[key] or [])} extracted vs "
                    f"{len(pinned_tables[key] or [])} pinned)")
    elif not args.write_manifest:
        manifest_failures.append(f"{MANIFEST} does not exist")

    control_checks, control_failures, controls = negative_controls(
        authorities, args.negative_control, args.il_cache)

    by_type = {item["type"]: item for item in manifest_records}
    sentinel_count, sentinel_failures = sentinel_checks(by_type)
    mutation_count, mutation_failures = mutation_self_tests(manifest_records)

    failed = bool(
        identity_failures or manifest_failures or sentinel_failures or
        mutation_failures or cross_check_failures or control_failures or
        resource_failures)

    report = {
        "schemaVersion": 1,
        "registrySha256": hashlib.sha256(REGISTRY.read_bytes()).hexdigest(),
        "manifestSha256": (
            hashlib.sha256(MANIFEST.read_bytes()).hexdigest()
            if MANIFEST.exists() else None),
        "BCL_AUTHORITY_STATUS": "FAIL" if failed else "PASS",
        "BCL_AUTHORITY_ASSEMBLIES": len(admitted),
        "BCL_AUTHORITY_TYPES": len(manifest_records),
        "BCL_AUTHORITY_MEMBERS": sum(
            len(item["members"]) for item in manifest_records),
        "BCL_IDENTITY_CHECKS": identity_checks,
        "BCL_SENTINEL_CHECKS": sentinel_count,
        "BCL_MANIFEST_CHECKS": manifest_checks,
        "BCL_MUTATION_SELF_TESTS": mutation_count,
        "BCL_MUTATION_SURVIVORS": len(mutation_failures),
        "BCL_CROSS_CHECKS": cross_check_checks,
        "BCL_RESOURCE_CHECKS": resource_check_count,
        "BCL_IL_LITERAL_CHECKS": literal_check_count,
        "BCL_STATIC_TABLE_CHECKS": table_check_count,
        "BCL_NEGATIVE_CONTROLS": control_checks,
        "BCL_CROSS_CHECK_TOOL": "monodis" if cross_check_ran else "not run",
        "identityFailures": identity_failures,
        "manifestFailures": manifest_failures,
        "sentinelFailures": sentinel_failures,
        "mutationFailures": mutation_failures,
        "crossCheckFailures": cross_check_failures,
        "resourceFailures": resource_failures,
        "controlFailures": control_failures,
        "negativeControls": controls,
        "assemblies": admitted,
    }
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(
            json.dumps(report, indent=2) + "\n", encoding="utf-8")

    for record in admitted:
        print(
            f"{record['assembly']:20s} v{record['assemblyVersion']} "
            f"token={record['publicKeyTokenRecomputed']} "
            f"types={record['selectedTypes']:2d} "
            f"members={record['selectedMembers']:4d}")
    print(
        f"BCL_AUTHORITY_ASSEMBLIES={report['BCL_AUTHORITY_ASSEMBLIES']} "
        f"BCL_AUTHORITY_TYPES={report['BCL_AUTHORITY_TYPES']} "
        f"BCL_AUTHORITY_MEMBERS={report['BCL_AUTHORITY_MEMBERS']}")
    print(
        f"BCL_IDENTITY_CHECKS={identity_checks} "
        f"BCL_SENTINEL_CHECKS={sentinel_count} "
        f"BCL_MANIFEST_CHECKS={manifest_checks} "
        f"BCL_MUTATION_SELF_TESTS={mutation_count} "
        f"BCL_MUTATION_SURVIVORS={len(mutation_failures)} "
        f"BCL_CROSS_CHECKS={cross_check_checks} "
        f"({report['BCL_CROSS_CHECK_TOOL']}) "
        f"BCL_RESOURCE_CHECKS={resource_check_count} "
        f"BCL_IL_LITERAL_CHECKS={literal_check_count} "
        f"BCL_STATIC_TABLE_CHECKS={table_check_count} "
        f"BCL_NEGATIVE_CONTROLS={control_checks}")
    for record in controls:
        print(
            f"  rejected {Path(record['control']).name} offered as "
            f"{record['offeredAs']}: {record['rejectedBy']}/"
            f"{record['checksRun']} checks failed")
    print(f"BCL_AUTHORITY_STATUS={report['BCL_AUTHORITY_STATUS']}")
    for line in (identity_failures + manifest_failures + sentinel_failures +
                 mutation_failures + cross_check_failures + resource_failures +
                 control_failures):
        print(f"  {line}", file=sys.stderr)
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
