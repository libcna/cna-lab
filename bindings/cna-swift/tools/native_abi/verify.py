#!/usr/bin/env python3
"""Compiler-backed CNA C ABI verifier for the Swift native route manifest.

Everything the report counts is derived from the sources it just compiled. The
chain has five independent links:

1. the manifest's C declaration for a symbol is compared **textually** to the
   canonical declaration in the CNA headers, so recording a merely compatible
   spelling (`CNA_Handle` where the header says
   `CNA_GraphicsDeviceManagerHandle`) is a mismatch, not a pass;
2. the same declaration is compiled into a `__builtin_types_compatible_p`
   assertion against `&symbol`, so the header and the manifest must also agree
   after typedef resolution;
3. every Swift stored property is paired with exactly one symbol and one route
   type of its own, and each route type is derived from the symbol rather than
   trusted, so two routes cannot share a structurally identical alias;
4. every mirrored `CNASwift_*` structure is compared field for field with its
   canonical counterpart — names, order, offsets and widths — so an omitted,
   reordered or resized field fails;
5. the two callback types are compared with `__builtin_types_compatible_p`.
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import re
import subprocess
import tempfile
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "Sources/CNA/Native/NativeManifest.swift"
FUNCTIONS = ROOT / "Sources/CNA/Native/NativeFunctions.swift"
SHIM_INCLUDE = ROOT / "Sources/CNAShim/include"
SHIM_HEADER = SHIM_INCLUDE / "CNAShim.h"
PROBE = ROOT / "tools/native_abi/probe.c"
KEYS = ROOT / "Sources/CNA/Xna/Input/Keyboard.swift"

# The shim mirrors exactly these canonical structures, one for one.
MIRRORED_STRUCTS = [
    "BoundingSphere", "VertexBufferInfo", "IndexBufferInfo",
    "StringView", "Color", "Vector2", "Rectangle", "BackBufferReadback",
    "GameTime", "CallbackError",
    "GameCallbacks", "GameFrameHooks", "GameCreateInfo", "Viewport",
    "Texture2DInfo", "Texture2DCreateInfo", "Texture2DTransfer",
    "Texture2DDecodeInfo",
    "SpriteScaledCommand", "SpriteTextCommand",
    "SpriteFontGlyph", "SpriteFontCreateInfo", "SpriteFontInfo",
    "KeyboardState", "GamePadAnalogState",
    "GamePadState", "GamePadCapabilities", "TextureInfo",
    "RenderTarget2DCreateInfo", "RenderTargetInfo",
    "RenderTargetCubeCreateInfo", "RenderTargetBinding", "TextureSlotInfo",
    "EffectParameterInfo", "EffectAnnotationInfo",
    "UserPrimitives", "UserIndices",
    "Vector3", "Vector4", "Quaternion", "Matrix",
    "BlendState", "DepthStencilState", "RasterizerState", "SamplerState",
    "SpriteCommand",
    "PresentationParameters",
    # Added after its absence let a mirrored structure ship one field short:
    # CNA_ContentManagerCreateInfo ends in a reserved uint64 that is part of
    # sizeof, so leaving it out made struct_size too small and every create
    # call was refused as an invalid configuration. Nothing in this gate
    # looked at the structure, so only a run caught it.
    "ContentManagerCreateInfo",
    "SoundEffectCreateInfo", "SoundEffectInstanceInfo",
    "VisualizationData",
    "TouchCapabilities", "TouchLocation", "TouchState", "GestureSample",
    "AudioEmitter", "AudioListener",
    "VertexElement", "VertexBufferCreateInfo", "VertexBufferBinding",
    "IndexBufferCreateInfo", "IndexBufferTransfer",
    "TextureCubeCreateInfo", "TextureCubeInfo", "TextureCubeTransfer",
    "Texture3DCreateInfo", "Texture3DInfo", "Texture3DTransfer",
]

# The shim mirrors exactly these canonical callback types.
MIRRORED_CALLBACKS = [
    "GameLifecycleCallback", "GameBeginDrawCallback",
    "RenderTargetContentLostCallback", "GameEventCallback",
    "VertexBufferContentLostCallback", "IndexBufferContentLostCallback",
    # The audio event callback carries no data at all -- `void (*)(void*)` --
    # and DynamicSoundEffectInstance.BufferNeeded is what consumes it.
    "AudioEventCallback", "MediaPlayerEventCallback",
    # Same `void (*)(void*)` shape as the audio one and mirrored anyway: this
    # table pairs by name, so a shared typedef would leave the storage routes'
    # callback parameter unpaired and unchecked.
    "StorageCompletionCallback",
]

# The scalar typedefs a mirrored declaration may name on either side. The
# callback wall proves each pair is the same underlying type, which is what
# makes translating a shim signature into canonical names sound.
MIRRORED_SCALARS = ["Result", "Bool", "Handle", "AudioStopOptions",
                    "MicrophoneState", "FileMode", "FileAccess", "FileShare",
                    "SeekOrigin", "VertexBufferHandle", "IndexBufferHandle"]


# --------------------------------------------------------------------------
# Sources of record
# --------------------------------------------------------------------------

def manifest_entries() -> list[dict[str, Any]]:
    text = MANIFEST.read_text(encoding="utf-8")
    pattern = re.compile(
        r'\.init\(symbol: "(?P<symbol>[^"]+)", swiftField: "(?P<field>[^"]+)", '
        r'routeType: "(?P<route>[^"]+)", cReturn: "(?P<ret>[^"]+)", '
        r'cParameters: \[(?P<params>.*?)\], ownership:', re.S
    )
    entries: list[dict[str, Any]] = []
    for match in pattern.finditer(text):
        params = re.findall(r'"([^"]+)"', match.group("params"))
        entries.append({
            "symbol": match.group("symbol"),
            "swiftField": match.group("field"),
            "routeType": match.group("route"),
            "return": match.group("ret"),
            "parameters": params,
        })
    if not entries:
        raise RuntimeError("NativeManifest.swift contained no parseable entries")
    return entries


def derived_route_type(symbol: str) -> str:
    """A route type is a function of its symbol, so it cannot be shared."""
    words = symbol[len("cna_"):].split("_") if symbol.startswith("cna_") else symbol.split("_")
    return "".join(word[:1].upper() + word[1:].lower() for word in words) + "Route"


def abi_policy() -> dict[str, int]:
    text = FUNCTIONS.read_text(encoding="utf-8")
    def value(name: str) -> int:
        match = re.search(rf"static let {name}: UInt32 = (\d+)", text)
        if not match:
            raise RuntimeError(f"NativeFunctions.swift does not declare {name}")
        return int(match.group(1))
    qualified = re.search(
        r"static let qualifiedVersion: UInt32 = encode\(major: (\d+), minor: (\d+), patch: (\d+)\)",
        text,
    )
    if not qualified:
        raise RuntimeError("NativeFunctions.swift does not declare qualifiedVersion")
    major, minor, patch = (int(group) for group in qualified.groups())
    return {
        "admittedMajor": value("admittedMajor"),
        "minimumMinor": value("minimumMinor"),
        "qualifiedMajor": major,
        "qualifiedMinor": minor,
        "qualifiedPatch": patch,
    }


# --------------------------------------------------------------------------
# Swift side
# --------------------------------------------------------------------------

def strip_parameter_name(value: str) -> str:
    return re.sub(r"\s+[A-Za-z_][A-Za-z0-9_]*$", "", value.strip())


def canonical_type(value: str) -> str:
    text = strip_parameter_name(value)
    text = re.sub(r"\s+", " ", text)
    aliases = {
        "CNA_Result": "uint32_t", "CNA_Bool": "uint8_t", "CNA_Handle": "uint64_t",
        "CNA_GraphicsDeviceManagerHandle": "uint64_t", "CNA_PlayerIndex": "uint32_t",
        "CNA_GamePadDeadZone": "uint32_t",
        "CNA_RenderTargetEventRegistrationHandle": "uint64_t",
        "CNA_GraphicsResourceEventRegistrationHandle": "uint64_t",
        "CNA_GameEventRegistrationHandle": "uint64_t", "CNA_GameEvent": "uint32_t",
        "CNA_GraphicsDeviceManagerEvent": "uint32_t",
        # `typedef uint32_t CNA_ShaderStage;` in graphics_state.h:214. The
        # canonical-declaration check compares the manifest's TEXT with the
        # header's, so the manifest keeps CNA's own spelling; this table is
        # only for the separate type-compatibility comparison, which needs the
        # underlying type. Adding an alias here never weakens the textual
        # check.
        "CNA_ShaderStage": "uint32_t",
        # `typedef uint32_t CNA_GraphicsDeviceStatus;` in graphics_device.h:34.
        "CNA_GraphicsDeviceStatus": "uint32_t",
        # graphics_device.h:24 and the display/surface typedefs the
        # presentation parameters name.
        "CNA_ClearOptions": "uint32_t", "CNA_SurfaceFormat": "uint32_t",
        "CNA_DepthFormat": "uint32_t", "CNA_PresentInterval": "uint32_t",
        "CNA_DisplayOrientation": "uint32_t", "CNA_RenderTargetUsage": "uint32_t",
        "CNA_GraphicsProfile": "uint32_t", "CNA_SpriteSortMode": "uint32_t",
        "CNA_TextureDataType": "uint32_t",
        # `typedef uint32_t CNA_TextureImageFormat;` in texture.h:54,
        # which names PNG and JPEG.
        "CNA_TextureImageFormat": "uint32_t",
        # `typedef uint32_t CNA_CubeMapFace;` in render_target.h:38,
        # which names the six faces.
        "CNA_CubeMapFace": "uint32_t",
        # graphics3d.h's buffer and vertex-element typedefs, and the two
        # buffer-handle aliases in vertex_resources.h and index_resources.h.
        "CNA_BufferUsage": "uint32_t", "CNA_IndexElementSize": "uint32_t",
        "CNA_SetDataOptions": "uint32_t",
        "CNA_VertexElementFormat": "uint32_t",
        "CNA_VertexElementUsage": "uint32_t",
        "CNA_VertexDeclarationHandle": "uint64_t",
        "CNA_VertexBufferHandle": "uint64_t",
        "CNA_IndexBufferHandle": "uint64_t",
        "CNA_VertexBufferEventRegistrationHandle": "uint64_t",
        "CNA_IndexBufferEventRegistrationHandle": "uint64_t",
        # graphics_device.h:1280 gives the occlusion query its own alias for
        # the same reason effects.h gives nine: the header naming which handle
        # a route wants, not a wider ABI.
        "CNA_OcclusionQueryHandle": "uint64_t",
        # audio.h:13 and :21 -- the channel count and the playback state are
        # both uint32_t enumerations the header names.
        "CNA_AudioChannels": "uint32_t",
        "CNA_SoundState": "uint32_t",
        # audio.h:608 -- one owned handle per audio event subscription.
        "CNA_AudioEventRegistrationHandle": "uint64_t",
        # media.h gives each media entity its own handle alias, the same way
        # effects.h does; every one of them is `uint64_t`.
        "CNA_SongHandle": "uint64_t",
        "CNA_SongCollectionHandle": "uint64_t",
        "CNA_ArtistHandle": "uint64_t",
        "CNA_AlbumHandle": "uint64_t",
        "CNA_GenreHandle": "uint64_t",
        "CNA_ArtistCollectionHandle": "uint64_t",
        "CNA_AlbumCollectionHandle": "uint64_t",
        "CNA_GenreCollectionHandle": "uint64_t",
        "CNA_MediaLibraryHandle": "uint64_t",
        "CNA_StorageDeviceHandle": "uint64_t",
        "CNA_StorageContainerHandle": "uint64_t",
        "CNA_StorageStreamHandle": "uint64_t",
        "CNA_AnimationPlayerHandle": "uint64_t",
        "CNA_ModelAnimationsEXTHandle": "uint64_t",
        "CNA_ModelBoneCollectionHandle": "uint64_t",
        "CNA_ModelBoneHandle": "uint64_t",
        "CNA_ModelEffectCollectionHandle": "uint64_t",
        "CNA_ModelHandle": "uint64_t",
        "CNA_ModelMeshCollectionHandle": "uint64_t",
        "CNA_ModelMeshHandle": "uint64_t",
        "CNA_ModelMeshPartCollectionHandle": "uint64_t",
        "CNA_ModelMeshPartHandle": "uint64_t",
        "CNA_MorphTargetDataEXTHandle": "uint64_t",
        "CNA_ObjectDictionaryHandle": "uint64_t",
        "CNA_SkinnedModelEXTHandle": "uint64_t",
        "CNA_SkinningDataHandle": "uint64_t",
        "CNA_PictureHandle": "uint64_t",
        "CNA_PictureAlbumHandle": "uint64_t",
        "CNA_PictureCollectionHandle": "uint64_t",
        "CNA_PictureAlbumCollectionHandle": "uint64_t",
        "CNA_PlaylistHandle": "uint64_t",
        "CNA_PlaylistCollectionHandle": "uint64_t",
        "CNA_MediaSourceType": "uint32_t",
        "CNA_MediaState": "uint32_t",
        "CNA_MediaQueueHandle": "uint64_t",
        "CNA_TouchLocationState": "uint32_t",
        "CNA_GestureType": "uint32_t",
        "CNA_VideoHandle": "uint64_t",
        "CNA_VideoPlayerHandle": "uint64_t",
        "CNA_VideoSoundtrackType": "uint32_t",
        "CNA_MediaPlayerEventRegistrationHandle": "uint64_t",
        # effects.h gives every effect object its own handle alias, and two
        # enumerations of its own. Nine aliases for one `CNA_Handle` is a lot,
        # and it is the header being precise about which handle a route wants
        # rather than the ABI being wider -- every one of them is `uint64_t`.
        "CNA_EffectHandle": "uint64_t",
        "CNA_EffectParameterHandle": "uint64_t",
        "CNA_EffectParameterCollectionHandle": "uint64_t",
        "CNA_EffectTechniqueHandle": "uint64_t",
        "CNA_EffectTechniqueCollectionHandle": "uint64_t",
        "CNA_EffectPassHandle": "uint64_t",
        "CNA_EffectPassCollectionHandle": "uint64_t",
        "CNA_EffectAnnotationHandle": "uint64_t",
        "CNA_EffectAnnotationCollectionHandle": "uint64_t",
        "CNA_EffectParameterClass": "uint32_t",
        "CNA_EffectParameterType": "uint32_t",
        "CNA_EffectValueType": "uint32_t",
        "CNA_EffectTextureType": "uint32_t",
        # graphics_device.h's user-primitive vertex-source identity, and
        # graphics3d.h's primitive topology.
        "CNA_UserVertexSource": "uint32_t",
        "CNA_PrimitiveType": "uint32_t",
        # effects.h's stable member-view handle for one of an effect's three
        # directional lights.
        "CNA_DirectionalLightHandle": "uint64_t",
        # graphics_state.h's comparison identity, which AlphaTestEffect's
        # AlphaFunction is spelled with.
        "CNA_CompareFunction": "uint32_t",
        # sprite_font.h's "one UTF-16 code unit matching the native XNA `char`
        # representation" -- a CLR `char` is UTF-16, so the projection carries
        # code units and never a Swift `Character`, which is a grapheme.
        "CNA_Char16": "uint16_t",
    }
    for old, new in aliases.items():
        text = re.sub(rf"\b{old}\b", new, text)
    return text.replace(" *", "*")


def swift_type(value: str) -> str:
    text = value.strip().rstrip("?")
    mutable = re.fullmatch(r"UnsafeMutablePointer<(.+)>", text)
    immutable = re.fullmatch(r"UnsafePointer<(.+)>", text)
    if mutable:
        return f"{swift_type(mutable.group(1))}*"
    if immutable:
        return f"const {swift_type(immutable.group(1))}*"
    mapping = {
        "UInt8": "uint8_t", "UInt16": "uint16_t", "UInt32": "uint32_t",
        "UInt64": "uint64_t", "Int16": "int16_t", "Int32": "int32_t",
        "Int64": "int64_t", "Float": "float", "Double": "double", "CChar": "char",
        "UnsafeMutableRawPointer": "void*", "UnsafeRawPointer": "const void*",
    }
    if text.startswith("CNASwift_"):
        return "CNA_" + text[len("CNASwift_"):]
    return mapping.get(text, text)


def split_swift_types(text: str) -> list[str]:
    values: list[str] = []
    depth = 0
    start = 0
    for index, char in enumerate(text):
        if char == "<": depth += 1
        elif char == ">": depth -= 1
        elif char == "," and depth == 0:
            values.append(text[start:index].strip())
            start = index + 1
    tail = text[start:].strip()
    if tail: values.append(tail)
    return values


def swift_routes() -> tuple[dict[str, tuple[str, list[str]]], dict[str, str], dict[str, tuple[str, str]]]:
    """Route typealiases, declared property types, and field -> (symbol, type)."""
    text = FUNCTIONS.read_text(encoding="utf-8")
    aliases: dict[str, tuple[str, list[str]]] = {}
    for match in re.finditer(r"typealias\s+(\w+)\s*=\s*@convention\(c\)\s*\((.*?)\)\s*->\s*([^\n]+)", text):
        aliases[match.group(1)] = (
            swift_type(match.group(3)),
            [swift_type(item) for item in split_swift_types(match.group(2))],
        )
    declared = {
        field: kind
        for field, kind in re.findall(r"^\s{4}let (\w+): (\w+Route)$", text, re.M)
    }
    resolutions = {
        field: (symbol, kind)
        for field, symbol, kind in re.findall(
            r'(\w+) = try library\.resolve\("([^"]+)", as:\s*(\w+)\.self\)', text
        )
    }
    return aliases, declared, resolutions


# --------------------------------------------------------------------------
# Canonical C headers
# --------------------------------------------------------------------------

def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", " ", text)


def header_text(include: Path) -> str:
    parts = []
    for path in sorted((include / "CNA" / "C").glob("*.h")):
        parts.append(strip_comments(path.read_text(encoding="utf-8")))
    return "\n".join(parts)


def canonical_declarations(text: str) -> dict[str, tuple[str, list[str]]]:
    declarations: dict[str, tuple[str, list[str]]] = {}
    for match in re.finditer(
        r"CNA_C_API\s+([A-Za-z_][A-Za-z0-9_]*(?:\s*\*)?)\s+(cna_[A-Za-z0-9_]+)\s*\(([^;]*?)\)\s*;",
        text, re.S,
    ):
        ret = re.sub(r"\s+", " ", match.group(1)).strip()
        raw = re.sub(r"\s+", " ", match.group(3)).strip()
        params = [] if raw in ("", "void") else [item.strip() for item in raw.split(",")]
        declarations[match.group(2)] = (ret, [re.sub(r"\s*\*\s*", "* ", item) for item in params])
    return declarations


def callback_signatures(text: str, prefix: str) -> dict[str, tuple[str, list[str]]]:
    """Ordered (return, parameters) for every `<prefix><name>` callback typedef."""
    result: dict[str, tuple[str, list[str]]] = {}
    for match in re.finditer(
        rf"typedef\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*\*\s*{prefix}([A-Za-z0-9_]+)\s*\)\s*\(([^;]*?)\)\s*;",
        text, re.S,
    ):
        raw = re.sub(r"\s+", " ", match.group(3)).strip()
        params = [] if raw in ("", "void") else [item.strip() for item in raw.split(",")]
        result[match.group(2)] = (
            match.group(1),
            [re.sub(r"\s*\*\s*", "* ", item) for item in params],
        )
    return result


def translate(text: str, prefix: str) -> str:
    """Rewrite every `<prefix>X` name as its canonical `CNA_X` counterpart."""
    return re.sub(rf"\b{prefix}([A-Za-z0-9_]+)\b", r"CNA_\1", text)


def struct_fields(text: str, prefix: str, names: list[str]) -> dict[str, list[tuple[str, str]]]:
    """Ordered (type, field) pairs for `typedef struct <prefix><name> { ... }`."""
    result: dict[str, list[tuple[str, str]]] = {}
    for name in names:
        full = prefix + name
        match = re.search(rf"typedef\s+struct\s+{full}\s*\{{(.*?)\}}\s*{full}\s*;", text, re.S)
        if not match:
            continue
        fields: list[tuple[str, str]] = []
        for line in match.group(1).split(";"):
            line = re.sub(r"\s+", " ", line).strip()
            if not line:
                continue
            # An array bound may be a macro rather than a literal --
            # `float samples[CNA_VISUALIZATION_DATA_SIZE]` is the first one in
            # this ABI. The NAME is what the layout comparison uses, and the
            # generated static assertions compare `offsetof` and `sizeof` on
            # both sides, so an unresolved bound is not a hole: whatever the
            # macro expands to, the two structures are still measured against
            # each other by the C compiler.
            member = re.fullmatch(
                r"(.+?)\s*\*?\s*([A-Za-z_][A-Za-z0-9_]*)(\[[A-Za-z0-9_]+\])?", line)
            if not member:
                raise RuntimeError(f"unparsed member in {full}: {line!r}")
            fields.append((line, member.group(2)))
        result[name] = fields
    return result


# --------------------------------------------------------------------------
# Generated compilation walls
# --------------------------------------------------------------------------

def prototype_assertions(entries: list[dict[str, Any]]) -> str:
    lines = ['#include "CNA/C/cna.h"']
    for index, entry in enumerate(entries):
        params = ", ".join(entry["parameters"]) or "void"
        lines.append(f'typedef {entry["return"]} (*Expected_{index})({params});')
        lines.append(
            f'_Static_assert(__builtin_types_compatible_p(__typeof__(&{entry["symbol"]}), Expected_{index}), '
            f'"prototype mismatch: {entry["symbol"]}");'
        )
    lines.append("int main(void) { return 0; }")
    return "\n".join(lines) + "\n"


def layout_assertions(canonical: dict[str, list[tuple[str, str]]],
                      shim: dict[str, list[tuple[str, str]]]) -> tuple[str, int, int]:
    lines = ['#include <stddef.h>', '#include "CNA/C/cna.h"', '#include "CNAShim.h"']
    layouts = 0
    fields = 0
    for name in MIRRORED_STRUCTS:
        c_name, s_name = "CNA_" + name, "CNASwift_" + name
        lines.append(f'_Static_assert(sizeof({c_name}) == sizeof({s_name}), "size: {c_name}");')
        lines.append(f'_Static_assert(_Alignof({c_name}) == _Alignof({s_name}), "align: {c_name}");')
        layouts += 1
        for _, field in canonical[name]:
            lines.append(
                f'_Static_assert(offsetof({c_name}, {field}) == offsetof({s_name}, {field}), '
                f'"offset: {c_name}.{field}");'
            )
            lines.append(
                f'_Static_assert(sizeof((({c_name}*)0)->{field}) == sizeof((({s_name}*)0)->{field}), '
                f'"width: {c_name}.{field}");'
            )
            fields += 1
    lines.append("int main(void) { return 0; }")
    return "\n".join(lines) + "\n", layouts, fields


def callback_assertions(shim: dict[str, tuple[str, list[str]]]) -> str:
    """Three linked facts per callback.

    A. the shim typedef is exactly the shape parsed out of `CNAShim.h`;
    B. that shape, with every `CNASwift_` name rewritten to its canonical
       counterpart, *is* the canonical typedef — so a dropped, added, reordered
       or retyped parameter fails here;
    C. every scalar typedef named on both sides is the same underlying type,
       which is what makes the rewrite in B sound. The structure types named on
       both sides are proven layout-identical field for field by the layout
       wall.
    """
    lines = ['#include "CNA/C/cna.h"', '#include "CNAShim.h"']
    for name in MIRRORED_SCALARS:
        lines.append(
            f'_Static_assert(__builtin_types_compatible_p(CNA_{name}, CNASwift_{name}), '
            f'"scalar typedef: {name}");'
        )
    for name in MIRRORED_CALLBACKS:
        ret, params = shim[name]
        spelled = ", ".join(params) or "void"
        lines.append(f'typedef {ret} (*ShimShape_{name})({spelled});')
        lines.append(
            f'_Static_assert(__builtin_types_compatible_p(ShimShape_{name}, CNASwift_{name}), '
            f'"shim callback shape: {name}");'
        )
        lines.append(
            f'typedef {translate(ret, "CNASwift_")} (*Translated_{name})'
            f'({translate(spelled, "CNASwift_")});'
        )
        lines.append(
            f'_Static_assert(__builtin_types_compatible_p(Translated_{name}, CNA_{name}), '
            f'"callback ABI: {name}");'
        )
    lines.append("int main(void) { return 0; }")
    return "\n".join(lines) + "\n"


def key_cases() -> dict[str, int]:
    result: dict[str, int] = {}
    active = False
    depth = 0
    for line in KEYS.read_text(encoding="utf-8").splitlines():
        if re.search(r"public enum Keys\s*:\s*Int32", line):
            active = True
            depth = line.count("{") - line.count("}")
            continue
        if active:
            depth += line.count("{") - line.count("}")
            match = re.search(r"\bcase\s+(\w+)\s*=\s*(-?\d+)", line)
            if match: result[match.group(1)] = int(match.group(2))
            if depth <= 0: break
    return result


def c_key_name(swift_name: str) -> str:
    words = re.findall(r"[A-Z]+(?=[A-Z][a-z]|\d|$)|[A-Z]?[a-z]+|\d+", swift_name)
    name = "_".join(word.upper() for word in words)
    # The canonical constants join a trailing numeric identity to the word
    # which precedes it: D0, F12, NUM_PAD0, OEM8, APPLICATION1.
    name = re.sub(r"(?<=[A-Z])_(?=\d)", "", name)
    return "CNA_KEY_" + name


def probe_constants() -> int:
    """Distinct canonical constants the probe proves, counted from its source."""
    text = strip_comments(PROBE.read_text(encoding="utf-8"))
    names: set[str] = set()
    for match in re.finditer(r"_Static_assert\((.*?)\);", text, re.S):
        names.update(re.findall(r"\bCNA_[A-Z0-9_]+\b", match.group(1)))
    main = text[text.index("int main(void)"):]
    main = main[:main.index("printf")]
    names.update(re.findall(r"\bCNA_[A-Z0-9_]+\b", main))
    return len({n for n in names if not n.startswith(("CNA_ABI_VERSION", "CNA_SWIFT_"))})


def compile_and_run(cc: str, source: Path, include: Path, output: Path,
                    defines: list[str] | None = None) -> str:
    compile_result = subprocess.run([
        cc, "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
        *(defines or []), f"-I{include}", f"-I{SHIM_INCLUDE}", str(source), "-o", str(output),
    ], text=True, capture_output=True)
    if compile_result.returncode != 0:
        raise SystemExit(
            f"{source.name} failed the compiler wall:\n{compile_result.stderr.strip()}")
    return subprocess.run([str(output)], check=True, text=True, capture_output=True).stdout


# --------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cna-include", required=True, type=Path)
    parser.add_argument("--library", required=True, type=Path)
    parser.add_argument("--cc", default="cc")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    entries = manifest_entries()
    policy = abi_policy()
    aliases, declared, resolutions = swift_routes()
    mismatches: list[str] = []

    headers = header_text(args.cna_include)
    declarations = canonical_declarations(headers)
    shim_text = strip_comments(SHIM_HEADER.read_text(encoding="utf-8"))
    canonical_structs = struct_fields(headers, "CNA_", MIRRORED_STRUCTS)
    shim_structs = struct_fields(shim_text, "CNASwift_", MIRRORED_STRUCTS)
    canonical_callbacks = callback_signatures(headers, "CNA_")
    shim_callbacks = callback_signatures(shim_text, "CNASwift_")

    # --- route pairing, one property to one symbol to one route type --------
    route_pairings = 0
    seen_fields: set[str] = set()
    seen_routes: set[str] = set()
    for entry in entries:
        symbol, field, route = entry["symbol"], entry["swiftField"], entry["routeType"]
        expected_route = derived_route_type(symbol)
        if route != expected_route:
            mismatches.append(f"{symbol}: manifest route type {route} != derived {expected_route}")
        if field in seen_fields:
            mismatches.append(f"{symbol}: Swift field {field} is bound to more than one route")
        if route in seen_routes:
            mismatches.append(f"{symbol}: route type {route} is shared by more than one route")
        seen_fields.add(field)
        seen_routes.add(route)
        if declared.get(field) != route:
            mismatches.append(
                f"{symbol}: NativeFunctions declares {field}: {declared.get(field)!r}, manifest says {route}")
        resolved = resolutions.get(field)
        if resolved is None:
            mismatches.append(f"{symbol}: NativeFunctions never resolves into {field}")
        elif resolved != (symbol, route):
            mismatches.append(
                f"{symbol}: {field} resolves {resolved[0]} as {resolved[1]}, manifest says {symbol} as {route}")
        else:
            route_pairings += 1

    for name in sorted(set(aliases) - seen_routes):
        mismatches.append(f"{name}: route type is declared but no manifest route uses it")
    for field, (symbol, _) in sorted(resolutions.items()):
        if field not in seen_fields:
            mismatches.append(f"{field}: resolves {symbol} but is not in the manifest")

    # --- manifest vs canonical declaration, textually ------------------------
    missing_header = []
    declaration_checks = 0
    for entry in entries:
        declaration = declarations.get(entry["symbol"])
        if declaration is None:
            missing_header.append(entry["symbol"])
            continue
        ret, params = declaration
        declaration_checks += 1 + len(params)
        if ret != entry["return"]:
            mismatches.append(f'{entry["symbol"]}: manifest return {entry["return"]!r} != canonical {ret!r}')
        manifest_params = [re.sub(r"\s*\*\s*", "* ", item.strip()) for item in entry["parameters"]]
        if manifest_params != params:
            mismatches.append(
                f'{entry["symbol"]}: manifest parameters {manifest_params} != canonical {params}')

    # --- Swift signature vs manifest ----------------------------------------
    measured_positions = 0
    for entry in entries:
        alias = aliases.get(entry["routeType"])
        if alias is None:
            mismatches.append(f'{entry["symbol"]}: no Swift route type {entry["routeType"]}')
            continue
        swift_return, swift_parameters = alias
        c_return = canonical_type(entry["return"])
        c_parameters = [canonical_type(item) for item in entry["parameters"]]
        measured_positions += 1 + len(c_parameters)
        if swift_return != c_return:
            mismatches.append(f'{entry["symbol"]}: return {swift_return} != {c_return}')
        if swift_parameters != c_parameters:
            mismatches.append(f'{entry["symbol"]}: parameters {swift_parameters} != {c_parameters}')

    # --- mirrored structures, field for field --------------------------------
    for name in MIRRORED_STRUCTS:
        canonical_names = [field for _, field in canonical_structs.get(name, [])]
        shim_names = [field for _, field in shim_structs.get(name, [])]
        if not canonical_names:
            mismatches.append(f"CNA_{name}: no canonical definition in the CNA headers")
        elif not shim_names:
            mismatches.append(f"CNASwift_{name}: no shim definition")
        elif canonical_names != shim_names:
            mismatches.append(
                f"CNA_{name}: shim fields {shim_names} != canonical {canonical_names}")

    # --- mirrored callbacks, position for position ---------------------------
    if sorted(shim_callbacks) != sorted(MIRRORED_CALLBACKS):
        mismatches.append(
            f"CNAShim.h declares callbacks {sorted(shim_callbacks)}, "
            f"the mirrored set is {sorted(MIRRORED_CALLBACKS)}")
    for name in MIRRORED_CALLBACKS:
        if name not in canonical_callbacks:
            mismatches.append(f"CNA_{name}: no canonical callback typedef in the CNA headers")
            continue
        if name not in shim_callbacks:
            mismatches.append(f"CNASwift_{name}: no shim callback typedef")
            continue
        c_return, c_params = canonical_callbacks[name]
        s_return, s_params = shim_callbacks[name]
        translated = (translate(s_return, "CNASwift_"), [translate(item, "CNASwift_") for item in s_params])
        if translated != (c_return, c_params):
            mismatches.append(
                f"CNA_{name}: shim signature {translated} != canonical {(c_return, c_params)}")

    # Probes are compiled beside the package (build-probe/ is gitignored), never under /tmp:
    # the machine's build rules forbid compiling where nothing can be reused or audited.
    (ROOT / "build-probe").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="cna-swift-abi-", dir=ROOT / "build-probe") as temporary:
        temp = Path(temporary)
        probe_output = compile_and_run(
            args.cc, PROBE, args.cna_include, temp / "probe",
            defines=[f'-DCNA_SWIFT_ABI_MAJOR={policy["admittedMajor"]}',
                     f'-DCNA_SWIFT_ABI_MINOR={policy["minimumMinor"]}'],
        )
        generated = temp / "prototypes.c"
        generated.write_text(prototype_assertions(entries), encoding="utf-8")
        compile_and_run(args.cc, generated, args.cna_include, temp / "prototypes")

        callback_path = temp / "callbacks.c"
        callback_path.write_text(callback_assertions(shim_callbacks), encoding="utf-8")
        compile_and_run(args.cc, callback_path, args.cna_include, temp / "callbacks")

        layout_source, layouts, layout_fields = layout_assertions(canonical_structs, shim_structs)
        layout_path = temp / "layouts.c"
        layout_path.write_text(layout_source, encoding="utf-8")
        compile_and_run(args.cc, layout_path, args.cna_include, temp / "layouts")

        key_source = temp / "keys.c"
        key_lines = ['#include "CNA/C/input.h"']
        keys = key_cases()
        for name, value in keys.items():
            key_lines.append(f'_Static_assert({c_key_name(name)} == {value}, "Keys.{name}");')
        key_lines.append("int main(void) { return 0; }")
        key_source.write_text("\n".join(key_lines) + "\n", encoding="utf-8")
        compile_and_run(args.cc, key_source, args.cna_include, temp / "keys")

    exported = subprocess.run(
        ["nm", "-D", "--defined-only", str(args.library)], check=True, text=True, capture_output=True
    ).stdout
    exported_names = {line.split()[-1].split("@@", 1)[0] for line in exported.splitlines() if line.split()}
    missing_library = sorted(entry["symbol"] for entry in entries if entry["symbol"] not in exported_names)

    loaded_abi: int | None = None
    load_error: str | None = None
    try:
        library = ctypes.CDLL(str(args.library.resolve()))
        library.cna_get_abi_version.restype = ctypes.c_uint32
        loaded_abi = int(library.cna_get_abi_version())
    except OSError as error:
        load_error = str(error)
    if load_error:
        mismatches.append(f"library load: {load_error}")
    elif loaded_abi is None:
        mismatches.append("library did not answer cna_get_abi_version")
    else:
        major, minor = (loaded_abi >> 16) & 0xFFFF, (loaded_abi >> 8) & 0xFF
        if major != policy["admittedMajor"] or minor < policy["minimumMinor"]:
            mismatches.append(
                f"loaded ABI {major}.{minor}.{loaded_abi & 0xFF} is outside the admitted window "
                f'major {policy["admittedMajor"]} minor >= {policy["minimumMinor"]}')

    probe_values = dict(line.split("=", 1) for line in probe_output.splitlines() if "=" in line)
    header_abi = int(probe_values["ABI_VERSION"])
    if loaded_abi is not None and loaded_abi != header_abi:
        mismatches.append(
            f"library ABI {loaded_abi} disagrees with the canonical header ABI {header_abi}")

    constants = probe_constants() + len(keys)
    report = {
        "schemaVersion": 2,
        "ADMITTED_ABI_MAJOR": policy["admittedMajor"],
        "ADMITTED_ABI_MINIMUM_MINOR": policy["minimumMinor"],
        "QUALIFIED_ABI_VERSION": f'{policy["qualifiedMajor"]}.{policy["qualifiedMinor"]}.{policy["qualifiedPatch"]}',
        "HEADER_ABI_VERSION": f'{probe_values["ABI_MAJOR"]}.{probe_values["ABI_MINOR"]}.{probe_values["ABI_PATCH"]}',
        "HEADER_ABI_ENCODED": header_abi,
        "LOADED_ABI_ENCODED": loaded_abi,
        "BOUND_FUNCTIONS": len(entries),
        "ROUTE_PAIRINGS": route_pairings,
        "PROTOTYPE_TYPE_POSITIONS": sum(1 + len(entry["parameters"]) for entry in entries),
        "CANONICAL_DECLARATION_CHECKS": declaration_checks,
        "C_SWIFT_MEASUREMENTS": measured_positions,
        "LAYOUTS": layouts,
        "LAYOUT_FIELDS": layout_fields,
        "CALLBACKS": len(MIRRORED_CALLBACKS),
        "CONSTANTS": constants,
        "SCALAR_FACTS": sum(1 for key in probe_values if key.endswith("_SIZE")),
        "MISSING_HEADER_SYMBOLS": len(missing_header),
        "MISSING_LIBRARY_SYMBOLS": len(missing_library),
        "ABI_MISMATCHES": len(mismatches),
        "missingHeaderSymbols": missing_header,
        "missingLibrarySymbols": missing_library,
        "mismatches": mismatches,
        "probeOutput": probe_values,
        "library": {
            "filename": args.library.name,
            "sha256": hashlib.sha256(args.library.read_bytes()).hexdigest(),
        },
    }
    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    print(" ".join(f"{key}={report[key]}" for key in (
        "BOUND_FUNCTIONS", "ROUTE_PAIRINGS", "PROTOTYPE_TYPE_POSITIONS",
        "CANONICAL_DECLARATION_CHECKS", "C_SWIFT_MEASUREMENTS", "LAYOUTS", "LAYOUT_FIELDS",
        "CALLBACKS", "CONSTANTS", "SCALAR_FACTS", "MISSING_HEADER_SYMBOLS",
        "MISSING_LIBRARY_SYMBOLS", "ABI_MISMATCHES"
    )))
    return 1 if missing_library or missing_header or mismatches else 0


if __name__ == "__main__":
    raise SystemExit(main())
