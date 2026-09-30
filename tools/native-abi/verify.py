#!/usr/bin/env python3
"""Verifies the CNA-Java native boundary against the live CNA C headers.

The check has five independent layers, each of which can fail on its own:

``manifest``   every function in ``bindings.json`` still exists in the live CNA
               headers with the exact same return type and parameter types.
``jni``        the JNI adapter loads exactly the manifest's symbols, and every
               dispatch-table slot is declared with ``CNA_JNI_ROUTE`` naming the
               same symbol it loads, so the compiler type-checks each call site
               against the real declaration.
``layout``     ``probe.c`` compiles, asserting the ABI identity plus the exact
               size, alignment and field offsets of every bound structure.
``library``    an actual ``libcna_c_api`` exports every bound symbol.
``policy``     the loaded library's ABI version satisfies the manifest's stated
               compatibility policy.
"""

from __future__ import annotations

import argparse
import ctypes
import json
import os
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from inventory import abi_version, inventory  # noqa: E402


ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "tools/native-abi/bindings.json"
JNI_SOURCES = sorted(list((ROOT / "src/main/c").glob("*.c"))
                     + list((ROOT / "src/main/c").glob("*.inc"))
                     + list((ROOT / "src/main/c/generated").glob("*.inc")))
PROBE = ROOT / "tools/native-abi/probe.c"

LOAD_PATTERN = re.compile(r'LOAD\(\s*([A-Za-z0-9_]+)\s*,\s*"(cna_[A-Za-z0-9_]+)"\s*\)')
ROUTE_PATTERN = re.compile(r"CNA_JNI_ROUTE\((cna_[A-Za-z0-9_]+)\)\s+([A-Za-z0-9_]+);")


def resolve_cna_root(argument: str | None) -> Path:
    """Resolve the authoritative CNA checkout, refusing to guess a different one."""
    candidate = argument or os.environ.get("CNA_ROOT")
    if not candidate:
        raise SystemExit("CNA_ROOT or --cna-root is required; CNA-Java qualifies against the sibling ../cna")
    root = Path(candidate).resolve()
    if not (root / "modules/c-api/include/CNA/C/cna.h").is_file():
        raise SystemExit(f"not a CNA checkout with a C API: {root}")
    return root


def check_manifest(manifest: dict, live: dict) -> list[str]:
    failures: list[str] = []
    functions = live["functions"]
    for entry in manifest["functions"]:
        name = entry["name"]
        declaration = functions.get(name)
        if declaration is None:
            failures.append(f"BOUND_SYMBOL_REMOVED_UPSTREAM={name}")
            continue
        if declaration["returnType"] != entry["returnType"]:
            failures.append(
                f"RETURN_TYPE_DRIFT={name} manifest={entry['returnType']} header={declaration['returnType']}")
        actual = [value["type"] for value in declaration["parameters"]]
        if actual != entry["parameters"]:
            failures.append(
                f"PARAMETER_DRIFT={name} manifest={entry['parameters']} header={actual}")
        if declaration["header"] != entry["header"]:
            failures.append(
                f"HEADER_DRIFT={name} manifest={entry['header']} header={declaration['header']}")
    return failures


def check_jni(manifest: dict) -> list[str]:
    failures: list[str] = []
    source = "\n".join(path.read_text(encoding="utf-8") for path in JNI_SOURCES)
    loaded = dict((field, symbol) for field, symbol in LOAD_PATTERN.findall(source))
    routes = dict((field, symbol) for symbol, field in ROUTE_PATTERN.findall(source))
    declared = {entry["name"] for entry in manifest["functions"]}

    for symbol in sorted(set(loaded.values()) - declared):
        failures.append(f"JNI_SYMBOL_MISSING_FROM_MANIFEST={symbol}")
    for symbol in sorted(declared - set(loaded.values())):
        failures.append(f"MANIFEST_SYMBOL_MISSING_FROM_JNI={symbol}")
    for field in sorted(loaded.keys() - routes.keys()):
        failures.append(f"JNI_SLOT_NOT_HEADER_DERIVED={field}")
    for field in sorted(routes.keys() & loaded.keys()):
        if routes[field] != loaded[field]:
            failures.append(
                f"JNI_SLOT_SYMBOL_MISMATCH={field} declared={routes[field]} loaded={loaded[field]}")
    for field in sorted(routes.keys() - loaded.keys()):
        failures.append(f"JNI_SLOT_NEVER_LOADED={field}")
    return failures


def compile_probe(include: Path) -> None:
    compiler = os.environ.get("CC", "cc")
    import tempfile
    with tempfile.TemporaryDirectory(prefix="cna-java-abi-") as directory:
        output = Path(directory) / "probe.o"
        subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
                        "-I", str(include), "-c", str(PROBE), "-o", str(output)], check=True)


JAVA_COMPILED_ABI = re.compile(
    r"COMPILED_ABI_VERSION\s*=\s*encodeVersion\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)")


