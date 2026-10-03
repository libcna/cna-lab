#!/usr/bin/env python3
"""Every user-visible message an implemented member can raise is accounted for.

Foundations 48, 49 and 50 each found a member that was implemented, tested and
green while disagreeing with its pinned IL, and each was found the same way: by
reading the IL rather than by a failing test. Three times is a pattern, and a
pattern deserves a gate instead of a fourth careful reading.

The rule: for every member this binding implements, every resource key its
pinned IL can reach must either have its message reproduced in the Swift
sources, or be listed in `recorded-message-absences.json` with a reason. A key
that is neither is a finding.

Scope is derived, not hand-written. `docs/generated/api-compat-report.json`
names the COMPLETE and PARTIAL types and, for the partial ones, the members
that are missing; everything else in those types is implemented. Types whose IL
is not in the cache are reported as UNRESOLVED rather than silently counted as
clean -- an unmeasured type is not a passing one.

Reachability follows XNA's own calls. `SpriteBatch.Draw` raises nothing itself;
it forwards to the private `InternalDraw`, which is where
`BeginMustBeCalledBeforeDraw` lives. So the walk starts at each implemented
member and follows calls to other methods **of the same class**, to a bounded
depth, which is what a caller can actually reach.

Where the tables live is not obvious and getting it wrong is silent:
`Microsoft.Xna.Framework.Graphics.dll` has NO embedded string table at all, and
its `Microsoft.Xna.Framework.Graphics.Resources` resolves against
`Microsoft.Xna.Framework.dll`'s. The first draft of this gate looked each key
up in its own assembly and reported "no gaps" for the whole Graphics assembly,
SpriteBatch's three included. All tables are searched, and a run that finds no
table at all fails rather than passing vacuously.
"""

from __future__ import annotations

import argparse
from collections import deque
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/api_compat"))

REPORT = ROOT / "docs/generated/api-compat-report.json"
CONTRACT = ROOT / "tools/api_compat/reference/xna40-windows-runtime-contract.json"
ABSENCES = ROOT / "tools/api_compat/recorded-message-absences.json"
SOURCES = ROOT / "Sources"

IL_STEMS = [
    "Microsoft.Xna.Framework-38e7093f52d7474b.il",
    "Microsoft.Xna.Framework.Game-b5dffdd8125abef2.il",
    "Microsoft.Xna.Framework.Graphics-560080fc39021c61.il",
]
ASSEMBLIES = [
    "Microsoft.Xna.Framework.dll",
    "Microsoft.Xna.Framework.Game.dll",
    "Microsoft.Xna.Framework.Graphics.dll",
]

MAX_CALL_DEPTH = 3

# Names the IL declaration parser must never mistake for a method name.
NOT_A_NAME = {
    "instance", "void", "managed", "cil", "valuetype", "class", "string",
    "modopt", "modreq", "marshal", "native", "unsigned", "int", "float",
    "bool", "char", "object", "specialname", "rtspecialname",
}

METHOD_HEAD = re.compile(r"^  \.method [^\n]*\n(?:\s{10}[^\n]*\n)*?\s*\{\n", re.M)


def method_name(head: str) -> str | None:
    """The declared name, taken from the signature rather than guessed.

    The name is the identifier immediately before the parameter list of the
    declaration -- not the last identifier before any `(`, which on a signature
    carrying `modopt(...)` or `marshal(...)` is the modifier's own name.
    """
    cut = head.find("cil managed")
    signature = head[:cut if cut > 0 else len(head)]
    depth = 0
    for index in range(len(signature) - 1, -1, -1):
        char = signature[index]
        if char == ")":
            depth += 1
        elif char == "(":
            depth -= 1
            if depth == 0:
                before = signature[:index]
                # A generic method's declaration is `Name<T>(...)`; drop the
                # parameter list so the name is not read as `T>`.
                before = re.sub(r"<[^<>]*>\s*$", "", before)
                match = re.search(r"([A-Za-z_][A-Za-z0-9_.`]*)\s*$", before)
                if not match:
                    return None
                name = match.group(1).rsplit(".", 1)[-1]
                name = name.split("`")[0]
                return None if name in NOT_A_NAME else name
    return None


