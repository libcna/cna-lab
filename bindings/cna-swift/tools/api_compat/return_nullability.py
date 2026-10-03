#!/usr/bin/env python3
"""Reference-return nullability inventory for the registered Microsoft XNA assemblies.

A CLR reference return and a CLR failure are independent facts. `null` is a
normal successful result; a thrown exception is not a value at all. Swift
spells the two with orthogonal features -- `T?` and `throws` -- so a faithful
projection needs a per-return-position answer to a question the accessor
fallibility inventory does not ask:

    on a normal, successful return, can this reference be null?

The answer is derived mechanically from the CIL of the hash-registered
assemblies, and only from it. Every public reference-typed return position in
the pinned contract -- property getter, method, or field read -- is classified
into exactly one of:

* ``PROVEN_NULLABLE_SUCCESS`` -- a normal return of null is demonstrated: an
  ``ldnull`` reaching a ``ret``, a field whose lifecycle leaves it null at a
  reachable public read, or a callee whose own return is proven nullable.
* ``PROVEN_NONNULL_SUCCESS`` -- every reachable ``ret`` yields a value whose
  non-nullness is demonstrated: ``newobj``, ``newarr``, ``ldstr``, ``box``, a
  reference the member's own null guard refined non-null, a field every
  constructor initialises and nothing ever nulls, or a callee proven non-null.
* ``UNKNOWN_REFERENCE_NULLABILITY`` -- neither is demonstrated inside the
  registered set. A call leaving the registered assemblies, an ``isinst``, an
  array element, a native or mixed-mode boundary, and an abstract member with
  no registered implementor all land here. Nothing is guessed into either
  proven class to improve a scoreboard.

The analysis is an abstract interpretation of each method body over

    NONNULL  <  UNKNOWN  <  NULLABLE            (join = max)

carried on a simulated evaluation stack, with an environment for arguments,
locals and field reads so a null guard refines exactly what it tests. Method
and field summaries are computed together to a least fixpoint; a ``callvirt``
joins the declared method with the registered overrides, exactly as the
accessor inventory decides an abstract accessor from its implementors.

A field's summary is its whole observable lifecycle, not one read: the value
every declared constructor leaves behind, joined with every store anywhere in
the registered set. That is what separates "this reference field could
theoretically be null" from "XNA normally leaves it null at a reachable public
read". ``GraphicsDeviceManager.device`` is the canonical case: the constructor
never assigns it and ``Dispose`` explicitly stores ``ldnull`` into it.

Two approximations are made, both bounded and both reported:

* Same-named, same-arity overloads are resolved at a call site only when they
  agree; where they disagree the call result is UNKNOWN rather than guessed.
* Fallibility is the pinned accessor verdict for a getter, and the arity-merged
  call-graph verdict for a method. The two-sided bound that proves the merge
  immaterial is re-run over this inventory's own method scope.

The emitted inventory records only CLR facts and the SHA-256 of the assembly
each entry came from. It carries no Swift spelling, no mapping rule and no
machine-local path, so it is a function of the pinned contract and the seven
registered binaries alone.
"""
from __future__ import annotations

import argparse
import collections
import copy as copy_module
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

import accessor_fallibility as accessor  # noqa: E402
import pinned_assembly_audit as audit  # noqa: E402

TOOL = Path(__file__).resolve().parent
REFERENCE = TOOL / "reference" / "xna40-windows-runtime-contract.json"
REGISTRY = TOOL / "registered-assemblies.json"
FALLIBILITY = TOOL / "reference" / "xna40-accessor-fallibility.json"

PROVEN_NONNULL = "PROVEN_NONNULL_SUCCESS"
PROVEN_NULLABLE = "PROVEN_NULLABLE_SUCCESS"
UNKNOWN = "UNKNOWN_REFERENCE_NULLABILITY"

# Why a position carries no reference verdict at all.
NOT_REFERENCE = "NOT_A_REFERENCE_RETURN"
NOT_REFERENCE_GENERIC = "NOT_A_REFERENCE_RETURN_UNCONSTRAINED_GENERIC"
NOT_REFERENCE_EVENT = "NOT_A_REFERENCE_RETURN_EVENT_PROJECTION"
NOT_REFERENCE_CONSTRUCTOR = "NOT_A_REFERENCE_RETURN_CONSTRUCTOR"
NOT_REFERENCE_WRITE_ONLY = "NOT_A_REFERENCE_RETURN_WRITE_ONLY_PROPERTY"

# How a verdict was reached.
EVIDENCE_RETURN = "IL_RETURN_VALUE_ANALYSIS"
EVIDENCE_FIELD = "IL_FIELD_LIFECYCLE"
EVIDENCE_ABSTRACT = "IL_ABSTRACT_DECLARATION"
EVIDENCE_NO_IMPLEMENTOR = "IL_ABSTRACT_NO_REGISTERED_IMPLEMENTOR"
EVIDENCE_NO_BODY = "IL_NO_MANAGED_BODY"
EVIDENCE_UNDECODABLE = "IL_BODY_NOT_DECODABLE"
EVIDENCE_UNRESOLVED = "IL_MEMBER_NOT_FOUND"

# Reference-typed BCL spellings the pinned contract uses in a return position.
# A self-test re-derives the closure of this table from the contract, so a new
# BCL return type cannot silently be classified as a value type.
REFERENCE_BCL = frozenset({
    "System.String",
    "System.Object",
    "System.IO.Stream",
    "System.IAsyncResult",
    "System.IServiceProvider",
    "System.Type",
    "System.ComponentModel.PropertyDescriptorCollection",
})
REFERENCE_BCL_GENERIC_PREFIXES = (
    "System.Collections.Generic.IEnumerable`1[",
    "System.Collections.Generic.IEnumerator`1[",
    "System.Collections.Generic.ICollection`1[",
    "System.Collections.Generic.IList`1[",
    "System.Collections.Generic.List`1[",
    "System.Collections.ObjectModel.ReadOnlyCollection`1[",
    "System.EventHandler`1[",
)
# `List<T>.Enumerator` and `Nullable<T>` are generic *value* types.
VALUE_BCL_GENERIC_PREFIXES = (
    "System.Nullable`1[",
    "System.Collections.Generic.List`1+Enumerator[",
)
VALUE_BCL = frozenset({
    "System.Void", "System.Boolean", "System.Byte", "System.SByte",
    "System.Int16", "System.UInt16", "System.Int32", "System.UInt32",
    "System.Int64", "System.UInt64", "System.Single", "System.Double",
    "System.Char", "System.TimeSpan", "System.IntPtr", "System.UIntPtr",
    "System.DateTime", "System.Guid", "System.Decimal", "System.TypedReference",
})

LABELLED = re.compile(r"^\s*IL_([0-9a-fA-F]+):\s*(\S+)(.*)$")
TARGET = re.compile(r"IL_([0-9a-fA-F]+)")
HANDLER_MARKER = re.compile(r"^(catch\b|filter\b|finally\b|fault\b)")
LOCALS = re.compile(r"^\.locals\b")

METHOD_FLAGS = frozenset({
    "public", "private", "family", "assembly", "famorassem", "famandassem",
    "static", "instance", "hidebysig", "specialname", "rtspecialname",
    "virtual", "abstract", "final", "newslot", "pinvokeimpl", "strict",
    "explicit", "reqsecobj", "unmanagedexp", "compilercontrolled", "cil",
    "managed", "runtime", "synchronized", "noinlining",
})

# --- the nullness lattice ---------------------------------------------------
# A value is a (bits, precise) pair. `bits` records the outcomes *proved*
# reachable; `precise` says whether that set is exact. NONNULL and NULLABLE are
# the two exact answers, everything else is UNKNOWN.
BIT_NULL = 1
BIT_NONNULL = 2

V_NULL = (BIT_NULL, True)
V_NONNULL = (BIT_NONNULL, True)
V_UNKNOWN = (0, False)
V_BOTTOM = (0, True)


def join(left: tuple[int, bool], right: tuple[int, bool]) -> tuple[int, bool]:
    return (left[0] | right[0], left[1] and right[1])


def verdict_of(value: tuple[int, bool]) -> str:
    if value[0] & BIT_NULL:
        return PROVEN_NULLABLE
    if value[1]:
        # Exact and null is not among the outcomes: proven non-null, including
        # the vacuous case of a member with no reachable successful return.
        return PROVEN_NONNULL
    return UNKNOWN


ORDER = {PROVEN_NONNULL: 0, UNKNOWN: 1, PROVEN_NULLABLE: 2}
INVERSE_ORDER = {value: key for key, value in ORDER.items()}


def verdict_join(left: str, right: str) -> str:
    return INVERSE_ORDER[max(ORDER[left], ORDER[right])]


class Undecodable(Exception):
    """The body used a shape this interpreter does not model."""


# --- decoding ----------------------------------------------------------------


def decode(body: list[str]) -> tuple[list[tuple[int, str, str]], dict[int, int],
                                     list[tuple[int, int]], dict[str, int]]:
    """(instructions, offset->index, roots, local slot names) for one body.

    `roots` pairs an instruction index with the evaluation-stack depth the CLR
    guarantees on entry: zero for the method entry and for a finally or fault
    handler, one for a catch or filter handler, which starts with the exception
    object already pushed.
    """
    instructions: list[tuple[int, str, str]] = []
    roots: list[tuple[int, int]] = []
    pending: int | None = None
    after_filter = False
    locals_text: list[str] = []
    index = 0
    while index < len(body):
        line = body[index]
        clean = accessor.structural(line).strip()
        match = LABELLED.match(accessor.uncommented(line))
        if match is None:
            if LOCALS.match(clean):
                collected = accessor.uncommented(line).strip()
                while index + 1 < len(body) and accessor.unbalanced(collected):
                    index += 1
                    collected += " " + accessor.uncommented(body[index]).strip()
                locals_text.append(collected)
            elif HANDLER_MARKER.match(clean):
                if clean.startswith("filter"):
                    pending, after_filter = 1, True
                else:
                    pending, after_filter = (
                        0 if clean.startswith(("finally", "fault")) else 1, False)
            elif clean.startswith("}") and "filter" in clean:
                after_filter = True
            elif clean == "{" and after_filter:
                pending, after_filter = 1, False
            index += 1
            continue
        operand = match.group(3)
        index += 1
        while index < len(body) and accessor.unbalanced(operand):
            operand += " " + accessor.uncommented(body[index]).strip()
            index += 1
        if pending is not None:
            roots.append((len(instructions), pending))
            pending = None
        instructions.append((int(match.group(1), 16), match.group(2), operand))
    offsets = {item[0]: position for position, item in enumerate(instructions)}
    slots = local_slots(" ".join(locals_text))
    return instructions, offsets, ([(0, 0)] + roots if instructions else []), slots


