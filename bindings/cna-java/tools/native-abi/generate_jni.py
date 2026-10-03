#!/usr/bin/env python3
"""Generates the JNI adapter and Java declarations for scalar CNA C API routes.

The native boundary is large and CNA is still evolving, so the mechanical part
of it is generated from the live headers instead of hand-written.  A route
listed in ``routes.json`` names only its Java class, its Java method name and
its CNA symbol; every JNI type, every marshalling step and the Java ``native``
declaration are derived from that symbol's declaration in ``CNA/C/*.h``.

What is generated is only the *boundary*.  The public Java API is a semantic
XNA/CNA facade written by hand; nothing here produces public API.

Supported parameter shapes, all derived from the header:

``value``     a scalar, identity or handle passed by value
``out``       a scalar, identity or handle out-parameter (its C name starts
              with ``out_``), projected as a one-element Java array
``string``    a ``CNA_StringView`` input, projected as a UTF-8 ``byte[]``
``text``      the ``char* buffer, uint64_t capacity, uint64_t* out_written``
              triple CNA uses for a copy-out string, projected as one
              ``byte[]`` whose length supplies the capacity plus a ``long[]``
``struct``    a flat POD struct out-parameter, flattened to its scalar leaves
              and projected as a ``long[]`` and/or a ``float[]``
``bytes``     an opaque ``void*`` buffer, projected as a ``byte[]``, whose byte
              extent ``byteBuffers`` names -- see below
``string_array``
              a ``const CNA_StringView*`` input followed by its count, projected
              as one ``byte[][]``; each element is copied out rather than pinned

``parallelArrays`` names two or more array parameters that share one count parameter, which C
writes as ``T* a, U* b, uint64_t n`` without saying whether ``n`` counts one of them or both.
Java carries each length in its own array, so the count disappears and a mismatch between the
arrays is refused before either is acquired.

A route whose declaration uses anything else -- a callback, an undeclared
``void*``, an array of structs -- is refused with a diagnostic rather than
guessed at, and stays hand-written.

A ``void*`` says nothing at all: not what it points at, not how much of it CNA
reads or writes, and not whether the extent is one parameter or the product of
two.  ``byteBuffers`` names the parameters whose product is its byte count, and
the adapter refuses a Java array too small for what it was told to pass rather
than letting CNA read past the end of it.

A bare ``T*`` is one structure or an array of them and C does not say which, so
the generator reads the parameter's own documentation: one that names a count is
refused until ``routes.json`` says what it is.  ``arrayLengths`` gives the
element count; ``singleStructs`` says the count in the prose is the structure's
own fields rather than a number of structures.

``elementFloats`` is the same problem for a counted array whose count is not its
length: a ``vec3`` array is "three floats per element, tightly packed", so a Java
``float[]`` of ``3n`` floats is ``n`` vectors and passing ``3n`` would describe an
array three times the size the shader declares.  Both parameters are just floats
and a count, so the grouping cannot be derived; ``routes.json`` states it and the
adapter refuses a length that is not a multiple of it.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from inventory import inventory  # noqa: E402


ROOT = Path(__file__).resolve().parents[2]
ROUTES = ROOT / "tools/native-abi/routes.json"
GENERATED_C = ROOT / "src/main/c/generated"
GENERATED_JAVA = ROOT / "src/main/java/org/openeggbert/cna/internal/generated"

INTEGRAL = {
    "uint8_t": ("jbyte", "int8"), "int8_t": ("jbyte", "int8"),
    "uint16_t": ("jint", "int32"), "int16_t": ("jshort", "int16"),
    "uint32_t": ("jint", "int32"), "int32_t": ("jint", "int32"),
    "uint64_t": ("jlong", "int64"), "int64_t": ("jlong", "int64"),
    "CNA_Bool": ("jboolean", "bool"),
}
JAVA_OF_JNI = {"jbyte": "byte", "jshort": "short", "jint": "int", "jlong": "long",
               "jfloat": "float", "jdouble": "double", "jboolean": "boolean"}


# The names the generated JNI adapter gives its own two parameters. A CNA parameter that shares
# one of them would be a duplicate declaration in C, so it is renamed on the way in.
ADAPTER_RESERVED_NAMES = frozenset({"environment", "declaring_class"})


class Unsupported(Exception):
    pass


# A number word followed closely by a plural, or a reference to one of CNA's own count
# constants. Either one in a parameter's documentation means the parameter is more than one of
# whatever it points at.  The plural is what keeps "range twenty" and "two-sided" out of it: a
# number alone is a value, and a number of *things* is a count.
COUNT_IN_PROSE = re.compile(
    r"\b(?:two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|sixteen|"
    r"twenty|thirty-two|sixty-four)\s+(?:\w+\s+){0,1}\w+s\b"
    r"|CNA_[A-Z0-9_]*COUNT[A-Z0-9_]*",
    re.IGNORECASE)


def counts_more_than_one(documentation: str) -> bool:
    """Reports whether a parameter's own documentation says it holds several elements."""
    return bool(documentation) and bool(COUNT_IN_PROSE.search(documentation))


def bare(type_name: str) -> tuple[str, int]:
    value = type_name.replace("const ", "").strip()
    pointers = value.count("*")
    return value.replace("*", "").strip(), pointers


def classify_scalar(name: str, live: dict) -> tuple[str, str]:
    """Return (jni type, marshalling kind) for one scalar C type."""
    if name in INTEGRAL:
        return INTEGRAL[name]
    if name == "float":
        return "jfloat", "float"
    if name == "double":
        return "jdouble", "double"
    if name == "CNA_Handle" or name in live["handles"]:
        return "jlong", "handle"
    identity = live["identities"].get(name)
    if identity is not None:
        return INTEGRAL.get(identity["underlying"], ("jint", "int32"))
    raise Unsupported(f"not a scalar C type: {name}")


def array_extent(where: str, extent: str, live: dict) -> int:
    """Resolve a fixed array's extent, which CNA writes as a literal or as one of its own macros.

    ``CNA_Matrix world_to_atlas[CNA_SHADOW_CASCADE_MAX_EXT]`` is the same layout as
    ``[4]`` and has to flatten the same way, so the macro is looked up rather than
    the field refused. A macro whose value is not a plain positive integer is
    refused instead of evaluated: guessing at an expression is exactly the kind of
    arithmetic that would silently mis-size a structure.
    """
    if extent.isdigit():
        count = int(extent)
    else:
        constant = live["constants"].get(extent)
        if constant is None:
            raise Unsupported(f"{where}: array extent {extent} is not a CNA constant")
        value = str(constant["value"]).strip()
        # CNA writes some of these with the standard fixed-width constant macros --
        # INT32_C(7) rather than 7 -- which mean exactly their argument and nothing else.
        # Unwrapping that one form is reading the header, not evaluating an expression;
        # anything else is still refused below.
        wrapped = re.fullmatch(r"U?INT(?:8|16|32|64)_C\(\s*(\d+)\s*\)", value)
        if wrapped is not None:
            value = wrapped.group(1)
        if not value.isdigit():
            raise Unsupported(f"{where}: array extent {extent} is {value!r}, not a plain integer")
        count = int(value)
    if count < 1:
        raise Unsupported(f"{where}: array extent {extent} is not positive")
    return count


def prefix_fields(name: str, live: dict, prefix: dict | None) -> list[dict]:
    """Return a structure's fields, stopped before the one a declared prefix names.

    CNA grows some of its structures by appending, and documents the earlier
    size as a constant: a caller compiled against version one sets
    ``struct_size`` to it and every route still works, because CNA never reads
    past what ``struct_size`` declares. That is the mechanism this uses, not a
    truncation invented here -- which is why the declaration has to name both
    the field to stop before and CNA's own size constant, and why both are
    checked against the live headers.
    """
    fields = live["structures"][name]["fields"]
    if prefix is None:
        return fields
    stop = prefix["stopBefore"]
    names = [field["name"] for field in fields]
    if stop not in names:
        raise Unsupported(f"{name}: no field named {stop} to stop before")
    if prefix["sizeConstant"] not in live["constants"]:
        raise Unsupported(f"{name}: {prefix['sizeConstant']} is not a CNA constant")
    return fields[:names.index(stop)]


