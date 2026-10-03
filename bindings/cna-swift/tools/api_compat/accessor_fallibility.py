#!/usr/bin/env python3
"""Accessor-level fallibility inventory for the registered Microsoft XNA assemblies.

Swift property syntax cannot express a throwing setter, and a getter can only
throw through the `get throws` effect. Deciding which XNA property accessor may
be projected as ordinary Swift property syntax therefore needs a per-*accessor*
answer to one question: does the authoritative implementation have a
contract-relevant failure path?

The answer is derived mechanically from the CIL of the hash-registered
assemblies, and only from it:

* An accessor is **fallible** when a `throw` is reachable from it through calls
  that stay inside the seven registered assemblies.
* Reachability is computed on an arity-sensitive call graph, so one throwing
  overload does not condemn its siblings, and a `callvirt` onto a virtual or
  abstract method also reaches the overrides declared by registered subtypes.
* An abstract or interface accessor declares no body. It is fallible exactly
  when one of its registered implementors is, because a Swift protocol
  requirement must be able to witness them all.
* Calls that leave the registered set are **not** failure evidence. Allocation,
  `System.String.Format`, boxing and every other universal CLR failure such as
  `OutOfMemoryException` are excluded by construction: they are not XNA
  contract behaviour, and a binding is not expected to preserve them.

Every fallible verdict carries the shortest call chain that reaches the throw
and the exception types constructed there, so the verdict can be re-derived by
hand from the same IL. The emitted inventory records the SHA-256 of the
assembly each entry came from and contains no machine-local path.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

TOOL = Path(__file__).resolve().parent
REFERENCE = TOOL / "reference" / "xna40-windows-runtime-contract.json"
REGISTRY = TOOL / "registered-assemblies.json"

EVIDENCE_DIRECT_THROW = "IL_DIRECT_THROW"
EVIDENCE_REACHABLE_THROW = "IL_REACHABLE_THROW"
EVIDENCE_INDEX_OPERATION = "IL_UNGUARDED_INDEX_OPERATION"
EVIDENCE_ABSTRACT = "IL_ABSTRACT_DECLARATION"
EVIDENCE_NONE = "IL_NO_FAILURE_PATH"
EVIDENCE_ABSENT = "ACCESSOR_ABSENT"
EVIDENCE_UNRESOLVED = "IL_ACCESSOR_NOT_FOUND"

INSTRUCTION = re.compile(r"^\s*IL_[0-9a-fA-F]+:\s*(\S+)(.*)$")
MEMBER_REFERENCE = re.compile(r"([^\s(]+::[^\s(]+)")
# A CLR explicit interface implementation names its interface method in an
# `.override` directive, in one of two spellings:
#
#     .override Microsoft.Xna.Framework.Graphics.IEffectLights::set_LightingEnabled
#     .override method instance void [asm]Some.IFoo`1<!0>::Bar(int32)
#
# The interface member's arity is the OVERRIDING method's own arity -- an
# override has the interface method's signature -- so the directive is only
# read for the pair of names and the signature is never parsed.
OVERRIDE_DIRECTIVE = re.compile(r"^\s*\.override\s+(.+?)\s*$")
ASSEMBLY_REFERENCE = re.compile(r"^\[[^\]]*\]")


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


def mask(line: str) -> str:
    """Blank string-literal and quoted-identifier contents, preserving length.

    Both carry braces in this IL: `ldstr "{{X:{0}}}"`, and the C++/CLI global
    name `'ThreadLock.{dtor}'`. Counting their braces desynchronises the
    nesting depth and silently truncates the whole parse. Masking keeps
    offsets, so a comment position found here also indexes the original line.
    """
    out: list[str] = []
    quote: str | None = None
    index = 0
    size = len(line)
    while index < size:
        char = line[index]
        if quote is not None:
            if char == "\\" and index + 1 < size:
                out.append("__")
                index += 2
                continue
            out.append(char if char == quote else "_")
            if char == quote:
                quote = None
            index += 1
            continue
        if char in "\"'":
            quote = char
        out.append(char)
        index += 1
    return "".join(out)


def structural(line: str) -> str:
    """The line reduced to what may carry IL structure."""
    masked = mask(line)
    comment = masked.find("//")
    return masked if comment < 0 else masked[:comment]


def uncommented(line: str) -> str:
    """The original line with any trailing IL comment removed."""
    comment = mask(line).find("//")
    return line if comment < 0 else line[:comment]


def instructions(body: list[str]) -> list[tuple[str, str]]:
    """(opcode, operand) per instruction, rejoining wrapped operands.

    ikdasm wraps a long member reference across lines at its argument commas:

        IL_000e:  call  void [A]N.Helpers::CheckDisposed(object,
        native int)

    Reading only the first line leaves an unbalanced operand, so the call is
    silently invisible to the graph. Continuation lines are consumed while the
    operand's brackets are still open, which stops exactly at the end of the
    reference and never swallows the next instruction or an EH clause.
    """
    result: list[tuple[str, str]] = []
    index = 0
    while index < len(body):
        match = INSTRUCTION.match(uncommented(body[index]))
        index += 1
        if not match:
            continue
        operand = match.group(2)
        while index < len(body) and unbalanced(operand):
            operand += " " + uncommented(body[index]).strip()
            index += 1
        result.append((match.group(1), operand))
    return result


def unbalanced(text: str) -> bool:
    masked = structural(text)
    return masked.count("(") > masked.count(")")


def strip_assembly_refs(text: str) -> str:
    return re.sub(r"\[[^\]\[]+\]", "", text)


def split_top_level(text: str) -> list[str]:
    """Split a parameter list on commas that are not nested."""
    parts: list[str] = []
    depth = 0
    current: list[str] = []
    for char in text:
        if char in "([<{":
            depth += 1
        elif char in ")]>}":
            depth -= 1
        if char == "," and depth == 0:
            parts.append("".join(current))
            current = []
            continue
        current.append(char)
    tail = "".join(current)
    if tail.strip() or parts:
        parts.append(tail)
    return [part for part in parts if part.strip()]


def balanced_argument_list(text: str, open_index: int) -> str | None:
    depth = 0
    for index in range(open_index, len(text)):
        char = text[index]
        if char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
            if depth == 0:
                return text[open_index + 1:index]
    return None


def override_targets(
    body: list[str], arity: int,
) -> tuple[tuple[str, str, int], ...]:
    """The interface methods a body's `.override` directives name.

    Returned as call-graph keys, so an explicit implementation can be looked up
    by the interface member it implements rather than by its own IL name -- an
    explicit implementation is named
    `Microsoft.Xna.Framework.Graphics.IEffectLights.set_LightingEnabled`, which
    matches no interface member by name and is why the two effects that refuse
    to disable lighting were invisible to the implementor search.
    """
    targets: list[tuple[str, str, int]] = []
    for line in body:
        match = OVERRIDE_DIRECTIVE.match(uncommented(line))
        if match is None:
            continue
        text = match.group(1)
        head = text.split("(", 1)[0]
        if "::" not in head:
            continue
        owner, name = head.rsplit("::", 1)
        # The long spelling prefixes the return type and calling convention,
        # and either spelling may carry an assembly reference or generic
        # arguments on the owner.
        owner = owner.split()[-1].split("<", 1)[0]
        owner = ASSEMBLY_REFERENCE.sub("", owner)
        name = name.strip()
        if owner and name:
            targets.append((owner, name, arity))
    return tuple(targets)


class Method:
    __slots__ = ("owner", "name", "arity", "header", "body", "static",
                 "virtual", "abstract", "has_body", "overrides")

    def __init__(self, owner: str, name: str, arity: int, header: str,
                 body: list[str]) -> None:
        self.owner = owner
        self.name = name
        self.arity = arity
        self.header = header
        self.body = body
        prefix = header.split("(")[0]
        self.static = " static " in f" {prefix} "
        self.virtual = "virtual" in prefix
        self.abstract = "abstract" in prefix
        self.has_body = not self.abstract and bool(
            [line for line in body if INSTRUCTION.match(uncommented(line))])
        self.overrides = override_targets(body, arity)

    @property
    def key(self) -> tuple[str, str, int]:
        return (self.owner, self.name, self.arity)


class Assembly:
    """Types, methods, properties and method bodies of one IL dump."""

    def __init__(self, name: str, digest: str, text: str) -> None:
        self.name = name
        self.sha256 = digest
        self.methods: dict[tuple[str, str, int], list[Method]] = {}
        self.properties: dict[str, list[dict[str, Any]]] = {}
        self.bases: dict[str, list[str]] = {}
        self.types: set[str] = set()
        self._parse(text)

    def _parse(self, text: str) -> None:
        lines = text.splitlines()
        total = len(lines)
        stack: list[tuple[str, int]] = []
        depth = 0
        index = 0
        while index < total:
            clean = structural(lines[index])
            stripped = clean.strip()
            if stripped.startswith(".class ") and not stripped.startswith(".class extern"):
                header, end = self._header(lines, index)
                name, nested, supers = self._class_shape(header)
                depth += 1
                owner = f"{stack[-1][0]}/{name}" if nested and stack else name
                stack.append((owner, depth))
                self.types.add(owner)
                self.bases[owner] = supers
                index = end + 1
                continue
            if stripped.startswith(".method "):
                header, end = self._header(lines, index)
                body, index = self._block(lines, end + 1)
                self._record_method(stack[-1][0] if stack else "<global>", header, body)
                continue
            if stripped.startswith(".property "):
                header, end = self._header(lines, index)
                body, index = self._block(lines, end + 1)
                self._record_property(stack[-1][0] if stack else "<global>", header, body)
                continue
            depth += clean.count("{")
            for _ in range(clean.count("}")):
                if stack and stack[-1][1] == depth:
                    stack.pop()
                depth -= 1
            index += 1

    @staticmethod
    def _header(lines: list[str], start: int) -> tuple[str, int]:
        """Join a declaration's continuation lines up to its opening brace.

        Quoted identifiers stay verbatim -- accessor and method names need
        them -- while the brace is located in the masked form, so a brace
        inside a name does not end the header early.
        """
        parts: list[str] = []
        index = start
        while index < len(lines):
            line = uncommented(lines[index])
            brace = structural(lines[index]).find("{")
            parts.append((line[:brace] if brace >= 0 else line).strip())
            if brace >= 0:
                break
            index += 1
        return " ".join(part for part in parts if part), index

    @staticmethod
    def _block(lines: list[str], start: int) -> tuple[list[str], int]:
        depth = 1
        index = start
        body: list[str] = []
        while index < len(lines) and depth > 0:
            clean = structural(lines[index])
            depth += clean.count("{") - clean.count("}")
            if depth > 0:
                body.append(lines[index])
            index += 1
        return body, index

    @staticmethod
    def _class_shape(header: str) -> tuple[str, bool, list[str]]:
        text = header[len(".class"):].strip()
        parts = re.split(r"\bextends\b|\bimplements\b", text)
        declaration = parts[0]
        # A constrained generic parameter list contains spaces, so cut at the
        # first parameter bracket before taking the name.
        name = re.sub(r"<.*$", "", declaration).split()[-1]
        supers: list[str] = []
        for item in parts[1:]:
            for entry in split_top_level(item):
                cleaned = strip_assembly_refs(entry).strip()
                cleaned = re.sub(r"^class\s+", "", cleaned)
                cleaned = re.sub(r"<.*$", "", cleaned).strip()
                if cleaned:
                    supers.append(cleaned)
        return name, "nested" in declaration.split(), supers

    def _record_method(self, owner: str, header: str, body: list[str]) -> None:
        name, arity = method_identity(header)
        if name is None:
            return
        method = Method(owner, name, arity, header, body)
        self.methods.setdefault(method.key, []).append(method)

    def _record_property(self, owner: str, header: str, body: list[str]) -> None:
        match = re.search(r"([^\s(]+)\s*\(", header[len(".property"):])
        if not match:
            return
        record: dict[str, Any] = {
            "name": match.group(1).strip("'"), "get": None, "set": None,
        }
        for line in body:
            clean = uncommented(line).strip()
            for accessor in ("get", "set"):
                if clean.startswith(f".{accessor} "):
                    target = re.search(r"::([^\s(]+)\s*\(", clean)
                    if target:
                        record[accessor] = target.group(1).strip("'")
        self.properties.setdefault(owner, []).append(record)


def method_identity(header: str) -> tuple[str | None, int]:
    """Extract a method's declared name and parameter count from its header.

    A return type or parameter may itself contain parentheses -- `modopt(...)`,
    `marshal(...)` and `pinvokeimpl(...)` all do -- so the parameter list is
    the balanced group whose closing bracket is followed only by the
    implementation flags that end every `.method` declaration.
    """
    text = header.strip()
    if text.startswith(".method"):
        text = text[len(".method"):].strip()
    for open_index in range(len(text) - 1, -1, -1):
        if text[open_index] != "(":
            continue
        arguments = balanced_argument_list(text, open_index)
        if arguments is None:
            continue
        tail = text[open_index + len(arguments) + 2:].strip().rstrip("{").strip()
        if tail and not re.fullmatch(r"[A-Za-z][A-Za-z ]*", tail):
            continue
        head = text[:open_index].strip()
        if not head:
            continue
        # A generic parameter list may carry constraints, and a constraint
        # contains spaces: `DrawUserIndexedPrimitives<valuetype .ctor
        # (IVertexType) T>`. Cutting at the first `<` of the *last token* then
        # names the method `T>`, merging every such method on a type into one
        # bucket and losing the call edges its properly spelled call sites
        # carry. The whole balanced group has to go first.
        tokens = strip_generic_arguments(head).split()
        if not tokens:
            continue
        name = tokens[-1].strip("'")
        return (name or None), len(split_top_level(arguments))
    return None, 0


def strip_generic_arguments(text: str) -> str:
    """Remove balanced `<...>` groups, which may contain spaces and nesting."""
    out: list[str] = []
    depth = 0
    for char in text:
        if char == "<":
            depth += 1
            continue
        if char == ">":
            depth = max(0, depth - 1)
            continue
        if depth == 0:
            out.append(char)
    return "".join(out)


def call_reference(operand: str) -> tuple[str, str, int] | None:
    """Resolve a call/newobj/ldftn operand to (type, method, arity).

    The operand carries a full signature, so the owner has to be read from the
    right: a generic instantiation such as
    `List`1<valuetype N.SurfaceFormat>::Add` contains both spaces and a second
    dotted name that a left-to-right match would mistake for the owner. A
    module-level C++/CLI function has no owner at all and resolves to
    `<global>`.
    """
    text = uncommented(operand)
    open_index = text.find("(")
    while open_index >= 0:
        arguments = balanced_argument_list(text, open_index)
        if arguments is not None:
            head = strip_assembly_refs(text[:open_index]).strip()
            marker = head.rfind("::")
            if marker >= 0:
                owner = strip_generic_arguments(head[:marker]).strip()
                owner = re.sub(r"[(),*&\[\]]", " ", owner).split()
                name = strip_generic_arguments(head[marker + 2:]).strip().strip("'")
                if owner and name:
                    return owner[-1], name, len(split_top_level(arguments))
            else:
                tokens = strip_generic_arguments(head).split()
                if tokens:
                    name = tokens[-1].strip("'")
                    if not name.endswith("*") and "::" not in name:
                        return "<global>", name, len(split_top_level(arguments))
            return None
        open_index = text.find("(", open_index + 1)
    return None


# CLR index operations that validate a caller-supplied index. Reaching one
# with an unguarded index argument propagates its range check, which is
# documented, deterministic and caller-triggerable -- unlike an allocation
# failure. Only these members appear as index operations anywhere a contract
# accessor can reach.
INDEX_OPERATIONS = frozenset({
    "System.Collections.Generic.List`1::get_Item",
    "System.Collections.Generic.List`1::set_Item",
    "System.Collections.Generic.List`1::RemoveAt",
    "System.Collections.Generic.List`1::Insert",
    "System.Collections.Generic.Dictionary`2::get_Item",
    "System.Collections.ObjectModel.ReadOnlyCollection`1::get_Item",
    "System.Collections.Generic.IList`1::get_Item",
    "System.Collections.Generic.IList`1::set_Item",
    "System.String::get_Chars",
})