def local_slots(text: str) -> dict[str, int]:
    """Map each declared local's name onto its slot index.

    ikdasm spells slots four and above by name (`ldloc.s V_4`), so the slot
    index has to come from the `.locals` declaration rather than the operand.
    An explicitly numbered declaration (`[4] class Foo bar`) is honoured.
    """
    open_index = text.find("(")
    if open_index < 0:
        return {}
    inner = accessor.balanced_argument_list(text, open_index)
    if inner is None:
        return {}
    result: dict[str, int] = {}
    position = 0
    for fragment in accessor.split_top_level(inner):
        item = fragment.strip()
        explicit = re.match(r"^\[(\d+)\]\s*(.*)$", item)
        if explicit:
            position = int(explicit.group(1))
            item = explicit.group(2).strip()
        tokens = item.rsplit(" ", 1)
        if len(tokens) == 2 and re.fullmatch(r"'?[A-Za-z_$][\w$]*'?", tokens[1]):
            result[tokens[1].strip("'")] = position
        position += 1
    return result


def argument_slots(method: accessor.Method) -> dict[str, int]:
    """Map each declared parameter's name onto its argument index."""
    body = audit.strip_directives(method.header[len(".method"):].strip())
    split = audit.split_call(body)
    if split is None:
        return {}
    head, parameter_text, _ = split
    head, method_generics = audit.split_generic_suffix(head)
    base = 0 if method.static else 1
    result: dict[str, int] = {}
    for position, parameter in enumerate(
        audit.split_parameters(parameter_text, (), method_generics)
    ):
        if parameter["name"]:
            result[parameter["name"]] = base + position
    return result


def field_reference(operand: str) -> tuple[str, str] | None:
    """Resolve an ldfld/stfld/ldsfld/stsfld operand to (owner, field name)."""
    text = accessor.strip_assembly_refs(accessor.uncommented(operand)).strip()
    marker = text.rfind("::")
    if marker < 0:
        return None
    owner = accessor.strip_generic_arguments(text[:marker]).strip()
    parts = re.sub(r"[(),*&\[\]]", " ", owner).split()
    name = re.sub(r"[\s(].*$", "", text[marker + 2:].strip()).strip("'")
    if not parts or not name:
        return None
    return parts[-1], name


CALL_CONVENTIONS = frozenset({
    "instance", "explicit", "unmanaged", "stdcall", "cdecl", "fastcall",
    "thiscall", "vararg", "default", "method",
})


def split_signature(operand: str) -> tuple[list[str], str, int] | None:
    """(head tokens, argument text, arity) for a call, newobj or calli operand.

    A custom modifier carries its own parentheses -- C++/CLI signatures are
    full of `modopt(...)` and `modreq(...)` -- so the argument list is located
    only after the directives are removed. Reading the first bracket instead
    silently mistakes a modifier for the parameter list and desynchronises the
    simulated evaluation stack.
    """
    text = audit.strip_directives(accessor.uncommented(operand))
    split = audit.split_call(text)
    if split is None:
        return None
    head, arguments, _ = split
    head = accessor.strip_generic_arguments(
        accessor.strip_assembly_refs(head))
    # `...` opens a vararg signature's variable part; it is a marker, not an
    # argument, and counting it pops one value too many.
    arity = sum(1 for item in audit.split_top(arguments) if item != "...")
    return head.split(), arguments, arity


def returns_a_value(tokens: list[str]) -> bool:
    """Whether a signature's return type is something other than `void`.

    `void*` is a pointer and is emphatically not `void`: mistaking the two
    loses a pushed value and desynchronises the simulated stack from there on,
    which is how every C++/CLI body in this set used to become undecodable.
    The test is therefore made on the unpunctuated token.
    """
    rest = [item for item in tokens if item not in CALL_CONVENTIONS]
    return not (rest and rest[0] == "void")


def call_shape(operand: str) -> tuple[str, str, int, bool, bool] | None:
    """(owner, name, arity, has this, returns a value) for a call operand."""
    parsed = split_signature(operand)
    if parsed is None:
        return None
    tokens, _, arity = parsed
    head = " ".join(tokens)
    marker = head.rfind("::")
    if marker >= 0:
        prefix = head[:marker].split()
        name = head[marker + 2:].strip().strip("'")
        if not prefix or not name:
            return None
        owner_parts = re.sub(r"[(),*&\[\]]", " ", prefix[-1]).split()
        if not owner_parts:
            return None
        owner, remainder = owner_parts[-1], prefix[:-1]
    else:
        if not tokens:
            return None
        owner, name, remainder = "<global>", tokens[-1].strip("'"), tokens[:-1]
    return (owner, name, arity, "instance" in tokens, returns_a_value(remainder))


def calli_shape(operand: str) -> tuple[int, bool, bool]:
    """(argument count, has this, returns a value) for a calli signature."""
    parsed = split_signature(operand)
    if parsed is None:
        raise Undecodable("calli signature")
    tokens, _, arity = parsed
    return arity, "instance" in tokens, returns_a_value(tokens)


PREFIX_OPCODES = ("unaligned.", "volatile.", "constrained.", "readonly.",
                  "tail.", "no.")
TERMINATORS = ("rethrow", "endfinally", "endfault", "jmp")
BINARY_VALUE = ("add", "sub", "mul", "div", "rem", "and", "or", "xor", "shl",
                "shr", "ceq", "cgt", "clt")
UNARY_VALUE = ("neg", "not", "conv.", "ckfinite", "refanytype", "refanyval")
EQUALITY_BRANCH = ("beq", "bne.un")


def numeric_slot(opcode: str, operand: str, prefix: str,
                 names: dict[str, int]) -> int | None:
    if opcode.startswith(prefix + "."):
        tail = opcode[len(prefix) + 1:]
        if tail.isdigit():
            return int(tail)
        if tail == "s":
            return slot_operand(operand, names)
        return None
    if opcode == prefix:
        return slot_operand(operand, names)
    return None


def slot_operand(operand: str, names: dict[str, int]) -> int:
    text = accessor.uncommented(operand).strip().strip("'")
    text = re.split(r"[\s,]", text)[0].strip("'")
    if text.isdigit():
        return int(text)
    if text in names:
        return names[text]
    match = re.fullmatch(r"V_(\d+)", text)
    if match:
        return int(match.group(1))
    raise Undecodable(f"unresolved slot operand {text!r}")


class State:
    __slots__ = ("stack", "locals", "arguments", "refine", "fields")

    def __init__(self, stack, local_values, arguments, refine, fields) -> None:
        self.stack = stack
        self.locals = local_values
        self.arguments = arguments
        self.refine = refine
        self.fields = fields

    def key(self) -> tuple:
        return (
            tuple(value for value, _ in self.stack),
            tuple(sorted(self.locals.items())),
            tuple(sorted(self.arguments.items())),
            tuple(sorted(self.refine.items())),
            tuple(sorted(self.fields.items())),
        )

    def copy(self) -> "State":
        return State(list(self.stack), dict(self.locals), dict(self.arguments),
                     dict(self.refine), dict(self.fields))


def merge_state(existing: State | None, incoming: State) -> tuple[State, bool]:
    if existing is None:
        return incoming.copy(), True
    if len(existing.stack) != len(incoming.stack):
        raise Undecodable("stack depth differs at a merge point")
    merged = State(
        [
            (join(a[0], b[0]), a[1] if a[1] == b[1] else None)
            for a, b in zip(existing.stack, incoming.stack)
        ],
        dict(existing.locals), dict(existing.arguments),
        {
            key: join(value, incoming.refine[key])
            for key, value in existing.refine.items() if key in incoming.refine
        },
        dict(existing.fields),
    )
    for source, target, absent in (
        (incoming.locals, merged.locals, V_UNKNOWN),
        (incoming.arguments, merged.arguments, V_UNKNOWN),
        (incoming.fields, merged.fields, V_NULL),
    ):
        for key, value in source.items():
            target[key] = join(target[key], value) if key in target else join(value, absent)
    for target, source, absent in (
        (merged.locals, incoming.locals, V_UNKNOWN),
        (merged.arguments, incoming.arguments, V_UNKNOWN),
        (merged.fields, incoming.fields, V_NULL),
    ):
        for key in list(target):
            if key not in source:
                target[key] = join(target[key], absent)
    return merged, merged.key() != existing.key()


def refine_origin(state: State, origin, value) -> None:
    if origin is None:
        return
    if origin[0] == "l":
        state.locals[origin[1]] = value
    elif origin[0] == "a":
        state.arguments[origin[1]] = value
    elif origin[0] in ("f", "sf"):
        state.refine[origin] = value


def describe_origin(origin) -> str | None:
    if origin is None:
        return None
    if origin[0] == "f":
        return f"field {origin[2]}"
    if origin[0] == "sf":
        return f"static field {origin[1]}"
    if origin[0] == "call":
        return f"call {origin[1]}"
    if origin[0] == "isNullOrEmpty":
        inner = describe_origin(origin[1])
        return f"String.IsNullOrEmpty({inner})" if inner else None
    if origin[0] == "native":
        return "native calli"
    if origin[0] == "l":
        return "local"
    if origin[0] == "a":
        return "argument"
    return None