def flatten_struct(name: str, live: dict, seen: frozenset[str] = frozenset(),
                   prefix: dict | None = None) -> list[tuple[str, str]]:
    """Flatten a POD struct into its scalar leaves as (field path, C type).

    A fixed-size array field expands to one leaf per element, so padding and
    fixed character buffers cross the boundary exactly as they are laid out
    rather than forcing the whole route to be hand-written.
    """
    if name in seen:
        raise Unsupported(f"recursive structure: {name}")
    structure = live["structures"].get(name)
    if structure is None:
        raise Unsupported(f"not a known structure: {name}")
    leaves: list[tuple[str, str]] = []
    for field in prefix_fields(name, live, prefix):
        field_type, pointers = bare(field["type"])
        if pointers or not field["name"]:
            raise Unsupported(f"{name}.{field['name']}: unsupported field type {field['type']}")
        extent = re.fullmatch(
            r"(?P<element>[A-Za-z_][A-Za-z0-9_]*)\[(?P<count>[A-Za-z0-9_]+)\]", field_type)
        if extent is not None:
            element = extent.group("element")
            count = array_extent(f"{name}.{field['name']}", extent.group("count"), live)
            if element in live["structures"]:
                # An array of structures is an array and a structure, and both are already
                # understood one at a time. Expanding it here is what keeps a fixed-extent
                # member -- four cascade transforms, seven texture transforms -- a layout the
                # generator lays out rather than a shape it refuses.
                for index in range(count):
                    for path, leaf in flatten_struct(element, live, seen | {name}):
                        leaves.append((f"{field['name']}[{index}].{path}", leaf))
                continue
            if element != "char":
                classify_scalar(element, live)
            for index in range(count):
                leaves.append((f"{field['name']}[{index}]", element))
            continue
        if "[" in field_type:
            raise Unsupported(f"{name}.{field['name']}: unsupported field type {field['type']}")
        if field_type == "CNA_StringView":
            # A borrowed string, not a POD leaf. Recursing would reach its ``const char*``
            # and refuse the whole structure; carrying it as its own leaf lets one ``byte[]``
            # per field cross, pinned for the length of the call the way a top-level
            # ``CNA_StringView`` parameter already is.
            leaves.append((field["name"], "CNA_StringView"))
            continue
        if field_type in live["structures"]:
            for path, leaf in flatten_struct(field_type, live, seen | {name}):
                leaves.append((f"{field['name']}.{path}", leaf))
        else:
            classify_scalar(field_type, live)
            leaves.append((field["name"], field_type))
    return leaves


VERSION_FIELDS = ("struct_size", "struct_version")

# The Java arrays a flattened structure's leaves travel in. A C ``double`` gets its
# own carrier rather than sharing the float one: narrowing it to ``jfloat`` would
# lose precision silently, which is exactly the kind of guess this generator refuses
# to make elsewhere.
STRUCT_GROUPS = (
    ("bytes", "jbyte", "Byte", "byte"),
    ("integral", "jlong", "Long", "long"),
    ("floating", "jfloat", "Float", "float"),
    ("doubles", "jdouble", "Double", "double"),
)


def version_paths(name: str, live: dict, prefix: str = "",
                  seen: frozenset[str] = frozenset(),
                  declared: dict | None = None) -> list[tuple[str, str, str]]:
    """Return every ``struct_size``/``struct_version`` leaf, nested ones included.

    CNA requires each versioned structure to be stamped from the exact header the
    caller compiled against, which is a fact C knows and Java does not.  A nested
    versioned structure needs stamping just as much as the outer one, so the walk
    is recursive: leaving a nested pair at whatever Java happened to send would
    hand CNA a structure that says it is zero bytes long.
    """
    structure = live["structures"].get(name)
    if structure is None or name in seen:
        return []
    visible = prefix_fields(name, live, declared)
    fields = {field["name"] for field in visible}
    found: list[tuple[str, str, str]] = []
    for version_field in VERSION_FIELDS:
        if version_field in fields:
            # A declared prefix says both how big the structure the caller filled in is and
            # which version that makes it, and CNA names both. Stamping sizeof here instead
            # would tell CNA the whole structure was written when the tail never was.
            if version_field == "struct_size" and declared is not None:
                found.append((prefix + version_field, "declared_size",
                              declared["sizeConstant"]))
            elif version_field == "struct_version" and declared is not None:
                found.append((prefix + version_field, version_field,
                              f"UINT32_C({declared['version']})"))
            else:
                found.append((prefix + version_field, version_field,
                              struct_version(name, live)))
    for field in visible:
        field_type, pointers = bare(field["type"])
        if pointers:
            continue
        extent = re.fullmatch(
            r"(?P<element>[A-Za-z_][A-Za-z0-9_]*)\[(?P<count>[A-Za-z0-9_]+)\]", field_type)
        if extent is not None:
            # Every element of a fixed array of versioned structures needs stamping, not just
            # the first. A material carries seven texture transforms, and seven unstamped ones
            # would each tell CNA they were zero bytes long -- which is the silent half of the
            # same mistake this whole walk exists to prevent.
            element = extent.group("element")
            if element not in live["structures"]:
                continue
            for index in range(array_extent(f"{name}.{field['name']}",
                                            extent.group("count"), live)):
                found.extend(version_paths(element, live,
                                           f"{prefix}{field['name']}[{index}].", seen | {name}))
            continue
        if field_type not in live["structures"]:
            continue
        found.extend(version_paths(field_type, live, f"{prefix}{field['name']}.",
                                   seen | {name}))
    return found


def stamp_versions(target: str, versions: list[tuple[str, str, str]]) -> list[str]:
    """Emit the assignments that stamp every versioned structure, nested ones included."""
    lines: list[str] = []
    for path, kind, version in versions:
        member = path.rsplit(".", 1)[0] if "." in path else None
        subject = f"{target}.{member}" if member else target
        if kind == "declared_size":
            lines.append(f"    {target}.{path} = (uint32_t)({version});")
        elif kind == "struct_size":
            lines.append(f"    {target}.{path} = (uint32_t)(sizeof {subject});")
        else:
            lines.append(f"    {target}.{path} = {version};")
    return lines


def group_leaves(leaves: list[tuple[str, str]]) -> dict[str, list[tuple[str, str]]]:
    """Split a struct's leaves into the byte, float and integral arrays that carry them.

    ``struct_size`` and ``struct_version`` never cross into Java, at any nesting
    depth. CNA requires the caller to set them from the exact header it compiled
    against, which is a fact C knows and Java does not, so the generated adapter
    fills them in itself.
    """
    leaves = [(path, leaf) for path, leaf in leaves
              if path.rsplit(".", 1)[-1] not in VERSION_FIELDS]
    views = [(path, leaf) for path, leaf in leaves if leaf == "CNA_StringView"]
    leaves = [(path, leaf) for path, leaf in leaves if leaf != "CNA_StringView"]
    return {
        "bytes": [(path, leaf) for path, leaf in leaves
                  if leaf in ("char", "uint8_t") and path.endswith("]")],
        "floating": [(path, leaf) for path, leaf in leaves if leaf == "float"],
        "doubles": [(path, leaf) for path, leaf in leaves if leaf == "double"],
        "integral": [(path, leaf) for path, leaf in leaves
                     if leaf not in ("float", "double")
                     and not (leaf in ("char", "uint8_t") and path.endswith("]"))],
        # Each string view is its own ``byte[]`` rather than a slot in a shared array,
        # because a view is a pointer and a length, not a value that fits beside the others.
        "views": views,
    }


