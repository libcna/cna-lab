#!/usr/bin/env python3
"""The `ProfileCapabilities` table, read out of the registered assembly's IL.

`GraphicsProfile` is not a flag a caller sets and forgets: nine messages this
binding must raise — `Texture2D`'s six and the two buffer constructors' three —
read a per-profile limit and compare it against what the caller asked for. Those
limits live in `Microsoft.Xna.Framework.Graphics.ProfileCapabilities`, an
internal class whose class constructor builds one instance for `Reach` and one
for `HiDef` out of literal `stfld` stores.

That is a table of constants, and a table of constants is exactly the thing a
reimplementation gets subtly wrong by hand. So it is **extracted**, not
transcribed: this tool walks the `.cctor` of the hash-registered
`Microsoft.Xna.Framework.Graphics.dll`, reads each `ldc`/`stfld` pair and each
`List<T>` element, and writes the result to
`reference/xna40-profile-capabilities.json`. `--check` re-extracts and fails if
the pinned file differs, exactly as the resource-string reference is checked.

The IL shape it relies on is the C# compiler's for an object initializer. The
scalars are stored directly and the lists — which the *instance* constructor
allocates — are filled through `Add`:

    newobj  ProfileCapabilities::.ctor();  stloc.1
    ldloc.1; ldc.i4.<n>;                   stfld    <int or bool field>
    ldloc.1; ldfld <list field>; ldc.i4.<n>; callvirt List`1::Add(!0)
    ldloc.1;                               stsfld   ProfileCapabilities::Reach

Anything it cannot read is an error rather than a default, because a silently
missing limit is a check that silently never fires.

Run from the repository root:

    python3 tools/api_compat/profile_capabilities.py \\
        --assembly-dir /path/to/xna/redistributable --il-cache ~/deps/xna-il-cache
    python3 tools/api_compat/profile_capabilities.py --check \\
        --assembly-dir /path/to/xna/redistributable --il-cache ~/deps/xna-il-cache
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REFERENCE = ROOT / "tools/api_compat/reference/xna40-profile-capabilities.json"
ASSEMBLY = "Microsoft.Xna.Framework.Graphics.dll"
CLASS = "Microsoft.Xna.Framework.Graphics.ProfileCapabilities"

# Every field the class declares, with the kind each holds. A field that is
# neither listed nor stored is a change in the assembly, and both directions are
# checked: a listed field that is never stored fails, and a stored field that is
# not listed fails.
SCALAR_FIELDS = {
    "Profile": "enum",
    "VertexShaderVersion": "int",
    "PixelShaderVersion": "int",
    "OcclusionQuery": "bool",
    "GetBackBufferData": "bool",
    "SeparateAlphaBlend": "bool",
    "DestBlendSrcAlphaSat": "bool",
    "MinMaxSrcDestBlend": "bool",
    "MaxPrimitiveCount": "int",
    "IndexElementSize32": "bool",
    "MaxVertexStreams": "int",
    "MaxStreamStride": "int",
    "MaxVertexBufferSize": "int",
    "MaxIndexBufferSize": "int",
    "MaxTextureSize": "int",
    "MaxCubeSize": "int",
    "MaxVolumeExtent": "int",
    "MaxTextureAspectRatio": "int",
    "MaxSamplers": "int",
    "MaxVertexSamplers": "int",
    "MaxRenderTargets": "int",
    "NonPow2Unconditional": "bool",
    "NonPow2Cube": "bool",
    "NonPow2Volume": "bool",
}
LIST_FIELDS = [
    "ValidTextureFormats", "ValidCubeFormats", "ValidVolumeFormats",
    "ValidVertexTextureFormats", "InvalidFilterFormats", "InvalidBlendFormats",
    "ValidDepthFormats", "ValidVertexFormats",
]
PROFILES = ["Reach", "HiDef"]


def disassemble(assembly_dir: Path, il_cache: Path) -> str:
    """The assembly's IL, from the cache when it is there and `ikdasm` when not."""
    path = assembly_dir / ASSEMBLY
    if not path.exists():
        raise SystemExit(f"{ASSEMBLY} is not in {assembly_dir}")
    digest = hashlib.sha256(path.read_bytes()).hexdigest()[:16]
    cached = il_cache / f"{path.stem}-{digest}.il"
    if cached.exists():
        return cached.read_text(encoding="utf-8", errors="replace")
    completed = subprocess.run(
        ["ikdasm", str(path)], capture_output=True, text=True, check=False)
    if completed.returncode != 0:
        raise SystemExit(f"ikdasm failed on {path}: {completed.stderr[:400]}")
    il_cache.mkdir(parents=True, exist_ok=True)
    cached.write_text(completed.stdout, encoding="utf-8")
    return completed.stdout


LDC = re.compile(r"^\s*IL_[0-9a-f]{4}:\s+ldc\.i4(?:\.(?P<short>m?[0-9s]+))?(?:\s+(?P<long>-?(?:0x[0-9a-fA-F]+|\d+)))?\s*$")
STFLD = re.compile(r"^\s*IL_[0-9a-f]{4}:\s+stfld\s+.*" + re.escape(CLASS) + r"::(?P<field>\w+)\s*$")
LDFLD = re.compile(r"^\s*IL_[0-9a-f]{4}:\s+ldfld\s+.*" + re.escape(CLASS) + r"::(?P<field>\w+)\s*$")
ADD = re.compile(r"^\s*IL_[0-9a-f]{4}:\s+callvirt\s+.*System\.Collections\.Generic\.List.*::Add")
STSFLD = re.compile(r"^\s*IL_[0-9a-f]{4}:\s+stsfld\s+.*" + re.escape(CLASS) + r"::(?P<name>\w+)\s*$")