class Nullness:
    """Least-fixpoint method and field summaries over the registered set."""

    ROUND_LIMIT = 60

    def __init__(self, assemblies: list[accessor.Assembly],
                 graph: accessor.CallGraph) -> None:
        self.graph = graph
        self.assemblies = assemblies
        self.methods = graph.methods
        self.decoded: dict[int, tuple] = {}
        self.arg_names: dict[int, dict[str, int]] = {}
        self.returns_value: dict[int, bool] = {}
        self.summary: dict[int, tuple[int, bool]] = {}
        self.field_stores: dict[tuple[str, str], tuple[int, bool]] = {}
        self.field_constructed: dict[tuple[str, str], tuple[int, bool]] = {}
        self.field_evidence: dict[tuple[str, str], list[str]] = {}
        self.identity: dict[int, accessor.Method] = {}
        self.method_key: dict[int, tuple[str, str, int]] = {}
        self.instance_ctors: dict[str, list[int]] = collections.defaultdict(list)
        self.static_ctors: dict[str, list[int]] = collections.defaultdict(list)
        self.undecodable: set[int] = set()
        self.rounds = 0
        self._index()
        self._fixpoint()

    # -- indexing ----------------------------------------------------------
    def _index(self) -> None:
        for key, methods in self.methods.items():
            for method in methods:
                token = id(method)
                self.identity[token] = method
                self.method_key[token] = key
                self.summary[token] = V_BOTTOM
                self.returns_value[token] = declared_returns_value(method)
                if method.has_body:
                    try:
                        self.decoded[token] = decode(method.body)
                        self.arg_names[token] = argument_slots(method)
                    except Exception:
                        self.undecodable.add(token)
                        self.decoded.pop(token, None)
                if key[1] == ".ctor":
                    self.instance_ctors[key[0]].append(token)
                elif key[1] == ".cctor":
                    self.static_ctors[key[0]].append(token)

    # -- summaries ---------------------------------------------------------
    def field_value(self, reference: tuple[str, str]) -> tuple[int, bool]:
        return join(self.field_constructed.get(reference, V_NULL),
                    self.field_stores.get(reference, V_BOTTOM))

    def implementation_value(self, method: accessor.Method) -> tuple[int, bool] | None:
        """One method's contribution, or None when it declares no implementation.

        An abstract or interface declaration contributes nothing -- its
        registered overrides carry the answer. A body this interpreter could
        not decode, and a body that is not managed CIL at all, contribute
        UNKNOWN: a native or runtime-implemented return is not evidence of
        anything.
        """
        token = id(method)
        if method.abstract or not method.has_body:
            return None if method.abstract else V_UNKNOWN
        if token in self.undecodable or token not in self.decoded:
            return V_UNKNOWN
        return self.summary[token]

    def member_value(self, owner: str, name: str, arity: int,
                     candidate: accessor.Method) -> tuple[tuple[int, bool], int]:
        """(value, contributing implementations) for one declared member.

        A Swift return type is fixed by the declaration, and an `override`
        cannot widen it to Optional, so a member's projected nullability has to
        accommodate every registered implementation of it -- its own body and
        the overrides that may be dispatched in its place. An abstract
        declaration contributes nothing and its implementors carry the answer;
        a member with no override is decided by its own body alone.
        """
        own = self.implementation_value(candidate)
        total = V_BOTTOM if own is None else own
        contributors = 0 if own is None else 1
        if candidate.virtual or candidate.abstract:
            for override in self.graph._overrides((owner, name, arity)):
                values = {
                    value for value in (
                        self.implementation_value(item)
                        for item in self.methods.get(override, ())
                    ) if value is not None
                }
                if not values:
                    continue
                contributors += 1
                total = join(total,
                             values.pop() if len(values) == 1 else V_UNKNOWN)
        return total, contributors

    def call_value(self, owner: str, name: str, arity: int,
                   virtual: bool) -> tuple[int, bool]:
        owners = [owner]
        if virtual:
            declared = self.methods.get((owner, name, arity), [])
            if any(item.virtual or item.abstract for item in declared):
                owners.extend(
                    candidate[0]
                    for candidate in self.graph._overrides((owner, name, arity))
                )
        total = V_BOTTOM
        contributors = 0
        for candidate in owners:
            values = {
                value for value in (
                    self.implementation_value(item)
                    for item in self.methods.get((candidate, name, arity), ())
                ) if value is not None
            }
            if not values:
                continue
            contributors += 1
            # Same-arity overloads that disagree are not resolved
            # signature-precisely here, so nothing is proved.
            total = join(total, values.pop() if len(values) == 1 else V_UNKNOWN)
        return total if contributors else V_UNKNOWN

    def _fixpoint(self) -> None:
        changed = True
        while changed and self.rounds < self.ROUND_LIMIT:
            changed = False
            self.rounds += 1
            stores: dict[tuple[str, str], tuple[int, bool]] = {}
            evidence: dict[tuple[str, str], set[str]] = collections.defaultdict(set)
            posts: dict[int, dict[tuple[str, str], tuple[int, bool]]] = {}
            for token in list(self.identity):
                if token not in self.decoded:
                    continue
                try:
                    value, method_stores, post, _ = self.run(token)
                except Undecodable:
                    self.undecodable.add(token)
                    self.decoded.pop(token, None)
                    changed = True
                    continue
                if value != self.summary[token]:
                    self.summary[token] = value
                    changed = True
                for reference, (stored, site) in method_stores.items():
                    stores[reference] = join(stores.get(reference, V_BOTTOM), stored)
                    if stored[0] & BIT_NULL:
                        evidence[reference].add(site)
                posts[token] = post
            constructed = self._construction(posts, evidence)
            if stores != self.field_stores or constructed != self.field_constructed:
                self.field_stores = stores
                self.field_constructed = constructed
                changed = True
            self.field_evidence = {
                reference: sorted(sites) for reference, sites in evidence.items()
            }

    def _construction(self, posts, evidence) -> dict[tuple[str, str], tuple[int, bool]]:
        """What each constructor leaves in the fields of its own type.

        A field no constructor assigns holds the CLR's zero -- null -- when the
        constructor returns, so it is nullable at every later public read
        unless something else always assigns it first. A field every
        constructor assigns non-null starts non-null.
        """
        owners: dict[str, set[tuple[str, str]]] = collections.defaultdict(set)
        for token, post in posts.items():
            for reference in post:
                owners[reference[0]].add(reference)
        result: dict[tuple[str, str], tuple[int, bool]] = {}
        for owner, references in owners.items():
            for kind, tokens in (("i", self.instance_ctors.get(owner, [])),
                                 ("s", self.static_ctors.get(owner, []))):
                if not tokens:
                    continue
                for reference in references:
                    if not any(reference in posts.get(token, {}) for token in tokens):
                        continue
                    total = V_BOTTOM
                    for token in tokens:
                        observed = posts.get(token, {}).get(reference, V_NULL)
                        total = join(total, observed)
                        if observed[0] & BIT_NULL:
                            evidence[reference].add(
                                f"{self.method_key[token][0]}::"
                                f"{self.method_key[token][1]} leaves it null")
                    result[reference] = total
        for reference in set(self.field_stores) | set(result):
            if reference not in result:
                evidence[reference].add(
                    f"no constructor of {reference[0]} assigns {reference[1]}")
        return result

    # -- the interpreter ---------------------------------------------------
    def run(self, token: int, explain: bool = False):
        method = self.identity[token]
        instructions, offsets, roots, slots = self.decoded[token]
        owner, name, arity = self.method_key[token]
        track_instance = name == ".ctor"
        track_static = name == ".cctor"
        names = self.arg_names.get(token, {})
        address_taken: set[tuple[str, int]] = set()
        for _, opcode, operand in instructions:
            if opcode.startswith("ldloca"):
                address_taken.add(("l", slot_operand(operand, slots)))
            elif opcode.startswith("ldarga"):
                address_taken.add(("a", slot_operand(operand, names)))

        entry = State([], {}, {}, {}, {})
        if not method.static:
            entry.arguments[0] = V_NONNULL
        states: dict[int, State] = {}
        work: collections.deque[int] = collections.deque()
        for index, depth in roots:
            if index >= len(instructions):
                continue
            initial = entry.copy()
            initial.stack = [(V_UNKNOWN, None)] * depth
            merged, moved = merge_state(states.get(index), initial)
            states[index] = merged
            if moved:
                work.append(index)

        stores: dict[tuple[str, str], tuple[tuple[int, bool], str]] = {}
        post: dict[tuple[str, str], tuple[int, bool]] = {}
        result = V_BOTTOM
        sites: list[tuple[str, str, str | None]] = []
        steps = 0
        while work:
            index = work.popleft()
            steps += 1
            if steps > 400000:
                raise Undecodable("interpretation did not settle")
            state = states[index].copy()
            offset, opcode, operand = instructions[index]
            successors, returned = self.step(
                state, opcode, operand, method, token, offset, offsets, slots,
                names, stores, post, track_instance, track_static, address_taken,
            )
            if returned is not None:
                result = join(result, returned[0])
                if explain:
                    sites.append((f"IL_{offset:04x}", verdict_of(returned[0]),
                                  returned[1]))
                continue
            for target, next_state in successors:
                position = index + 1 if target is None else target
                if position is None or position >= len(instructions):
                    continue
                merged, moved = merge_state(states.get(position), next_state)
                states[position] = merged
                if moved:
                    work.append(position)
        return result, stores, post, sites

    def step(self, state, opcode, operand, method, token, offset, offsets, slots,
             names, stores, post, track_instance, track_static, address_taken):
        base = opcode
        if base.startswith(PREFIX_OPCODES):
            return ([(None, state)], None)
        if base in ("nop", "break"):
            return ([(None, state)], None)

        def pop():
            if not state.stack:
                raise Undecodable(f"stack underflow at {opcode}")
            return state.stack.pop()

        def push(value, origin=None):
            state.stack.append((value, origin))

        def kill():
            state.refine = {
                key: value for key, value in state.refine.items() if key[0] == "l"
            }
            for slot in list(state.locals):
                if ("l", slot) in address_taken:
                    state.locals[slot] = V_UNKNOWN
            for slot in list(state.arguments):
                if ("a", slot) in address_taken:
                    state.arguments[slot] = V_UNKNOWN

        # --- terminators ----------------------------------------------------
        if base == "throw":
            pop()
            return ([], None)
        if base == "endfilter":
            pop()
            return ([], None)
        if base in TERMINATORS:
            return ([], None)
        if base == "ret":
            if self.returns_value[token]:
                value, origin = pop()
                return ([], (value, describe_origin(origin)))
            if track_instance or track_static:
                for reference, value in state.fields.items():
                    post[reference] = join(post.get(reference, V_BOTTOM), value)
            return ([], (V_BOTTOM, None))

        # --- argument and local slots ---------------------------------------
        slot = numeric_slot(base, operand, "ldarg", names)
        if slot is not None:
            push(state.arguments.get(slot, V_UNKNOWN), ("a", slot))
            return ([(None, state)], None)
        slot = numeric_slot(base, operand, "ldloc", slots)
        if slot is not None:
            push(state.locals.get(slot, V_UNKNOWN), ("l", slot))
            return ([(None, state)], None)
        if base.startswith(("ldarga", "ldloca")):
            push(V_NONNULL)
            return ([(None, state)], None)
        slot = numeric_slot(base, operand, "starg", names)
        if slot is not None:
            state.arguments[slot] = pop()[0]
            return ([(None, state)], None)
        slot = numeric_slot(base, operand, "stloc", slots)
        if slot is not None:
            state.locals[slot] = pop()[0]
            return ([(None, state)], None)

        # --- constants -------------------------------------------------------
        if base.startswith("ldc.") or base in ("ldstr", "ldtoken", "ldftn",
                                               "sizeof", "arglist"):
            push(V_NONNULL)
            return ([(None, state)], None)
        if base == "ldnull":
            push(V_NULL)
            return ([(None, state)], None)

        # --- fields ----------------------------------------------------------
        if base == "ldsfld":
            reference = field_reference(operand)
            if reference is None:
                push(V_UNKNOWN)
                return ([(None, state)], None)
            origin = ("sf", f"{reference[0]}::{reference[1]}")
            if track_static and reference[0] == self.method_key[token][0]:
                push(state.fields.get(reference, V_NULL), origin)
            else:
                push(state.refine.get(origin, self.field_value(reference)), origin)
            return ([(None, state)], None)
        if base == "ldsflda":
            push(V_NONNULL)
            return ([(None, state)], None)
        if base == "ldfld":
            _, base_origin = pop()
            reference = field_reference(operand)
            if reference is None:
                push(V_UNKNOWN)
                return ([(None, state)], None)
            origin = ("f", base_origin, f"{reference[0]}::{reference[1]}")
            if track_instance and base_origin == ("a", 0):
                push(state.fields.get(reference, V_NULL), origin)
            else:
                push(state.refine.get(origin, self.field_value(reference)), origin)
            return ([(None, state)], None)
        if base == "ldflda":
            pop()
            push(V_NONNULL)
            return ([(None, state)], None)
        if base == "stfld":
            value = pop()[0]
            _, base_origin = pop()
            reference = field_reference(operand)
            if reference is not None:
                own = reference[0] == self.method_key[token][0]
                if track_instance and own and base_origin == ("a", 0):
                    state.fields[reference] = value
                else:
                    record_store(stores, reference, value, method, offset)
            kill()
            return ([(None, state)], None)
        if base == "stsfld":
            value = pop()[0]
            reference = field_reference(operand)
            if reference is not None:
                if track_static and reference[0] == self.method_key[token][0]:
                    state.fields[reference] = value
                else:
                    record_store(stores, reference, value, method, offset)
            kill()
            return ([(None, state)], None)

        # --- indirection, arrays and casts ------------------------------------
        if base.startswith("ldind."):
            pop()
            push(V_UNKNOWN if base == "ldind.ref" else V_NONNULL)
            return ([(None, state)], None)
        if base == "ldobj":
            pop()
            push(V_UNKNOWN)
            return ([(None, state)], None)
        if base == "ldlen":
            pop()
            push(V_NONNULL)
            return ([(None, state)], None)
        if base == "ldelema" or base.startswith("ldelem"):
            pop()
            pop()
            push(V_UNKNOWN if base == "ldelem.ref" else V_NONNULL)
            return ([(None, state)], None)
        if base == "castclass":
            value, origin = pop()
            push(value, origin)
            return ([(None, state)], None)
        if base in ("isinst", "unbox.any"):
            pop()
            push(V_UNKNOWN)
            return ([(None, state)], None)
        if base in ("unbox", "box", "mkrefany"):
            pop()
            push(V_NONNULL)
            return ([(None, state)], None)
        if base in ("newarr", "localloc", "ldvirtftn"):
            pop()
            push(V_NONNULL)
            return ([(None, state)], None)

        # --- arithmetic and comparison ----------------------------------------
        if base.split(".")[0] in BINARY_VALUE:
            pop()
            pop()
            push(V_NONNULL)
            return ([(None, state)], None)
        if base.startswith(UNARY_VALUE):
            pop()
            push(V_NONNULL)
            return ([(None, state)], None)

        # --- stack shuffling ---------------------------------------------------
        if base == "dup":
            value, origin = pop()
            push(value, origin)
            push(value, origin)
            return ([(None, state)], None)
        if base == "pop":
            pop()
            return ([(None, state)], None)

        # --- indirect and block stores -----------------------------------------
        if base.startswith("stind.") or base in ("stobj", "cpobj"):
            pop()
            pop()
            kill()
            return ([(None, state)], None)
        if base.startswith("stelem") or base in ("cpblk", "initblk"):
            pop()
            pop()
            pop()
            kill()
            return ([(None, state)], None)
        if base == "initobj":
            pop()
            kill()
            return ([(None, state)], None)

        # --- calls --------------------------------------------------------------
        if base in ("call", "callvirt"):
            shape = call_shape(operand)
            if shape is None:
                raise Undecodable(f"unresolved call operand {operand[:60]!r}")
            call_owner, call_name, call_arity, has_this, returns_value = shape
            arguments = [pop() for _ in range(call_arity + (1 if has_this else 0))]
            kill()
            if returns_value:
                # `String.IsNullOrEmpty(x)` is a NULL TEST that happens to be
                # spelled as a call, and the branch that follows tests its
                # RESULT rather than `x`. Without carrying the argument's
                # origin through, a `false` branch -- which proves `x` is not
                # null -- refines nothing, and a field read on that branch is
                # reported nullable although it demonstrably cannot be. The
                # result therefore remembers what was tested; the branch
                # handler uses it in the one direction that proves something.
                origin = ("call", f"{call_owner}::{call_name}/{call_arity}")
                if (
                    call_owner == "System.String" and
                    call_name == "IsNullOrEmpty" and
                    call_arity == 1 and not has_this and
                    arguments and arguments[0][1] is not None
                ):
                    origin = ("isNullOrEmpty", arguments[0][1])
                push(self.call_value(call_owner, call_name, call_arity,
                                     base == "callvirt"), origin)
            return ([(None, state)], None)
        if base == "newobj":
            shape = call_shape(operand)
            if shape is None:
                raise Undecodable(f"unresolved newobj operand {operand[:60]!r}")
            for _ in range(shape[2]):
                pop()
            kill()
            push(V_NONNULL)
            return ([(None, state)], None)
        if base == "calli":
            count, has_this, returns_value = calli_shape(operand)
            for _ in range(count + 1 + (1 if has_this else 0)):
                pop()
            kill()
            if returns_value:
                push(V_UNKNOWN, ("native", "calli"))
            return ([(None, state)], None)

        # --- control flow --------------------------------------------------------
        if base in ("br", "br.s"):
            return ([(offsets.get(branch_target(operand)), state)], None)
        if base in ("leave", "leave.s"):
            state.stack = []
            return ([(offsets.get(branch_target(operand)), state)], None)
        if base.split(".")[0] in ("brtrue", "brfalse", "brinst", "brnull", "brzero"):
            value, origin = pop()
            taken_is_null = base.split(".")[0] in ("brfalse", "brnull", "brzero")
            taken, fallthrough = state.copy(), state.copy()
            if origin is not None and origin[0] == "isNullOrEmpty":
                # `IsNullOrEmpty(x) == false` proves x is neither null nor
                # empty, so x is NON-NULL on that branch. The `true` side
                # proves nothing: x may be null OR merely empty, and treating
                # it as null would be the mirror-image over-approximation.
                tested = origin[1]
                if taken_is_null:
                    refine_origin(taken, tested, V_NONNULL)
                else:
                    refine_origin(fallthrough, tested, V_NONNULL)
            else:
                refine_origin(taken, origin, V_NULL if taken_is_null else V_NONNULL)
                refine_origin(
                    fallthrough, origin, V_NONNULL if taken_is_null else V_NULL)
            return ([(offsets.get(branch_target(operand)), taken),
                     (None, fallthrough)], None)
        stem = base[:-2] if base.endswith(".s") else base
        if stem.split(".")[0] in ("beq", "bne", "bge", "bgt", "ble", "blt"):
            right, left = pop(), pop()
            taken, fallthrough = state.copy(), state.copy()
            if stem in EQUALITY_BRANCH:
                tested = None
                if right[0] == V_NULL and left[1] is not None:
                    tested = left
                elif left[0] == V_NULL and right[1] is not None:
                    tested = right
                if tested is not None:
                    taken_is_null = stem == "beq"
                    refine_origin(taken, tested[1],
                                  V_NULL if taken_is_null else V_NONNULL)
                    refine_origin(fallthrough, tested[1],
                                  V_NONNULL if taken_is_null else V_NULL)
            return ([(offsets.get(branch_target(operand)), taken),
                     (None, fallthrough)], None)
        if base == "switch":
            pop()
            targets = [
                offsets.get(int(item, 16))
                for item in TARGET.findall(accessor.uncommented(operand))
            ]
            return ([(target, state.copy()) for target in targets]
                    + [(None, state)], None)

        raise Undecodable(f"unmodelled opcode {opcode}")