def plan(route: dict, live: dict) -> dict:
    """Derive the complete marshalling plan for one route from its declaration."""
    declaration = live["functions"].get(route["symbol"])
    if declaration is None:
        raise Unsupported(f"{route['symbol']}: not declared by the live CNA headers")
    if declaration["returnType"] != "CNA_Result":
        raise Unsupported(f"{route['symbol']}: returns {declaration['returnType']}, not CNA_Result")

    parameters = declaration["parameters"]
    declared_names = {parameter["name"] for parameter in parameters}
    steps: list[dict] = []
    # Two arrays that share one count. C writes them as `T* a, U* b, uint64_t n` and says
    # nothing about whether `n` counts one of them or both, so routes.json states it: the key
    # is the count parameter and the value is every array it counts. Java carries the length in
    # each array, so the count disappears and a mismatch between the two is refused.
    shared_count: dict[str, str] = {}
    for count_name, members in route.get("parallelArrays", {}).items():
        for member in members:
            shared_count[member] = count_name
    # Where each parameter's value can be read in C before anything has been acquired. A count
    # that CNA declares as its own parameter is a JNI argument; one that a preceding array
    # consumed is that array's length, which GetArrayLength answers with no side effect.
    available: dict[str, str] = {}
    index = 0
    while index < len(parameters):
        parameter = parameters[index]
        raw = parameter["type"]
        type_name, pointers = bare(raw)
        constant = raw.startswith("const ")
        name = parameter["name"] or f"argument{index}"
        if name in ADAPTER_RESERVED_NAMES:
            # The adapter's own JNI parameters are called `environment` and `declaring_class`,
            # and CNA has a route whose parameter is called `environment` too. Two parameters of
            # one function cannot share a name, so the CNA one is suffixed here -- in the C
            # adapter only. The Java declaration derives its own name separately and is
            # untouched, so nothing a caller sees changes.
            name = f"{name}_parameter"
            parameter = dict(parameter, name=name)
            parameters = list(parameters)
            parameters[index] = parameter
            following = parameters[index + 1] if index + 1 < len(parameters) else None
        following = parameters[index + 1] if index + 1 < len(parameters) else None

        if (route.get("nullCallback") and following is not None
                and type_name in live["callbacks"] and bare(following["type"])[0] == "void"):
            # CNA's asynchronous routes complete synchronously and document the callback as
            # optional, so a route explicitly marked nullCallback in routes.json passes a null
            # callback and null context and reads its result from the out-parameters.
            steps.append({"shape": "null_callback", "name": name, "ctype": type_name})
            index += 2
            continue
        if pointers == 0 and name in route.get("parallelArrays", {}):
            members = route["parallelArrays"][name]
            steps.append({"shape": "shared_count", "name": name, "members": members,
                          "ctype": type_name})
            index += 1
            continue
        if pointers == 0 and type_name == "CNA_StringView":
            steps.append({"shape": "string", "name": name})
            index += 1
            continue
        if pointers == 0 and type_name in live["structures"]:
            declared = route.get("structPrefixes", {}).get(name)
            raw_leaves = flatten_struct(type_name, live, prefix=declared)
            groups = group_leaves(raw_leaves)
            steps.append({"shape": "struct_value", "name": name, "ctype": type_name,
                          "versions": version_paths(type_name, live, declared=declared),
                          **groups})
            index += 1
            continue
        if pointers == 0:
            jni, kind = classify_scalar(type_name, live)
            steps.append({"shape": "value", "name": name, "jni": jni, "kind": kind,
                          "ctype": type_name})
            available[name] = name
            index += 1
            continue
        if pointers == 1 and type_name == "void":
            # A void* is the one C type that says nothing whatsoever: not what it points at, not
            # how many bytes of it CNA touches, and not whether that count is one parameter or
            # the product of two. Every other shape here is derived from the declaration; this
            # one cannot be, and a wrong guess is a read or a write past the end of a Java array
            # from inside C. So it is refused until routes.json states the extent.
            declared = route.get("byteBuffers", {}).get(name)
            if declared is None:
                raise Unsupported(
                    f"{route['symbol']}: '{name}' is a {raw} whose byte extent the declaration "
                    "does not state -- C says neither what it points at nor how much of it CNA "
                    "reads; name the parameters whose product is its byte count in byteBuffers")
            extent = list(declared["extent"])
            if not extent:
                raise Unsupported(
                    f"{route['symbol']}: byteBuffers names no extent parameter for '{name}'")
            for part in extent:
                if part not in declared_names:
                    raise Unsupported(
                        f"{route['symbol']}: byteBuffers names '{part}', which is not a "
                        "parameter of this route")
            steps.append({"shape": "bytes", "name": name, "input": constant, "extent": extent})
            index += 1
            continue
        if pointers == 1 and type_name == "char" and index + 2 < len(parameters):
            capacity, written = parameters[index + 1], parameters[index + 2]
            if bare(capacity["type"]) == ("uint64_t", 0) and bare(written["type"]) == ("uint64_t", 1):
                steps.append({"shape": "text", "name": name, "written": written["name"]})
                index += 3
                continue
            raise Unsupported(f"{route['symbol']}: char* is not a count/copy triple")
        if (pointers == 1 and type_name == "CNA_StringView"
                and (name in shared_count
                     or (following is not None
                         and bare(following["type"]) == ("uint64_t", 0)))):
            # An array of string views: a message box's button labels, a file dialog's answer
            # list. Every other string crosses as one pinned byte[]; several at once cannot,
            # because each pinned array needs a local reference held for as long as it is
            # pinned and JNI promises only sixteen. The adapter copies each element out
            # instead, which bounds the local references at one.
            #
            # Only an input. An output array of views would be CNA's memory on terms the
            # declaration does not state, which is the same refusal an output structure
            # carrying a view already gets.
            if not constant:
                raise Unsupported(
                    f"{route['symbol']}: '{name}' is a non-const CNA_StringView*, so it is an "
                    "output array of views whose lifetime the declaration does not state")
            if name in shared_count:
                steps.append({"shape": "string_array", "name": name, "shared": True,
                              "count": shared_count[name]})
                index += 1
                continue
            steps.append({"shape": "string_array", "name": name, "shared": False,
                          "count": following["name"] or "count"})
            index += 2
            continue
        if (pointers == 1 and type_name in live["structures"] and following is not None
                and bare(following["type"]) == ("uint64_t", 0)):
            # A pointer to a struct followed by a count is an array of structs, not one
            # struct. Each element's leaves are laid out end to end, so the Java array's
            # length divided by the leaves per element gives the count.
            raw_leaves = flatten_struct(type_name, live)
            groups = group_leaves(raw_leaves)
            if not any(groups[group] for group, _, _, _ in STRUCT_GROUPS):
                raise Unsupported(f"{route['symbol']}: empty structure array {type_name}")
            if groups["views"]:
                raise Unsupported(
                    f"{route['symbol']}: {type_name}[] carries a CNA_StringView, whose pointer "
                    "would have to outlive one element's marshalling")
            steps.append({"shape": "struct_array", "name": name, "ctype": type_name,
                          "input": constant, "versions": version_paths(type_name, live),
                          "count": following["name"] or "count", **groups})
            index += 2
            continue
        if pointers == 1 and type_name in live["structures"]:
            declared = route.get("structPrefixes", {}).get(name)
            raw_leaves = flatten_struct(type_name, live, prefix=declared)
            groups = group_leaves(raw_leaves)
            declared_extent = route.get("arrayLengths", {}).get(name)
            if declared_extent is not None:
                # A struct pointer whose element count CNA states in prose rather than in a
                # count parameter -- "destination for eight corners". Marshalled exactly like a
                # counted array of structs, with the extent taken from the declaration and the
                # Java array required to match it.
                if groups["views"]:
                    raise Unsupported(
                        f"{route['symbol']}: {type_name}[] carries a CNA_StringView, whose "
                        "pointer would have to outlive one element's marshalling")
                steps.append({"shape": "struct_array", "name": name, "ctype": type_name,
                              "input": constant, "versions": version_paths(type_name, live),
                              "count": None,
                              "extent": int(declared_extent["length"]), **groups})
                index += 1
                continue
            if (counts_more_than_one(parameter.get("doc", ""))
                    and name not in route.get("singleStructs", ())):
                # CNA's own documentation for this parameter names a count, so it is an array
                # rather than one structure -- and nothing in the C declaration says so. This is
                # the shape that would otherwise be marshalled as a single element and then
                # handed to a function that reads or writes several: a stack overflow on the way
                # out and a heap overread on the way in, both silent. Refused until routes.json
                # states the extent.
                raise Unsupported(
                    f"{route['symbol']}: '{name}' is a {type_name}* whose documentation names a "
                    f"count -- \"{parameter['doc']}\" -- so whether it is one structure or an "
                    "array cannot be read off the declaration; declare its length in "
                    "arrayLengths, or name it in singleStructs when the count is the "
                    "structure's own fields rather than a number of structures")
            in_out = name in route.get("inOut", ())
            if not constant and not in_out and not (
                    name.startswith("out") or name == "destination"):
                # A non-const struct pointer is an output, or it is read and written. The
                # declaration does not say which, and guessing wrong is silent: an in/out
                # structure treated as an output starts zeroed, so the caller's values are
                # discarded and the route answers about a structure nobody asked about. CNA
                # names a pure output `out_*` or `destination`; anything else has to be declared.
                raise Unsupported(
                    f"{route['symbol']}: '{name}' is a non-const {type_name}* that is neither "
                    "named as an output nor declared in inOut, so whether it is read as well as "
                    "written cannot be read off the declaration")
            if groups["views"] and not constant:
                # An output structure's string view points at memory CNA owns, on terms this
                # generator cannot read off the declaration. Reading it here would be a guess
                # about a lifetime, so the route is refused instead.
                raise Unsupported(
                    f"{route['symbol']}: output {type_name} carries a CNA_StringView, whose "
                    "lifetime the declaration does not state")
            steps.append({"shape": "struct", "name": name, "ctype": type_name,
                          "input": constant, "inOut": in_out,
                          "optional": name in route.get("optionalStructs", ()),
                          "versions": version_paths(type_name, live, declared=declared),
                          **groups})
            index += 1
            continue
        if pointers == 1 and name in shared_count:
            jni, kind = classify_scalar(type_name, live)
            steps.append({"shape": "array", "name": name, "jni": jni, "kind": kind,
                          "ctype": type_name, "input": constant, "shared": True,
                          "countType": "uint64_t", "grouping": 1,
                          "count": shared_count[name]})
            index += 1
            continue
        if (pointers == 1 and following is not None
                and (bare(following["type"]) == ("uint64_t", 0)
                     or (bare(following["type"]) == ("int32_t", 0)
                         and (following["name"] or "").endswith("count")))):
            # CNA passes an array as a pointer immediately followed by its element count or
            # capacity. Java carries the length in the array itself, so the count parameter
            # disappears from the Java declaration.
            #
            # The count is a `uint64_t` in almost every route, and CNA writes a few as a signed
            # `int32_t` instead. Those are accepted only where the parameter is actually named a
            # count: a `uint64_t capacity` beside a copy-out buffer is a count too, but a bare
            # trailing `int32_t` next to a pointer is not necessarily one at all, and reading it
            # as a length is the kind of guess this generator refuses.
            #
            # `elementFloats` is for the case where the count is not the array's length: a vec3
            # array is "three floats per element, tightly packed", so a Java float[] of 3n floats
            # is n vectors, and passing 3n would describe an array three times the size of the one
            # the shader declares. The grouping cannot be read off the declaration -- both
            # parameters are floats and a count -- so routes.json states it, and the adapter
            # refuses a length that is not a multiple of it rather than truncating.
            grouping = int(route.get("elementFloats", {}).get(name, 1))
            if grouping < 1:
                raise Unsupported(
                    f"{route['symbol']}: elementFloats for '{name}' must be positive")
            jni, kind = classify_scalar(type_name, live)
            count_name = following["name"] or "count"
            steps.append({"shape": "array", "name": name, "jni": jni, "kind": kind,
                          "ctype": type_name, "input": constant,
                          "countType": bare(following["type"])[0],
                          "grouping": grouping,
                          "count": count_name})
            available[count_name] = (
                f"(*environment)->GetArrayLength(environment, {name})")
            index += 2
            continue
        if pointers == 1 and name in route.get("arrayLengths", {}):
            # An input array whose length CNA states in prose rather than in a count parameter:
            # "16 floats per joint", "one matrix per bone". The generator will not infer that --
            # a wrong guess is a buffer overrun in C -- so routes.json declares it explicitly and
            # the adapter checks the Java array against the declaration before passing it.
            declared = route["arrayLengths"][name]
            jni, kind = classify_scalar(type_name, live)
            count = declared.get("count")
            if count is not None and count not in available:
                raise Unsupported(
                    f"{route['symbol']}: arrayLengths names '{count}', which is not a parameter "
                    "this route reads before it acquires anything")
            steps.append({"shape": "sized_array", "name": name, "jni": jni, "kind": kind,
                          "ctype": type_name,
                          "per": int(declared.get("per", declared.get("length", 1))),
                          "count": available.get(count) if count else None,
                          "nullable": bool(declared.get("nullable", False))})
            index += 1
            continue
        if pointers == 1:
            jni, kind = classify_scalar(type_name, live)
            if not name.startswith("out"):
                raise Unsupported(f"{route['symbol']}: pointer parameter '{name}' is not an output")
            steps.append({"shape": "out", "name": name, "jni": jni, "kind": kind,
                          "ctype": type_name})
            index += 1
            continue
        raise Unsupported(f"{route['symbol']}: unsupported parameter '{parameter['type']}'")
    return {"symbol": route["symbol"], "java": route["java"], "steps": steps,
            "header": declaration["header"]}


