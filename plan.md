# CNA-Swift normative plan and status

**Current state.** The native boundary is CNA C ABI **major 0, minor 35 or
later**, qualified against `0.35.0` (HEADLESS and OPENGLES3, 2026-09-30; see
`docs/cna-0-35-requalification.md`). Foundation Milestones 1 through 106 are
complete. Foundation 104 closes the selected XNA runtime-reader family:
`ContentReader` is a real subclass of the admitted .NET Framework 4.0
`BinaryReader` subset; `ContentTypeReader`, its generic subclass and the
manager form one typed runtime path; `ContentManager.OpenStream` and
`ReadAsset<T>` now feed an uncompressed XNB parser; and
`ResourceContentManager` reads the same bytes through the selected,
self-contained `ResourceManager` projection. The deterministic test fixture is
authored byte-for-byte in this project. Foundation 105 closes all thirteen
Design types with real `TypeConverter`, reflection-constructor and
`InstanceDescriptor` support, including culture-aware text conversion and
invocable constructor descriptors. Foundation 106 closes the complete retained
XNA 4.0 public profile: the six XACT/microphone types use canonical CNA routes,
the storage stream surface is a real duplex `InputStream` subclass, backbuffer
readback has its three generic overloads, both pinned serialization constructors
forward their admitted state, and the last redeclared dynamic-audio accessors
match their IL. No `Encoding` API was admitted, no
storage/save-game `cna_content_reader_*` route was misbound, and compressed XNB
remains an explicit unsupported capability rather than a silent claim.

This file states what is true **now**, and a gate keeps that literal:
`tools/status_gate/verify.py` derives the current Foundation from the highest
`docs/foundation-<N>-*-evidence.md` on disk and every policed count from the
generated reports, then refuses any normative sentence here, in `README.md` or
in `NEXT.md` that disagrees. The milestone-by-milestone progression lives in
`NEXT.md` and in the per-milestone `docs/foundation-*-evidence.md` files, which
are not summarised away here; where this document once carried a milestone's own
prose, that milestone's evidence file carries it still.

## Normative rules

1. **XNA authority.** Pinned Microsoft XNA 4.0 Windows runtime metadata is the
   public shape authority, and the hash-matched assembly IL is the behaviour
   authority. An assembly earns behaviour authority only by **registration**:
   its SHA-256 is recorded and `tools/api_compat/pinned_assembly_audit.py`
   machine-compares its public metadata against every retained contract entry it
   declares. The audit is calibrated on the assemblies whose provenance predates
   it and carries its own mutation self-tests, so it cannot pass vacuously.
   Seven assemblies are registered and together reproduce the contract's 257
   types and 2,964 members exactly. FNA and MonoGame are engineering
   comparators only. **CNA is never XNA behaviour authority**: a value observed
   from the native runtime is native evidence and is recorded separately.
2. **A user-visible XNA message is read, not transcribed.** XNA loads a
   resource key and the runtime resolves it against the assembly's own string
   table, so every reproduced message is read out of the registered binary,
   pinned in `tools/api_compat/reference/xna40-selected-resource-strings.json`,
   and compared against the Swift source by the verifier. Foundation 49 made
   that literally true: five messages were byte-identical to the assembly's own
   and yet unpinned, so nothing was comparing them. All 98 selected XNA
   resource strings are pinned now.
3. **A complete type does not imply runtime capability.** Profile selection,
   presentation, primitives, vertex and index buffers, cube textures, effects,
   compressed XNB, five unwired built-in content loaders, windows, and audio
   playback all remain unclaimed. Uncompressed custom XNB and ResourceManager
   content are verified managed capabilities as of Foundation 104. Render
   targets are created, bound and consumed natively. The four state objects are
   applied to a device as of Foundation 45, and device status, the viewport,
   the scissor rectangle and all three `Clear` overloads followed in
   Foundations 47 and 48. Two measured divergences shaped how that was done and
   are recorded rather than worked around: CNA's graphics-device handle is a
   per-callback capability token and not a stable identity, so device-owned
   managed state is anchored to the game; and `BlendFunction.Min`/`.Max` are
   numbered the other way round in CNA than in XNA, so every state enum crosses
   the boundary through an explicit map. Both are stated with the rest of that
   research in
   `docs/frontier-research-graphics-device-state-and-vertex-declaration.md`.

   Foundation 48 is the reminder that this cuts both ways: `Clear(Color)` had
   been *implemented* since the earliest graphics work and was **wrong**,
   clearing colour where XNA also clears depth, on the only device
   configuration a projected game ever gets (`DepthFormat.Depth24`, measured).
   A member being present, tested and green is not evidence that it agrees with
   the pinned IL; only reading the IL is. See
   `docs/foundation-48-clear-evidence.md`.

   Reading every implemented `GraphicsDevice` member's IL by size, which is how
   that was found, immediately found two more: `set_Viewport` and
   `set_ScissorRectangle` were projected with none of their validation
   (Foundation 49). Generalising the reading into a gate found a fourth,
   `DrawableGameComponent.GraphicsDevice` raising its sibling's message, and
   `SpriteBatch`'s whole begin/end rule (Foundation 50).

   Four defects in three milestones, none found by a failing test, is why
   `tools/api_compat/message_coverage.py` exists: for every implemented member,
   every message its pinned IL can raise is either reproduced or recorded with
   a reason and a kind. A member that disagrees with XNA is worse than one that
   is honestly absent, and the gate is what keeps the fifth one from being
   found by a user.