def argument_count(text: str) -> int:
    """Count top-level IL signature arguments without splitting generic types."""
    value = text.strip()
    if not value:
        return 0
    depths = {"(": 0, "[": 0, "<": 0}
    closing = {")": "(", "]": "[", ">": "<"}
    count = 1
    for character in value:
        if character in depths:
            depths[character] += 1
        elif character in closing:
            opening = closing[character]
            depths[opening] = max(0, depths[opening] - 1)
        elif character == "," and not any(depths.values()):
            count += 1
    return count


def method_arity(head: str) -> int | None:
    """Return the arity of the declaration parameter list in `head`."""
    cut = head.find("cil managed")
    signature = head[:cut if cut > 0 else len(head)]
    close = signature.rfind(")")
    if close < 0:
        return None
    depth = 0
    for index in range(close, -1, -1):
        character = signature[index]
        if character == ")":
            depth += 1
        elif character == "(":
            depth -= 1
            if depth == 0:
                return argument_count(signature[index + 1:close])
    return None


def class_body(texts: dict[str, str], full: str) -> str | None:
    for text in texts.values():
        for opening in (f"beforefieldinit {full}\n", f"ansi {full}\n",
                        f"beforefieldinit {full} ", f"ansi sealed {full}\n"):
            start = text.find(opening)
            if start >= 0:
                end = text.find(f"}} // end of class {full}", start)
                if end > start:
                    return text[start:end]
    return None


def methods_of(body: str) -> dict[tuple[str, int], list[str]]:
    """Every method body in a class, keyed by declared name and arity.

    A name-only key merged private overloads into public members. In
    ContentTypeReaderManager that made public `GetTypeReader(Type)` appear to
    reach the unrelated manifest-only reflective-reader failure.
    """
    out: dict[tuple[str, int], list[str]] = {}
    for match in METHOD_HEAD.finditer(body):
        head = " ".join(match.group(0).split())
        name = method_name(head)
        arity = method_arity(head)
        if not name or arity is None:
            continue
        end = body.find("} // end of method", match.end())
        out.setdefault((name, arity), []).append(body[match.end():end])
    return out


def called_methods(segment: str) -> set[tuple[str, int]]:
    result: set[tuple[str, int]] = set()
    pattern = re.compile(r"::(\.?[A-Za-z_][A-Za-z0-9_]*)(?:<[^<>(]*>)?\(")
    for match in pattern.finditer(segment):
        opening = match.end() - 1
        depth = 0
        close = None
        for index in range(opening, len(segment)):
            character = segment[index]
            if character == "(":
                depth += 1
            elif character == ")":
                depth -= 1
                if depth == 0:
                    close = index
                    break
        if close is None:
            continue
        name = match.group(1).lstrip(".")
        result.add((name, argument_count(segment[opening + 1:close])))
    return result


def reachable_keys(
    methods: dict[tuple[str, int], list[str]],
    start: tuple[str, int] | str,
) -> set[str]:
    """Resource keys raised by `start` or by what it calls within its class.

    A call site spells a GENERIC method as `Type::CopyData<!!0>(`, with the
    instantiation between the name and the parenthesis. A pattern that required
    the parenthesis to follow the name immediately therefore saw no call at all,
    and every message reachable only through a generic helper went unread: the
    whole `CopyData<T>` family of `Texture2D`, `TextureCube`, `Texture3D`,
    `VertexBuffer` and `IndexBuffer` -- which is where texture and buffer
    transfer validation lives. The instantiation is now skipped over.
    """
    seen: set[tuple[str, int]] = set()
    keys: set[str] = set()
    starts = ([key for key in methods if key[0] == start]
              if isinstance(start, str) else [start])
    # Breadth-first traversal is contract-significant here. A method can be
    # reached both directly and through a longer helper chain; depth-first
    # traversal used to mark whichever path happened to come first in a set,
    # so Python's hash seed could make the longer path consume the method at
    # MAX_CALL_DEPTH and hide one of its callees. BFS visits every method at
    # its shortest depth, and sorting makes the generated report byte-stable.
    frontier = deque((key, 0) for key in sorted(starts))
    while frontier:
        method, depth = frontier.popleft()
        if method in seen or depth > MAX_CALL_DEPTH:
            continue
        seen.add(method)
        for segment in methods.get(method, []):
            keys.update(re.findall(r"Resources::get_(\w+)", segment))
            if depth < MAX_CALL_DEPTH:
                for callee in sorted(called_methods(segment)):
                    if callee in methods and callee not in seen:
                        frontier.append((callee, depth + 1))
    return keys