def java_name(name: str) -> str:
    """Convert a C parameter name to the Java spelling used in generated declarations.

    A parameter renamed to avoid colliding with the adapter's own C parameters gets
    its CNA name back here: the collision is a C one and Java has none, so a Java
    declaration should read the way the header does.
    """
    if name.endswith("_parameter") and name[: -len("_parameter")] in ADAPTER_RESERVED_NAMES:
        name = name[: -len("_parameter")]
    pieces = name.split("_")
    return pieces[0] + "".join(piece[:1].upper() + piece[1:] for piece in pieces[1:])


def view_parameter(name: str, path: str) -> str:
    """Name the ``byte[]`` that carries one ``CNA_StringView`` field of a structure."""
    pieces = path.replace(".", "_").split("_")
    return java_name(name) + "".join(piece[:1].upper() + piece[1:] for piece in pieces)


def java_signature(entry: dict) -> tuple[str, list[str]]:
    parameters: list[str] = []
    for step in entry["steps"]:
        if step["shape"] == "value":
            parameters.append(f"{JAVA_OF_JNI[step['jni']]} {java_name(step['name'])}")
        elif step["shape"] == "out":
            parameters.append(f"{JAVA_OF_JNI[step['jni']]}[] {java_name(step['name'])}")
        elif step["shape"] in ("string", "text"):
            parameters.append(f"byte[] {java_name(step['name'])}")
            if step["shape"] == "text":
                parameters.append(f"long[] {java_name(step['written'])}")
        elif step["shape"] in ("array", "sized_array"):
            parameters.append(f"{JAVA_OF_JNI[step['jni']]}[] {java_name(step['name'])}")
        elif step["shape"] == "bytes":
            parameters.append(f"byte[] {java_name(step['name'])}")
        elif step["shape"] == "string_array":
            parameters.append(f"byte[][] {java_name(step['name'])}")
        elif step["shape"] == "shared_count":
            # Java carries the length in each array, so the parameter the two share disappears.
            continue
        elif step["shape"] in ("struct", "struct_array", "struct_value"):
            for group, _, _, java in STRUCT_GROUPS:
                if step[group]:
                    parameters.append(f"{java}[] {java_name(step['name'])}{group.capitalize()}")
            for path, _ in step.get("views", ()):
                parameters.append(f"byte[] {view_parameter(step['name'], path)}")
    return "int " + entry["java"], parameters