4. **A CLR field's writability is metadata, not convention.** `literal` and
   `initonly` are two different `FieldAttributes`, and neither is assignable, so
   both project to a Swift `let`. `readonly` is recorded on all 557 contract
   fields, proven against the registered binaries by the pinned-assembly audit,
   and independently cross-checked with a second disassembler. See
   `docs/foundation-41-graphics-state-evidence.md`.
5. **Public strict names** use `Microsoft.Xna.Framework...` and exact XNA
   PascalCase. Formal Swift projections are measured; manual diagnostic
   allowlisting is forbidden, and `ALLOWLIST_ENTRIES` is 0.
6. **Mapping rules are measured, never invented for convenience.** The full set
   is in `docs/xna-swift-mapping.md` and `tools/api_compat/mapping-rules.json`.
   The load-bearing ones: a non-flags CLR enum maps to a Swift `enum` with the
   CLR underlying raw type and one explicitly valued case per literal; a
   `[Flags]` enum maps to an `OptionSet`, with `[Flags]` read out of the pinned
   binary rather than assumed; a CLR interface maps to a Swift `protocol` with
   no invented conformance; a CLR class maps to a Swift class, and CLR
   derivability is not strengthened casually; `System.Type` maps to the Swift
   metatype; `Dictionary<K,V>` maps to a reference class with the CLR's own
   storage algorithm; a CLR event maps to the `CNAEvent` architecture;
   `System.IntPtr` maps to Swift `Int` as a documented general language rule
   whose expected projection is never `RAW_HANDLE_LEAK` — an exemption that
   covers the mapped XNA IntPtr value and never a CNA FFI or native handle.
7. **The native boundary.** CNA-Swift admits the ABI window CNA itself
   publishes for a consumer — reject a different major, require a minimum minor
   — which is major `0` exactly and minor `35` or later, qualified against
   `0.35.0`. Native selection is an absolute `CNA_NATIVE_LIBRARY` override or
   the installed soname, never a developer-tree fallback. Every bound symbol
   must resolve by name before the runtime starts. See `docs/native-abi.md` and
   `docs/native-abi-migration-evidence.md`.
8. **Two error channels stay separate, and the split is by cause.** `CNAError`
   is the CNA runtime's own failure channel. Projected CLR/XNA failures are the
   `CNAException` class hierarchy, which conforms to `Error`. `catch is
   CNAException` must not swallow a native CNA runtime failure, and the two are
   never merged for convenience. The line is what *caused* the failure, not
   which layer noticed it: a consumer using a resource it disposed is a CLR
   failure and raises `CNAObjectDisposedException`, while a native object whose
   C lifetime ended has no XNA counterpart and stays on `CNAError`. The same
   Swift enum case can serve both, and four `CNAError.disposedObject` sites do.
9. **Native ownership is explicit.** Every native handle has a recorded
   ownership — `OWNED`, `BORROWED`, `PARENT_OWNED`, `PROCESS_GLOBAL` or
   `MANAGED_VALUE` — and disposal is deterministic. A Swift `deinit` is a
   backstop, never the only correctness mechanism, and no callback may outlive
   the rooted Swift state.
10. **Every gate must be shown to fail.** A verifier, audit or runtime gate is
    evidence only once a planted, realistic defect has been proven to break it.
    `tools/native_abi/mutations.py`, `tools/projection_mutations/run.py`, the
    API verifier's self-tests and graph fixtures, the pinned-assembly audit's
    mutations and the BCL authority's negative controls are all of this kind.