INDEX_EXCEPTIONS = {
    "System.Collections.Generic.Dictionary`1::get_Item": "System.Collections.Generic.KeyNotFoundException",
    "System.Collections.Generic.Dictionary`2::get_Item": "System.Collections.Generic.KeyNotFoundException",
    "System.Collections.Generic.List`1::get_Item": "System.ArgumentOutOfRangeException",
    "System.Collections.Generic.List`1::set_Item": "System.ArgumentOutOfRangeException",
    "System.Collections.Generic.List`1::RemoveAt": "System.ArgumentOutOfRangeException",
    "System.Collections.Generic.List`1::Insert": "System.ArgumentOutOfRangeException",
    "System.Collections.ObjectModel.ReadOnlyCollection`1::get_Item": "System.ArgumentOutOfRangeException",
    "System.Collections.Generic.IList`1::get_Item": "System.ArgumentOutOfRangeException",
    "System.Collections.Generic.IList`1::set_Item": "System.ArgumentOutOfRangeException",
    "System.String::get_Chars": "System.IndexOutOfRangeException",
}

BRANCH_OPCODES = (
    "br", "brtrue", "brfalse", "brnull", "brzero", "brinst",
    "beq", "bne.un", "bge", "bgt", "ble", "blt", "switch",
)

ARGUMENT_LOAD = re.compile(r"^ldarg(?:\.(\d+)|\.s|)$")


