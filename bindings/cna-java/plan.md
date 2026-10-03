# CNA-Java measured engineering plan

**Status:** the complete XNA 4.0 runtime superset is structurally at zero diagnostics, and
the CNA extension census is at `ACTIONABLE_LOCAL = 0`

**Updated:** 2026-09-30 (requalified against CNA C ABI 0.35.0 before the repository was archived
into `cna-lab`; `NEXT.md` has the measured state)

**Profiles:** the seven-assembly XNA 4.0 Windows runtime subset gate, and the ten-assembly full
runtime superset that adds GamerServices, Net and Avatar

**Native dependency:** the sibling `../cna` (`next`) built against `../sharp-runtime`, CNA C ABI
0.35.0

**Runtime-qualified platform:** Linux x86-64, CNA SDL3 platform, two single-renderer builds:
HEADLESS (SDL3 audio, no extended or device layer) and OPENGLES3 (extended graphics and device
layers, compiled effects), the latter only inside CNA's private GPU runner.

## Authority and normative boundaries

Microsoft XNA 4.0 metadata, IL, and reference behavior are authoritative. CNA-C# is the strongest
behavioral engineering comparison; CNA-TS/Rust, FNA, and MonoGame remain comparisons only. CNA C
headers are authoritative only at the native boundary.

The binding qualifies against the live sibling `cna`, itself built against the live sibling
`sharp-runtime`. It does not fall back to an unrelated CNA checkout: doing so would make every
ABI, symbol and layout result in this build describe a library nobody ships.

Structural API completeness and runtime capability are separate. Zero diagnostics means every
formally mapped type and member matches the pinned original assemblies. It does not mean every
runtime capability is present; `docs/runtime-capabilities.json` and the honest-boundaries section
of `NEXT.md` are the machine-readable and prose records of what is not.

Do not weaken the verifier, add an allowlist, expose CNA implementation types in strict packages,
or put non-XNA API inside `Microsoft.Xna.Framework.*`.

## Measured result

| Metric | Session start | Now |
|---|---:|---:|
| Selected-profile diagnostics | 0 | 0 |
| Full-profile diagnostics | 0 | 0 |
| Allowlist entries | 0 | 0 |
| CNA C ABI | 0.21.0 | 0.21.0 |
| Canonical C API functions | 4,054 | 4,054 |
| Bound native routes | 2,528 | 2,774 |
| Unbound CNA-extension routes (the census) | 394 | 73 |
| Locally actionable unbound routes | not measurable | **0** |
| Routes with no binding answer at all | 4,054 | 0 |
| Unexplained native routes | 0 | 0 |
| Bound routes no JNI entry point reaches | 0 | 0 |
| Bound routes no Java call site reaches | 0 | 0 |
| Coverage rule problems | not checked | 0 |
| Native tool tests | 146 | 179 |
| Tests | 516 | 593 |
| Renderers qualified | 5 | 5 |

Bound routes by header, where the session moved them:

| Header | Bound | Total |
|---|---:|---:|
| `engine_layer.h` | 845 | 857 |
| `cnb.h` | 264 | 272 |
| `effects.h` | 249 | 290 |
| `models.h` | 177 | 216 |
| `sensors.h` | 84 | 144 |
| `devices.h` | 47 | 62 |
| `core_ext.h` | 39 | 57 |
| `content_readers.h` | 15 | 62 |

## The one structural change: two questions, two fields

A route's classification used to carry a single `reason`, and the census could not be trusted
because of it. A hundred and forty-three routes carried text explaining *why the route is a CNA
extension rather than XNA*, which reads like an explanation and answers a question nobody asked
about binding.

`coverage-rules.json` schema 2 splits them. `classification` and `purposeReason` answer **why the
route exists for CNA-Java**. `bindingStatus`, `bindingReason` and `evidence` answer, separately,
**why it is not bound**, from a closed set: `BOUND`, `ACTIONABLE_LOCAL`, `BLOCKED_UPSTREAM`,
`BLOCKED_RENDERER`, `BLOCKED_HARDWARE`, `BLOCKED_PLATFORM`, `DEFERRED_TRACKED`,
`DELIBERATE_NON_BINDING`. A rule that supplies only a purpose is a gate failure, not a silent
pass, and `nativeCensusCheck` fails at any `ACTIONABLE_LOCAL` above zero.