def record_store(stores, reference, value, method, offset) -> None:
    site = f"{method.owner}::{method.name} IL_{offset:04x}"
    previous = stores.get(reference)
    if previous is None:
        stores[reference] = (value, site)
        return
    merged = join(previous[0], value)
    keep = previous[1]
    if value[0] & BIT_NULL:
        keep = site if not (previous[0][0] & BIT_NULL) else min(previous[1], site)
    stores[reference] = (merged, keep)


def branch_target(operand: str) -> int:
    match = TARGET.search(accessor.uncommented(operand))
    if match is None:
        raise Undecodable("branch without a target")
    return int(match.group(1), 16)


def declared_returns_value(method: accessor.Method) -> bool:
    if method.name in (".ctor", ".cctor"):
        return False
    body = audit.strip_directives(method.header[len(".method"):].strip())
    split = audit.split_call(body)
    if split is None:
        return True
    head, _, _ = split
    head, _ = audit.split_generic_suffix(head)
    tokens = head.split()
    if not tokens:
        return True
    rest = [item for item in tokens[:-1] if item not in METHOD_FLAGS]
    return not (rest and rest[0] == "void")


# --- contract scope ----------------------------------------------------------


def type_kinds(contract: dict[str, Any]) -> dict[str, str]:
    return {item["name"]: item["kind"] for item in contract["types"]}