def is_branch(opcode: str) -> bool:
    base = opcode[:-2] if opcode.endswith(".s") else opcode
    return base in BRANCH_OPCODES or base.startswith("bge.") or base.startswith(
        "bgt.") or base.startswith("ble.") or base.startswith("blt.")


def argument_index(opcode: str, operand: str) -> int | None:
    if opcode.startswith("ldarg.") and opcode[6:].isdigit():
        return int(opcode[6:])
    if opcode in ("ldarg", "ldarg.s"):
        text = operand.strip().strip("'")
        return int(text) if text.isdigit() else -1
    return None


def unguarded_index_operation(
    decoded: list[tuple[str, str]], index_arguments: set[int],
) -> str | None:
    """Name the index operation an unguarded caller index reaches, if any.

    The index argument must be the instruction immediately before the index
    operation -- that is how every such accessor in the registered set is
    compiled -- and no conditional branch may precede it, because a preceding
    branch is exactly the range check that makes XNA return null instead of
    letting the operation throw. `EffectParameterCollection.Item` guards;
    `CurveKeyCollection.Item` does not.
    """
    for position, (opcode, operand) in enumerate(decoded):
        indexed = opcode.startswith("ldelem") or opcode.startswith("stelem")
        member: str | None = None
        if not indexed:
            if opcode not in ("call", "callvirt"):
                continue
            reference = call_reference(operand)
            if reference is None:
                continue
            member = f"{reference[0]}::{reference[1]}"
            if member not in INDEX_OPERATIONS:
                continue
        else:
            member = opcode
        if position == 0:
            continue
        previous, previous_operand = decoded[position - 1]
        slot = argument_index(previous, previous_operand)
        if slot is None or slot not in index_arguments:
            continue
        if any(is_branch(item) for item, _ in decoded[:position]):
            return None
        return member
    return None