def render_java(class_name: str, entries: list[dict]) -> str:
    lines = [
        "package org.openeggbert.cna.internal.generated;",
        "",
        "/**",
        f" * Generated CNA C ABI declarations for {class_name}.",
        " *",
        " * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.",
        " * Do not edit: every signature here is the header's own declaration, and regenerating",
        " * is how a change upstream reaches Java. This class is not application API.",
        " */",
        f"public final class {class_name} {{",
        "",
        f"    private {class_name}() {{",
        "    }",
    ]
    for entry in entries:
        signature, parameters = java_signature(entry)
        lines.append("")
        lines.append("    /**")
        lines.append(f"     * {entry['symbol']} ({entry['header']}).")
        for step in entry["steps"]:
            if step["shape"] != "struct":
                continue
            for group, _, _, _ in STRUCT_GROUPS:
                if not step[group]:
                    continue
                lines.append("     *")
                lines.append(f"     * <p>{java_name(step['name'])}{group.capitalize()} carries "
                             f"{step['ctype']} in this order:")
                lines.append("     * <ol start=\"0\">")
                for path, leaf in step[group]:
                    lines.append(f"     *   <li>{{@code {path}}} ({leaf})</li>")
                lines.append("     * </ol>")
            for path, _ in step.get("views", ()):
                lines.append("     *")
                lines.append(f"     * <p>{view_parameter(step['name'], path)} carries "
                             f"{step['ctype']}.{path} as UTF-8 bytes, borrowed for the call.")
        lines.append("     */")
        lines.append(f"    public static native {signature}({', '.join(parameters)});")
    lines.append("}")
    return "\n".join(lines) + "\n"