def classify_return_type(clr: str | None, kinds: dict[str, str]) -> tuple[bool, str | None]:
    """(is a CLR reference, exclusion reason when it is not)."""
    if clr is None or clr == "System.Void":
        return False, NOT_REFERENCE
    if re.fullmatch(r"!!?\d+", clr):
        return False, NOT_REFERENCE_GENERIC
    if clr.endswith("[]"):
        return True, None
    if clr in VALUE_BCL:
        return False, NOT_REFERENCE
    if clr in REFERENCE_BCL:
        return True, None
    if clr.startswith(VALUE_BCL_GENERIC_PREFIXES):
        return False, NOT_REFERENCE
    if clr.startswith(REFERENCE_BCL_GENERIC_PREFIXES):
        return True, None
    kind = kinds.get(clr)
    if kind is not None:
        return (kind in ("class", "interface")), (
            None if kind in ("class", "interface") else NOT_REFERENCE)
    raise SystemExit(
        f"return type {clr!r} is neither a contract type nor a known BCL "
        "spelling; classify it explicitly rather than guessing")


def method_parameter_types(method: accessor.Method,
                           type_parameters: tuple[str, ...]) -> tuple[str, ...] | None:
    body = audit.strip_directives(method.header[len(".method"):].strip())
    split = audit.split_call(body)
    if split is None:
        return None
    head, parameter_text, _ = split
    _, method_generics = audit.split_generic_suffix(head)
    return tuple(
        item["type"] for item in
        audit.split_parameters(parameter_text, type_parameters, method_generics)
    )


def select_overload(candidates: list[accessor.Method], expected: tuple[str, ...],
                    static: bool, type_parameters: tuple[str, ...]
                    ) -> accessor.Method | None:
    if not candidates:
        return None
    if len(candidates) == 1:
        return candidates[0]
    exact = [
        item for item in candidates
        if item.static == static
        and method_parameter_types(item, type_parameters) == expected
    ]
    if len(exact) == 1:
        return exact[0]
    return None


def field_lifecycle_evidence(analysis: Nullness, reference: tuple[str, str]) -> list[str]:
    return list(analysis.field_evidence.get(reference, []))


class Inventory:
    def __init__(self, contract: dict[str, Any], assemblies, graph, analysis,
                 fallibility: dict[tuple[str, str, str], dict[str, Any]]) -> None:
        self.contract = contract
        self.assemblies = assemblies
        self.graph = graph
        self.analysis = analysis
        self.kinds = type_kinds(contract)
        self.fallibility = fallibility
        self.coverage: collections.Counter = collections.Counter()
        self.unresolved: list[str] = []
        self.override_raised: list[str] = []
        self.fallibility_override_disagreements: list[str] = []
        self.selected: dict[tuple[str, str, tuple[str, ...]], accessor.Method] = {}
        self.entries = self.build()

    # -- one member ------------------------------------------------------
    def value_and_evidence(self, il_name: str, method_name: str, arity: int,
                           candidate: accessor.Method | None) -> dict[str, Any]:
        analysis = self.analysis
        if candidate is None:
            return {"verdict": UNKNOWN, "evidence": EVIDENCE_UNRESOLVED,
                    "value": V_UNKNOWN, "sites": [], "implementors": [],
                    "fieldLifecycle": []}
        token = id(candidate)
        if candidate.abstract or not candidate.has_body:
            if not candidate.abstract:
                return {"verdict": UNKNOWN, "evidence": EVIDENCE_NO_BODY,
                        "value": V_UNKNOWN, "sites": [], "implementors": [],
                        "fieldLifecycle": []}
            implementors = analysis.graph.implementors(il_name, method_name, arity)
            value, contributors = analysis.member_value(
                il_name, method_name, arity, candidate)
            if not contributors:
                return {"verdict": UNKNOWN, "evidence": EVIDENCE_NO_IMPLEMENTOR,
                        "value": V_UNKNOWN, "sites": [], "implementors": [],
                        "fieldLifecycle": []}
            return {
                "verdict": verdict_of(value), "evidence": EVIDENCE_ABSTRACT,
                "value": value, "sites": [],
                "implementors": [accessor.render(item) for item in implementors],
                "fieldLifecycle": [],
            }
        if token in analysis.undecodable or token not in analysis.decoded:
            return {"verdict": UNKNOWN, "evidence": EVIDENCE_UNDECODABLE,
                    "value": V_UNKNOWN, "sites": [], "implementors": [],
                    "fieldLifecycle": []}
        own = analysis.summary[token]
        value, _ = analysis.member_value(il_name, method_name, arity, candidate)
        if verdict_of(value) != verdict_of(own):
            self.override_raised.append(
                f"{il_name}::{method_name}/{arity} {verdict_of(own)} -> "
                f"{verdict_of(value)} through a registered override")
        _, _, _, sites = analysis.run(token, explain=True)
        lifecycle: list[str] = []
        for _, _, origin in sites:
            if origin and origin.startswith(("field ", "static field ")):
                name = origin.split(" ", 1)[1].split("::")
                if len(name) == 2:
                    lifecycle.extend(field_lifecycle_evidence(analysis, (name[0], name[1])))
        evidence = EVIDENCE_FIELD if lifecycle else EVIDENCE_RETURN
        return {
            "verdict": verdict_of(value), "evidence": evidence, "value": value,
            "sites": [
                {"offset": offset, "verdict": site_verdict, "origin": origin}
                for offset, site_verdict, origin in sorted(set(sites))
            ],
            "implementors": [],
            "fieldLifecycle": sorted(set(lifecycle)),
        }

    def build(self) -> list[dict[str, Any]]:
        entries: list[dict[str, Any]] = []
        for source_type in self.contract["types"]:
            il_name = source_type["name"].replace("+", "/")
            assembly = accessor.declaring_assembly(self.assemblies, il_name)
            type_parameters = tuple(
                item["name"] for item in source_type.get("genericParameters", [])
            )
            for member in source_type["members"]:
                entry = self.member(source_type, il_name, assembly,
                                    type_parameters, member)
                if entry is not None:
                    entries.append(entry)
        return entries

    def member(self, source_type, il_name, assembly, type_parameters, member):
        kind = member["kind"]
        if kind == "constructor":
            self.coverage[NOT_REFERENCE_CONSTRUCTOR] += 1
            return None
        if kind == "event":
            self.coverage[NOT_REFERENCE_EVENT] += 1
            return None
        if kind == "property" and not member.get("get"):
            self.coverage[NOT_REFERENCE_WRITE_ONLY] += 1
            return None
        clr = member.get("returnType") if kind == "method" else member.get("type")
        reference, exclusion = classify_return_type(clr, self.kinds)
        if not reference:
            self.coverage[exclusion] += 1
            return None
        self.coverage["REFERENCE_RETURN"] += 1

        parameters = tuple(
            item["type"] for item in member.get("parameters", [])
        )
        common = {
            "type": source_type["name"],
            "member": member["name"],
            "static": bool(member.get("static")),
            "clrReturnType": clr,
            "assembly": assembly.name if assembly else None,
            "assemblySha256": assembly.sha256 if assembly else None,
        }
        if kind == "field":
            reference_key = (il_name, member["name"])
            value = self.analysis.field_value(reference_key)
            evidence = field_lifecycle_evidence(self.analysis, reference_key)
            if reference_key not in self.analysis.field_stores and \
                    reference_key not in self.analysis.field_constructed:
                self.unresolved.append(f"{source_type['name']}.{member['name']} field")
            return {
                **common, "kind": "field", "parameters": [],
                "nullability": {
                    "verdict": verdict_of(value),
                    "evidence": EVIDENCE_FIELD,
                    "normalNullPossible": bool(value[0] & BIT_NULL),
                    "normalNonNullPossible": bool(value[0] & BIT_NONNULL),
                    "outcomesExhaustive": bool(value[1]),
                    "fieldLifecycle": sorted(set(evidence)),
                    "returnSites": [],
                    "implementors": [],
                },
                "fallibility": {
                    "fallible": False,
                    "evidence": "CLR_FIELD_READ_CANNOT_FAIL",
                    "chain": [], "exceptions": [],
                },
            }

        indexed = bool(member.get("parameters")) and kind == "property"
        if kind == "property":
            names = accessor.accessor_names(self.assemblies, il_name, member["name"])
            method_name = names.get("get") or f"get_{member['name']}"
            arity = len(member.get("parameters", []))
        else:
            method_name = member["name"]
            arity = len(member.get("parameters", []))
        candidates = self.graph.methods.get((il_name, method_name, arity), [])
        candidate = select_overload(candidates, parameters,
                                    bool(member.get("static")), type_parameters)
        if candidate is not None:
            self.selected[(source_type["name"], member["name"], parameters)] = candidate
        if candidate is None:
            self.unresolved.append(
                f"{source_type['name']}.{member['name']}/{arity} "
                f"{'getter' if kind == 'property' else 'method'}")
        nullability = self.value_and_evidence(il_name, method_name, arity, candidate)
        value = nullability["value"]
        self.record_override_disagreement(il_name, method_name, arity, candidate,
                                          source_type["name"], member["name"])
        if kind == "property":
            verdict = self.fallibility.get(
                (source_type["name"], member["name"], "getter"))
            failure = {
                "fallible": bool(verdict and verdict["fallible"]),
                "evidence": (verdict or {}).get("evidence", EVIDENCE_UNRESOLVED),
                "chain": (verdict or {}).get("chain", []),
                "exceptions": (verdict or {}).get("exceptions", []),
            }
        else:
            failure = self.method_fallibility(il_name, method_name, arity)
        return {
            **common,
            "kind": "propertyGetter" if kind == "property" else "method",
            "indexed": indexed,
            "parameters": list(parameters),
            "ilMethod": method_name,
            "nullability": {
                "verdict": nullability["verdict"],
                "evidence": nullability["evidence"],
                "normalNullPossible": bool(value[0] & BIT_NULL),
                "normalNonNullPossible": bool(value[0] & BIT_NONNULL),
                "outcomesExhaustive": bool(value[1]),
                "returnSites": nullability["sites"],
                "implementors": nullability["implementors"],
                "fieldLifecycle": nullability["fieldLifecycle"],
            },
            "fallibility": failure,
        }

    def method_fallibility(self, il_name: str, method_name: str, arity: int) -> dict[str, Any]:
        verdict = accessor.classify(self.graph, il_name, method_name, arity)
        return {
            "fallible": bool(verdict["fallible"]),
            "evidence": verdict["evidence"],
            "chain": verdict.get("chain", []),
            "exceptions": verdict.get("exceptions", []),
        }

    def record_override_disagreement(self, il_name, method_name, arity, candidate,
                                     type_name, member_name) -> None:
        """Registered overrides that are fallible while the declaration is not.

        Swift forbids an `override` from adding `throws` to a non-throwing
        requirement, so this is the fallibility analogue of the nullability
        join above. Foundation 22 decided a member with a body from that body
        alone; this measures what that costs rather than assuming it costs
        nothing.
        """
        if candidate is None or not candidate.has_body or not candidate.virtual:
            return
        if (il_name, method_name, arity) in self.graph.fallible:
            return
        for override in self.graph._overrides((il_name, method_name, arity)):
            if override in self.graph.fallible:
                self.fallibility_override_disagreements.append(
                    f"{type_name}.{member_name}: infallible but "
                    f"{accessor.render(override)} is fallible")