class CallGraph:
    """Fallibility over the registered assemblies' own call graph."""

    def __init__(self, assemblies: list[Assembly]) -> None:
        self.methods: dict[tuple[str, str, int], list[Method]] = {}
        self.bases: dict[str, list[str]] = {}
        self.owner_of: dict[str, str] = {}
        for assembly in assemblies:
            for key, methods in assembly.methods.items():
                self.methods.setdefault(key, []).extend(methods)
            self.bases.update(assembly.bases)
            for name in assembly.types:
                self.owner_of.setdefault(name, assembly.name)
        self.subtypes: dict[str, set[str]] = collections.defaultdict(set)
        for name, supers in self.bases.items():
            for parent in supers:
                self.subtypes[parent].add(name)
        # Interface member -> the explicit implementations that override it.
        # Keyed by what the `.override` directive names, which is the only
        # place an explicit implementation records what it implements.
        self.explicit: dict[
            tuple[str, str, int], set[tuple[str, str, int]]] = collections.defaultdict(set)
        for key, methods in self.methods.items():
            for method in methods:
                for target in method.overrides:
                    self.explicit[target].add(key)
        self.direct: dict[tuple[str, str, int], list[str]] = {}
        self.edges: dict[tuple[str, str, int], set[tuple[str, str, int]]] = {}
        self._build()
        self.fallible, self.via = self._propagate()

    def _build(self) -> None:
        for key, methods in self.methods.items():
            throws: list[str] = []
            targets: set[tuple[str, str, int]] = set()
            for method in methods:
                if not method.has_body:
                    continue
                decoded = instructions(method.body)
                if any(opcode in ("throw", "rethrow") for opcode, _ in decoded):
                    throws.extend(exception_types(decoded) or ["rethrow"])
                for opcode, operand in decoded:
                    if opcode not in ("call", "callvirt", "ldftn", "ldvirtftn"):
                        continue
                    reference = call_reference(operand)
                    if reference is None or reference not in self.methods:
                        continue
                    targets.add(reference)
                    if opcode in ("callvirt", "ldvirtftn"):
                        targets |= self._overrides(reference)
            if throws:
                self.direct[key] = sorted(set(throws))
            self.edges[key] = targets

    def _overrides(self, reference: tuple[str, str, int]) -> set[tuple[str, str, int]]:
        owner, name, arity = reference
        declared = self.methods.get(reference, [])
        if not any(item.virtual or item.abstract for item in declared):
            return set()
        result: set[tuple[str, str, int]] = set()
        pending = list(self.subtypes.get(owner, ()))
        seen: set[str] = set()
        while pending:
            subtype = pending.pop()
            if subtype in seen:
                continue
            seen.add(subtype)
            candidate = (subtype, name, arity)
            if candidate in self.methods:
                result.add(candidate)
            pending.extend(self.subtypes.get(subtype, ()))
        # An explicit implementation carries the interface member's name
        # nowhere in its own IL name, so the walk above cannot see it. A
        # `callvirt` on the interface member still reaches it.
        return result | self.explicit.get(reference, set())

    def _propagate(self) -> tuple[set[tuple[str, str, int]], dict[Any, Any]]:
        """Reverse breadth-first search keeps the shortest chain to a throw."""
        reverse: dict[Any, set[Any]] = collections.defaultdict(set)
        for source, targets in self.edges.items():
            for target in targets:
                reverse[target].add(source)
        fallible = set(self.direct)
        via: dict[Any, Any] = {key: None for key in self.direct}
        queue = collections.deque(self.direct)
        while queue:
            current = queue.popleft()
            for caller in reverse.get(current, ()):
                if caller in fallible:
                    continue
                fallible.add(caller)
                via[caller] = current
                queue.append(caller)
        return fallible, via

    def chain(self, key: tuple[str, str, int]) -> list[tuple[str, str, int]]:
        result = [key]
        while self.via.get(key) is not None:
            key = self.via[key]
            result.append(key)
        return result

    def implementors(self, owner: str, name: str, arity: int) -> list[tuple[str, str, int]]:
        result: list[tuple[str, str, int]] = []
        pending = list(self.subtypes.get(owner, ()))
        seen: set[str] = set()
        while pending:
            subtype = pending.pop()
            if subtype in seen:
                continue
            seen.add(subtype)
            candidate = (subtype, name, arity)
            for method in self.methods.get(candidate, []):
                if method.has_body:
                    result.append(candidate)
                    break
            pending.extend(self.subtypes.get(subtype, ()))
        # Same blindness as `_overrides`, and the same repair: an implementor
        # that implements the member EXPLICITLY is named after the interface,
        # not after the member, so only the `.override` index finds it.
        for candidate in self.explicit.get((owner, name, arity), ()):
            if any(method.has_body for method in self.methods.get(candidate, [])):
                result.append(candidate)
        return sorted(set(result))


def overload_bounds(graph: CallGraph, assemblies: list[Assembly]) -> dict[str, Any]:
    """Per-overload lower and upper bounds on fallibility, and the overload sets.

    `graph` merges same-named, same-arity overloads, which is an upper bound on
    a signature-precise analysis. Two per-overload closures bracket the precise
    answer: the *upper* one propagates fallibility from any called overload,
    the *lower* one only when every overload of the called group is fallible.
    Wherever the two agree, the precise answer is known and the merge provably
    changed nothing.

    Returns `lower`, `upper`, `siblings` (group -> its overload keys) and
    `overloads` (overload key -> Method). An overload key is the group key with
    the declaration's ordinal appended.
    """
    per_overload: dict[tuple[str, str, int, int], Method] = {}
    for assembly in assemblies:
        for key, methods in assembly.methods.items():
            for ordinal, method in enumerate(methods):
                per_overload[(*key, ordinal)] = method
    siblings: dict[tuple[str, str, int], list[tuple[str, str, int, int]]] = \
        collections.defaultdict(list)
    for key in per_overload:
        siblings[key[:3]].append(key)

    direct: set[tuple[str, str, int, int]] = set()
    groups: dict[tuple[str, str, int, int], set[tuple[str, str, int]]] = \
        collections.defaultdict(set)
    for key, method in per_overload.items():
        if not method.has_body:
            continue
        decoded = instructions(method.body)
        if any(opcode in ("throw", "rethrow") for opcode, _ in decoded):
            direct.add(key)
        for opcode, operand in decoded:
            if opcode not in ("call", "callvirt", "ldftn", "ldvirtftn"):
                continue
            reference = call_reference(operand)
            if reference is None:
                continue
            if reference in siblings:
                groups[key].add(reference)
            if opcode in ("callvirt", "ldvirtftn"):
                for override in graph._overrides(reference):
                    if override in siblings:
                        groups[key].add(override)

    waiting: dict[tuple[str, str, int], set[tuple[str, str, int, int]]] = \
        collections.defaultdict(set)
    for key, targets in groups.items():
        for target in targets:
            waiting[target].add(key)
    lower = set(direct)
    queue = collections.deque(direct)
    while queue:
        current = queue.popleft()
        group = current[:3]
        if not all(item in lower for item in siblings[group]):
            continue
        for caller in waiting.get(group, ()):
            if caller in lower:
                continue
            lower.add(caller)
            queue.append(caller)

    upper = set(direct)
    queue = collections.deque(direct)
    while queue:
        current = queue.popleft()
        for caller in waiting.get(current[:3], ()):
            if caller in upper:
                continue
            upper.add(caller)
            queue.append(caller)
    return {"lower": lower, "upper": upper, "siblings": siblings,
            "overloads": per_overload}