def literal(match: re.Match[str]) -> int:
    short, long_form = match.group("short"), match.group("long")
    if long_form is not None:
        return int(long_form, 0)
    if short is None:
        raise SystemExit("an ldc.i4 with neither a short form nor an operand")
    if short == "s":
        raise SystemExit("ldc.i4.s without its operand")
    if short.startswith("m"):
        return -int(short[1:])
    return int(short)


def cctor_lines(il: str) -> list[str]:
    start = il.index(f".class private auto ansi beforefieldinit {CLASS}\n") \
        if f".class private auto ansi beforefieldinit {CLASS}\n" in il \
        else il.index(CLASS)
    end = il.index(f"}} // end of class {CLASS}", start)
    body = il[start:end]
    marker = "void  .cctor() cil managed"
    if marker not in body:
        raise SystemExit(f"{CLASS} has no class constructor in the disassembly")
    cctor = body[body.index(marker):]
    cctor = cctor[:cctor.index("end of method ProfileCapabilities::.cctor")]
    return cctor.splitlines()


def extract(il: str) -> dict[str, dict[str, object]]:
    """One dictionary per profile, in the order the class constructor builds them."""
    profiles: dict[str, dict[str, object]] = {}
    current: dict[str, object] = {name: [] for name in LIST_FIELDS}
    pending: int | None = None
    pending_list: str | None = None
    for line in cctor_lines(il):
        if (match := LDC.match(line)) is not None:
            pending = literal(match)
            continue
        if (match := LDFLD.match(line)) is not None:
            field = match.group("field")
            if field not in LIST_FIELDS:
                raise SystemExit(
                    f"the class constructor reads {field}, which is not one of "
                    "the list fields this tool knows")
            pending_list = field
            pending = None
            continue
        if ADD.match(line) is not None:
            if pending_list is None or pending is None:
                raise SystemExit("a List.Add with no field and value before it")
            current[pending_list].append(pending)  # type: ignore[union-attr]
            pending, pending_list = None, None
            continue
        if (match := STFLD.match(line)) is not None:
            field = match.group("field")
            if field not in SCALAR_FIELDS:
                raise SystemExit(
                    f"{CLASS} stores {field}, which this tool does not know; "
                    "the assembly's capability table has changed")
            if pending is None:
                raise SystemExit(f"{field} stored with no literal before it")
            current[field] = (bool(pending) if SCALAR_FIELDS[field] == "bool"
                              else pending)
            pending, pending_list = None, None
            continue
        if (match := STSFLD.match(line)) is not None:
            name = match.group("name")
            if name not in PROFILES:
                raise SystemExit(f"{CLASS} assigns an unexpected static {name}")
            missing = set(SCALAR_FIELDS) - set(current)
            if missing:
                raise SystemExit(
                    f"{name} leaves {sorted(missing)} unstored; a limit that is "
                    "never read is a check that never fires")
            # An empty list is a real value, not a parse failure: Reach
            # supports no volume textures and no vertex textures, and has no
            # invalid filter or blend formats, so four of its eight lists are
            # genuinely empty. What cannot be empty is a profile's own texture
            # and depth formats -- a profile with neither is not a profile, and
            # that is the invariant that would catch a list walk that read
            # nothing.
            for field in ("ValidTextureFormats", "ValidDepthFormats"):
                if not current[field]:
                    raise SystemExit(
                        f"{name}.{field} came out empty, which no profile is")
            profiles[name] = current
            current = {field: [] for field in LIST_FIELDS}
            pending, pending_list = None, None
    if sorted(profiles) != sorted(PROFILES):
        raise SystemExit(f"expected {PROFILES}, extracted {sorted(profiles)}")
    return profiles


def document(profiles: dict[str, dict[str, object]], sha: str) -> str:
    payload = {
        "schemaVersion": 1,
        "profile": "XNA 4.0 Windows runtime ProfileCapabilities",
        "note": (
            "Extracted from the class constructor of "
            f"{CLASS} in the hash-registered {ASSEMBLY} by "
            "tools/api_compat/profile_capabilities.py. Only the values are "
            "retained; no Microsoft binary is stored in the repository."),
        "assembly": ASSEMBLY,
        "assemblySha256": sha,
        "profiles": {name: profiles[name] for name in PROFILES},
    }
    return json.dumps(payload, indent=2) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--assembly-dir", required=True, type=Path)
    parser.add_argument("--il-cache", required=True, type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    il = disassemble(args.assembly_dir, args.il_cache)
    sha = hashlib.sha256((args.assembly_dir / ASSEMBLY).read_bytes()).hexdigest()
    rendered = document(extract(il), sha)

    if args.check:
        current = REFERENCE.read_text(encoding="utf-8") if REFERENCE.exists() else ""
        if current != rendered:
            print("PROFILE_CAPABILITIES=STALE")
            return 1
        profiles = json.loads(rendered)["profiles"]
        print(f"PROFILE_CAPABILITIES=CURRENT PROFILES={len(profiles)} "
              f"FIELDS={len(SCALAR_FIELDS) + len(LIST_FIELDS)}")
        return 0

    REFERENCE.parent.mkdir(parents=True, exist_ok=True)
    REFERENCE.write_text(rendered, encoding="utf-8")
    print(f"PROFILE_CAPABILITIES=WRITTEN PROFILES={len(PROFILES)} "
          f"FIELDS={len(SCALAR_FIELDS) + len(LIST_FIELDS)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