The value of the split is that it made a specific mistake visible: **a reason that was true when
it was written and false when it was read**. Five were found this session. Three were about the
generator or about Java's own surface -- "the generator refuses this shape", "the Java
`ModelMeshPart` has no native handle" -- and each stopped being true when something else in the
same session was built. One was wrong about direction: the glTF import report was recorded as
belonging to a model CNA imports, when every one of its routes takes a model handle Java produces.
One was simply attached to the wrong route.

Two of the five shapes are now checked mechanically, with mutation tests behind both:
`STALE_BLOCKER_RULE_DECIDES_NOTHING` for a blocker that is the first match for no unbound route,
and `HALF_BOUND_PAIR`/`PAIR_CLASSIFIED_APART`/`PAIR_DECIDED_APART` for a two-call size-then-copy
pair whose halves were decided differently. The third shape -- a stale `DELIBERATE_NON_BINDING` --
reads exactly like a live decision and was found only by reading all fifty-six of them against
today's Java surface.

## Milestones reached this session

**H1, skinning and animation.** `JAVA-EXT-007` had eighty routes blocked on "a clip enters a
skinned model only through a `CNA_SkinningDataDescriptor`, a pointer graph the generator refuses".
That is true about the generator and says nothing about the lifetime: the graph is borrowed for
exactly one call, which is a marshaller, not a barrier. `CnaSkeleton`, `CnaSkinningData`,
`CnaAnimationPlayer`, `CnaSkinnedModel`, `CnaModelMeshPartHandle`, `CnaMorphTargetData` and
`CnaMorphWeightTrack` are the result, over hand-written flatteners that sum every count in a
`jlong` and check it before allocating.

Two ownership readings were wrong and were corrected by measurement, not by re-reading:
`add_part` **retains** a part rather than taking it, so the close order is model, then part, then
buffers; and `get_part_at` lends an **owned** alias the caller must close, while the texture
handle beside it is the model's own and releasing it is refused.

**H2, `.cnb` gained its writing half.** `CnbByteWriter`, `CnbAnimationClip`, `CnbBoneTrack`,
`CnbClip` and `CnbKeyframes`. Measured, not assumed: the string prefix is a fixed 32-bit count
rather than a 7-bit one, and the external-reference flags are reserved and must be zero.

**H3, the host surface.** `MessageBox`, `FileDialog`, `SystemTray`, and the sensors, all
qualified through CNA's own test backends on a machine that has none of the hardware. The
file-dialog callback is one-shot and deletes its own global reference; the tray's is not.

**H4, a model's provenance.** `GltfImportReport`, `GltfImportSourceCounts` and
`GltfImportDiagnostic`. CNA stores twelve counts and derives five, and refuses a report carrying
any of the five -- so the Java API takes the twelve alone, because one that accepted a whole
report would be offering a call that cannot succeed.

**H5, the log sink.** `CnaLogger.setSink`. CNA's default sink writes to stderr and never stdout,
deliberately, because a terminal-hosted game draws its frame on stdout; a game that already logs
somewhere can now put CNA's lines there too. The reference discipline is the whole risk, and
clearing an atomic pointer is not enough: a thread already inside the callback is not recalled by
it, so the adapter counts readers and waits for them to leave before deleting the reference.

**H6, two new upstream findings**, each reproduced in pure C first. `JAVA-UPSTREAM-022`: the
shader dialect query answers `UNKNOWN` on all five renderers. `JAVA-UPSTREAM-023`:
`copy_tangent_deltas` bounds-checks the wrong array.

## What the generator learned, and what it still refuses

Two shapes were added, both with tool tests including their refusals:

- **`string_array`** -- `const CNA_StringView* xs, uint64_t n` becomes one `byte[][]`, the count
  disappears because a Java array carries its own length, and the elements are copied to bound
  local references rather than pinned.
- **`parallelArrays`** -- `routes.json` declares which arrays one count governs; the count
  disappears and a length mismatch is refused in the prologue.

It still refuses, correctly, a C function pointer and a pointer to an array of string views inside
a structure. Those are hand-written, and this session wrote five: the tray entry, the file dialog,
the sensor leaves, the glTF diagnostic descriptor and the log sink.

## Mapping rules (added 2026-08-31)

Each is in `tools/api-compat/mapping-rules.json` with its reason, and each is a general rule or an
explicitly reviewed full signature, never a suppression:

- `clrSerializationParameterTypes` omits every member whose whole parameter list is the CLR
  `(SerializationInfo, StreamingContext)` pair. Java has no equivalent and Java serialization is a
  different contract with different security properties.
- `omittedCovariantInterfaces` records that `GamerCollection<T>` keeps `Iterable<T>` and bounds
  `T` by `Gamer`. CLR adds a covariant `IEnumerable<Gamer>`; Java forbids two parameterizations of
  one interface, and the bound gives the same call sites what they wanted.
- `clrInternalMembers` records the two members that replace CLR `internal` access across packages:
  a parameterless protected `Gamer` constructor and a protected `GamerCollection` constructor over
  two functional interfaces. The native handle is recorded through an internal handle table
  instead of crossing a protected signature.
- `javaMapBridgeTypes` gives `PropertyDictionary` the full `java.util.Map` bridge beside its
  XNA-named members; both reach one native dictionary.
- Three `refOutMemberMappings` entries collapse `LocalNetworkGamer.ReceiveData`'s CLR
  `out NetworkGamer` into one `ReceiveResult`, and one collapses
  `PropertyDictionary.TryGetValue` to the value, as `ModelBoneCollection.TryGetValue` already did.
- A generic struct's copy constructor takes its own parameterization, the only form Java can write
  without a raw type.

## Native boundary

```text
HEADER_ABI=0.21.0
CANONICAL_FUNCTIONS=4054
BOUND_FUNCTIONS=2774
MANIFEST_SIGNATURE_CHECK=PASS
MANIFEST_JNI_BINDING_CHECK=PASS
JNI_HEADER_DERIVED_SLOT_CHECK=PASS
LAYOUT_SIGNATURE_PROBE=PASS
LIBRARY_ABI=0.21.0
LIBRARY_SYMBOL_CHECK=PASS (2774/2774)
ABI_POLICY_CHECK=PASS
NATIVE_TOOL_TESTS=179 passed, 0 failed
```

Every slot is declared `CNA_JNI_ROUTE(symbol)`, so a signature that moves upstream stops the
adapter compiling instead of crashing at runtime. `tools/native-abi/generate_jni.py` derives the
mechanical half of the boundary from the headers and refuses a shape it does not understand rather
than guessing; `generateJniCheck` fails the build when the committed output goes stale.

Hand-written entry points exist only where the generator refuses and must: a C function pointer, a
pointer to an array of string views inside a structure, and the descriptor pointer graphs the
skinning and morph families are entered through. Every callback is classified by lifetime -- one
call, one shot, or registered until an explicit token releases it -- and the whole suite runs
clean under `-Xcheck:jni` on all five renderers.

## Coverage

```text
PURPOSE                          BINDING
XNA_BACKING              986     BOUND                    2774
JAVA_INTERNAL_ONLY        11     DEFERRED_TRACKED          320
CNA_EXTENSION_CANDIDATE 1850     DELIBERATE_NON_BINDING    938
DEFERRED_RUNTIME         321     BLOCKED_UPSTREAM           22
NOT_USEFUL_IN_JAVA       886     ACTIONABLE_LOCAL            0
UNMAPPED_REQUIRES_REVIEW   0     UNREVIEWED                  0

EXTENSION_CENSUS  73    RULE_PROBLEMS 0
BOUND_BUT_UNREACHED 0   BOUND_WITHOUT_JAVA_CALL_SITE 0
```

`NOT_USEFUL_IN_JAVA` is dominated by value math -- vectors, matrices, quaternions, geometry,
curves, packed vectors -- which is managed in XNA and managed here; routing it through JNI would
add cost and a native failure mode the original API cannot produce.

## Next

`docs/backlog.json` is the machine-readable backlog. Every `JAVA-UPSTREAM-*` finding is closed as
of 2026-09-30: fixed in CNA, removed with CNA's engine layer at ABI 0.30, or withdrawn as a probe
artifact (-018, -020, -021 read outputs before the route wrote them). No route is
`BLOCKED_UPSTREAM`; `ACTIONABLE_LOCAL` is 0 and `nativeCensusCheck` holds it there.

320 routes are `DEFERRED_TRACKED`: XNA-backing routes whose Java members are projected and behave,
where what is deferred is moving the implementation onto the native route.

What would genuinely unblock more is external: a licensed authored XACT bank, a second machine for
a real network session, an authored `.xnb` effect, and hosts other than Linux x86-64.