def render_c(class_name: str, entries: list[dict]) -> str:
    lines = [
        "/* SPDX-License-Identifier: MS-PL */",
        "/*",
        f" * Generated JNI adapter for {class_name}.",
        " *",
        " * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and",
        " * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.",
        " */",
        "",
    ]
    for entry in entries:
        symbol = entry["symbol"]
        # The declaring-class parameter is spelled distinctively: a CNA parameter really is
        # named `type` in places, and a plain `type` here would collide with it. The call's own
        # return value is spelled `call_result` for the same reason -- cna_cnb_cnj_result_* takes
        # a handle parameter named `result`, and a plain `result` shadowed it.
        parameters = ["JNIEnv* environment", "jclass declaring_class"]
        body: list[str] = []
        # Refusals that must run before anything is acquired, so a rejected call leaks nothing.
        prologue: list[str] = []
        # Unconditional releases for everything acquired so far, in acquisition order. An early
        # return part way through marshalling replays them in reverse; the guarded write-backs in
        # `cleanup` cannot be used for that, because they name the call's result.
        unwind: list[str] = []
        arguments: list[str] = []
        epilogue: list[str] = []
        cleanup: list[str] = []
        for step in entry["steps"]:
            name = step["name"]
            shape = step["shape"]
            if shape == "null_callback":
                arguments.append("NULL")
                arguments.append("NULL")
            elif shape == "value":
                parameters.append(f"{step['jni']} {name}")
                arguments.append(f"({step['ctype']}){name}")
            elif shape == "out":
                parameters.append(f"{step['jni']}Array {name}")
                body.append(f"    {step['ctype']} {name}_value = 0;")
                arguments.append(f"&{name}_value")
                epilogue.append(
                    f"        {step['jni']} {name}_element = ({step['jni']}){name}_value;\n"
                    f"        (*environment)->Set{jni_array_kind(step['jni'])}ArrayRegion(\n"
                    f"            environment, {name}, 0, 1, &{name}_element);")
            elif shape == "string":
                parameters.append(f"jbyteArray {name}")
                body.extend(borrow_bytes(name, unwind))
                unwind.append(release_bytes(name, abort=True))
                body.append(f"    CNA_StringView {name}_view = "
                            f"{{(const char*){name}_bytes, (uint64_t){name}_size}};")
                arguments.append(f"{name}_view")
                cleanup.append(release_bytes(name, abort=True))
            elif shape == "shared_count":
                # Refused in the prologue, before anything is acquired: two arrays that CNA is
                # told are the same length and are not would make it read past the end of the
                # shorter one, which no Java exception could describe afterwards.
                first = step["members"][0]
                for member in step["members"][1:]:
                    prologue.append(
                        f"    if ((*environment)->GetArrayLength(environment, {first}) !="
                        f"\n            (*environment)->GetArrayLength(environment, {member})) {{")
                    prologue.append("        return (jint)CNA_RESULT_INVALID_ARGUMENT;")
                    prologue.append("    }")
                arguments.append(
                    f"({step['ctype']})(*environment)->GetArrayLength(environment, {first})")
            elif shape == "string_array":
                parameters.append(f"jobjectArray {name}")
                body.append(f"    CnaJniStringViews {name}_views;")
                body.append(f"    const CNA_Result {name}_borrowed = cna_jni_borrow_string_views(")
                body.append(f"        environment, {name}, &{name}_views);")
                body.append(f"    if ({name}_borrowed != CNA_RESULT_SUCCESS) {{")
                body.extend(unwind_lines(unwind, ""))
                body.append(f"        return (jint){name}_borrowed;")
                body.append("    }")
                arguments.append(f"(const CNA_StringView*){name}_views.views")
                if not step.get("shared"):
                    arguments.append(f"(uint64_t){name}_views.count")
                release = f"    cna_jni_free_string_views(&{name}_views);"
                unwind.append(release)
                cleanup.append(release)
            elif shape == "text":
                parameters.append(f"jbyteArray {name}")
                parameters.append(f"jlongArray {step['written']}")
                body.extend(borrow_bytes(name, unwind))
                unwind.append(release_bytes(name, abort=False))
                body.append(f"    uint64_t {name}_written = 0;")
                arguments.append(f"(char*){name}_bytes")
                arguments.append(f"(uint64_t){name}_size")
                arguments.append(f"&{name}_written")
                cleanup.append(release_bytes(name, abort=False))
                epilogue.append(
                    f"        jlong {name}_element = (jlong){name}_written;\n"
                    f"        (*environment)->SetLongArrayRegion(\n"
                    f"            environment, {step['written']}, 0, 1, &{name}_element);")
            elif shape == "sized_array":
                parameters.append(f"{step['jni']}Array {name}")
                element = JAVA_OF_JNI[step["jni"]].capitalize()
                required = (f"(jsize)({step['per']} * {step['count']})" if step["count"]
                            else f"(jsize){step['per']}")
                length = f"(*environment)->GetArrayLength(environment, {name})"
                # Checked in the prologue, before anything is acquired, so a caller's wrong
                # length is a plain refusal rather than a refusal that leaks whatever the
                # earlier parameters had already pinned. CNA states these lengths in prose --
                # "16 floats per joint" -- and a short array would be a read past the end
                # inside CNA that no Java exception could describe.
                if step["nullable"]:
                    prologue.append(f"    if ({name} != NULL && {length} != {required}) {{")
                else:
                    prologue.append(f"    if ({name} == NULL || {length} != {required}) {{")
                prologue.append("        return (jint)CNA_RESULT_INVALID_ARGUMENT;")
                prologue.append("    }")
                body.append(f"    {step['ctype']}* {name}_values = NULL;")
                body.append(f"    {step['jni']}* {name}_elements = NULL;")
                body.append(f"    if ({name} != NULL) {{")
                body.append(f"        jsize {name}_size = {length};")
                body.append(f"        {name}_elements = (*environment)->Get{element}"
                            f"ArrayElements(environment, {name}, NULL);")
                body.append(f"        if ({name}_elements == NULL) {{")
                body.extend(unwind_lines(unwind, "        "))
                body.append("            return (jint)CNA_RESULT_OUT_OF_MEMORY;")
                body.append("        }")
                body.append(f"        {name}_values = ({step['ctype']}*)malloc(")
                body.append(f"            ((size_t){name}_size + 1U) * sizeof(*{name}_values));")
                body.append(f"        if ({name}_values == NULL) {{")
                body.append(f"            (*environment)->Release{element}ArrayElements(")
                body.append(f"                environment, {name}, {name}_elements, JNI_ABORT);")
                body.extend(unwind_lines(unwind, "        "))
                body.append("            return (jint)CNA_RESULT_OUT_OF_MEMORY;")
                body.append("        }")
                body.append(f"        for (jsize index = 0; index < {name}_size; ++index) {{")
                body.append(f"            {name}_values[index] = "
                            f"({step['ctype']}){name}_elements[index];")
                body.append("        }")
                body.append("    }")
                arguments.append(f"{name}_values")
                unwind.append(
                    f"    if ({name}_elements != NULL) {{\n"
                    f"        (*environment)->Release{element}ArrayElements(\n"
                    f"            environment, {name}, {name}_elements, JNI_ABORT);\n"
                    f"    }}\n"
                    f"    free({name}_values);")
                cleanup.append(unwind[-1])
            elif shape == "bytes":
                # A void* whose byte extent routes.json states, as the product of one or two of
                # CNA's own parameters. Those parameters are ordinary JNI arguments of this same
                # function, so they can be read here whatever order the steps were emitted in.
                #
                # The product is folded one factor at a time and each multiplication is checked
                # against the array's length BEFORE it happens, so an extent that would overflow
                # a jlong is refused rather than wrapping into a small number that passes. That
                # is the failure this check exists for: a wrapped product would let CNA read or
                # write far past the end of a pinned Java array.
                parameters.append(f"jbyteArray {name}")
                body.append(f"    jsize {name}_length = "
                            f"(*environment)->GetArrayLength(environment, {name});")
                body.append(f"    jlong {name}_required = 1;")
                body.append(f"    int {name}_refused = 0;")
                for part in step["extent"]:
                    body.append(f"    if ((jlong){part} < 0) {{")
                    body.append(f"        {name}_refused = 1;")
                    body.append(f"    }} else if ((jlong){part} != 0 && {name}_required > "
                                f"(jlong){name}_length / (jlong){part}) {{")
                    body.append(f"        {name}_refused = 1;")
                    body.append("    } else {")
                    body.append(f"        {name}_required *= (jlong){part};")
                    body.append("    }")
                body.append(f"    if ({name}_refused || {name}_required > (jlong){name}_length) {{")
                body.extend(unwind_lines(unwind, "    "))
                body.append("        return (jint)CNA_RESULT_INVALID_ARGUMENT;")
                body.append("    }")
                body.append(f"    jbyte* {name}_bytes = "
                            f"(*environment)->GetByteArrayElements(environment, {name}, NULL);")
                body.append(f"    if ({name}_bytes == NULL) {{")
                body.extend(unwind_lines(unwind, ""))
                body.append("        return (jint)CNA_RESULT_OUT_OF_MEMORY;")
                body.append("    }")
                arguments.append(f"({'const void*' if step['input'] else 'void*'}){name}_bytes")
                release = (
                    f"    (*environment)->ReleaseByteArrayElements(\n"
                    f"        environment, {name}, {name}_bytes, "
                    f"{'JNI_ABORT' if step['input'] else '0'});")
                unwind.append(release)
                cleanup.append(release)
            elif shape == "array":
                parameters.append(f"{step['jni']}Array {name}")
                element = JAVA_OF_JNI[step["jni"]].capitalize()
                body.append(f"    jsize {name}_size = "
                            f"(*environment)->GetArrayLength(environment, {name});")
                body.append(f"    {step['jni']}* {name}_elements = "
                            f"(*environment)->Get{element}ArrayElements(environment, {name}, NULL);")
                body.append(f"    if ({name}_elements == NULL) {{")
                body.extend(unwind_lines(unwind, ""))
                body.append("        return (jint)CNA_RESULT_OUT_OF_MEMORY;")
                body.append("    }")
                body.append(f"    {step['ctype']}* {name}_values = ({step['ctype']}*)malloc(")
                body.append(f"        ((size_t){name}_size + 1U) * sizeof(*{name}_values));")
                body.append(f"    if ({name}_values == NULL) {{")
                body.append(f"        (*environment)->Release{element}ArrayElements(")
                body.append(f"            environment, {name}, {name}_elements, JNI_ABORT);")
                body.extend(unwind_lines(unwind, ""))
                body.append("        return (jint)CNA_RESULT_OUT_OF_MEMORY;")
                body.append("    }")
                body.append(f"    for (jsize index = 0; index < {name}_size; ++index) {{")
                body.append(f"        {name}_values[index] = ({step['ctype']}){name}_elements[index];")
                body.append("    }")
                arguments.append(f"{name}_values")
                grouping = step.get("grouping", 1)
                if grouping != 1:
                    # A declared grouping means the count CNA takes is not the array's length:
                    # a vec3 array is three tightly packed floats per element. A length that is
                    # not a whole number of elements describes an array neither side agrees on,
                    # so it is refused rather than truncated.
                    body.append(f"    if ({name}_size % {grouping} != 0) {{")
                    body.append(f"        free({name}_values);")
                    body.append(f"        (*environment)->Release{element}ArrayElements(")
                    body.append(f"            environment, {name}, {name}_elements, JNI_ABORT);")
                    body.extend(unwind_lines(unwind, ""))
                    body.append("        return (jint)CNA_RESULT_INVALID_ARGUMENT;")
                    body.append("    }")
                count_type = step.get("countType", "uint64_t")
                if not step.get("shared"):
                    arguments.append(
                        f"({count_type})({name}_size / {grouping})" if grouping != 1
                        else f"({count_type}){name}_size")
                writeback = ""
                if not step["input"]:
                    # The copy back has to happen before the buffer is released, so it
                    # belongs in the cleanup sequence rather than in the epilogue.
                    writeback = (
                        f"    if (call_result == CNA_RESULT_SUCCESS) {{\n"
                        f"        for (jsize index = 0; index < {name}_size; ++index) {{\n"
                        f"            {name}_elements[index] = ({step['jni']}){name}_values[index];\n"
                        f"        }}\n"
                        f"    }}\n")
                cleanup.append(
                    writeback
                    + f"    free({name}_values);\n"
                    f"    (*environment)->Release{element}ArrayElements(\n"
                    f"        environment, {name}, {name}_elements, "
                    f"{'JNI_ABORT' if step['input'] else '0'});")
                unwind.append(
                    f"    free({name}_values);\n"
                    f"    (*environment)->Release{element}ArrayElements(\n"
                    f"        environment, {name}, {name}_elements, JNI_ABORT);")
            elif shape == "struct_array":
                groups = [(group, jni, java) for group, jni, java, _ in STRUCT_GROUPS
                          if step[group]]
                primary = groups[0]
                for group, jni, java in groups:
                    parameters.append(f"{jni}Array {name}{group.capitalize()}")
                if "extent" in step:
                    # CNA reads or writes exactly this many, so a Java array of any other size
                    # is a buffer overrun waiting to happen rather than a smaller request.
                    body.append(f"    const jsize {name}_count = {step['extent']};")
                else:
                    body.append(f"    jsize {name}_count = (*environment)->GetArrayLength("
                                f"environment, {name}{primary[0].capitalize()}) / "
                                f"{len(step[primary[0]])};")
                # One struct is split across up to four parallel Java arrays, and the element
                # count comes from only one of them. A caller whose other arrays are shorter
                # would have every element past their end read out of bounds in C -- a heap
                # overread inside the marshalling loop, not a Java exception. So every carrier
                # is required to hold exactly the same number of elements, and a mismatch is
                # refused before anything is allocated.
                conditions = " ||\n        ".join(
                    f"(*environment)->GetArrayLength(environment, "
                    f"{name}{group.capitalize()}) != {name}_count * {len(step[group])}"
                    for group, _, _ in groups)
                body.append(f"    if ({conditions}) {{")
                body.extend(unwind_lines(unwind, "    "))
                body.append("        return (jint)CNA_RESULT_INVALID_ARGUMENT;")
                body.append("    }")
                body.append(f"    {step['ctype']}* {name}_values = ({step['ctype']}*)calloc(")
                body.append(f"        (size_t){name}_count + 1U, sizeof(*{name}_values));")
                body.append(f"    if ({name}_values == NULL) {{")
                body.extend(unwind_lines(unwind, ""))
                body.append("        return (jint)CNA_RESULT_OUT_OF_MEMORY;")
                body.append("    }")
                unwind.append(f"    free({name}_values);")
                for group, jni, java in groups:
                    field = f"{name}{group.capitalize()}"
                    body.append(f"    {{")
                    body.append(f"        jsize {field}_length = "
                                f"(*environment)->GetArrayLength(environment, {field});")
                    body.append(f"        {jni}* {field}_values = ({jni}*)malloc(")
                    body.append(f"            ((size_t){field}_length + 1U) * "
                                f"sizeof(*{field}_values));")
                    body.append(f"        if ({field}_values == NULL) {{")
                    body.extend(unwind_lines(unwind, "        "))
                    body.append("            return (jint)CNA_RESULT_OUT_OF_MEMORY;")
                    body.append("        }")
                    body.append(f"        (*environment)->Get{java}ArrayRegion(environment, "
                                f"{field}, 0, {field}_length, {field}_values);")
                    body.append(f"        for (jsize element = 0; element < {name}_count; "
                                f"++element) {{")
                    for position, (path, leaf) in enumerate(step[group]):
                        body.append(f"            {name}_values[element].{path} = ({leaf})"
                                    f"{field}_values[element * {len(step[group])} + {position}];")
                    body.append("        }")
                    body.append(f"        free({field}_values);")
                    body.append("    }")
                if step["versions"]:
                    body.append(f"    for (jsize element = 0; element < {name}_count; ++element) {{")
                    body.extend("    " + line for line in stamp_versions(
                        f"{name}_values[element]", step["versions"]))
                    body.append("    }")
                arguments.append(f"{name}_values")
                if step.get("count") is not None or "extent" not in step:
                    # A counted array passes its count; a fixed-extent one does not, because
                    # CNA already knows how many elements it reads or writes.
                    arguments.append(f"(uint64_t){name}_count")
                if not step["input"]:
                    # An output array of structs has to be copied back before the C
                    # buffer is freed, so the write-back belongs in the cleanup
                    # sequence rather than in the success epilogue. Every element of
                    # the requested capacity is written: calloc zeroed the tail, and
                    # the route's own out-count says how much of it CNA filled.
                    writeback = ["    if (call_result == CNA_RESULT_SUCCESS) {"]
                    for group, jni, java in groups:
                        field = f"{name}{group.capitalize()}"
                        writeback.append("        {")
                        writeback.append(f"            {jni}* {field}_out = ({jni}*)malloc(")
                        writeback.append(f"                ((size_t){name}_count * "
                                         f"{len(step[group])} + 1U) * sizeof(*{field}_out));")
                        writeback.append(f"            if ({field}_out != NULL) {{")
                        writeback.append(f"                for (jsize element = 0; element < "
                                         f"{name}_count; ++element) {{")
                        for position, (leaf_path, leaf) in enumerate(step[group]):
                            writeback.append(
                                f"                    {field}_out[element * "
                                f"{len(step[group])} + {position}] = ({jni})"
                                f"{name}_values[element].{leaf_path};")
                        writeback.append("                }")
                        writeback.append(f"                (*environment)->Set{java}ArrayRegion(")
                        writeback.append(f"                    environment, {field}, 0,")
                        writeback.append(f"                    (jsize)({name}_count * "
                                         f"{len(step[group])}), {field}_out);")
                        writeback.append(f"                free({field}_out);")
                        writeback.append("            }")
                        writeback.append("        }")
                    writeback.append("    }")
                    cleanup.append("\n".join(writeback))
                cleanup.append(f"    free({name}_values);")
            elif shape == "struct_value":
                body.append(f"    {step['ctype']} {name}_value;")
                body.append(f"    memset(&{name}_value, 0, sizeof {name}_value);")
                body.extend(stamp_versions(f"{name}_value", step["versions"]))
                for group, jni, java, _ in STRUCT_GROUPS:
                    if not step[group]:
                        continue
                    field = f"{name}{group.capitalize()}"
                    parameters.append(f"{jni}Array {field}")
                    body.append("    {")
                    body.append(f"        {jni} {field}_values[{len(step[group])}];")
                    body.append(f"        (*environment)->Get{java}ArrayRegion(environment, "
                                f"{field}, 0, {len(step[group])}, {field}_values);")
                    for position, (path, leaf) in enumerate(step[group]):
                        body.append(f"        {name}_value.{path} = "
                                    f"({leaf}){field}_values[{position}];")
                    body.append("    }")
                for path, _ in step.get("views", ()):
                    view = view_parameter(step["name"], path)
                    parameters.append(f"jbyteArray {view}")
                    body.extend(borrow_bytes(view, unwind))
                    body.append(f"    {name}_value.{path}.data = (const char*){view}_bytes;")
                    body.append(f"    {name}_value.{path}.byte_length = (uint64_t){view}_size;")
                    unwind.append(release_bytes(view, abort=True))
                    cleanup.append(unwind[-1])
                arguments.append(f"{name}_value")
            elif shape == "struct":
                optional = step.get("optional", False)
                carriers = [f"{name}{group.capitalize()}"
                            for group, _, _, _ in STRUCT_GROUPS if step[group]]
                if optional:
                    # CNA documents this structure as optional -- "or null for the default" --
                    # and a null pointer is a different instruction from an all-zero structure,
                    # which would mean whatever zero happens to select. Java says "none" by
                    # passing no arrays, and that becomes NULL rather than a zeroed value.
                    body.append(f"    {step['ctype']} {name}_value;")
                    body.append(f"    const {step['ctype']}* {name}_pointer = NULL;")
                    body.append("    if (" + " && ".join(
                        f"{field} != NULL" for field in carriers) + ") {")
                    optional_from = len(body)
                    arguments.append(f"{name}_pointer")
                else:
                    body.append(f"    {step['ctype']} {name}_value;")
                    if carriers:
                        # A borrowed array parameter that arrived null would make every
                        # GetXArrayRegion below raise and then be read as uninitialised stack.
                        # Refusing here turns a JVM-level fault into the result CNA itself
                        # would have given for a missing argument.
                        prologue.append("    if (" + " || ".join(
                            f"{field} == NULL" for field in carriers) + ") {")
                        prologue.append("        return (jint)CNA_RESULT_INVALID_ARGUMENT;")
                        prologue.append("    }")
                    arguments.append(f"&{name}_value")
                body.append(f"    memset(&{name}_value, 0, sizeof {name}_value);")
                body.extend(stamp_versions(f"{name}_value", step["versions"]))
                for group, jni, java, _ in STRUCT_GROUPS:
                    if not step[group]:
                        continue
                    field = f"{name}{group.capitalize()}"
                    parameters.append(f"{jni}Array {field}")
                    if step["input"] or step.get("inOut"):
                        body.append(f"    {{")
                        body.append(f"        {jni} {field}_values["
                                    f"{len(step[group])}];")
                        body.append(f"        (*environment)->Get{java}ArrayRegion(environment, "
                                    f"{field}, 0, {len(step[group])}, {field}_values);")
                        for position, (path, leaf) in enumerate(step[group]):
                            body.append(f"        {name}_value.{path} = "
                                        f"({leaf}){field}_values[{position}];")
                        body.append("    }")
                    if not step["input"]:
                        values = ",\n            ".join(
                            f"({jni}){name}_value.{path}" for path, _ in step[group])
                        epilogue.append(
                            f"        {jni} {field}_values[] = {{\n            {values}\n        }};\n"
                            f"        (*environment)->Set{java}ArrayRegion(environment, {field}, 0,\n"
                            f"            (jsize)(sizeof {field}_values / sizeof {field}_values[0]),\n"
                            f"            {field}_values);")
                for path, _ in step.get("views", ()):
                    # Only an input structure reaches here: plan() refuses a view in an output
                    # one, because the pointer would belong to CNA on terms the declaration
                    # does not state.
                    view = view_parameter(step["name"], path)
                    parameters.append(f"jbyteArray {view}")
                    body.extend(borrow_bytes(view, unwind))
                    body.append(f"    {name}_value.{path}.data = (const char*){view}_bytes;")
                    body.append(f"    {name}_value.{path}.byte_length = (uint64_t){view}_size;")
                    unwind.append(release_bytes(view, abort=True))
                    cleanup.append(unwind[-1])
                if step.get("optional", False):
                    # Everything marshalled since the guard belongs inside it, indented to say so.
                    body[optional_from:] = [f"    {line}" for line in body[optional_from:]]
                    body.append(f"        {name}_pointer = &{name}_value;")
                    body.append("    }")
        body = (["    (void)environment;", "    (void)declaring_class;"]
                + prologue + body)
        body.append(f"    CNA_Result call_result = "
                    f"cna.{slot_of(symbol)}({', '.join(arguments)});")
        body.extend(cleanup)
        if epilogue:
            # Scalar and structure outputs are also copied back on BUFFER_TOO_SMALL. CNA
            # documents the required byte count of a copy-out route as "always written on a
            # valid output", and that is the whole two-call protocol: ask with no buffer, be
            # told the size, ask again. Writing outputs only on success would leave the caller
            # with a zero and no way to size the buffer. The buffer itself is untouched, because
            # CNA performs no partial write, so only these outputs cross.
            body.append("    if (call_result == CNA_RESULT_SUCCESS "
                        "|| call_result == CNA_RESULT_BUFFER_TOO_SMALL) {")
            body.extend(epilogue)
            body.append("    }")
        body.append("    return (jint)call_result;")

        lines.append("JNIEXPORT jint JNICALL")
        lines.append(f"Java_org_openeggbert_cna_internal_generated_{class_name}_{entry['java']}(")
        lines.append("    " + ", ".join(parameters) + ")")
        lines.append("{")
        lines.extend(body)
        lines.append("}")
        lines.append("")
    return "\n".join(lines)