# --- fallibility input -------------------------------------------------------


def load_fallibility() -> tuple[dict[tuple[str, str, str], dict[str, Any]], str]:
    """The pinned per-accessor fallibility verdicts and their digest.

    Getter fallibility is not recomputed here: Foundation 22 already derived it
    from the same seven assemblies and pinned it, and one answer with one
    provenance is worth more than two that could drift.
    """
    raw = FALLIBILITY.read_bytes()
    document = json.loads(raw.decode("utf-8"))
    table: dict[tuple[str, str, str], dict[str, Any]] = {}
    for item in document["entries"]:
        for kind in ("getter", "setter"):
            table[(item["type"], item["property"], kind)] = item[kind]
    return table, hashlib.sha256(raw).hexdigest()


def fallibility_precision(graph, assemblies, inventory) -> list[str]:
    """In-scope methods whose fallibility the arity merge does not decide.

    The group verdict this inventory records is compared against the two
    per-overload closures that bracket a signature-precise analysis. Where they
    agree, the precise answer is known and the merge provably changed nothing;
    where they do not, the member is named rather than trusted.
    """
    bounds = accessor.overload_bounds(graph, assemblies)
    ordinal_of = {
        id(method): key for key, method in bounds["overloads"].items()
    }
    result: list[str] = []
    for entry in inventory.entries:
        if entry["kind"] != "method":
            continue
        method = inventory.selected.get(
            (entry["type"], entry["member"], tuple(entry["parameters"])))
        if method is None or not method.has_body:
            continue
        overload = ordinal_of.get(id(method))
        group = (entry["type"].replace("+", "/"), entry["ilMethod"],
                 len(entry["parameters"]))
        if overload is None or group not in graph.methods:
            continue
        precise = overload in bounds["lower"]
        if precise != (overload in bounds["upper"]):
            result.append(f"{entry['type']}.{entry['member']} bounds disagree")
        elif precise != (group in graph.fallible):
            result.append(
                f"{entry['type']}.{entry['member']} arity merge says "
                f"{group in graph.fallible}, signature-precise says {precise}")
    return sorted(set(result))


# --- self-tests --------------------------------------------------------------


def lattice_checks(expect) -> None:
    values = [V_NULL, V_NONNULL, V_UNKNOWN, V_BOTTOM]
    for left in values:
        expect(join(left, left) == left, "join must be idempotent")
        expect(join(left, V_BOTTOM) == left, "bottom must be the join identity")
        for right in values:
            expect(join(left, right) == join(right, left),
                   "join must be commutative")
            for third in values:
                expect(join(join(left, right), third) ==
                       join(left, join(right, third)),
                       "join must be associative")
    expect(verdict_of(V_NULL) == PROVEN_NULLABLE, "ldnull is proven nullable")
    expect(verdict_of(V_NONNULL) == PROVEN_NONNULL, "newobj is proven non-null")
    expect(verdict_of(V_UNKNOWN) == UNKNOWN, "an inexact value is unknown")
    expect(verdict_of(join(V_NULL, V_NONNULL)) == PROVEN_NULLABLE,
           "one nullable path makes the whole return nullable")
    expect(verdict_of(join(V_NONNULL, V_UNKNOWN)) == UNKNOWN,
           "one unproven path costs the non-null proof")
    expect(verdict_of(join(V_NULL, V_UNKNOWN)) == PROVEN_NULLABLE,
           "an unproven path cannot undo a null witness")
    expect(verdict_join(PROVEN_NONNULL, UNKNOWN) == UNKNOWN and
           verdict_join(UNKNOWN, PROVEN_NULLABLE) == PROVEN_NULLABLE and
           verdict_join(PROVEN_NONNULL, PROVEN_NULLABLE) == PROVEN_NULLABLE,
           "the verdict order is NONNULL < UNKNOWN < NULLABLE")


def find(entries, type_name, member, kind=None):
    for item in entries:
        if item["type"] == type_name and item["member"] == member and (
                kind is None or item["kind"] == kind):
            return item
    return None


def self_test(inventory: Inventory, analysis: Nullness, graph, assemblies,
              contract) -> tuple[int, list[str]]:
    checks = 0
    failures: list[str] = []

    def expect(condition: bool, message: str) -> None:
        nonlocal checks
        checks += 1
        if not condition:
            failures.append(message)

    lattice_checks(expect)
    entries = inventory.entries

    # --- the canonical proof: null is not an error -------------------------
    manager = find(entries, "Microsoft.Xna.Framework.GraphicsDeviceManager",
                   "GraphicsDevice")
    expect(manager is not None,
           "GraphicsDeviceManager.GraphicsDevice must be in scope")
    if manager:
        expect(manager["nullability"]["verdict"] == PROVEN_NULLABLE,
               "GraphicsDeviceManager.GraphicsDevice must be proven nullable")
        expect(not manager["fallibility"]["fallible"],
               "GraphicsDeviceManager.GraphicsDevice getter must be infallible")
        expect(any("Dispose" in item for item in
                   manager["nullability"]["fieldLifecycle"]),
               "the nullable verdict must name the store that nulls the field")
        expect(any("no constructor" in item for item in
                   manager["nullability"]["fieldLifecycle"]),
               "the nullable verdict must record that no constructor assigns it")

    # --- nullable and fallible are independent -----------------------------
    game = find(entries, "Microsoft.Xna.Framework.Game", "GraphicsDevice")
    expect(game is not None and
           game["nullability"]["verdict"] == PROVEN_NULLABLE and
           game["fallibility"]["fallible"],
           "Game.GraphicsDevice must be both nullable and fallible")
    collection = find(
        entries, "Microsoft.Xna.Framework.Graphics.EffectParameterCollection",
        "Item")
    expect(collection is not None and
           collection["nullability"]["verdict"] == PROVEN_NULLABLE and
           not collection["fallibility"]["fallible"],
           "EffectParameterCollection.Item must be nullable and infallible")

    # --- the four-way matrix must be populated, not assumed ----------------
    matrix = collections.Counter(
        (item["nullability"]["verdict"], bool(item["fallibility"]["fallible"]))
        for item in entries
    )
    expect(matrix[(PROVEN_NULLABLE, False)] > 0,
           "the contract must exercise nullable and infallible")
    expect(matrix[(PROVEN_NULLABLE, True)] > 0,
           "the contract must exercise nullable and fallible")
    expect(matrix[(PROVEN_NONNULL, False)] > 0,
           "the contract must exercise non-null and infallible")
    expect(matrix[(PROVEN_NONNULL, True)] > 0,
           "the contract must exercise non-null and fallible")

    # --- static singletons -------------------------------------------------
    opaque = find(entries, "Microsoft.Xna.Framework.Graphics.BlendState", "Opaque")
    expect(opaque is not None and
           opaque["nullability"]["verdict"] == PROVEN_NONNULL,
           "a static singleton the type initialiser constructs is non-null")

    # --- the scope must reconcile with the contract -------------------------
    total = sum(len(item["members"]) for item in contract["types"])
    expect(sum(inventory.coverage.values()) == total,
           f"every one of the {total} contract members must be classified")
    expect(all(
        (item["nullability"]["verdict"] == PROVEN_NULLABLE) ==
        item["nullability"]["normalNullPossible"] for item in entries),
        "a nullable verdict and a proven null outcome must be the same fact")

    # --- the arity-merge approximation must be immaterial -------------------
    imprecise = fallibility_precision(graph, assemblies, inventory)
    expect(not imprecise,
           "the arity merge leaves "
           f"{len(imprecise)} method fallibility verdict(s) undecided: "
           f"{imprecise[:5]}")

    checks += mutation_tests(assemblies, graph, failures)
    return checks, failures