11. **A normative status claim is derived, never remembered.** The current
    Foundation is the highest `docs/foundation-<N>-*-evidence.md` on disk and
    every policed count comes out of a generated report;
    `tools/status_gate/verify.py` refuses any sentence in this file, `README.md`
    or `NEXT.md` that disagrees, and additionally regenerates the strict report,
    the missing-type inventory, the dependency graph and the native ABI report
    to prove each committed copy is still what a live run produces. Historical
    prose is exempt only inside an explicit `<!-- status-gate:historical -->`
    region and is never deleted to make a claim true. Both failures this rule
    exists for had already happened: see
    `docs/foundation-58-normative-status-gate-evidence.md`.

## Measurement status

Reproduced live on CNA 0.35.0 at the current HEAD.

```text
REFERENCE_TYPES=257            REFERENCE_MEMBERS=2964
EXPECTED_SWIFT_TYPES=257       EXPECTED_SWIFT_MEMBERS=2887
TARGET_TYPES=257               TARGET_MEMBERS=2890
COMPLETE_TYPES=257             PARTIAL_TYPES=0      MISSING_TYPE=0
MISSING_MEMBER=0               TOTAL_DIAGNOSTICS=0
ALLOWLIST_ENTRIES=0            UNMEASURED_STRUCTURAL_CATEGORY=0
NONDERIVABLE_UNSEALED_CLASSES=0    PENDING_BCL_BASE_TYPES=0
XNA_RESOURCE_STRING_PROJECTIONS=103 API_COMPAT_SELF_TESTS=2684
```

The retained public profile is strict-complete: every one of the 257 types is
complete and all structural, absence and leak diagnostic categories are zero.

Every category that would mean the projection **disagrees** with XNA is 0:
`TYPE_KIND_MISMATCH`, `BASE_MAPPING_MISMATCH`, `INTERFACE_MAPPING_MISMATCH`,
`FIELD_MAPPING_MISMATCH`, `PROPERTY_MAPPING_MISMATCH`,
`METHOD_SIGNATURE_MAPPING_MISMATCH`, `PARAMETER_MAPPING_MISMATCH`,
`RETURN_MAPPING_MISMATCH`, `GENERIC_MAPPING_MISMATCH`, `ENUM_VALUE_MISMATCH`,
`FLAGS_MAPPING_MISMATCH`, `EVENT_MAPPING_MISMATCH`, `OPERATOR_MAPPING_MISMATCH`,
`REF_OUT_MAPPING_MISMATCH`, `LANGUAGE_MAPPING_MISMATCH`,
`INHERITANCE_MAPPING_MISMATCH`, `UNEXPECTED_TYPE`, `UNEXPECTED_MEMBER`,
`UNMEASURED_STRUCTURAL_CATEGORY`, `INTERNAL_TYPE_LEAK`, `RAW_HANDLE_LEAK` and
`PUBLIC_NATIVE_FFI_LEAK`. Wherever this binding has projected an XNA member it
agrees with the pinned metadata.

Native boundary:

```text
BOUND_FUNCTIONS=778  ROUTE_PAIRINGS=778  PROTOTYPE_TYPE_POSITIONS=2657
CANONICAL_DECLARATION_CHECKS=2657  C_SWIFT_MEASUREMENTS=2657
LAYOUTS=68  LAYOUT_FIELDS=522  CALLBACKS=9  CONSTANTS=228  SCALAR_FACTS=3
MISSING_HEADER_SYMBOLS=0  MISSING_LIBRARY_SYMBOLS=0  ABI_MISMATCHES=0
NATIVE_ABI_MUTATIONS=14  NATIVE_ABI_MUTATIONS_CAUGHT=14
NATIVE_ABI_MUTATION_SURVIVORS=0
PROJECTION_MUTATIONS=413  PROJECTION_MUTATIONS_LAST_FULL_RUN=413
PROJECTION_MUTATIONS_CAUGHT=413  PROJECTION_MUTATION_SURVIVORS=0
PROJECTION_MUTATION_HUNG=0  PROJECTION_MUTATION_UNSCORED=0
CONTENT_READER_MUTATIONS=11  CONTENT_READER_MUTATIONS_CAUGHT=11
CONTENT_READER_MUTATION_SURVIVORS=0  CONTENT_READER_MUTATION_HUNG=0
CONTENT_READER_MUTATION_UNSCORED=0
WITHDRAWN_IN_SOURCE=27  REPLACED_NO_OPS_IN_SOURCE=3
```

