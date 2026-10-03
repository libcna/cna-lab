#!/usr/bin/env python3
"""Strict compiled-metadata verifier for the normative XNA-to-Java projection."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
PROFILE = ROOT / "tools/api-compat/profiles/xna40-windows-runtime.json"
RULES = ROOT / "tools/api-compat/mapping-rules.json"
EXTRACTOR = ROOT / "tools/api-compat/extractor/XnaContractExtractor.cs"
CLASS_READER = ROOT / "tools/api-compat/java/ClassContractReader.java"

PRIMITIVES = {
    "System.Void": "void",
    "System.Boolean": "boolean",
    "System.Byte": "int",
    "System.SByte": "byte",
    "System.Int16": "short",
    "System.UInt16": "int",
    "System.Int32": "int",
    "System.UInt32": "long",
    "System.Int64": "long",
    "System.UInt64": "long",
    "System.Single": "float",
    "System.Double": "double",
    "System.Char": "char",
    "System.String": "java.lang.String",
    "System.Object": "java.lang.Object",
    "System.Exception": "java.lang.RuntimeException",
    "System.TimeSpan": "java.time.Duration",
    "System.Type": "java.lang.Class<?>",
}

OPERATOR_METHODS = {
    "op_Addition": "Add",
    "op_Subtraction": "Subtract",
    "op_Multiply": "Multiply",
    "op_Division": "Divide",
    "op_UnaryNegation": "Negate",
}

GENERIC_RENAMES: dict[str, str] = {}
TYPE_RENAMES: dict[str, str] = {}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--reference-dir", default=os.environ.get("XNA_REFERENCE_PATH"))
    parser.add_argument("--target", required=True, help="Class directory/JAR, or os.pathsep-separated inputs")
    parser.add_argument("--profile", default=str(PROFILE))
    parser.add_argument("--mapping-rules", default=str(RULES))
    parser.add_argument("--format", choices=("text", "json"), default="text")
    parser.add_argument("--output")
    parser.add_argument("--report-only", action="store_true")
    parser.add_argument("--summary-only", action="store_true")
    parser.add_argument("--leak-only", action="store_true")
    return parser.parse_args()


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(command, check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def read_target(target: str, temporary: Path) -> dict[str, Any]:
    classes = temporary / "reader-classes"
    classes.mkdir()
    run(["javac", "--release", "17", "-parameters", "-Xlint:all", "-Werror",
         "-d", str(classes), str(CLASS_READER)])
    output = temporary / "target.json"
    inputs = [item for item in target.split(os.pathsep) if item]
    run(["java", "-cp", str(classes),
         "org.openeggbert.cna.tools.apicompat.ClassContractReader", str(output), *inputs])
    return json.loads(output.read_text(encoding="utf-8"))


def read_reference(reference_dir: str, profile: dict[str, Any], temporary: Path) -> dict[str, Any]:
    validate_reference_files(reference_dir, profile)
    executable = temporary / "XnaContractExtractor.exe"
    run(["mcs", "-warnaserror+", "-r:System.Core", "-r:System.Web.Extensions",
         "-out:" + str(executable), str(EXTRACTOR)])
    output = temporary / "reference.json"
    run(["mono", str(executable), reference_dir, str(output), *profile["referenceAssemblies"]])
    return json.loads(output.read_text(encoding="utf-8"))


def validate_reference_files(reference_dir: str, profile: dict[str, Any]) -> None:
    root = Path(reference_dir)
    expected_hashes = profile.get("referenceSha256", {})
    for name in profile["referenceAssemblies"]:
        assembly = root / name
        if not assembly.is_file():
            raise FileNotFoundError(f"XNA reference assembly is missing: {assembly}")
        expected = expected_hashes.get(name)
        if expected is None:
            raise ValueError(f"XNA profile has no SHA-256 identity for {name}")
        actual = hashlib.sha256(assembly.read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError(
                f"XNA reference assembly SHA-256 mismatch for {name}: "
                f"expected {expected}, actual {actual}")


def diagnostic(code: str, subject: str, expected: Any = None, actual: Any = None) -> dict[str, Any]:
    value: dict[str, Any] = {"code": code, "subject": subject}
    if expected is not None:
        value["expected"] = expected
    if actual is not None:
        value["actual"] = actual
    return value


def leak_diagnostics(target: dict[str, Any]) -> list[dict[str, Any]]:
    findings: list[dict[str, Any]] = []
    banned = ("org.openeggbert.cna.internal", "java.lang.foreign.MemorySegment", "jdk.incubator.foreign")
    for type_contract in target["types"]:
        type_name = type_contract["name"]
        for relationship in [type_contract.get("baseType"), *type_contract.get("interfaces", [])]:
            if relationship and any(value in relationship for value in banned):
                findings.append(diagnostic("CNA_INTERNAL_LEAK", type_name, actual=relationship))
        for member in type_contract["members"]:
            values = [member.get("type"), member.get("returnType")]
            values.extend(parameter.get("type") for parameter in member.get("parameters", []))
            subject = f"{type_name}.{member['name']}"
            leaked = next((value for value in values if value and any(item in value for item in banned)), None)
            suspicious_name = "native" in member["name"].lower()
            raw_handle = "handle" in member["name"].lower() and any(value == "long" for value in values)
            if leaked or suspicious_name or raw_handle:
                findings.append(diagnostic("CNA_INTERNAL_LEAK", subject, actual=leaked or member["name"]))
    return sorted(findings, key=lambda item: (item["code"], item["subject"]))


def strip_arity(name: str) -> str:
    return re.sub(r"`\d+", "", name)


def split_generic(value: str) -> tuple[str, list[str]] | None:
    bracket = value.find("[")
    if bracket < 0 or not value.endswith("]"):
        return None
    base = value[:bracket]
    body = value[bracket + 1:-1]
    arguments: list[str] = []
    start = 0
    depth = 0
    for index, character in enumerate(body):
        if character == "[":
            depth += 1
        elif character == "]":
            depth -= 1
        elif character == "," and depth == 0:
            arguments.append(body[start:index])
            start = index + 1
    arguments.append(body[start:])
    return base, arguments


def boxed(value: str) -> str:
    return {
        "boolean": "java.lang.Boolean", "byte": "java.lang.Byte", "short": "java.lang.Short",
        "int": "java.lang.Integer", "long": "java.lang.Long", "float": "java.lang.Float",
        "double": "java.lang.Double", "char": "java.lang.Character",
    }.get(value, value)


def map_type(value: str | None, *, identity: bool = False) -> str | None:
    if value is None:
        return None
    if value.endswith("&"):
        return map_type(value[:-1], identity=identity)
    if value.endswith("[]"):
        return map_type(value[:-2], identity=identity) + "[]"
    if value.startswith("!"):
        return "T" if value == "!0" else "T" + value[1:]
    if value in PRIMITIVES:
        return PRIMITIVES[value]
    if value in TYPE_RENAMES:
        return TYPE_RENAMES[value]
    parsed = split_generic(value)
    if parsed:
        base, arguments = parsed
        mapped_arguments = [boxed(map_type(argument) or "java.lang.Object") for argument in arguments]
        collection = {
            "System.Collections.Generic.IEnumerable`1": "java.lang.Iterable",
            "System.Collections.Generic.IEnumerator`1": "java.util.Iterator",
            "System.Collections.Generic.ICollection`1": "java.util.Collection",
            "System.Collections.Generic.IList`1": "java.util.List",
            "System.Collections.Generic.List`1": "java.util.List",
            "System.Collections.Generic.IDictionary`2": "java.util.Map",
            "System.Collections.Generic.KeyValuePair`2": "java.util.Map.Entry",
            "System.Nullable`1": None,
        }
        if base == "System.Nullable`1":
            return mapped_arguments[0]
        mapped_base = collection.get(
            base,
            TYPE_RENAMES.get(base, GENERIC_RENAMES.get(base, strip_arity(base).replace("+", "."))),
        )
        return mapped_base + "<" + ",".join(mapped_arguments) + ">"
    mapped = GENERIC_RENAMES.get(value, strip_arity(value))
    if value == "System.IDisposable":
        return "java.lang.AutoCloseable"
    return mapped.replace("+", "$" if identity else ".")


def visible(access: str | None) -> bool:
    return access in ("public", "protected")


def parameter(name: str, type_name: str) -> dict[str, Any]:
    return {"name": name, "type": type_name, "out": False, "optional": False}


def callable_member(kind: str, name: str, source: dict[str, Any], parameters: list[dict[str, Any]], return_type: str | None) -> dict[str, Any]:
    java_final = source.get("final", False)
    if kind != "constructor" and not source.get("static", False) \
            and not source.get("abstract", False) and source.get("virtual") is False:
        java_final = True
    return {
        "kind": kind,
        "name": name,
        "access": source.get("access", "public"),
        "static": source.get("static", False),
        "abstract": source.get("abstract", False),
        "final": java_final,
        "genericArity": source.get("genericArity", 0),
        "returnType": return_type,
        "parameters": parameters,
    }


def clr_member_signature(type_name: str, member: dict[str, Any]) -> str:
    return type_name + "." + member["name"] + "(" \
        + ",".join(value["type"] for value in member.get("parameters", [])) + ")"


def map_parameters(
        values: list[dict[str, Any]],
        overrides: dict[str, str] | None = None,
        omitted_types: set[str] | None = None) -> list[dict[str, Any]]:
    selected = overrides or {}
    omitted = omitted_types or set()
    return [parameter(value["name"] or f"arg{index}",
                      selected.get(value["name"], map_type(value["type"]) or "java.lang.Object"))
            for index, value in enumerate(values) if value["type"] not in omitted]


def map_member_parameters(
        type_name: str,
        member: dict[str, Any],
        rules: dict[str, Any]) -> list[dict[str, Any]]:
    overrides = rules.get("memberParameterTypeMappings", {}).get(
        clr_member_signature(type_name, member), {})
    omitted = set(rules.get("omittedFrameworkParameterTypes", []))
    return map_parameters(member.get("parameters", []), overrides, omitted)


def map_member_return_type(
        type_name: str,
        member: dict[str, Any],
        rules: dict[str, Any]) -> str | None:
    override = rules.get("memberReturnTypeMappings", {}).get(
        clr_member_signature(type_name, member))
    return override if override is not None else map_type(member["returnType"])


def mapped_property_type(
        type_contract: dict[str, Any],
        member: dict[str, Any],
        rules: dict[str, Any]) -> str:
    mapped = map_type(member["type"]) or "java.lang.Object"
    for adaptation in rules.get("boxedGenericInterfaceProperties", []):
        if member["name"] != adaptation["property"]:
            continue
        for interface_name in type_contract.get("interfaces", []):
            parsed = split_generic(interface_name)
            if parsed and parsed[0] == adaptation["interface"]:
                return boxed(mapped)
    return mapped


def mapped_enum_members(type_contract: dict[str, Any]) -> list[dict[str, Any]]:
    fields = [value for value in type_contract["members"]
              if value["kind"] == "field" and value["name"] != "value__"]
    expected = [{**value, "type": map_type(value["type"])} for value in fields]
    underlying_member = next((value for value in type_contract["members"]
                              if value["kind"] == "field" and value["name"] == "value__"), None)
    underlying = map_type(underlying_member["type"] if underlying_member else "System.Int32") or "int"
    values = [int(value["constant"]) for value in fields]
    sequential = sorted(values) == list(range(len(values)))
    source = {"access": "public", "static": False, "abstract": False,
              "final": False, "genericArity": 0}
    if type_contract.get("flags"):
        type_name = map_type(type_contract["name"], identity=True) or type_contract["name"]
        expected.extend([
            callable_member("method", "getValue", source, [], underlying),
            callable_member("method", "FromValue", {**source, "static": True},
                            [parameter("value", underlying)], type_name),
            callable_member("method", "Or", source,
                            [parameter("other", type_name)], type_name),
            callable_member("method", "Contains", source,
                            [parameter("value", type_name)], "boolean"),
            callable_member("method", "equals", source,
                            [parameter("obj", "java.lang.Object")], "boolean"),
            callable_member("method", "hashCode", source, [], "int"),
        ])
    elif not sequential:
        expected.append(callable_member("method", "getValue", source, [], underlying))
    return expected


def mapped_members(type_contract: dict[str, Any], rules: dict[str, Any], mapping_findings: list[dict[str, Any]]) -> list[dict[str, Any]]:
    expected: list[dict[str, Any]] = []
    type_name = type_contract["name"]
    if type_contract["kind"] == "enum":
        return mapped_enum_members(type_contract)
    if type_contract["kind"] == "struct":
        empty = {"access": "public", "static": False, "abstract": False, "final": False, "genericArity": 0}
        # A struct's copy constructor takes its own type. For a generic struct that is the
        # type's own parameterization, which is the only form Java can write without a raw type.
        arity = type_contract.get("genericArity", 0)
        own = map_type(type_name) or type_name
        if arity:
            own += "<" + ",".join("T" if index == 0 else f"T{index}" for index in range(arity)) + ">"
        expected.append(callable_member("constructor", ".ctor", empty, [], None))
        expected.append(callable_member("constructor", ".ctor", empty,
                                        [parameter("value", own)], None))

    ordinary_methods = [value for value in type_contract["members"]
                        if value["kind"] == "method"
                        and not any(parameter_value["type"].endswith("&")
                                    for parameter_value in value.get("parameters", []))]

    serialization = tuple(rules.get("clrSerializationParameterTypes", []))
    for member in type_contract["members"]:
        kind = member["kind"]
        clr_signature = clr_member_signature(type_name, member)
        if clr_signature in rules.get("excludedClrMembers", []):
            continue
        # CLR binary serialization has no Java equivalent, and Java serialization is a
        # different contract with different security properties. Every member whose whole
        # parameter list is the (SerializationInfo, StreamingContext) pair is omitted by
        # one general rule rather than by a per-exception exclusion entry.
        if serialization and tuple(
                value["type"] for value in member.get("parameters", [])) == serialization:
            continue
        if kind == "field":
            expected.append({**member, "type": map_type(member["type"])})
        elif kind == "constructor":
            expected.append(callable_member("constructor", ".ctor", member,
                                            map_member_parameters(type_name, member, rules), None))
        elif kind == "property":
            indexes = map_member_parameters(type_name, member, rules)
            property_type = mapped_property_type(type_contract, member, rules)
            named_value_type = (type_name in rules.get("namedImmutableValuePropertyTypes", [])
                                and member.get("static", False)
                                and visible(member.get("getterAccess"))
                                and not visible(member.get("setterAccess")))
            if named_value_type:
                expected.append({"kind": "field", "name": member["name"], "access": "public",
                                 "type": property_type, "static": True, "final": True, "constant": None})
            elif visible(member.get("getterAccess")):
                getter_name = "get" if indexes else "get" + member["name"]
                source = {"access": member["getterAccess"], "static": member["static"],
                          "abstract": member.get("getterAbstract", type_contract["kind"] == "interface"),
                          "final": member.get("getterFinal", False),
                          "virtual": member.get("getterVirtual")}
                expected.append(callable_member("method", getter_name, source, indexes, property_type))
            if visible(member.get("setterAccess")):
                setter_name = "set" if indexes else "set" + member["name"]
                source = {"access": member["setterAccess"], "static": member["static"],
                          "abstract": member.get("setterAbstract", type_contract["kind"] == "interface"),
                          "final": member.get("setterFinal", False),
                          "virtual": member.get("setterVirtual")}
                setter_return = property_type \
                    if type_name in rules.get("javaListBridgeTypes", []) \
                    and member["name"] == "Item" and indexes else "void"
                expected.append(callable_member("method", setter_name, source,
                                                [*indexes, parameter("value", property_type)],
                                                setter_return))
        elif kind == "event":
            listener = map_type(member["type"]) or "java.lang.Object"
            if visible(member.get("addAccess")):
                source = {"access": member["addAccess"], "static": member["static"],
                          "abstract": member.get("addAbstract", type_contract["kind"] == "interface"),
                          "final": member.get("addFinal", False),
                          "virtual": member.get("addVirtual")}
                expected.append(callable_member("method", "add" + member["name"] + "Listener", source,
                                                [parameter("listener", listener)], "void"))
            if visible(member.get("removeAccess")):
                source = {"access": member["removeAccess"], "static": member["static"],
                          "abstract": member.get("removeAbstract", type_contract["kind"] == "interface"),
                          "final": member.get("removeFinal", False),
                          "virtual": member.get("removeVirtual")}
                expected.append(callable_member("method", "remove" + member["name"] + "Listener", source,
                                                [parameter("listener", listener)], "void"))
        elif kind == "method":
            name = rules.get("memberNameMappings", {}).get(
                clr_signature, member["name"])
            if name == "Finalize" and "System.Object.Finalize" in rules.get("excludedClrMethods", []):
                continue
            if name in ("op_Equality", "op_Inequality"):
                continue
            if name in ("op_Implicit", "op_Explicit"):
                adaptation = rules.get("conversionMemberMappings", {}).get(clr_signature)
                if adaptation is not None:
                    expected.append(callable_member(
                        "method", adaptation["name"], member,
                        map_parameters(
                            member.get("parameters", []),
                            adaptation.get("parameterTypeMappings")),
                        adaptation.get("returnType", map_type(member["returnType"]))))
                    continue
                mapping_findings.append(diagnostic("XNA_MAPPING_MISMATCH", type_name + "." + name,
                                                   expected="explicit conversion rule", actual="unclassified conversion"))
                continue
            name = OPERATOR_METHODS.get(name, name)
            # `Dispose()` keeps its XNA name. Java has no reserved meaning for it, and the
            # project's own identity rule is that an XNA member keeps its spelling whenever
            # Java syntax permits. `close()` is added separately as the AutoCloseable bridge,
            # so try-with-resources still works and both names are the same operation.
            name = {"Equals": "equals", "GetHashCode": "hashCode",
                    "ToString": "toString"}.get(name, name)
            if type_name == "Microsoft.Xna.Framework.Content.ContentManager" \
                    and name in ("Load", "ReadAsset") \
                    and member.get("genericArity") == 1:
                source = dict(member)
                source["genericArity"] = 1
                expected.append(callable_member("method", name, source,
                                                [parameter("assetType", "java.lang.Class<T>"),
                                                 *map_member_parameters(type_name, member, rules)], "T"))
                continue
            if type_name == "Microsoft.Xna.Framework.Content.ContentReader" \
                    and member.get("genericArity") == 1:
                source = dict(member)
                source["genericArity"] = 1
                expected.append(callable_member(
                    "method", name, source,
                    [parameter("targetType", "java.lang.Class<T>"),
                     *map_member_parameters(type_name, member, rules)],
                    map_member_return_type(type_name, member, rules)))
                continue
            by_reference = any(value["type"].endswith("&") for value in member["parameters"])
            if by_reference:
                adaptation = rules.get("refOutMemberMappings", {}).get(clr_signature)
                if adaptation is not None:
                    inputs = [value for value in member["parameters"] if not value.get("out")]
                    expected.append(callable_member(
                        "method", adaptation.get("name", name), member,
                        map_parameters(inputs, adaptation.get("parameterTypeMappings")),
                        adaptation["returnType"]))
                    continue
                out_values = [value for value in member["parameters"] if value.get("out")]
                inputs = [value for value in member["parameters"] if not value.get("out")]
                if len(out_values) == 1:
                    input_types = [map_type(value["type"]) for value in inputs]
                    output_type = map_type(out_values[0]["type"])
                    redundant = any(value["name"] == member["name"]
                                    and [map_type(parameter_value["type"])
                                         for parameter_value in value["parameters"]] == input_types
                                    and map_type(value["returnType"]) == output_type
                                    for value in ordinary_methods)
                    if redundant:
                        continue
                if not out_values:
                    expected.append(callable_member("method", name, member,
                                                    map_member_parameters(type_name, member, rules),
                                                    map_type(member["returnType"])))
                    continue
                if type_name == "Microsoft.Xna.Framework.Matrix" and member["name"] == "Decompose":
                    expected.append(callable_member("method", "Decompose", member, [],
                                                    "Microsoft.Xna.Framework.Matrix.Decomposition"))
                    continue
                signature = ",".join(value["type"] for value in member["parameters"])
                mapping_findings.append(diagnostic("XNA_MAPPING_MISMATCH",
                                                   type_name + "." + member["name"] + "(" + signature + ")",
                                                   expected="reviewed ref/out transformation", actual="unclassified ref/out overload"))
                continue
            expected.append(callable_member("method", name, member,
                                            map_member_parameters(type_name, member, rules),
                                            map_member_return_type(type_name, member, rules)))

    for member in type_contract.get("explicitInterfaceMethods", []):
        expected.append(callable_member(
            "method", member["name"], member,
            map_member_parameters(type_name, member, rules),
            map_type(member["returnType"])))

    # Every disposable type also carries the Java `close()` bridge, whether it reaches
    # IDisposable directly or through IEnumerator<T>, and whether its CLR Dispose is public
    # or an explicit interface implementation. `close()` and `Dispose()` are the same
    # operation; `close()` exists so the type is a real java.lang.AutoCloseable.
    disposable = any(value == "System.IDisposable"
                     or value.startswith("System.Collections.Generic.IEnumerator`1")
                     for value in type_contract.get("interfaces", []))
    if disposable \
            and not any(value["kind"] == "method" and value["name"] == "close"
                        and not value.get("parameters") for value in expected):
        # The bridge is concrete wherever a concrete Dispose exists to delegate to, which
        # is every disposable class: XNA's own Dispose() is `virtual final`. Only a
        # disposable interface leaves it abstract.
        interface = type_contract["kind"] == "interface"
        source = {"access": "public", "static": False, "abstract": interface,
                  "final": not interface, "genericArity": 0}
        expected.append(callable_member("method", "close", source, [], "void"))

    base = split_generic(type_contract.get("baseType") or "")
    if base and base[0] == "System.Collections.ObjectModel.Collection`1":
        element_type = map_type(base[1][0]) or "java.lang.Object"
        source = {"access": "public", "static": False, "abstract": False,
                  "final": False, "genericArity": 0}
        expected.extend([
            callable_member("method", "get", source, [parameter("index", "int")], element_type),
            callable_member("method", "size", source, [], "int"),
            callable_member("method", "add", source,
                            [parameter("index", "int"), parameter("element", element_type)], "void"),
            callable_member("method", "set", source,
                            [parameter("index", "int"), parameter("element", element_type)], element_type),
            callable_member("method", "remove", source, [parameter("index", "int")], element_type),
            callable_member("method", "clear", source, [], "void"),
        ])
    elif base and base[0] == "System.Collections.ObjectModel.ReadOnlyCollection`1":
        element_type = map_type(base[1][0]) or "java.lang.Object"
        source = {"access": "public", "static": False, "abstract": False,
                  "final": False, "genericArity": 0}
        expected.extend([
            callable_member("method", "get", source,
                            [parameter("index", "int")], element_type),
            callable_member("method", "size", source, [], "int"),
        ])

    if type_name in rules.get("javaCollectionBridgeTypes", []):
        collection_interface = next(
            (split_generic(value) for value in type_contract.get("interfaces", [])
             if value.startswith("System.Collections.Generic.ICollection`1[")
             or value.startswith("System.Collections.Generic.IList`1[")),
            None)
        if collection_interface is None:
            raise ValueError(f"Java collection bridge type has no ICollection<T>: {type_name}")
        element_type = map_type(collection_interface[1][0]) or "java.lang.Object"
        bridge = {"access": "public", "static": False, "abstract": False,
                  "final": True, "genericArity": 0}
        generic_bridge = {**bridge, "genericArity": 1}
        expected.extend([
            callable_member("method", "size", bridge, [], "int"),
            callable_member("method", "isEmpty", bridge, [], "boolean"),
            callable_member("method", "contains", bridge,
                            [parameter("item", "java.lang.Object")], "boolean"),
            callable_member("method", "iterator", bridge, [],
                            f"java.util.Iterator<{element_type}>"),
            callable_member("method", "toArray", bridge, [], "java.lang.Object[]"),
            callable_member("method", "toArray", generic_bridge,
                            [parameter("array", "T[]")], "T[]"),
            callable_member("method", "add", bridge,
                            [parameter("item", element_type)], "boolean"),
            callable_member("method", "remove", bridge,
                            [parameter("item", "java.lang.Object")], "boolean"),
            callable_member("method", "containsAll", bridge,
                            [parameter("collection", "java.util.Collection<?>")], "boolean"),
            callable_member("method", "addAll", bridge,
                            [parameter("collection", f"java.util.Collection<? extends {element_type}>")],
                            "boolean"),
            callable_member("method", "removeAll", bridge,
                            [parameter("collection", "java.util.Collection<?>")], "boolean"),
            callable_member("method", "retainAll", bridge,
                            [parameter("collection", "java.util.Collection<?>")], "boolean"),
            callable_member("method", "clear", bridge, [], "void"),
        ])

    if type_name in rules.get("javaListBridgeTypes", []):
        list_interface = next(
            (split_generic(value) for value in type_contract.get("interfaces", [])
             if value.startswith("System.Collections.Generic.IList`1[")),
            None)
        if list_interface is None:
            raise ValueError(f"Java list bridge type has no IList<T>: {type_name}")
        element_type = map_type(list_interface[1][0]) or "java.lang.Object"
        bridge = {"access": "public", "static": False, "abstract": False,
                  "final": True, "genericArity": 0}
        expected.extend([
            callable_member("method", "get", bridge,
                            [parameter("index", "int")], element_type),
            callable_member("method", "set", bridge,
                            [parameter("index", "int"), parameter("item", element_type)],
                            element_type),
            callable_member("method", "add", bridge,
                            [parameter("index", "int"), parameter("item", element_type)], "void"),
            callable_member("method", "remove", bridge,
                            [parameter("index", "int")], element_type),
            callable_member("method", "indexOf", bridge,
                            [parameter("item", "java.lang.Object")], "int"),
            callable_member("method", "lastIndexOf", bridge,
                            [parameter("item", "java.lang.Object")], "int"),
            callable_member("method", "listIterator", bridge, [],
                            f"java.util.ListIterator<{element_type}>"),
            callable_member("method", "listIterator", bridge,
                            [parameter("index", "int")],
                            f"java.util.ListIterator<{element_type}>"),
            callable_member("method", "subList", bridge,
                            [parameter("fromIndex", "int"), parameter("toIndex", "int")],
                            f"java.util.List<{element_type}>"),
            callable_member("method", "addAll", bridge,
                            [parameter("index", "int"),
                             parameter("collection", f"java.util.Collection<? extends {element_type}>")],
                            "boolean"),
        ])

    if type_name in rules.get("javaMapBridgeTypes", []):
        dictionary = next(
            (split_generic(value) for value in type_contract.get("interfaces", [])
             if value.startswith("System.Collections.Generic.IDictionary`2[")),
            None)
        if dictionary is None:
            raise ValueError(f"Java map bridge type has no IDictionary<K,V>: {type_name}")
        key_type = boxed(map_type(dictionary[1][0]) or "java.lang.Object")
        value_type = boxed(map_type(dictionary[1][1]) or "java.lang.Object")
        entry = f"java.util.Map.Entry<{key_type},{value_type}>"
        bridge = {"access": "public", "static": False, "abstract": False,
                  "final": True, "genericArity": 0}
        expected.extend([
            callable_member("method", "size", bridge, [], "int"),
            callable_member("method", "isEmpty", bridge, [], "boolean"),
            callable_member("method", "containsKey", bridge,
                            [parameter("key", "java.lang.Object")], "boolean"),
            callable_member("method", "containsValue", bridge,
                            [parameter("value", "java.lang.Object")], "boolean"),
            callable_member("method", "get", bridge,
                            [parameter("key", "java.lang.Object")], value_type),
            callable_member("method", "put", bridge,
                            [parameter("key", key_type), parameter("value", value_type)],
                            value_type),
            callable_member("method", "remove", bridge,
                            [parameter("key", "java.lang.Object")], value_type),
            # javac writes `? extends java.lang.Object` as the unbounded `?`, so the expected
            # signature has to spell it the same way the class file will.
            callable_member("method", "putAll", bridge,
                            [parameter("map", f"java.util.Map<? extends {key_type},"
                                              + ("?" if value_type == "java.lang.Object"
                                                 else f"? extends {value_type}") + ">")], "void"),
            callable_member("method", "clear", bridge, [], "void"),
            callable_member("method", "keySet", bridge, [], f"java.util.Set<{key_type}>"),
            callable_member("method", "values", bridge, [],
                            f"java.util.Collection<{value_type}>"),
            callable_member("method", "entrySet", bridge, [], f"java.util.Set<{entry}>"),
        ])

    if type_name in rules.get("javaIteratorBridgeTypes", []):
        iterator_interface = next(
            (split_generic(value) for value in type_contract.get("interfaces", [])
             if value.startswith("System.Collections.Generic.IEnumerator`1[")),
            None)
        if iterator_interface is None:
            raise ValueError(f"Java iterator bridge type has no IEnumerator<T>: {type_name}")
        element_type = map_type(iterator_interface[1][0]) or "java.lang.Object"
        bridge = {"access": "public", "static": False, "abstract": False,
                  "final": True, "genericArity": 0}
        expected.extend([
            callable_member("method", "hasNext", bridge, [], "boolean"),
            callable_member("method", "next", bridge, [], element_type),
        ])

    if type_name in rules.get("javaIterableBridgeTypes", []):
        iterable_interface = next(
            (split_generic(value) for value in type_contract.get("interfaces", [])
             if value.startswith("System.Collections.Generic.IEnumerable`1[")),
            None)
        if iterable_interface is None:
            raise ValueError(f"Java iterable bridge type has no IEnumerable<T>: {type_name}")
        element_type = map_type(iterable_interface[1][0]) or "java.lang.Object"
        bridge = {"access": "public", "static": False, "abstract": False,
                  "final": True, "genericArity": 0}
        expected.append(callable_member(
            "method", "iterator", bridge, [], f"java.util.Iterator<{element_type}>"))

    # CLR `internal` grants assembly-wide access; Java has no equivalent across packages.
    # Where a CLR type is constructed by another namespace of the same assembly, the rules file
    # declares the Java member that replaces that access, with its exact signature and reason,
    # so it is an expected member rather than a suppressed one.
    for declared in rules.get("clrInternalMembers", {}).get(type_name, []):
        source = {"access": declared.get("access", "protected"), "static": declared.get("static", False),
                  "abstract": False, "final": declared.get("final", False),
                  "genericArity": declared.get("genericArity", 0)}
        expected.append(callable_member(
            declared["kind"], ".ctor" if declared["kind"] == "constructor" else declared["name"],
            source,
            [parameter(value["name"], value["type"]) for value in declared.get("parameters", [])],
            declared.get("returnType")))

    unique: dict[str, dict[str, Any]] = {}
    for member in expected:
        unique[member_key(member, include_return=True)] = member
    return list(unique.values())


def member_key(member: dict[str, Any], *, include_return: bool) -> str:
    parameters = ",".join(value["type"] for value in member.get("parameters", []))
    key = f"{member['kind']}:{member['name']}({parameters})"
    if include_return:
        key += "->" + str(member.get("returnType") if member["kind"] != "field" else member.get("type"))
    return key


def effective_member_final(member: dict[str, Any], declaring_type_final: bool) -> bool:
    """Return whether a Java member is observably non-overridable.

    A method in a final class is just as non-overridable as a method carrying
    ACC_FINAL. Fields and constructors retain their literal metadata state.
    """
    return bool(member.get("final")) or (
        member.get("kind") == "method"
        and not member.get("static", False)
        and declaring_type_final
    )


def compare(reference: dict[str, Any], target: dict[str, Any], rules: dict[str, Any],
            superset: set[str] | None = None) -> list[dict[str, Any]]:
    findings: list[dict[str, Any]] = []
    expected_types = {map_type(value["name"], identity=True): value for value in reference["types"]}
    for synthetic in rules.get("syntheticTypes", []):
        expected_types[synthetic["name"]] = {**synthetic, "javaSynthetic": True}
    actual_types = {value["name"]: value for value in target["types"]}

    for name in sorted(expected_types.keys() - actual_types.keys()):
        findings.append(diagnostic("MISSING_TYPE", name))
    for name in sorted(actual_types.keys() - expected_types.keys()):
        # A narrower profile is a subset gate, not an exclusivity claim: a type that a wider
        # profile of the same corpus declares is not an unexpected type here, it simply belongs
        # to a family this profile does not measure.
        if superset is not None and name in superset:
            continue
        findings.append(diagnostic("UNEXPECTED_TYPE", name))

    for name in sorted(expected_types.keys() & actual_types.keys()):
        expected_type = expected_types[name]
        actual_type = actual_types[name]
        expected_kind = {"struct": "class", "delegate": "interface"}.get(
            expected_type["kind"], expected_type["kind"])
        if expected_type.get("flags"):
            expected_kind = "class"
        if actual_type["kind"] != expected_kind:
            findings.append(diagnostic("TYPE_KIND_MISMATCH", name, expected_kind, actual_type["kind"]))
        if expected_type.get("access") != actual_type.get("access"):
            findings.append(diagnostic("ACCESSIBILITY_MISMATCH", name,
                                       expected_type.get("access"), actual_type.get("access")))
        expected_abstract = bool(expected_type.get("abstract")) and not bool(expected_type.get("sealed"))
        expected_sealed = bool(expected_type.get("sealed")) or expected_type["kind"] == "struct"
        if expected_abstract != bool(actual_type.get("abstract")) or expected_sealed != bool(actual_type.get("sealed")):
            findings.append(diagnostic("TYPE_MODIFIER_MISMATCH", name,
                                       {"abstract": expected_abstract, "final": expected_sealed},
                                       {"abstract": actual_type.get("abstract"), "final": actual_type.get("sealed")}))
        expected_base = expected_type.get("baseType") if expected_type.get("javaSynthetic") \
            else rules.get("typeBaseTypeMappings", {}).get(name, map_type(expected_type.get("baseType")))
        if expected_type["kind"] in ("struct", "enum"):
            expected_base = "java.lang.Object" if expected_type["kind"] == "struct" \
                or expected_type.get("flags") else actual_type.get("baseType")
        if expected_base != actual_type.get("baseType"):
            findings.append(diagnostic("BASE_TYPE_MISMATCH", name, expected_base, actual_type.get("baseType")))
        expected_interfaces = set(expected_type.get("interfaces", [])) if expected_type.get("javaSynthetic") \
            else set(filter(None, (map_type(value) for value in expected_type.get("interfaces", []))))
        expected_interfaces.discard("System.IEquatable<T0>")
        expected_interfaces = {value for value in expected_interfaces if not value.startswith("System.IEquatable")}
        expected_interfaces = {
            value for value in expected_interfaces
            if value.startswith("java.") or value.split("<", 1)[0] in expected_types
        }
        # Java forbids one class from implementing two parameterizations of one interface.
        # Where CLR declares a covariant duplicate -- GamerCollection<T> adds
        # IEnumerable<Gamer> beside the IEnumerable<T> its base already gives -- the rules
        # file records which parameterization Java keeps and why.
        for omitted in rules.get("omittedCovariantInterfaces", {}).get(name, []):
            expected_interfaces.discard(omitted)
        actual_interfaces = set(actual_type.get("interfaces", []))
        if expected_interfaces != actual_interfaces:
            findings.append(diagnostic("INTERFACE_MISMATCH", name, sorted(expected_interfaces), sorted(actual_interfaces)))
        if expected_type.get("genericArity", 0) != actual_type.get("genericArity", 0):
            findings.append(diagnostic("GENERIC_MISMATCH", name, expected_type.get("genericArity", 0), actual_type.get("genericArity", 0)))

        expected_members = expected_type["members"] if expected_type.get("javaSynthetic") \
            else mapped_members(expected_type, rules, findings)
        actual_members = actual_type["members"]
        expected_full = {member_key(value, include_return=True): value for value in expected_members}
        actual_full = {member_key(value, include_return=True): value for value in actual_members}
        expected_shapes = {member_key(value, include_return=False): value for value in expected_members}
        actual_shapes = {member_key(value, include_return=False): value for value in actual_members}

        paired_expected: set[str] = set()
        paired_actual: set[str] = set()
        unmatched_actual_keys = actual_full.keys() - expected_full.keys()
        for expected_key in sorted(expected_full.keys() - actual_full.keys()):
            expected_member = expected_full[expected_key]
            candidates = [value for key, value in actual_full.items()
                          if key in unmatched_actual_keys and key not in paired_actual
                          and value["kind"] == expected_member["kind"]
                          and value["name"] == expected_member["name"]
                          and len(value.get("parameters", [])) == len(expected_member.get("parameters", []))]
            if len(candidates) == 1:
                actual_member = candidates[0]
                expected_parameters = [value["type"] for value in expected_member.get("parameters", [])]
                actual_parameters = [value["type"] for value in actual_member.get("parameters", [])]
                if expected_parameters != actual_parameters:
                    actual_key = member_key(actual_member, include_return=True)
                    paired_expected.add(expected_key)
                    paired_actual.add(actual_key)
                    findings.append(diagnostic("PARAMETER_MISMATCH",
                                               name + "." + expected_member["name"],
                                               expected_parameters, actual_parameters))

        for key in sorted(expected_full.keys() - actual_full.keys()):
            if key in paired_expected:
                continue
            shape = member_key(expected_full[key], include_return=False)
            subject = name + "." + shape
            if shape in actual_shapes:
                code = "FIELD_TYPE_MISMATCH" if expected_full[key]["kind"] == "field" else "RETURN_TYPE_MISMATCH"
                expected_value = expected_full[key].get("type") or expected_full[key].get("returnType")
                actual_value = actual_shapes[shape].get("type") or actual_shapes[shape].get("returnType")
                findings.append(diagnostic(code, subject, expected_value, actual_value))
            else:
                findings.append(diagnostic("MISSING_MEMBER", subject, expected=key))
        for key in sorted(actual_full.keys() - expected_full.keys()):
            if key in paired_actual:
                continue
            shape = member_key(actual_full[key], include_return=False)
            if shape not in expected_shapes:
                findings.append(diagnostic("UNEXPECTED_MEMBER", name + "." + shape, actual=key))

        for shape in sorted(expected_shapes.keys() & actual_shapes.keys()):
            expected_member = expected_shapes[shape]
            actual_member = actual_shapes[shape]
            expected_names = [value["name"] for value in expected_member.get("parameters", [])]
            actual_names = [value["name"] for value in actual_member.get("parameters", [])]
            if expected_names != actual_names:
                findings.append(diagnostic("PARAMETER_NAME_MISMATCH", name + "." + shape, expected_names, actual_names))
            expected_modifiers = {key: expected_member.get(key) for key in
                                  ("access", "static", "abstract", "genericArity")}
            actual_modifiers = {key: actual_member.get(key) for key in
                                ("access", "static", "abstract", "genericArity")}
            expected_modifiers["final"] = effective_member_final(expected_member, expected_sealed)
            actual_modifiers["final"] = effective_member_final(
                actual_member, bool(actual_type.get("sealed")))
            if expected_modifiers != actual_modifiers:
                findings.append(diagnostic("MEMBER_MODIFIER_MISMATCH", name + "." + shape,
                                           expected_modifiers, actual_modifiers))
            if expected_member["kind"] == "field" and expected_type["kind"] == "enum" \
                    and expected_member.get("constant") != actual_member.get("constant"):
                findings.append(diagnostic("ENUM_VALUE_MISMATCH", name + "." + expected_member["name"],
                                           expected_member.get("constant"), actual_member.get("constant")))

    findings.extend(leak_diagnostics(target))
    return sorted(findings, key=lambda item: (item["code"], item["subject"], json.dumps(item, sort_keys=True)))


def expected_java_counts(
        reference: dict[str, Any] | None,
        rules: dict[str, Any]) -> tuple[int, int]:
    if reference is None:
        return 0, 0
    expected_types = {map_type(value["name"], identity=True): value for value in reference["types"]}
    for synthetic in rules.get("syntheticTypes", []):
        expected_types[synthetic["name"]] = {**synthetic, "javaSynthetic": True}
    members = 0
    for expected_type in expected_types.values():
        members += len(expected_type["members"] if expected_type.get("javaSynthetic")
                       else mapped_members(expected_type, rules, []))
    return len(expected_types), members


def make_report(
        profile: dict[str, Any],
        reference: dict[str, Any] | None,
        target: dict[str, Any],
        findings: list[dict[str, Any]],
        rules: dict[str, Any]) -> dict[str, Any]:
    counts: dict[str, int] = {}
    for item in findings:
        counts[item["code"]] = counts.get(item["code"], 0) + 1
    expected_types, expected_members = expected_java_counts(reference, rules)
    return {
        "schemaVersion": 1,
        "profile": profile["name"],
        "referenceTypes": 0 if reference is None else len(reference["types"]),
        "referenceMembers": 0 if reference is None else sum(len(value["members"]) for value in reference["types"]),
        "expectedJavaTypes": expected_types,
        "expectedJavaMembers": expected_members,
        "targetTypes": len(target["types"]),
        "targetMembers": sum(len(value["members"]) for value in target["types"]),
        "totalDiagnostics": len(findings),
        "allowlistEntries": 0,
        "diagnosticCounts": dict(sorted(counts.items())),
        "diagnostics": findings,
    }


def render_text(report: dict[str, Any], include_diagnostics: bool = True) -> str:
    lines = [
        f"PROFILE={report['profile']}",
        f"REFERENCE_TYPES={report['referenceTypes']}",
        f"REFERENCE_MEMBERS={report['referenceMembers']}",
        f"EXPECTED_JAVA_TYPES={report['expectedJavaTypes']}",
        f"EXPECTED_JAVA_MEMBERS={report['expectedJavaMembers']}",
        f"TARGET_TYPES={report['targetTypes']}",
        f"TARGET_MEMBERS={report['targetMembers']}",
        f"TOTAL_DIAGNOSTICS={report['totalDiagnostics']}",
        f"ALLOWLIST_ENTRIES={report['allowlistEntries']}",
    ]
    lines.extend(f"{key}={value}" for key, value in report["diagnosticCounts"].items())
    if include_diagnostics:
        lines.extend(f"{item['code']} {item['subject']}" for item in report["diagnostics"])
    return "\n".join(lines)


def main() -> int:
    arguments = parse_args()
    profile = json.loads(Path(arguments.profile).read_text(encoding="utf-8"))
    rules = json.loads(Path(arguments.mapping_rules).read_text(encoding="utf-8"))
    GENERIC_RENAMES.update(rules.get("genericTypeRenames", {}))
    TYPE_RENAMES.update(rules.get("frameworkTypeMappings", {}))
    if rules.get("allowlist"):
        print("mapping rules must retain an empty allowlist", file=sys.stderr)
        return 2

    with tempfile.TemporaryDirectory(prefix="cna-java-api-compat-") as directory:
        temporary = Path(directory)
        target = read_target(arguments.target, temporary)
        if arguments.leak_only:
            findings = leak_diagnostics(target)
            report = make_report(profile, None, target, findings, rules)
        else:
            if not arguments.reference_dir:
                print("XNA_REFERENCE_PATH or --reference-dir is required for the strict verifier", file=sys.stderr)
                return 2
            reference = read_reference(arguments.reference_dir, profile, temporary)
            superset = None
            if profile.get("supersetProfile"):
                wider = json.loads(
                    (Path(arguments.profile).parent / profile["supersetProfile"])
                    .read_text(encoding="utf-8"))
                wider_reference = read_reference(arguments.reference_dir, wider, temporary)
                superset = {map_type(value["name"], identity=True)
                            for value in wider_reference["types"]}
            findings = compare(reference, target, rules, superset)
            report = make_report(profile, reference, target, findings, rules)

    if arguments.summary_only:
        rendered = render_text(report, include_diagnostics=False)
    elif arguments.format == "json":
        rendered = json.dumps(report, indent=2, sort_keys=True)
    else:
        rendered = render_text(report, include_diagnostics=True)
    print(rendered)
    if arguments.output:
        output = Path(arguments.output)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return 0 if arguments.report_only or report["totalDiagnostics"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