def rebuild(assemblies, edit) -> tuple[Any, Any]:
    clone = copy_module.deepcopy(assemblies)
    edit(clone)
    graph = accessor.CallGraph(clone)
    return Nullness(clone, graph), graph


def summary_for(analysis, owner, name, arity, virtual=False) -> str:
    methods = analysis.methods.get((owner, name, arity), [])
    if not methods:
        return EVIDENCE_UNRESOLVED
    if virtual or any(item.abstract for item in methods):
        return verdict_of(analysis.call_value(owner, name, arity, True))
    return verdict_of(analysis.summary[id(methods[0])])


def body_of(clone, key):
    return [method for assembly in clone
            for method in assembly.methods.get(key, [])]


def mutation_tests(assemblies, graph, failures: list[str]) -> int:
    """Disable each ingredient of a verdict and require the verdict to change."""
    checks = 0

    collection = ("Microsoft.Xna.Framework.Graphics.EffectParameterCollection",
                  "get_Item", 1)

    def drop_null(clone) -> None:
        """Substitute, never delete: deleting a push would only unbalance the
        stack, and a verdict must change because the evidence changed."""
        for method in body_of(clone, collection):
            method.body = [
                re.sub(r"(:\s*)ldnull\b", r'\1ldstr      "x"',
                       accessor.uncommented(line))
                for line in method.body
            ]
    checks += 1
    analysis, _ = rebuild(assemblies, drop_null)
    if summary_for(analysis, *collection) == PROVEN_NULLABLE:
        failures.append(
            "removing the ldnull must stop EffectParameterCollection.Item "
            "being proven nullable")

    manager = "Microsoft.Xna.Framework.GraphicsDeviceManager"

    def initialise_field(clone) -> None:
        """Assign the field in the constructor and never null it again."""
        for method in body_of(clone, (manager, ".ctor", 1)):
            method.body = list(method.body[:-1]) + [
                "    IL_9000:  ldarg.0",
                "    IL_9001:  newobj     instance void "
                "[Microsoft.Xna.Framework.Graphics]"
                "Microsoft.Xna.Framework.Graphics.GraphicsDevice::.ctor()",
                "    IL_9002:  stfld      "
                "[Microsoft.Xna.Framework.Graphics]"
                "Microsoft.Xna.Framework.Graphics.GraphicsDevice "
                "Microsoft.Xna.Framework.GraphicsDeviceManager::device",
                "    IL_9003:  ret",
            ]
        for assembly in clone:
            for key, methods in assembly.methods.items():
                if key[0] != manager or key[1] == ".ctor":
                    continue
                for method in methods:
                    method.body = [
                        re.sub(r"(:\s*)ldnull\b",
                               r"\1newobj     instance void "
                               "[Microsoft.Xna.Framework.Graphics]"
                               "Microsoft.Xna.Framework.Graphics."
                               "GraphicsDevice::.ctor()",
                               accessor.uncommented(line))
                        for line in method.body
                    ]
    checks += 1
    analysis, _ = rebuild(assemblies, initialise_field)
    if summary_for(analysis, manager, "get_GraphicsDevice", 0) == PROVEN_NULLABLE:
        failures.append(
            "a constructor-initialised field that nothing nulls must stop "
            "GraphicsDeviceManager.GraphicsDevice being proven nullable")

    def null_the_singleton(clone) -> None:
        for method in body_of(clone, ("Microsoft.Xna.Framework.Graphics.BlendState",
                                      ".cctor", 0)):
            method.body = [
                re.sub(r"(:\s*)newobj\s+.*$", r"\1ldnull",
                       accessor.uncommented(line))
                if re.search(r":\s*newobj\b", accessor.uncommented(line)) else line
                for line in method.body
            ]
    checks += 1
    analysis, _ = rebuild(assemblies, null_the_singleton)
    if analysis.field_value(("Microsoft.Xna.Framework.Graphics.BlendState",
                            "Opaque")) != V_NULL:
        failures.append(
            "a type initialiser that stores null must make the singleton nullable")

    interface = ("Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService",
                 "get_GraphicsDevice", 0)
    checks += 1
    if summary_for(Nullness(assemblies, graph), *interface, virtual=True) != \
            PROVEN_NULLABLE:
        failures.append(
            "an interface member must inherit its registered implementor's "
            "nullable return")

    def silence_implementors(clone) -> None:
        for assembly in clone:
            for key, methods in assembly.methods.items():
                if key[1] != "get_GraphicsDevice":
                    continue
                for method in methods:
                    if not method.has_body:
                        continue
                    method.body = [
                        "    IL_0000:  newobj     instance void "
                        "[Microsoft.Xna.Framework.Graphics]"
                        "Microsoft.Xna.Framework.Graphics.GraphicsDevice::.ctor()",
                        "    IL_0005:  ret",
                    ]
    checks += 1
    analysis, _ = rebuild(assemblies, silence_implementors)
    if summary_for(analysis, *interface, virtual=True) != PROVEN_NONNULL:
        failures.append(
            "an interface member must become non-null when every registered "
            "implementor does")

    # `String.IsNullOrEmpty` is a null test spelled as a call, and the branch
    # that follows tests its RESULT. Without carrying the argument's origin
    # through the call, a field read on the `false` branch -- which is reached
    # only when the field is not null -- is reported nullable although it
    # demonstrably cannot be. `ContentSerializerAttribute.CollectionItemName`
    # is exactly that shape, and it is the member the refinement was found on.
    guarded = ("Microsoft.Xna.Framework.Content.ContentSerializerAttribute",
               "get_CollectionItemName", 0)
    checks += 1
    if summary_for(Nullness(assemblies, graph), *guarded) != PROVEN_NONNULL:
        failures.append(
            "a field read guarded by String.IsNullOrEmpty must be proven "
            "non-null on the branch the guard cannot reach with null")

    def defeat_the_guard(clone) -> None:
        """Rename the guard so the refinement no longer recognises it.

        The call keeps its exact shape -- one string argument, a bool result --
        so the body stays well formed and the branch still consumes what the
        call pushed; only the recognition is removed. The verdict must then
        stop being a proof, which is what shows the proof came from the guard
        and not from somewhere else.
        """
        for method in body_of(clone, guarded):
            method.body = [
                accessor.uncommented(line).replace(
                    "IsNullOrEmpty", "IsNullOrEmptyMutated")
                for line in method.body
            ]
    checks += 1
    analysis, _ = rebuild(assemblies, defeat_the_guard)
    if summary_for(analysis, *guarded) == PROVEN_NONNULL:
        failures.append(
            "removing the IsNullOrEmpty guard must stop "
            "ContentSerializerAttribute.CollectionItemName being proven "
            "non-null")

    # The mirror image, which the refinement must NOT claim: `IsNullOrEmpty`
    # returning true means null OR empty, so nothing about nullness is proven
    # on that branch. `GraphicsResource.ToString` returns the guarded value on
    # the false branch and `Object.ToString()` on the true one, so it must be
    # UNKNOWN -- never nullable, and never a non-null proof either.
    stringly = ("Microsoft.Xna.Framework.Graphics.GraphicsResource",
                "ToString", 0)
    checks += 1
    if summary_for(Nullness(assemblies, graph), *stringly) != UNKNOWN:
        failures.append(
            "IsNullOrEmpty being TRUE proves nothing about nullness, so "
            "GraphicsResource.ToString must stay unproven")
    return checks


# --- report ------------------------------------------------------------------


POLICY = {
    PROVEN_NULLABLE:
        "a normal return of null is demonstrated by the CIL: an ldnull that "
        "reaches a ret, a field the lifecycle leaves null at a reachable "
        "public read, or a callee whose own return is proven nullable",
    PROVEN_NONNULL:
        "every reachable ret yields a value whose non-nullness is "
        "demonstrated, and the set of outcomes is exact",
    UNKNOWN:
        "neither is demonstrated inside the registered assemblies; a call "
        "that leaves them, an isinst, an array element, a native or "
        "mixed-mode boundary and an abstract member with no registered "
        "implementor all land here",
    EVIDENCE_RETURN: "the verdict comes from this member's own returned values",
    EVIDENCE_FIELD:
        "a returned value is a field read, so the field's whole construction "
        "and store lifecycle decides it",
    EVIDENCE_ABSTRACT:
        "an abstract or interface member declares no body, so it is decided "
        "by joining its registered implementors",
    EVIDENCE_NO_IMPLEMENTOR:
        "an abstract member with no registered implementor proves nothing",
    EVIDENCE_NO_BODY:
        "the member declares no managed CIL body: native, runtime-implemented "
        "or mixed-mode, and not evidence of anything",
    EVIDENCE_UNDECODABLE:
        "the body used a shape the interpreter does not model; recorded as a "
        "hard error rather than resolved by guessing",
    EVIDENCE_UNRESOLVED:
        "the contract declares a member the IL does not; a hard error",
    "CLR_FIELD_READ_CANNOT_FAIL":
        "reading a field executes no user code and has no failure path",
}