The projection-mutation count is what the harness holds; `CAUGHT` is what the
latest **full run** proved. Foundation 106 executed all 413 declared mutations:
all 413 were caught, with zero survivors, hangs and unscored entries. Eleven of
those are the selected ContentReader defects. Three historical no-op mutations
have been replaced in source by observable defects; no no-op mutant is scored.

The projection-mutation harness refuses to run without a selected
`CNA_NATIVE_LIBRARY`: sixteen of its mutations are caught only by suites that
start a CNA runtime, and those suites *skip* rather than fail when no library
is selected, which would report a coverage loss as sixteen projection defects.

The seven registered reference assemblies reproduce 257 contract types and 2,964
contract members exactly; calibration and the audit's mutation self-tests pass
(`AUDIT_SELF_TESTS=80`, `RESOURCE_STRINGS_REPRODUCED=103`). The BCL authority
carries `BCL_SENTINEL_CHECKS=651`, `BCL_MUTATION_SELF_TESTS=514`,
`BCL_CROSS_CHECKS=297` against a second disassembler, and four negative
controls that are still refused. The four are not the same four binaries as in
earlier sessions -- this machine's Mono packages were upgraded since, and none
of the previously recorded control digests exists on disk any more, so the set
was rebuilt from what is here now. One of the four is the strongest control the
gate has had: a **genuine Microsoft** `mscorlib.dll` from another .NET 4.0
install, refused by 2 of 21 checks rather than by 12 to 16. A control that only
just fails is worth more than three that fail obviously.
Two BCL assemblies are admitted, 59 types and 753 members between them.
`mscorlib` `5634668d…acc63` now also supplies the exact `BinaryReader` and
`ResourceManager` subsets used by Foundation 104, while `System.dll`
`c3182e40…` was admitted at Foundation 100 for the five-type
`System.ComponentModel` closure the thirteen `Microsoft.Xna.Framework.Design`
converters are built from — `TypeConverter`, `ExpandableObjectConverter`,
`ITypeDescriptorContext`, `PropertyDescriptor` and
`PropertyDescriptorCollection`, 98 members — together with
`Globalization.CultureInfo` and `Collections.IDictionary` from mscorlib, which
those signatures reach for. Foundation 105 completes that closure with
`TextInfo`, the selected reflection constructor identities,
`TypeConverter.StandardValuesCollection` and
`ComponentModel.Design.Serialization.InstanceDescriptor`. The Swift
`CNAInstanceDescriptor` carries a real `CNAConstructorInfo`, preserves its
argument list and invokes the represented constructor; the projection therefore
keeps XNA's advertised conversion behavior without a string-only identity or a
false capability answer.

The identity work found a defect worth recording: the audit read an assembly's
own name with a regex that matched the FIRST `.assembly` line, which in any
assembly that references another is `.assembly extern mscorlib`. It was right
for `mscorlib` — which references nothing — and wrong for everything else. A
negative lookahead fixed it. Admitting a second assembly is what exposed it.

## Platform and release policy

Linux x86-64 with Swift 6.0.3 and an external CNA C ABI 0.35.0 library is the
qualified runtime, measured with HEADLESS and with OPENGLES3 (EasyGL) on a
private Weston + Xwayland display, SDL3 audio through SDL's dummy driver. This
host has no attached controller, so no positive controller behaviour is
claimed, and no live-desktop output is. Apple platforms, Windows, and Web/Wasm
remain unqualified.

## The frontier

The retained XNA 4.0 public profile has no local structural frontier:
`docs/generated/dependency-graph.json` reports no complete-dependency missing
type and `docs/generated/missing-type-inventory.md` contains no missing or
partial type. Runtime capability refusals that CNA cannot truthfully satisfy
remain documented separately; they are not missing Swift API identities.

Completion of a milestone requires debug and release builds and tests,
warnings-as-errors including tests, Symbol Graph and its self-tests, the
verifier's self-tests, strict and leak-only runs, pure XNA and BCL behaviour,
the pinned assembly audit, the BCL authority audit, the native ABI verifier and
its mutation controls, GamePad and Keyboard regression, native stress, Swift
ASan and TSan, an unchanged template at 60 and 600 frames, a deterministic exact
source archive, the isolated consumer and its rejected negative consumers,
`git diff --check`, and one coherent commit. Publication is a separate explicit
boundary and is never crossed without instruction.