def overload_merge_disagreements(
    graph: CallGraph, assemblies: list[Assembly], contract: dict[str, Any],
) -> list[str]:
    """The contract accessors where the two bounds disagree; there are none."""
    bounds = overload_bounds(graph, assemblies)
    lower, siblings = bounds["lower"], bounds["siblings"]
    disagreements: list[str] = []
    for source_type in contract["types"]:
        il_name = source_type["name"].replace("+", "/")
        for member in source_type["members"]:
            if member["kind"] != "property":
                continue
            index_arity = len(member.get("parameters", []))
            for accessor, arity in (("get", index_arity), ("set", index_arity + 1)):
                if not member.get(accessor):
                    continue
                key = (il_name, f"{accessor}_{member['name']}", arity)
                if key not in graph.methods:
                    continue
                upper = key in graph.fallible
                overloads = siblings[key]
                bottom = bool(overloads) and all(item in lower for item in overloads)
                if upper != bottom:
                    disagreements.append(f"{source_type['name']}.{member['name']} {accessor}")
    return disagreements


def exception_types(decoded: list[tuple[str, str]]) -> list[str]:
    names: list[str] = []
    for opcode, operand in decoded:
        if opcode != "newobj":
            continue
        reference = call_reference(operand)
        if reference and reference[0].endswith("Exception") and reference[0] not in names:
            names.append(reference[0])
    return names


def chain_part(part: str) -> str:
    """One `Owner::member/arity` chain entry, shortened for the table.

    The member alone is enough for an ordinary chain, where every entry is a
    differently-named method. It is NOT enough for an explicit interface
    implementation: its IL name is the interface member's fully-qualified name,
    so `EnvironmentMapEffect` and `SkinnedEffect` refusing to disable lighting
    render as the same string twice and the evidence reads as a loop. Those
    entries keep the owning type, which is the only thing that distinguishes
    them.
    """
    owner, _, member = part.rpartition("::")
    if "." not in member.split("/", 1)[0]:
        return member
    return f"{owner.rsplit('.', 1)[-1]}::{member.rsplit('.', 1)[-1]}"


def render(key: tuple[str, str, int]) -> str:
    owner, name, arity = key
    return f"{owner}::{name}/{arity}"


def classify(
    graph: CallGraph, owner: str, accessor: str, arity: int,
    index_count: int = 0,
) -> dict[str, Any]:
    key = (owner, accessor, arity)
    declared = graph.methods.get(key)
    if not declared:
        return {"fallible": False, "evidence": EVIDENCE_UNRESOLVED,
                "exceptions": [], "chain": [], "implementors": []}
    if index_count and any(method.has_body for method in declared):
        for method in declared:
            if not method.has_body:
                continue
            first = 0 if method.static else 1
            slots = set(range(first, first + index_count))
            operation = unguarded_index_operation(instructions(method.body), slots)
            if operation is not None:
                return {
                    "fallible": True,
                    "evidence": EVIDENCE_INDEX_OPERATION,
                    "exceptions": [INDEX_EXCEPTIONS.get(operation, "System.IndexOutOfRangeException")],
                    "chain": [render(key), operation],
                    "implementors": [],
                }
    if not any(method.has_body for method in declared):
        witnesses = graph.implementors(owner, accessor, arity)
        throwing = [item for item in witnesses if item in graph.fallible]
        return {
            "fallible": bool(throwing),
            "evidence": EVIDENCE_ABSTRACT,
            "exceptions": sorted({
                exception
                for item in throwing
                for exception in graph.direct.get(graph.chain(item)[-1], [])
            }),
            "chain": [render(item) for item in throwing],
            "implementors": [render(item) for item in witnesses],
        }
    if key not in graph.fallible:
        return {"fallible": False, "evidence": EVIDENCE_NONE,
                "exceptions": [], "chain": [], "implementors": []}
    chain = graph.chain(key)
    return {
        "fallible": True,
        "evidence": EVIDENCE_DIRECT_THROW if len(chain) == 1 else EVIDENCE_REACHABLE_THROW,
        "exceptions": graph.direct.get(chain[-1], []),
        "chain": [render(item) for item in chain],
        "implementors": [],
    }


def accessor_names(assemblies: list[Assembly], il_type: str, name: str) -> dict[str, str]:
    for assembly in assemblies:
        for record in assembly.properties.get(il_type, []):
            if record["name"] == name:
                return {key: record[key] for key in ("get", "set") if record[key]}
    return {}


def declaring_assembly(assemblies: list[Assembly], il_type: str) -> Assembly | None:
    for assembly in assemblies:
        if il_type in assembly.types:
            return assembly
    return None


def build(assemblies: list[Assembly], contract: dict[str, Any]) -> list[dict[str, Any]]:
    graph = CallGraph(assemblies)
    entries: list[dict[str, Any]] = []
    for source_type in contract["types"]:
        il_name = source_type["name"].replace("+", "/")
        assembly = declaring_assembly(assemblies, il_name)
        for member in source_type["members"]:
            if member["kind"] != "property":
                continue
            index_arity = len(member.get("parameters", []))
            names = accessor_names(assemblies, il_name, member["name"])
            entry: dict[str, Any] = {
                "type": source_type["name"],
                "property": member["name"],
                "indexed": bool(member.get("parameters")),
                "static": bool(member.get("static")),
                "propertyType": member["type"],
                "getDeclared": bool(member.get("get")),
                "setDeclared": bool(member.get("set")),
                "getAccess": member.get("getAccess"),
                "setAccess": member.get("setAccess"),
                "assembly": assembly.name if assembly else None,
                "assemblySha256": assembly.sha256 if assembly else None,
            }
            for accessor, kind, arity in (
                ("get", "getter", index_arity), ("set", "setter", index_arity + 1),
            ):
                if not member.get(accessor):
                    entry[kind] = {"fallible": False, "evidence": EVIDENCE_ABSENT,
                                   "exceptions": [], "chain": [], "implementors": [],
                                   "method": None}
                    continue
                method = names.get(accessor) or f"{accessor}_{member['name']}"
                verdict = classify(graph, il_name, method, arity, index_arity)
                verdict["method"] = method
                entry[kind] = compact(verdict)
            entries.append(entry)
    return entries, graph


