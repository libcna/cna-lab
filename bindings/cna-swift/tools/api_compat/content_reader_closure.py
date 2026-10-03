#!/usr/bin/env python3
"""Generate the demand-driven XNA ContentReader dependency closure.

The XNA contract is the public-shape authority.  The two BCL classes are read
from the admitted .NET Framework 4.0 IL through the same parser as the BCL
authority gate.  Everything that parser exposes is classified here as either
selected or refused; an unclassified declaration is a hard failure rather than
an implicit expansion of System.IO or System.Resources.

No machine-local path is written to the report.  Microsoft binaries and their
disassemblies are inputs only and are never copied into the repository.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
CONTRACT = ROOT / "tools/api_compat/reference/xna40-windows-runtime-contract.json"
STRICT_REPORT = ROOT / "docs/generated/api-compat-report.json"

SELECTED_XNA = (
    "Microsoft.Xna.Framework.Content.ContentReader",
    "Microsoft.Xna.Framework.Content.ContentTypeReader",
    "Microsoft.Xna.Framework.Content.ContentTypeReader`1",
    "Microsoft.Xna.Framework.Content.ContentTypeReaderManager",
    "Microsoft.Xna.Framework.Content.ResourceContentManager",
)

EXPECTED_DECLARED_COUNTS = {
    "Microsoft.Xna.Framework.Content.ContentReader": 20,
    "Microsoft.Xna.Framework.Content.ContentTypeReader": 6,
    "Microsoft.Xna.Framework.Content.ContentTypeReader`1": 3,
    "Microsoft.Xna.Framework.Content.ContentTypeReaderManager": 1,
    "Microsoft.Xna.Framework.Content.ResourceContentManager": 2,
}

XNA_ABSTRACTNESS = {
    "Microsoft.Xna.Framework.Content.ContentReader": False,
    "Microsoft.Xna.Framework.Content.ContentTypeReader": True,
    "Microsoft.Xna.Framework.Content.ContentTypeReader`1": True,
    "Microsoft.Xna.Framework.Content.ContentTypeReaderManager": False,
    "Microsoft.Xna.Framework.Content.ResourceContentManager": False,
}

XNA_BEHAVIOURAL_DEPENDENCIES = {
    "Microsoft.Xna.Framework.Content.ContentReader": {
        "bcl": [
            "System.Action`1[System.IDisposable]",
            "System.Collections.Generic.List`1[System.Action`1[System.Object]]",
            "System.Globalization.CultureInfo (formatting only)",
            "System.IO.BinaryReader",
            "System.IO.IOException (caught and wrapped)",
            "System.IO.Path (private reference-path composition)",
            "System.IO.Stream",
            "System.Type (identity and assignability only)",
        ],
        "xnaPrivateInternal": [
            "ContentReader.Create/PrepareStream/ReadHeader/ReadAsset",
            "ContentReader.ReadSharedResources/InvokeReader",
            "ContentLoadException",
            "ContentManager.Load/RecordDisposableObject",
            "ContentTypeReaderManager.ReadTypeManifest/internal GetTypeReader",
            "DecompressStream (compressed XNB branch only)",
            "FrameworkResources",
            "TitleContainer.GetCleanPath",
        ],
    },
    "Microsoft.Xna.Framework.Content.ContentTypeReader": {
        "bcl": ["System.Object", "System.Type (identity and value-type test)"],
        "xnaPrivateInternal": ["TargetIsValueType field"],
    },
    "Microsoft.Xna.Framework.Content.ContentTypeReader`1": {
        "bcl": ["System.Object (private erased bridge)"],
        "xnaPrivateInternal": ["ContentTypeReader.Read erased override"],
    },
    "Microsoft.Xna.Framework.Content.ContentTypeReaderManager": {
        "bcl": [
            "System.Activator and System.Reflection (private CLR construction path)",
            "System.Collections.Generic.Dictionary and List (private registries)",
            "System.Threading.Monitor (private registry locking)",
            "System.Type (public lookup identity)",
        ],
        "xnaPrivateInternal": [
            "ContentReader.ReadString/ReadInt32",
            "ContentTypeReader.Initialize/TargetType/TypeVersion",
            "FrameworkResources type-reader failures",
        ],
    },
    "Microsoft.Xna.Framework.Content.ResourceContentManager": {
        "bcl": [
            "System.Globalization.CultureInfo (message formatting only)",
            "System.IO.MemoryStream (maps to Foundation.InputStream over bytes)",
            "System.Resources.ResourceManager.GetObject(String)",
        ],
        "xnaPrivateInternal": ["FrameworkResources resource lookup failures"],
    },
}

ROUTE_RELEVANCE = {
    "Microsoft.Xna.Framework.Content.ContentReader": (
        "NONE: cna_content_reader_* consumes CNA_StorageStreamHandle and is a "
        "save-game/storage transport, not the XNA title-asset BinaryReader"
    ),
    "Microsoft.Xna.Framework.Content.ContentTypeReader": "NONE: managed reader contract",
    "Microsoft.Xna.Framework.Content.ContentTypeReader`1": "NONE: managed generic bridge",
    "Microsoft.Xna.Framework.Content.ContentTypeReaderManager": "NONE: managed registry",
    "Microsoft.Xna.Framework.Content.ResourceContentManager": (
        "DO NOT BIND: cna_content_manager_create_resource is a documented "
        "placeholder whose loads report IO"
    ),
}

FIXTURES = {
    "Microsoft.Xna.Framework.Content.ContentReader": [
        "byte-authored uncompressed Windows XNB v5",
        "primary object plus shared resource",
        "independent primitive/math/string bytes",
    ],
    "Microsoft.Xna.Framework.Content.ContentTypeReader": [
        "test subclass exercising TargetType, version, initialize, existing instance",
    ],
    "Microsoft.Xna.Framework.Content.ContentTypeReader`1": [
        "typed test reader plus erased bridge and wrong-T refusal",
    ],
    "Microsoft.Xna.Framework.Content.ContentTypeReaderManager": [
        "private creator registry: known name, duplicate, unknown name, stable identity",
    ],
    "Microsoft.Xna.Framework.Content.ResourceContentManager": [
        "project-owned ResourceManager subclass returning the same XNB bytes",
    ],
}


def public_encoding_dependencies(records: list[dict[str, Any]]) -> list[str]:
    findings: list[str] = []
    for record in records:
        for member in record["members"]:
            types = [member.get("returnType"), member.get("type")]
            types.extend(item.get("type") for item in member.get("parameters", []))
            if any(item == "System.Text.Encoding" for item in types):
                findings.append(
                    f"{record['name']}.{member.get('name')} names System.Text.Encoding")
    return findings


def closure_self_tests(records: list[dict[str, Any]]) -> int:
    # Negative control requested by the selected milestone: a planted public
    # Encoding parameter must make the supposedly zero dependency count fail.
    mutated = copy.deepcopy(records)
    mutated[0]["members"].append({
        "kind": "method",
        "name": "PlantedEncodingDependency",
        "returnType": "System.Void",
        "parameters": [{"name": "encoding", "type": "System.Text.Encoding"}],
    })
    if len(public_encoding_dependencies(mutated)) != 1:
        raise SystemExit("closure negative control survived planted Encoding dependency")

    stream_ctor = {
        "kind": "constructor", "name": ".ctor",
        "parameters": [{"type": "System.IO.Stream"}],
    }
    encoding_ctor = {
        "kind": "constructor", "name": ".ctor",
        "parameters": [
            {"type": "System.IO.Stream"}, {"type": "System.Text.Encoding"},
        ],
    }
    if not binary_reader_selected(stream_ctor)[0] or binary_reader_selected(encoding_ctor)[0]:
        raise SystemExit("BinaryReader constructor demand negative control failed")
    return 2


def load_bcl_parser():
    path = ROOT / "tools/api_compat/bcl_authority_audit.py"
    spec = importlib.util.spec_from_file_location("cna_bcl_authority_audit", path)
    if spec is None or spec.loader is None:
        raise SystemExit(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def member_identity(member: dict[str, Any]) -> str:
    kind = member["kind"]
    if kind in ("method", "constructor"):
        params = ",".join(item.get("type", "") for item in member.get("parameters", []))
        result = member.get("returnType")
        suffix = f"->{result}" if result else ""
        return f"{kind}:{member['name']}({params}){suffix}"
    if kind == "property":
        return f"property:{member['name']}->{member.get('type')}"
    if kind == "field":
        return f"field:{member['name']}->{member.get('type')}"
    return f"{kind}:{member['name']}"


def xna_signature_types(record: dict[str, Any]) -> list[str]:
    result: set[str] = set()
    if record.get("baseType"):
        result.add(record["baseType"])
    result.update(record.get("directInterfaces", []))
    for member in record["members"]:
        if member.get("returnType"):
            result.add(member["returnType"])
        if member.get("type"):
            result.add(member["type"])
        result.update(item["type"] for item in member.get("parameters", []))
    return sorted(item for item in result if item.startswith("System."))


def same_inherited_identity(a: dict[str, Any], b: dict[str, Any]) -> bool:
    if a["kind"] != b["kind"] or a["name"] != b["name"]:
        return False
    return [item.get("type") for item in a.get("parameters", [])] == [
        item.get("type") for item in b.get("parameters", [])]


def inherited_xna_members(
    record: dict[str, Any], by_name: dict[str, dict[str, Any]],
) -> list[dict[str, str]]:
    inherited: list[dict[str, str]] = []
    declared = record["members"]
    base = record.get("baseType")
    visited: set[str] = set()
    while base in by_name and base not in visited:
        visited.add(base)
        ancestor = by_name[base]
        for member in ancestor["members"]:
            if member["kind"] == "constructor":
                continue
            if any(same_inherited_identity(member, item) for item in declared):
                continue
            inherited.append({
                "declaredBy": base,
                "identity": member_identity(member),
            })
        base = ancestor.get("baseType")
    return inherited


def binary_reader_selected(member: dict[str, Any]) -> tuple[bool, str]:
    if member["kind"] == "constructor":
        parameters = [item["type"] for item in member.get("parameters", [])]
        if parameters == ["System.IO.Stream"]:
            return True, "internal base-construction support; constructors are not inherited"
        return False, "Encoding overload is not inherited and no selected XNA signature names Encoding"
    if member["kind"] == "method" and member["name"] == "ReadDecimal":
        return False, "no selected XNA member reaches Decimal; demand rule refuses the expansion"
    return True, "public/protected BinaryReader member inherited by ContentReader"


RESOURCE_MANAGER_SELECTED = {
    "method:GetObject(System.String)->System.Object":
        "the exact call made by ResourceContentManager.OpenStream",
    "method:GetString(System.String)->System.String":
        "same selected resource store and public string lookup semantics",
    "method:ReleaseAllResources()->System.Void":
        "observable cache/resource-set lifetime on the selected store",
    "property:BaseName->System.String": "selected resource identity",
    "property:IgnoreCase->System.Boolean": "selected lookup policy and its mutation test",
    "property:ResourceSetType->System.Type": "selected public type identity",
    "constructor:.ctor()": "protected subclass construction is part of the reachable surface",
}


def resource_manager_selected(member: dict[str, Any]) -> tuple[bool, str]:
    identity = member_identity(member)
    if identity in RESOURCE_MANAGER_SELECTED:
        return True, RESOURCE_MANAGER_SELECTED[identity]
    dependencies = sorted(item for item in ({
        parameter.get("type") for parameter in member.get("parameters", [])
    } | {member.get("type"), member.get("returnType")}) if item)
    return False, (
        "not reached by ResourceContentManager; refusing transitive surface "
        + ", ".join(dependencies)
    )


def classify_bcl(record: dict[str, Any], selector) -> dict[str, Any]:
    selected: list[dict[str, str]] = []
    refused: list[dict[str, str]] = []
    for member in record["members"]:
        keep, reason = selector(member)
        item = {
            "identity": member_identity(member),
            "access": member.get("access"),
            "static": member.get("static", False),
            "virtual": member.get("virtual", False),
            "abstract": member.get("abstract", False),
            "final": member.get("final", False),
            "reason": reason,
        }
        (selected if keep else refused).append(item)
    if len(selected) + len(refused) != len(record["members"]):
        raise SystemExit(f"unreviewed BCL member in {record['type']}")
    return {
        "assembly": record["assembly"],
        "type": record["type"],
        "declaredBase": record["baseType"],
        "interfaces": record["directInterfaces"],
        "authorityVisibleMembers": len(record["members"]),
        "selectedMembers": selected,
        "refusedMembers": refused,
        "unreviewedMembers": 0,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--bcl-il", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    contract_bytes = CONTRACT.read_bytes()
    contract = json.loads(contract_bytes)
    by_name = {item["name"]: item for item in contract["types"]}
    missing = sorted(set(SELECTED_XNA) - set(by_name))
    if missing:
        raise SystemExit(f"selected XNA types absent from contract: {missing}")
    selected_records = [by_name[name] for name in SELECTED_XNA]
    encoding_findings = public_encoding_dependencies(selected_records)
    if encoding_findings:
        raise SystemExit("; ".join(encoding_findings))
    self_test_count = closure_self_tests(selected_records)

    strict = json.loads(STRICT_REPORT.read_text(encoding="utf-8"))
    missing_subjects = {
        item["subject"] for item in strict.get("diagnostics", [])
        if item["category"] == "MISSING_TYPE"
    }
    selected_diagnostics = [
        item for item in strict.get("diagnostics", [])
        if item["subject"] in SELECTED_XNA
        or (
            item["subject"] == "Microsoft.Xna.Framework.Content.ContentManager"
            and any(name in item["detail"] for name in ("OpenStream", "ReadAsset"))
        )
    ]

    xna: list[dict[str, Any]] = []
    for name in SELECTED_XNA:
        record = by_name[name]
        count = len(record["members"])
        if count != EXPECTED_DECLARED_COUNTS[name]:
            raise SystemExit(f"{name}: expected {EXPECTED_DECLARED_COUNTS[name]} members, found {count}")
        mapped = name.replace("`1", "OfT")
        xna.append({
            "type": name,
            "swiftType": mapped,
            "kind": record["kind"],
            "abstract": XNA_ABSTRACTNESS[name],
            "sealed": record["sealed"],
            "declaredBase": record["baseType"],
            "interfaces": record.get("directInterfaces", []),
            "genericParameters": record.get("genericParameters", []),
            "declaredMembers": [
                {
                    "identity": member_identity(item),
                    "access": item.get("access") or item.get("getAccess") or item.get("setAccess"),
                    "static": item.get("static", False),
                    "genericParameters": item.get("genericParameters", []),
                    "parameters": item.get("parameters", []),
                }
                for item in record["members"]
            ],
            "declaredMemberCount": count,
            "inheritedXnaSurface": inherited_xna_members(record, by_name),
            "bclSignatureTypes": xna_signature_types(record),
            "bclBehaviouralDependencies": XNA_BEHAVIOURAL_DEPENDENCIES[name]["bcl"],
            "xnaPrivateInternalDependencies": XNA_BEHAVIOURAL_DEPENDENCIES[name]["xnaPrivateInternal"],
            "currentSwiftMappingStatus": "missing" if name in missing_subjects else "projected",
            "currentCnaRouteRelevance": ROUTE_RELEVANCE[name],
            "fixtureRequirements": FIXTURES[name],
            "unreviewedMembers": 0,
        })

    audit = load_bcl_parser()
    il_bytes = args.bcl_il.read_bytes()
    records, absent = audit.extract(
        "mscorlib.dll", il_bytes.decode("utf-8", errors="replace"),
        ["System.IO.BinaryReader", "System.Resources.ResourceManager", "System.Object"],
    )
    if absent:
        raise SystemExit(f"BCL types absent from authority IL: {absent}")
    bcl_by_name = {item["type"]: item for item in records}
    binary = classify_bcl(bcl_by_name["System.IO.BinaryReader"], binary_reader_selected)
    resource = classify_bcl(
        bcl_by_name["System.Resources.ResourceManager"], resource_manager_selected)
    object_inherited = [
        {
            "identity": member_identity(item),
            "status": "refused",
            "reason": "System.Object is the Swift language root; its reflection/finalizer surface is not fabricated",
        }
        for item in bcl_by_name["System.Object"]["members"]
        if item["kind"] != "constructor"
    ]
    binary["inheritedObjectSurface"] = object_inherited
    resource["inheritedObjectSurface"] = object_inherited
    for item in xna:
        item["inheritedBclSurface"] = (
            [member["identity"] for member in binary["selectedMembers"]
             if not member["identity"].startswith("constructor:")]
            if item["type"] == "Microsoft.Xna.Framework.Content.ContentReader"
            else []
        )

    content_unreviewed = sum(item["unreviewedMembers"] for item in xna)
    bcl_unreviewed = (
        binary["unreviewedMembers"] + resource["unreviewedMembers"])
    selected_missing = sum(
        item["currentSwiftMappingStatus"] == "missing" for item in xna)

    document = {
        "schemaVersion": 1,
        "scope": "XNA Content reader family",
        "shapeAuthority": {
            "profile": contract["profile"],
            "contractSha256": hashlib.sha256(contract_bytes).hexdigest(),
        },
        "behaviourAuthority": {
            "xna": "seven hash-registered Microsoft XNA 4.0 Windows assemblies",
            "bcl": "mscorlib.dll 4.0.0.0 sha256 5634668d4775b0113f08ea31093b281fea69bfc4e99227f5ca761b4ed98acc63",
            "bclIlSha256": hashlib.sha256(il_bytes).hexdigest(),
        },
        "xnaTypes": xna,
        "bclTypes": [binary, resource],
        "additionalBclSupport": [
            {
                "type": "System.IDisposable",
                "reason": "Action<IDisposable> is in ContentManager.ReadAsset and reader ownership",
                "projection": "CNADisposable protocol with Dispose()",
            },
            {
                "type": "System.Action`1",
                "reason": "ReadAsset and ReadSharedResource callback signatures",
                "projection": "typed Swift throwing closure",
            },
            {
                "type": "System.IO.EndOfStreamException",
                "reason": "BinaryReader exact EOF identity",
                "projection": "CNAEndOfStreamException : CNAIOException",
            },
            {
                "type": "System.FormatException",
                "reason": "BinaryReader malformed 7-bit integer identity",
                "projection": "CNAFormatException : CNASystemException",
            },
        ],
        "publicEncodingDependencies": len(encoding_findings),
        "closureMutationSelfTests": self_test_count,
        "contentReaderUnreviewed": content_unreviewed,
        "bclContentClosureUnreviewed": bcl_unreviewed,
        "CONTENT_READER_ACTIONABLE_LOCAL": selected_missing + len(selected_diagnostics),
        "CONTENT_READER_UNREVIEWED": content_unreviewed,
        "BCL_CONTENT_CLOSURE_UNREVIEWED": bcl_unreviewed,
        "CONTENT_READER_MISSING_TYPES": selected_missing,
        "CONTENT_READER_STRICT_DIAGNOSTICS": len(selected_diagnostics),
        "CONTENT_READER_ENCODING_DEPENDENCIES": len(encoding_findings),
        "BINARY_READER_AUTHORITY_FINDINGS": binary["unreviewedMembers"],
        "RESOURCE_MANAGER_AUTHORITY_FINDINGS": resource["unreviewedMembers"],
        "counts": {
            "selectedXnaTypes": len(xna),
            "selectedXnaDeclaredMembers": sum(item["declaredMemberCount"] for item in xna),
            "binaryReaderAuthorityVisibleMembers": binary["authorityVisibleMembers"],
            "binaryReaderSelectedMembers": len(binary["selectedMembers"]),
            "binaryReaderRefusedMembers": len(binary["refusedMembers"]),
            "resourceManagerAuthorityVisibleMembers": resource["authorityVisibleMembers"],
            "resourceManagerSelectedMembers": len(resource["selectedMembers"]),
            "resourceManagerRefusedMembers": len(resource["refusedMembers"]),
        },
    }
    rendered = json.dumps(document, indent=2, sort_keys=False) + "\n"

    if args.check:
        if args.output is None or not args.output.exists():
            raise SystemExit("--check requires an existing --output")
        if args.output.read_text(encoding="utf-8") != rendered:
            raise SystemExit(f"{args.output} is stale")
    elif args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    else:
        print(rendered, end="")

    print(
        f"CONTENT_READER_ACTIONABLE_LOCAL={document['CONTENT_READER_ACTIONABLE_LOCAL']} "
        f"CONTENT_READER_UNREVIEWED={content_unreviewed} "
        f"BCL_CONTENT_CLOSURE_UNREVIEWED={bcl_unreviewed} "
        f"CONTENT_READER_MISSING_TYPES={selected_missing} "
        f"CONTENT_READER_STRICT_DIAGNOSTICS={len(selected_diagnostics)} "
        f"CONTENT_READER_ENCODING_DEPENDENCIES={len(encoding_findings)} "
        f"BINARY_READER_AUTHORITY_FINDINGS={binary['unreviewedMembers']} "
        f"RESOURCE_MANAGER_AUTHORITY_FINDINGS={resource['unreviewedMembers']} "
        f"CONTENT_READER_CLOSURE_SELF_TESTS={self_test_count}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