def analyse(report, contract, absences, table, texts, literals, tables_read):
    """The whole measurement, as a function of its inputs, so the mutation
    controls can perturb one input at a time rather than edit this file."""
    KINDS = {"unreachable", "native-owned", "deferred"}
    absent: dict[str, dict[str, str]] = {}
    for entry in absences:
        if entry["kind"] not in KINDS:
            return {}, [], [], f"unknown absence kind {entry['kind']!r}"
        absent.setdefault(entry["type"], {})[entry["key"]] = entry["kind"]

    contract_types = (contract["types"] if isinstance(contract.get("types"), list)
                      else list(contract["types"].values()))
    contract_members: dict[str, set[tuple[str, int]]] = {}
    for entry in contract_types:
        name = entry.get("name") or entry.get("fullName")
        names: set[tuple[str, int]] = set()
        for member in entry.get("members", []):
            kind = member.get("kind")
            member_name = member.get("name", "")
            arity = len(member.get("parameters", []))
            if kind == "property":
                names.add((f"get_{member_name}", arity))
                if member.get("set"):
                    names.add((f"set_{member_name}", arity + 1))
            elif kind == "event":
                names.update({(f"add_{member_name}", 1), (f"remove_{member_name}", 1)})
            elif kind == "constructor" or member_name == ".ctor":
                names.add(("ctor", arity))
            else:
                names.add((member_name, arity))
        contract_members[name] = names

    missing_members: dict[str, set[str]] = {}
    for diagnostic in report["diagnostics"]:
        if diagnostic["category"] != "MISSING_MEMBER":
            continue
        owner, _, member = diagnostic["subject"].split("(")[0].rpartition(".")
        # A constructor's subject is `Type..ctor`, so the owner keeps a
        # trailing dot and matched no type until this was noticed: every
        # constructor in the contract was being treated as implemented.
        owner = owner.rstrip(".")
        missing_members.setdefault(owner, set()).add(member)

    scope = list(report["completeTypes"]) + list(report["partialTypes"])
    unresolved: list[str] = []
    members_read = 0
    findings: list[dict[str, str]] = []
    recorded = {"unreachable": 0, "native-owned": 0, "deferred": 0}
    reproduced = 0

    for full in scope:
        body = class_body(texts, full)
        if body is None:
            unresolved.append(full)
            continue
        methods = methods_of(body)
        gone = missing_members.get(full, set())
        declared = contract_members.get(full, set())
        for method in methods:
            name, _arity = method
            base = name[4:] if name.startswith(("get_", "set_")) else name
            if method not in declared:
                continue
            if name in gone or base in gone:
                continue
            members_read += 1
            for key in sorted(reachable_keys(methods, method)):
                value = table.get(key)
                if value is None:
                    continue
                if value in literals:
                    reproduced += 1
                elif key in absent.get(full, {}):
                    recorded[absent[full][key]] += 1
                else:
                    findings.append({"type": full, "member": name, "key": key})

    # A gate that reads nothing must not report PASS. These floors are the
    # difference between "measured and clean" and "measured nothing at all".
    if not scope:
        return {}, [], [], "the report named no implemented type"
    if members_read == 0:
        return {}, [], [], "no implemented member was read"
    if len(unresolved) == len(scope):
        return {}, [], [], "no type resolved to a disassembly"

    summary = {
        "MESSAGE_COVERAGE_TYPES": len(scope),
        "RESOLVED_TYPES": len(scope) - len(unresolved),
        "UNRESOLVED_TYPES": len(unresolved),
        "STRING_TABLES_READ": tables_read,
        "MEMBERS_READ": members_read,
        "MESSAGES_REPRODUCED": reproduced,
        "MESSAGES_UNREACHABLE": recorded["unreachable"],
        "MESSAGES_NATIVE_OWNED": recorded["native-owned"],
        "MESSAGES_DEFERRED": recorded["deferred"],
        "MESSAGE_COVERAGE_FINDINGS": len(findings),
    }
    return summary, findings, unresolved, None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--assembly-dir", type=Path, required=True)
    parser.add_argument("--il-cache", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()

    import pinned_assembly_audit as audit
    import verify as verifier

    if args.self_test:
        return self_test()

    table: dict[str, str] = {}
    tables_read = 0
    for name in ASSEMBLIES:
        path = args.assembly_dir / name
        if not path.exists():
            continue
        blob = audit.embedded_resource(path.read_bytes(), "")
        if blob is None:
            continue
        tables_read += 1
        for key, value in audit.resource_strings(blob).items():
            table.setdefault(key, value)
    if not table:
        print("MESSAGE_COVERAGE=UNMEASURED no embedded string table could be read")
        return 1

    texts = {}
    for stem in IL_STEMS:
        path = args.il_cache / stem
        if path.exists():
            texts[stem] = path.read_text(errors="replace")
    if not texts:
        print("MESSAGE_COVERAGE=UNMEASURED no disassembly could be read")
        return 1

    report = json.loads(REPORT.read_text(encoding="utf-8"))
    literals = verifier.source_string_literals(sorted(SOURCES.rglob("*.swift")))
    if literals is None:
        print("MESSAGE_COVERAGE=UNMEASURED the Swift sources could not be read")
        return 1
    absences = json.loads(ABSENCES.read_text(encoding="utf-8"))["absences"]
    contract = json.loads(CONTRACT.read_text(encoding="utf-8"))

    if args.mutations:
        return mutations(report, contract, absences, table, texts, literals,
                         tables_read)

    summary, findings, unresolved, error = analyse(
        report, contract, absences, table, texts, literals, tables_read)
    if error:
        print(f"MESSAGE_COVERAGE=UNMEASURED {error}")
        return 1

    print(" ".join(f"{k}={v}" for k, v in summary.items()))
    for finding in findings:
        print(f"  FINDING {finding['type']}.{finding['member']} "
              f"raises {finding['key']}, whose message is neither reproduced "
              f"nor recorded")
    for name in unresolved:
        print(f"  UNRESOLVED {name}")

    if args.output:
        args.output.write_text(json.dumps({
            "schemaVersion": 1,
            "profile": "messages reachable from implemented members",
            "summary": summary,
            "findings": findings,
            "unresolved": unresolved,
        }, indent=2) + "\n", encoding="utf-8")

    print("MESSAGE_COVERAGE_STATUS=" + ("PASS" if not findings else "FAIL"))
    return 1 if findings else 0


def mutations(report, contract, absences, table, texts, literals,
              tables_read) -> int:
    """The gate proved to fail. Every planted defect must be detected.

    A coverage gate is the easiest kind to write so that it always passes:
    read no members, resolve no types, look in the wrong string table. Each of
    those is planted here, because each of them has already happened once --
    the first draft of this gate reported "no gaps" for the whole Graphics
    assembly through a single wrong assumption about where its string table
    lives.
    """
    import copy

    def clean():
        return analyse(report, contract, absences, table, texts, literals,
                       tables_read)

    base_summary, base_findings, _unresolved, base_error = clean()
    if base_error or base_findings:
        print(f"MESSAGE_COVERAGE_MUTATION_BASELINE=RED {base_error or base_findings}")
        return 1

    results: list[tuple[str, str, bool]] = []

    # A message the projection reproduces, removed from the sources.
    a_reproduced = "Begin must be called successfully before End can be called."
    if a_reproduced not in literals:
        print("MUTATION_SITE=STALE the sample reproduced message is not in the sources")
        return 1
    thinned = {value for value in literals if value != a_reproduced}
    summary, findings, _u, error = analyse(
        report, contract, absences, table, texts, thinned, tables_read)
    results.append(("reproduced-message-deleted",
                    "a message the projection reproduces, removed",
                    bool(findings) or bool(error)))

    # An absence entry removed: the key must resurface as a finding.
    fewer = [a for a in absences if a["key"] != "ServiceTypeCannotBeNull"]
    summary, findings, _u, error = analyse(
        report, contract, fewer, table, texts, literals, tables_read)
    results.append(("absence-entry-removed",
                    "a recorded absence deleted from the list",
                    bool(findings) or bool(error)))

    # An absence with an invented kind.
    bogus = copy.deepcopy(absences)
    if bogus:
        bogus[0] = dict(bogus[0]); bogus[0]["kind"] = "someday"
    summary, findings, _u, error = analyse(
        report, contract, bogus, table, texts, literals, tables_read)
    results.append(("absence-kind-invented",
                    "an absence carrying a kind the gate does not define",
                    bool(error)))

    # The Graphics assembly's table missing -- the bug the first draft had.
    without_graphics = {k: v for k, v in table.items()
                        if k not in {"EndMustBeCalledBeforeBegin",
                                     "BeginMustBeCalledBeforeEnd",
                                     "BeginMustBeCalledBeforeDraw"}}
    summary, findings, _u, error = analyse(
        report, contract, absences, without_graphics, texts, literals,
        tables_read)
    detected = bool(error) or (
        summary and summary["MESSAGES_REPRODUCED"] < base_summary["MESSAGES_REPRODUCED"])
    results.append(("string-table-partially-missing",
                    "keys the projection reproduces absent from the tables",
                    detected))

    # No table at all.
    summary, findings, _u, error = analyse(
        report, contract, absences, {}, texts, literals, tables_read)
    detected = bool(error) or (summary and summary["MESSAGES_REPRODUCED"] == 0)
    results.append(("string-table-empty", "no string table read at all", detected))

    # No disassembly: every type unresolved.
    summary, findings, _u, error = analyse(
        report, contract, absences, {}, {}, literals, tables_read)
    results.append(("disassembly-missing", "no class body resolves",
                    bool(error)))

    # An empty contract: nothing is a start point, so nothing is read.
    summary, findings, _u, error = analyse(
        report, {"types": []}, absences, table, texts, literals, tables_read)
    results.append(("contract-emptied", "no member qualifies as a start point",
                    bool(error)))

    # The call walk stopped at depth zero: a key raised only by a private
    # callee stops being reproduced, which must show in the count.
    global MAX_CALL_DEPTH
    saved = MAX_CALL_DEPTH
    try:
        MAX_CALL_DEPTH = 0
        summary, findings, _u, error = analyse(
            report, contract, absences, table, texts, literals, tables_read)
        detected = bool(error) or (
            summary and summary["MESSAGES_REPRODUCED"] < base_summary["MESSAGES_REPRODUCED"])
    finally:
        MAX_CALL_DEPTH = saved
    results.append(("call-walk-disabled",
                    "keys reachable only through a private callee",
                    detected))

    # The call walk blind to a GENERIC callee -- the shape the walk had until
    # Foundation 64, which silently dropped every `CopyData<T>` family and with
    # them all texture and buffer transfer validation.
    renamed = {stem: re.sub(r"::(\w+)<[^<>(]*>\(", r"::\1_stripped(", text)
               for stem, text in texts.items()}
    summary, findings, _u, error = analyse(
        report, contract, absences, table, renamed, literals, tables_read)
    detected = bool(error) or (
        summary and summary["MESSAGES_REPRODUCED"] < base_summary["MESSAGES_REPRODUCED"])
    results.append(("generic-call-sites-unreadable",
                    "keys reachable only through a generic callee",
                    detected))

    after_summary, after_findings, _u, after_error = clean()
    if after_error or after_findings or after_summary != base_summary:
        results.append(("harness-restores-its-inputs",
                        "the measurement after the mutations differs", False))
    else:
        results.append(("harness-restores-its-inputs",
                        "the measurement is unchanged afterwards", True))

    survivors = [name for name, _d, detected in results if not detected]
    for name, description, detected in results:
        print(f"{'CAUGHT' if detected else 'SURVIVED':9} {name:34} {description}")
    print(f"MESSAGE_COVERAGE_MUTATIONS={len(results)} "
          f"CAUGHT={len(results) - len(survivors)} SURVIVORS={len(survivors)}")
    return 1 if survivors else 0


def self_test() -> int:
    """The parser and the walk, on inputs whose answers are known."""
    failures: list[str] = []

    cases = [
        (".method public hidebysig instance void Begin(valuetype X sortMode) cil managed {",
         "Begin"),
        (".method public hidebysig specialname instance int32 get_Width() cil managed {",
         "get_Width"),
        (".method private hidebysig instance void InternalDraw(class T t, "
         "valuetype V& v) cil managed {", "InternalDraw"),
        # The shape the first draft answered "marshal" and "modopt" for.
        (".method public hidebysig instance uint32 modopt([mscorlib]System.Runtime."
         "CompilerServices.IsLong) Read(int32 modopt([mscorlib]System.Runtime."
         "CompilerServices.IsLong) count) cil managed {", "Read"),
        (".method public hidebysig specialname rtspecialname instance void "
         ".ctor(class Game game) cil managed {", "ctor"),
    ]
    for head, expected in cases:
        actual = method_name(" ".join(head.split()))
        if actual != expected:
            failures.append(f"method_name({expected!r}) answered {actual!r}")

    body = """
  .method public hidebysig instance void Draw() cil managed
  {
    IL_0000:  call instance void Namespace.Type::InternalDraw()
  } // end of method Type::Draw
  .method private hidebysig instance void InternalDraw() cil managed
  {
    IL_0000:  call string Resources::get_BeginMustBeCalledBeforeDraw()
  } // end of method Type::InternalDraw
"""
    methods = methods_of(body)
    if set(methods) != {("Draw", 0), ("InternalDraw", 0)}:
        failures.append(f"methods_of found {sorted(methods)}")
    if reachable_keys(methods, "Draw") != {"BeginMustBeCalledBeforeDraw"}:
        failures.append("a key raised by a private callee is not reachable from "
                        "the public member that calls it")
    if reachable_keys(methods, "InternalDraw") != {"BeginMustBeCalledBeforeDraw"}:
        failures.append("a key raised directly is not reachable")

    # A cycle must terminate rather than recurse forever.
    cyclic = methods_of("""
  .method public hidebysig instance void A() cil managed
  {
    IL_0000:  call instance void T::B()
  } // end of method T::A
  .method public hidebysig instance void B() cil managed
  {
    IL_0000:  call instance void T::A()
    IL_0005:  call string Resources::get_Key()
  } // end of method T::B
""")
    if reachable_keys(cyclic, "A") != {"Key"}:
        failures.append("a call cycle is not walked correctly")

    # A GENERIC callee, spelled the way a call site spells one. This is the
    # shape the walk could not see until Foundation 64: `CopyData<!!0>(` put an
    # instantiation between the name and the parenthesis, so no call was found
    # and every `CopyData<T>` message went unread.
    generic = methods_of("""
  .method public hidebysig instance void SetData<valuetype .ctor T>(!!T[] data) cil managed
  {
    IL_0000:  call instance void T::CopyData<!!0>(!!0[], int32, bool)
  } // end of method T::SetData
  .method private hidebysig instance void CopyData<valuetype .ctor T>(!!T[] data,
                                                                      int32 startIndex,
                                                                      bool isSetting) cil managed
  {
    IL_0000:  call string Resources::get_NullNotAllowed()
  } // end of method T::CopyData
""")
    if set(generic) != {("SetData", 1), ("CopyData", 3)}:
        failures.append(f"methods_of found {sorted(generic)} for generic methods")
    if reachable_keys(generic, "SetData") != {"NullNotAllowed"}:
        failures.append("a key raised by a GENERIC callee is not reachable from "
                        "the public member that calls it")

    overloaded = methods_of("""
  .method public hidebysig instance object GetTypeReader(class Type targetType) cil managed
  {
    IL_0000:  call object T::GetTypeReader(class Type, class Reader)
  } // end of method T::GetTypeReader
  .method assembly hidebysig static object GetTypeReader(class Type targetType,
                                                          class Reader reader) cil managed
  {
    IL_0000:  call string Resources::get_TypeReaderNotRegistered()
  } // end of method T::GetTypeReader
  .method private hidebysig static object GetTypeReader(string name,
                                                         class Reader reader,
                                                         class List readers) cil managed
  {
    IL_0000:  call bool T::InstantiateTypeReader(string, class Reader)
  } // end of method T::GetTypeReader
  .method private hidebysig static bool InstantiateTypeReader(string name,
                                                               class Reader reader) cil managed
  {
    IL_0000:  call string Resources::get_ReflectiveReaderTypeNotFound()
  } // end of method T::InstantiateTypeReader
""")
    if reachable_keys(overloaded, ("GetTypeReader", 1)) != {
        "TypeReaderNotRegistered"
    }:
        failures.append("same-named private overload leaked a resource key into "
                        "the selected public overload")

    print(f"MESSAGE_COVERAGE_SELF_TESTS={len(cases) + 7} "
          f"MESSAGE_COVERAGE_SELF_TEST_STATUS={'PASS' if not failures else 'FAIL'}")
    for failure in failures:
        print(f"  {failure}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