def unwind_lines(unwind: list[str], indent: str) -> list[str]:
    """Release everything acquired so far, most recent first.

    An early return in the middle of marshalling would otherwise strand whatever the earlier
    parameters had already pinned or allocated. The paths that reach here are out-of-memory
    ones, which is exactly when leaking is least affordable.
    """
    return [indent + line for statement in reversed(unwind)
            for line in statement.splitlines()]


def borrow_bytes(name: str, unwind: list[str] | None = None) -> list[str]:
    lines = [
        f"    jsize {name}_size = (*environment)->GetArrayLength(environment, {name});",
        f"    jbyte* {name}_bytes = "
        f"(*environment)->GetByteArrayElements(environment, {name}, NULL);",
        f"    if ({name}_bytes == NULL) {{",
    ]
    lines.extend(unwind_lines(unwind or [], "    "))
    lines.append("        return (jint)CNA_RESULT_OUT_OF_MEMORY;")
    lines.append("    }")
    return lines


def release_bytes(name: str, *, abort: bool) -> str:
    mode = "JNI_ABORT" if abort else "0"
    return (f"    (*environment)->ReleaseByteArrayElements("
            f"environment, {name}, {name}_bytes, {mode});")


def jni_array_kind(jni: str) -> str:
    return {"jbyte": "Byte", "jshort": "Short", "jint": "Int", "jlong": "Long",
            "jfloat": "Float", "jdouble": "Double", "jboolean": "Boolean"}[jni]