def compact(verdict: dict[str, Any]) -> dict[str, Any]:
    """Drop the empty evidence fields; an infallible accessor has no chain."""
    return {
        key: value for key, value in verdict.items()
        if value or key in ("fallible", "evidence", "method")
    }


def self_test(graph: CallGraph, assemblies: list[Assembly]) -> tuple[int, list[str]]:
    """Prove the analysis is not vacuous, on facts read straight from the IL."""
    checks = 0
    failures: list[str] = []

    def expect(condition: bool, message: str) -> None:
        nonlocal checks
        checks += 1
        if not condition:
            failures.append(message)

    # --- infallible accessors of pure value types stay infallible -----------
    for owner, accessor, arity in (
        ("Microsoft.Xna.Framework.Vector3", "get_Zero", 0),
        ("Microsoft.Xna.Framework.Color", "get_R", 0),
        ("Microsoft.Xna.Framework.Rectangle", "get_Left", 0),
        ("Microsoft.Xna.Framework.Graphics.Viewport", "set_Width", 1),
        ("Microsoft.Xna.Framework.Graphics.PackedVector.Alpha8", "set_PackedValue", 1),
        ("Microsoft.Xna.Framework.Audio.AudioEmitter", "get_DopplerScale", 0),
        ("Microsoft.Xna.Framework.Audio.AudioEmitter", "set_Position", 1),
        ("Microsoft.Xna.Framework.Audio.AudioListener", "set_Up", 1),
        ("Microsoft.Xna.Framework.Graphics.BlendState", "get_AlphaBlendFunction", 0),
    ):
        expect((owner, accessor, arity) not in graph.fallible,
               f"{owner}.{accessor} must be infallible")

    # An internally indexed array read is not caller-triggerable: the index of
    # `BoundingFrustum.Left` is the constant 2, so the accessor cannot fail.
    expect(("Microsoft.Xna.Framework.BoundingFrustum", "get_Left", 0) not in graph.fallible,
           "BoundingFrustum.Left must be infallible despite indexing an array")

    # --- direct throws ------------------------------------------------------
    emitter = "Microsoft.Xna.Framework.Audio.AudioEmitter"
    expect((emitter, "set_DopplerScale", 1) in graph.direct,
           "AudioEmitter.set_DopplerScale must throw directly")
    expect(graph.direct.get((emitter, "set_DopplerScale", 1)) ==
           ["System.ArgumentOutOfRangeException"],
           "AudioEmitter.set_DopplerScale must construct ArgumentOutOfRangeException")

    # --- reachable throws ---------------------------------------------------
    blend = "Microsoft.Xna.Framework.Graphics.BlendState"
    expect((blend, "set_AlphaBlendFunction", 1) in graph.fallible,
           "BlendState.set_AlphaBlendFunction must reach ThrowIfBound")
    expect((blend, "set_AlphaBlendFunction", 1) not in graph.direct,
           "BlendState.set_AlphaBlendFunction must not throw directly")
    expect(len(graph.chain((blend, "set_AlphaBlendFunction", 1))) >= 2,
           "a reachable throw must record its chain")
    device = "Microsoft.Xna.Framework.Graphics.GraphicsDevice"
    expect((device, "get_Viewport", 0) in graph.fallible,
           "GraphicsDevice.get_Viewport must reach Helpers.CheckDisposed")
    enumerator = "Microsoft.Xna.Framework.Input.Touch.TouchCollection/Enumerator"
    expect((enumerator, "get_Current", 0) in graph.fallible,
           "TouchCollection.Enumerator.Current must reach the indexer's throw")

    # --- indexed accessors --------------------------------------------------
    curve = classify(graph, "Microsoft.Xna.Framework.CurveKeyCollection", "get_Item", 1, 1)
    expect(curve["fallible"] and curve["evidence"] == EVIDENCE_INDEX_OPERATION,
           "CurveKeyCollection.Item getter must propagate the List<T> range check")
    guarded = classify(
        graph, "Microsoft.Xna.Framework.Graphics.EffectParameterCollection",
        "get_Item", 1, 1)
    expect(not guarded["fallible"],
           "EffectParameterCollection.Item getter range-checks and returns null")
    keyboard = classify(graph, "Microsoft.Xna.Framework.Input.KeyboardState", "get_Item", 1, 1)
    expect(not keyboard["fallible"], "KeyboardState.Item is a total bit test")
    modes = classify(
        graph, "Microsoft.Xna.Framework.Graphics.DisplayModeCollection", "get_Item", 1, 1)
    expect(not modes["fallible"], "DisplayModeCollection.Item cannot fail")
    touch = classify(
        graph, "Microsoft.Xna.Framework.Input.Touch.TouchCollection", "get_Item", 1, 1)
    expect(touch["fallible"] and touch["evidence"] == EVIDENCE_DIRECT_THROW,
           "TouchCollection.Item getter validates its own index")

    # --- abstract declarations decided from implementors --------------------
    packed = classify(
        graph, "Microsoft.Xna.Framework.Graphics.PackedVector.IPackedVector`1",
        "set_PackedValue", 1)
    expect(packed["evidence"] == EVIDENCE_ABSTRACT,
           "an interface accessor must be reported as an abstract declaration")
    expect(not packed["fallible"],
           "IPackedVector<T>.PackedValue setter must be infallible")
    expect(bool(packed["implementors"]),
           "an interface accessor must name the implementors it was decided from")
    fog = classify(graph, "Microsoft.Xna.Framework.Graphics.IEffectFog", "set_FogColor", 1)
    expect(fog["fallible"] and fog["evidence"] == EVIDENCE_ABSTRACT,
           "IEffectFog.FogColor setter must be fallible through its implementors")
    matrices = classify(
        graph, "Microsoft.Xna.Framework.Graphics.IEffectMatrices", "set_World", 1)
    expect(not matrices["fallible"], "IEffectMatrices.World setter must be infallible")

    # --- explicit interface implementations are implementors too -------------
    #
    # `IEffectLights.LightingEnabled` is implemented three ways: BasicEffect
    # implements it IMPLICITLY with an infallible field-backed setter, while
    # EnvironmentMapEffect and SkinnedEffect implement it EXPLICITLY and throw
    # `NotSupportedException(CantDisableLighting)` when asked to disable it.
    #
    # An explicit implementation's IL name is the interface member's
    # fully-qualified name, so a search by member name finds only BasicEffect
    # and the interface accessor reads as infallible. That is what these checks
    # exist to keep from coming back: the projection would then declare
    # `LightingEnabled { get set }` and neither effect could report the refusal
    # XNA reports.
    lights = "Microsoft.Xna.Framework.Graphics.IEffectLights"
    lighting = classify(graph, lights, "set_LightingEnabled", 1)
    expect(lighting["fallible"] and lighting["evidence"] == EVIDENCE_ABSTRACT,
           "IEffectLights.LightingEnabled setter must be fallible through its "
           "two EXPLICIT implementors")
    expect(lighting["exceptions"] == ["System.NotSupportedException"],
           "IEffectLights.LightingEnabled setter must report NotSupportedException")
    expect(len(graph.implementors(lights, "set_LightingEnabled", 1)) == 3,
           "all three IEffectLights implementors must be found, explicit or not")
    for effect in ("EnvironmentMapEffect", "SkinnedEffect"):
        owner = f"Microsoft.Xna.Framework.Graphics.{effect}"
        name = f"{lights}.set_LightingEnabled"
        expect((owner, name, 1) in graph.direct,
               f"{effect}'s explicit LightingEnabled setter must throw directly")
    # The getter of the same property is the negative control: both explicit
    # implementations are `ldc.i4.1; ret`, so widening the implementor search
    # must not make it fallible.
    expect(not classify(graph, lights, "get_LightingEnabled", 0)["fallible"],
           "IEffectLights.LightingEnabled getter must stay infallible")
    # A second control, on the mechanism rather than the verdict: an explicit
    # implementation must be reachable from a virtual call on the interface.
    expect(graph._overrides((lights, "set_LightingEnabled", 1)) >= {
        ("Microsoft.Xna.Framework.Graphics.SkinnedEffect",
         f"{lights}.set_LightingEnabled", 1)},
        "a callvirt on an interface member must reach its explicit implementations")

    # --- the graph must distinguish overload arities ------------------------
    expect(any(
        any(other[0] == key[0] and other[1] == key[1] and other[2] != key[2]
            for other in graph.methods)
        for key in graph.methods), "the call graph must distinguish overload arities")

    # --- the one approximation must be provably immaterial ------------------
    contract = json.loads(REFERENCE.read_text(encoding="utf-8"))
    disagreements = overload_merge_disagreements(graph, assemblies, contract)
    expect(not disagreements,
           "merging overloads by arity changes "
           f"{len(disagreements)} accessor verdict(s): {disagreements[:5]}")

    # --- mutation: every rule must actually bite ----------------------------
    checks += mutation_tests(graph, assemblies, failures)
    return checks, failures