def check_java_abi(manifest: dict) -> list[str]:
    """The Java load-time gate must state the same ABI the manifest does.

    ``NativeBindings.COMPILED_ABI_VERSION`` is what a game actually hits when it loads the
    library; the manifest is what every tool here measures. A manifest that moved without the
    constant would pass every check and then refuse the library at run time, which is the one
    failure a build gate exists to prevent.
    """
    source = (ROOT / "src/main/java/org/openeggbert/cna/internal/NativeBindings.java").read_text(
        encoding="utf-8")
    found = JAVA_COMPILED_ABI.search(source)
    if found is None:
        return ["JAVA_COMPILED_ABI_NOT_FOUND=NativeBindings.COMPILED_ABI_VERSION"]
    java = ".".join(found.groups())
    if java != manifest["compiledAbi"]:
        return [f"JAVA_COMPILED_ABI_DRIFT=java {java}, manifest {manifest['compiledAbi']}"]
    return []


def check_policy(manifest: dict, encoded: int) -> list[str]:
    policy = manifest.get("abiPolicy", {})
    required = manifest["compiledAbi"].split(".")
    major, minor = int(required[0]), int(required[1])
    actual_major, actual_minor = encoded >> 16, (encoded >> 8) & 0xFF
    failures: list[str] = []
    if policy.get("requireExactMajor", True) and actual_major != major:
        failures.append(f"ABI_MAJOR_INCOMPATIBLE=required {major}, loaded {actual_major}")
    if policy.get("requireExactMinor", True) and actual_minor != minor:
        failures.append(f"ABI_MINOR_INCOMPATIBLE=required {minor}, loaded {actual_minor}")
    elif actual_minor < minor:
        failures.append(f"ABI_MINOR_TOO_OLD=required at least {minor}, loaded {actual_minor}")
    return failures


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cna-root", default=None)
    parser.add_argument("--library", default=os.environ.get("CNA_NATIVE_LIBRARY"))
    parser.add_argument("--inventory-output")
    arguments = parser.parse_args()

    cna_root = resolve_cna_root(arguments.cna_root)
    include = cna_root / "modules/c-api/include"
    live = inventory(include, strict=True)
    header_abi = abi_version(include)
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))

    if arguments.inventory_output:
        path = Path(arguments.inventory_output)
        path.parent.mkdir(parents=True, exist_ok=True)
        payload = dict(live)
        payload["abiVersion"] = header_abi
        path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    failures: list[str] = []
    if manifest["compiledAbi"] != header_abi:
        failures.append(
            f"MANIFEST_ABI_DRIFT=manifest {manifest['compiledAbi']}, headers {header_abi}")
    failures.extend(check_java_abi(manifest))
    failures.extend(check_manifest(manifest, live))
    failures.extend(check_jni(manifest))

    if failures:
        for failure in failures:
            print(failure)
        return 1

    compile_probe(include)

    bound = len(manifest["functions"])
    print(f"CNA_ROOT={cna_root}")
    print(f"HEADER_ABI={header_abi}")
    print(f"CANONICAL_FUNCTIONS={len(live['functions'])}")
    print(f"BOUND_FUNCTIONS={bound}")
    print(f"UNBOUND_FUNCTIONS={len(live['functions']) - bound}")
    print("MANIFEST_SIGNATURE_CHECK=PASS")
    print("MANIFEST_JNI_BINDING_CHECK=PASS")
    print("JNI_HEADER_DERIVED_SLOT_CHECK=PASS")
    print("JAVA_COMPILED_ABI_CHECK=PASS")
    print("LAYOUT_SIGNATURE_PROBE=PASS")

    if not arguments.library:
        print("LIBRARY_SYMBOL_CHECK=SKIPPED (CNA_NATIVE_LIBRARY not set)")
        return 0

    library_path = Path(arguments.library).resolve()
    if not library_path.is_file():
        print(f"native library not found: {library_path}", file=sys.stderr)
        return 2
    symbols = subprocess.run(["nm", "-D", "--defined-only", str(library_path)], check=True,
                             text=True, stdout=subprocess.PIPE).stdout
    # GNU ld appends the ELF version namespace (for example
    # ``cna_game_run@@CNA_C_API_0.1``) to nm's display name.  dlsym and JNI use
    # the unversioned public symbol, so compare that identity here as well.
    exported = {line.split()[-1].split("@", 1)[0] for line in symbols.splitlines() if line.split()}
    missing = sorted({entry["name"] for entry in manifest["functions"]} - exported)
    if missing:
        for name in missing:
            print("MISSING_SYMBOL=" + name)
        return 1

    library = ctypes.CDLL(str(library_path))
    library.cna_get_abi_version.restype = ctypes.c_uint32
    encoded = int(library.cna_get_abi_version())
    print(f"LIBRARY_ABI={encoded >> 16}.{(encoded >> 8) & 0xff}.{encoded & 0xff}")
    print(f"LIBRARY_SYMBOL_CHECK=PASS ({bound}/{bound})")
    policy_failures = check_policy(manifest, encoded)
    for failure in policy_failures:
        print(failure)
    if policy_failures:
        return 1
    print("ABI_POLICY_CHECK=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