def struct_version(name: str, live: dict) -> str:
    """Return the documented version constant for a versioned structure."""
    snake = re.sub(r"(?<=[a-z0-9])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])", "_", name[len("CNA_"):])
    candidate = "CNA_" + snake.upper() + "_STRUCT_VERSION"
    return candidate if candidate in live["constants"] else "UINT32_C(1)"


def slot_of(symbol: str) -> str:
    return symbol[len("cna_"):]


def render_table(entries: list[dict]) -> tuple[str, str]:
    slots = sorted({entry["symbol"] for entry in entries})
    declarations = "\n".join(
        f"    CNA_JNI_ROUTE({symbol}) {slot_of(symbol)};" for symbol in slots)
    loads = "\n".join(f"    LOAD({slot_of(symbol)}, \"{symbol}\");" for symbol in slots)
    return declarations + "\n", loads + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cna-root", required=True)
    parser.add_argument("--check", action="store_true",
                        help="fail instead of writing when the generated output would change")
    arguments = parser.parse_args()

    live = inventory(Path(arguments.cna_root).resolve() / "modules/c-api/include", strict=True)
    specification = json.loads(ROUTES.read_text(encoding="utf-8"))

    GENERATED_C.mkdir(parents=True, exist_ok=True)
    GENERATED_JAVA.mkdir(parents=True, exist_ok=True)
    all_entries: list[dict] = []
    changed: list[str] = []

    def emit(path: Path, content: str) -> None:
        previous = path.read_text(encoding="utf-8") if path.is_file() else None
        if previous == content:
            return
        changed.append(str(path.relative_to(ROOT)))
        if not arguments.check:
            path.write_text(content, encoding="utf-8")

    for class_name, routes in sorted(specification["classes"].items()):
        entries = []
        for route in routes:
            try:
                entries.append(plan(route, live))
            except Unsupported as failure:
                print(f"UNSUPPORTED_ROUTE={failure}", file=sys.stderr)
                return 2
        entries.sort(key=lambda value: value["java"])
        all_entries.extend(entries)
        emit(GENERATED_JAVA / f"{class_name}.java", render_java(class_name, entries))
        emit(GENERATED_C / f"{class_name}.inc", render_c(class_name, entries))

    declarations, loads = render_table(all_entries)
    emit(GENERATED_C / "routes_table.inc", declarations)
    emit(GENERATED_C / "routes_load.inc", loads)
    # The adapter includes this one file rather than each class by name. A hand-maintained list
    # is a list somebody forgets: a new generated class whose .inc was never included compiles
    # and links, and every one of its routes fails at first call with UnsatisfiedLinkError.
    # That happened once, which is why this file exists.
    emit(GENERATED_C / "routes_includes.inc",
         "/* SPDX-License-Identifier: MS-PL */\n"
         "/*\n"
         " * Every generated JNI implementation, in one place.\n"
         " *\n"
         " * Produced by tools/native-abi/generate_jni.py. Do not edit: adding a route class to\n"
         " * routes.json is what adds it here, so the adapter cannot be missing one.\n"
         " */\n\n"
         # Included from this file's own directory, not the adapter's, because that is where
         # the C preprocessor looks first for a quoted include.
         + "".join(f'#include "{class_name}.inc"\n'
                   for class_name in sorted(specification["classes"])))

    print(f"GENERATED_CLASSES={len(specification['classes'])}")
    print(f"GENERATED_ROUTES={len(all_entries)}")
    print(f"CHANGED_FILES={len(changed)}")
    for path in changed:
        print(f"CHANGED={path}")
    if arguments.check and changed:
        print("generated native boundary is stale; rerun tools/native-abi/generate_jni.py",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