def mutation_tests(
    graph: CallGraph, assemblies: list[Assembly], failures: list[str],
) -> int:
    """Disable each ingredient of a verdict and require the verdict to change."""
    checks = 0

    def rebuild(edit) -> CallGraph:
        import copy as copy_module
        clone = copy_module.deepcopy(assemblies)
        edit(clone)
        return CallGraph(clone)

    def body_of(clone: list[Assembly], key: tuple[str, str, int]) -> list[Method]:
        return [
            method for assembly in clone
            for method in assembly.methods.get(key, [])
        ]

    emitter = ("Microsoft.Xna.Framework.Audio.AudioEmitter", "set_DopplerScale", 1)
    def drop_throw(clone: list[Assembly]) -> None:
        for method in body_of(clone, emitter):
            method.body = [
                line for line in method.body
                if not re.search(r":\s*throw\b", uncommented(line))
            ]
    checks += 1
    if drop_throw and emitter in rebuild(drop_throw).fallible:
        failures.append("removing the throw must make AudioEmitter.DopplerScale infallible")

    blend = ("Microsoft.Xna.Framework.Graphics.BlendState", "set_AlphaBlendFunction", 1)
    def drop_call(clone: list[Assembly]) -> None:
        for method in body_of(clone, blend):
            method.body = [
                line for line in method.body if "ThrowIfBound" not in line
            ]
    checks += 1
    if blend in rebuild(drop_call).fallible:
        failures.append("removing the ThrowIfBound call must make BlendState infallible")

    guarded = ("Microsoft.Xna.Framework.Graphics.EffectParameterCollection", "get_Item", 1)
    def drop_guard(clone: list[Assembly]) -> None:
        for method in body_of(clone, guarded):
            method.body = [
                line for line in method.body
                if not re.search(r":\s*(blt|bge)\b", uncommented(line))
            ]
    checks += 1
    mutated = classify(rebuild(drop_guard), *guarded, 1)
    if not mutated["fallible"]:
        failures.append(
            "removing the range check must make EffectParameterCollection.Item fallible")

    fog = ("Microsoft.Xna.Framework.Graphics.IEffectFog", "set_FogColor", 1)
    def silence_implementors(clone: list[Assembly]) -> None:
        for assembly in clone:
            for key, methods in assembly.methods.items():
                if key[1] != "set_FogColor":
                    continue
                for method in methods:
                    method.body = [
                        line for line in method.body
                        if not re.search(r":\s*(call|callvirt|throw)\b", uncommented(line))
                    ]
    checks += 1
    if classify(rebuild(silence_implementors), *fog)["fallible"]:
        failures.append(
            "an interface accessor must become infallible when its implementors do")

    matrices = ("Microsoft.Xna.Framework.Graphics.IEffectMatrices", "set_World", 1)
    def poison_one_implementor(clone: list[Assembly]) -> None:
        for assembly in clone:
            methods = assembly.methods.get(
                ("Microsoft.Xna.Framework.Graphics.BasicEffect", "set_World", 1), [])
            for method in methods:
                method.body = list(method.body) + ["    IL_9999:  throw"]
    checks += 1
    if not classify(rebuild(poison_one_implementor), *matrices)["fallible"]:
        failures.append(
            "one throwing implementor must make the interface accessor fallible")
    return checks