def counters(entries: list[dict[str, Any]], coverage) -> dict[str, Any]:
    by_kind = collections.Counter(item["kind"] for item in entries)
    by_verdict = collections.Counter(
        item["nullability"]["verdict"] for item in entries)
    matrix = collections.Counter(
        (item["nullability"]["verdict"], bool(item["fallibility"]["fallible"]))
        for item in entries
    )
    return {
        "REFERENCE_RETURN_POSITIONS": len(entries),
        "REFERENCE_RETURN_PROPERTY_GETTERS": by_kind["propertyGetter"],
        "REFERENCE_RETURN_METHODS": by_kind["method"],
        "REFERENCE_RETURN_FIELDS": by_kind["field"],
        "REFERENCE_RETURN_STATIC": sum(1 for item in entries if item["static"]),
        "REFERENCE_RETURN_INSTANCE": sum(
            1 for item in entries if not item["static"]),
        "PROVEN_NONNULL_RETURNS": by_verdict[PROVEN_NONNULL],
        "PROVEN_NULLABLE_RETURNS": by_verdict[PROVEN_NULLABLE],
        "UNKNOWN_REFERENCE_RETURNS": by_verdict[UNKNOWN],
        "NULLABLE_INFALLIBLE": matrix[(PROVEN_NULLABLE, False)],
        "NULLABLE_FALLIBLE": matrix[(PROVEN_NULLABLE, True)],
        "NONNULL_INFALLIBLE": matrix[(PROVEN_NONNULL, False)],
        "NONNULL_FALLIBLE": matrix[(PROVEN_NONNULL, True)],
        "UNKNOWN_INFALLIBLE": matrix[(UNKNOWN, False)],
        "UNKNOWN_FALLIBLE": matrix[(UNKNOWN, True)],
        "CLASSIFIED_MEMBERS": sum(coverage.values()),
        "coverage": dict(sorted(coverage.items())),
    }


def render_markdown(report: dict[str, Any], projection) -> str:
    """The reviewable form: the proven-nullable rows and the deferrals in full."""
    lines = [
        "# XNA 4.0 reference-return nullability inventory",
        "",
        "Generated by `tools/api_compat/return_nullability.py` from the CIL of the",
        "hash-registered assemblies. Every row is re-derivable from that IL; no",
        "machine-local path is recorded. The pinned machine-readable form is",
        "`tools/api_compat/reference/xna40-reference-return-nullability.json`, which",
        "carries CLR facts only -- the Swift column below is the projection those",
        "facts imply under `tools/api_compat/mapping-rules.json`.",
        "",
        "| Measure | Value |",
        "|---|---:|",
    ]
    for key in ("REFERENCE_RETURN_POSITIONS", "REFERENCE_RETURN_PROPERTY_GETTERS",
                "REFERENCE_RETURN_METHODS", "REFERENCE_RETURN_FIELDS",
                "REFERENCE_RETURN_STATIC", "REFERENCE_RETURN_INSTANCE",
                "PROVEN_NONNULL_RETURNS", "PROVEN_NULLABLE_RETURNS",
                "UNKNOWN_REFERENCE_RETURNS", "NULLABLE_INFALLIBLE",
                "NULLABLE_FALLIBLE", "NONNULL_INFALLIBLE", "NONNULL_FALLIBLE",
                "UNKNOWN_INFALLIBLE", "UNKNOWN_FALLIBLE", "CLASSIFIED_MEMBERS"):
        lines.append(f"| {key} | {report[key]} |")
    lines.append(f"| Self-tests | {report['SELF_TESTS']} "
                 f"{report['SELF_TEST_STATUS']} |")
    lines += ["", "## Registered assemblies", "", "| Assembly | SHA-256 |",
              "|---|---|"]
    for item in report["assemblies"]:
        lines.append(f"| `{item['assembly']}` | `{item['sha256']}` |")
    lines += ["", "## Members not in scope", "",
              "Every contract member is classified; these carry no reference",
              "return at all.", "", "| Category | Members |", "|---|---:|"]
    for key, value in sorted(report["coverage"].items()):
        lines.append(f"| `{key}` | {value} |")
    lines += ["", "## Evidence vocabulary", "", "| Label | Meaning |", "|---|---|"]
    for label, meaning in sorted(report["policy"].items()):
        lines.append(f"| `{label}` | {meaning} |")
    for verdict, title in (
        (PROVEN_NULLABLE, "Proven nullable returns"),
        (UNKNOWN, "Unproven returns, projected non-Optional and named here"),
    ):
        lines += ["", f"## {title}", "",
                  "| Type | Member | Kind | CLR return | Swift | Throws | Evidence |",
                  "|---|---|---|---|---|---|---|"]
        for item in report["entries"]:
            if item["nullability"]["verdict"] != verdict:
                continue
            detail = item["nullability"]["fieldLifecycle"] or [
                f"{site['offset']} {site['origin'] or 'literal'}"
                for site in item["nullability"]["returnSites"]
                if site["verdict"] != PROVEN_NONNULL
            ]
            lines.append(
                f"| `{item['type']}` | `{item['member']}` | {item['kind']} | "
                f"`{item['clrReturnType']}` | `{projection(item)}` | "
                f"{'yes' if item['fallibility']['fallible'] else 'no'} | "
                f"`{item['nullability']['evidence']}`"
                f"{': ' + '; '.join(detail[:2]) if detail else ''} |")
    return "\n".join(lines) + "\n"


def swift_projection(report: dict[str, Any]):
    """The Swift return shape an entry's CLR facts imply, for the markdown.

    The pinned JSON stays free of any Swift spelling so that it is a function
    of the contract and the seven binaries alone; the human-readable inventory
    is allowed to show what the mapping rules then make of it.
    """
    import verify  # local import: the pinned inventory must not depend on it

    rules = verify.load_json(verify.RULES)

    def render(item: dict[str, Any]) -> str:
        mapped = verify.map_clr_type(item["clrReturnType"], rules)
        if item["nullability"]["verdict"] == PROVEN_NULLABLE and \
                not mapped.endswith("?"):
            mapped += "?"
        if item["fallibility"]["fallible"]:
            mapped += " throws" if item["kind"] == "method" else " { get throws }"
        return mapped
    return render


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

    assemblies: list[accessor.Assembly] = []
    for name, path in sorted(paths.items()):
        digest = accessor.sha256(path)
        if digest != expected[name]:
            raise SystemExit(
                f"{name} sha256 {digest} does not match registered {expected[name]}")
        cache = (args.il_cache / f"{path.stem}.il") if args.il_cache else None
        assemblies.append(
            accessor.Assembly(name, digest, accessor.disassemble(path, cache)))

    graph = accessor.CallGraph(assemblies)
    analysis = Nullness(assemblies, graph)
    fallibility, fallibility_digest = load_fallibility()
    inventory = Inventory(contract, assemblies, graph, analysis, fallibility)
    checks, failures = self_test(inventory, analysis, graph, assemblies, contract)

    undecodable = sorted({
        accessor.render(analysis.method_key[token])
        for token in analysis.undecodable
    })
    report = {
        "schemaVersion": 1,
        "referenceSha256": hashlib.sha256(REFERENCE.read_bytes()).hexdigest(),
        "accessorFallibilitySha256": fallibility_digest,
        "assemblies": [
            {"assembly": item.name, "sha256": item.sha256} for item in assemblies
        ],
        "policy": POLICY,
        "SELF_TESTS": checks,
        "SELF_TEST_STATUS": "FAIL" if failures else "PASS",
        "selfTestFailures": failures,
        **counters(inventory.entries, inventory.coverage),
        "UNRESOLVED_MEMBERS": sorted(set(inventory.unresolved)),
        "UNDECODABLE_BODIES": undecodable,
        "OVERRIDE_RAISED_VERDICTS": sorted(set(inventory.override_raised)),
        "FALLIBILITY_OVERRIDE_DISAGREEMENTS": sorted(
            set(inventory.fallibility_override_disagreements)),
        "FIXPOINT_ROUNDS": analysis.rounds,
        "entries": inventory.entries,
    }
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if args.markdown:
        args.markdown.parent.mkdir(parents=True, exist_ok=True)
        args.markdown.write_text(
            render_markdown(report, swift_projection(report)), encoding="utf-8")

    print(f"REFERENCE_RETURN_POSITIONS={report['REFERENCE_RETURN_POSITIONS']} "
          f"GETTERS={report['REFERENCE_RETURN_PROPERTY_GETTERS']} "
          f"METHODS={report['REFERENCE_RETURN_METHODS']} "
          f"FIELDS={report['REFERENCE_RETURN_FIELDS']}")
    print(f"PROVEN_NONNULL={report['PROVEN_NONNULL_RETURNS']} "
          f"PROVEN_NULLABLE={report['PROVEN_NULLABLE_RETURNS']} "
          f"UNKNOWN={report['UNKNOWN_REFERENCE_RETURNS']}")
    print(f"NULLABLE_INFALLIBLE={report['NULLABLE_INFALLIBLE']} "
          f"NULLABLE_FALLIBLE={report['NULLABLE_FALLIBLE']} "
          f"NONNULL_INFALLIBLE={report['NONNULL_INFALLIBLE']} "
          f"NONNULL_FALLIBLE={report['NONNULL_FALLIBLE']} "
          f"UNKNOWN_INFALLIBLE={report['UNKNOWN_INFALLIBLE']} "
          f"UNKNOWN_FALLIBLE={report['UNKNOWN_FALLIBLE']}")
    print(f"RETURN_NULLABILITY_SELF_TESTS={checks} "
          f"STATUS={report['SELF_TEST_STATUS']}")
    print(f"UNRESOLVED_MEMBERS={len(report['UNRESOLVED_MEMBERS'])} "
          f"UNDECODABLE_BODIES={len(undecodable)} "
          f"OVERRIDE_RAISED_VERDICTS={len(report['OVERRIDE_RAISED_VERDICTS'])} "
          "FALLIBILITY_OVERRIDE_DISAGREEMENTS="
          f"{len(report['FALLIBILITY_OVERRIDE_DISAGREEMENTS'])}")
    for line in failures + report["UNRESOLVED_MEMBERS"]:
        print(f"  {line}", file=sys.stderr)
    return 1 if (failures or report["UNRESOLVED_MEMBERS"]) else 0


if __name__ == "__main__":
    sys.exit(main())