def render_markdown(report: dict[str, Any]) -> str:
    """The reviewable form of the inventory: only what a human must check."""
    lines = [
        "# XNA 4.0 accessor fallibility inventory",
        "",
        "Generated by `tools/api_compat/accessor_fallibility.py` from the CIL of the",
        "hash-registered assemblies. Every row is re-derivable from that IL; no",
        "machine-local path is recorded. The pinned machine-readable form is",
        "`tools/api_compat/reference/xna40-accessor-fallibility.json`.",
        "",
        "| Measure | Value |",
        "|---|---:|",
        f"| Public properties | {report['PROPERTIES']} |",
        f"| Declared getters | {report['GETTERS']} |",
        f"| Declared setters | {report['SETTERS']} |",
        f"| Indexed properties | {report['INDEXED_PROPERTIES']} |",
        f"| Fallible getters | {report['THROWING_GETTERS']} |",
        f"| Fallible setters | {report['THROWING_SETTERS']} |",
        f"| Self-tests | {report['ACCESSOR_SELF_TESTS']} {report['SELF_TEST_STATUS']} |",
        "",
        "## Registered assemblies",
        "",
        "| Assembly | SHA-256 |",
        "|---|---|",
    ]
    for item in report["assemblies"]:
        lines.append(f"| `{item['assembly']}` | `{item['sha256']}` |")
    lines += ["", "## Evidence vocabulary", "", "| Label | Meaning |", "|---|---|"]
    for label, meaning in sorted(report["policy"].items()):
        lines.append(f"| `{label}` | {meaning} |")
    for kind, title in (("getter", "Fallible getters"), ("setter", "Fallible setters")):
        lines += ["", f"## {title}", "",
                  "| Type | Property | Evidence | CLR exceptions | Path |",
                  "|---|---|---|---|---|"]
        for item in report["entries"]:
            verdict = item[kind]
            if not verdict["fallible"]:
                continue
            chain = " → ".join(
                chain_part(part) for part in verdict.get("chain", []))
            lines.append(
                f"| `{item['type']}` | `{item['property']}` | `{verdict['evidence']}` | "
                f"{', '.join(verdict.get('exceptions', [])) or '—'} | "
                f"{chain if len(verdict.get('chain', [])) > 1 else '—'} |")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--assembly-dir", type=Path, action="append", default=[])
    parser.add_argument("--il-cache", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--markdown", type=Path)
    args = parser.parse_args()

    if not args.assembly_dir:
        raise SystemExit("--assembly-dir is required")

    contract = json.loads(REFERENCE.read_text(encoding="utf-8"))
    registry = json.loads(REGISTRY.read_text(encoding="utf-8"))
    expected = {item["assembly"]: item["sha256"] for item in registry["assemblies"]}

    paths: dict[str, Path] = {}
    for directory in args.assembly_dir:
        for path in sorted(directory.glob("Microsoft.Xna.Framework*.dll")):
            if path.name in expected and path.name not in paths:
                paths[path.name] = path
    missing = sorted(set(expected) - set(paths))
    if missing:
        raise SystemExit(f"registered assemblies not found: {missing}")

    assemblies: list[Assembly] = []
    for name, path in sorted(paths.items()):
        digest = sha256(path)
        if digest != expected[name]:
            raise SystemExit(
                f"{name} sha256 {digest} does not match registered {expected[name]}")
        cache = (args.il_cache / f"{path.stem}.il") if args.il_cache else None
        assemblies.append(Assembly(name, digest, disassemble(path, cache)))

    entries, graph = build(assemblies, contract)
    checks, failures = self_test(graph, assemblies)

    unresolved = [
        f"{item['type']}.{item['property']} {kind}"
        for item in entries for kind in ("getter", "setter")
        if item[kind]["evidence"] in (EVIDENCE_UNRESOLVED,)
    ] + [item["type"] for item in entries if item["assembly"] is None]

    report = {
        "schemaVersion": 1,
        "referenceSha256": hashlib.sha256(REFERENCE.read_bytes()).hexdigest(),
        "assemblies": [
            {"assembly": item.name, "sha256": item.sha256} for item in assemblies
        ],
        "policy": {
            EVIDENCE_DIRECT_THROW:
                "the accessor body itself executes throw or rethrow",
            EVIDENCE_REACHABLE_THROW:
                "a throw is reachable from the accessor through calls that stay "
                "inside the registered assemblies; the chain is recorded",
            EVIDENCE_ABSTRACT:
                "an abstract or interface accessor declares no body, so it is "
                "fallible exactly when a registered implementor is",
            EVIDENCE_NONE:
                "no throw is reachable inside the registered assemblies; "
                "allocation, external calls and universal CLR failures such as "
                "OutOfMemoryException are not XNA contract behaviour",
            EVIDENCE_ABSENT: "the CLR property does not declare this accessor",
            EVIDENCE_UNRESOLVED:
                "the contract declares an accessor the IL does not; a hard error",
        },
        "ACCESSOR_SELF_TESTS": checks,
        "SELF_TEST_STATUS": "FAIL" if failures else "PASS",
        "selfTestFailures": failures,
        "PROPERTIES": len(entries),
        "GETTERS": sum(1 for item in entries if item["getDeclared"]),
        "SETTERS": sum(1 for item in entries if item["setDeclared"]),
        "INDEXED_PROPERTIES": sum(1 for item in entries if item["indexed"]),
        "THROWING_GETTERS": sum(1 for item in entries if item["getter"]["fallible"]),
        "THROWING_SETTERS": sum(1 for item in entries if item["setter"]["fallible"]),
        "UNRESOLVED_ACCESSORS": sorted(set(unresolved)),
        "entries": entries,
    }
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if args.markdown:
        args.markdown.parent.mkdir(parents=True, exist_ok=True)
        args.markdown.write_text(render_markdown(report), encoding="utf-8")

    print(f"PROPERTIES={report['PROPERTIES']} GETTERS={report['GETTERS']} "
          f"SETTERS={report['SETTERS']} INDEXED={report['INDEXED_PROPERTIES']}")
    print(f"THROWING_GETTERS={report['THROWING_GETTERS']} "
          f"THROWING_SETTERS={report['THROWING_SETTERS']}")
    print(f"ACCESSOR_SELF_TESTS={report['ACCESSOR_SELF_TESTS']} "
          f"STATUS={report['SELF_TEST_STATUS']}")
    print(f"UNRESOLVED_ACCESSORS={len(report['UNRESOLVED_ACCESSORS'])}")
    for line in failures + report["UNRESOLVED_ACCESSORS"]:
        print(f"  {line}", file=sys.stderr)
    return 1 if (failures or report["UNRESOLVED_ACCESSORS"]) else 0


if __name__ == "__main__":
    sys.exit(main())
