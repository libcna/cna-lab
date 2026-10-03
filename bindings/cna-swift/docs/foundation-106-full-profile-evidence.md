# Foundation 106 — retained XNA profile closure

Foundation 106 closes every remaining identity in the retained Microsoft XNA
4.0 Windows runtime contract. The compiler Symbol Graph contains all 257 target
types and 2,890 mapped Swift members; all 257 types are complete and every
diagnostic category is zero.

The last runtime-backed family is `AudioEngine`, `AudioCategory`, `Cue`,
`SoundBank`, `WaveBank` and `Microphone`. They use canonical CNA 0.21 routes,
preserve XNA ownership and disposal order, and are exercised with deterministic
project-authored XACT fixtures. No Microsoft asset or binary is packaged.

The remaining member closure includes real duplex storage file streams, the
three `GraphicsDevice.GetBackBufferData<T>` overloads, the pinned
`MediaLibrary.SavePicture` refusal, the two protected exception serialization
constructors, and `DynamicSoundEffectInstance`'s redeclared `IsLooped` and
`Play` members. The serialization support is demand-driven and does not expose
unselected formatter, culture or `Encoding` APIs.

```text
REFERENCE_TYPES=257 REFERENCE_MEMBERS=2964
TARGET_TYPES=257 TARGET_MEMBERS=2890
COMPLETE_TYPES=257 PARTIAL_TYPES=0 MISSING_TYPES=0
TOTAL_DIAGNOSTICS=0 MISSING_TYPE=0 MISSING_MEMBER=0
OVERLOAD_MAPPING_MISMATCH=0 ALLOWLIST_ENTRIES=0
UNMEASURED_STRUCTURAL_CATEGORY=0
BOUND_FUNCTIONS=778 ROUTE_PAIRINGS=778 ABI_MISMATCHES=0
PROJECTION_MUTATIONS=413 PROJECTION_MUTATIONS_CAUGHT=413
PROJECTION_MUTATION_SURVIVORS=0 PROJECTION_MUTATION_HUNG=0
PROJECTION_MUTATION_UNSCORED=0
CONTENT_READER_MUTATIONS=11 CONTENT_READER_MUTATIONS_CAUGHT=11
CONTENT_READER_MUTATION_SURVIVORS=0
CONTENT_READER_ACTIONABLE_LOCAL=0 CONTENT_READER_UNREVIEWED=0
BCL_CONTENT_CLOSURE_UNREVIEWED=0 CONTENT_READER_MISSING_TYPES=0
CONTENT_READER_STRICT_DIAGNOSTICS=0 CONTENT_READER_ENCODING_DEPENDENCIES=0
BINARY_READER_AUTHORITY_FINDINGS=0 RESOURCE_MANAGER_AUTHORITY_FINDINGS=0
```

The full 413-mutation campaign used the real native library and the complete
994-test suite wherever a mutation had no narrower proven test. All 413 defects
were caught, with source restored byte-for-byte. Filesystem probes used only the
repository's documented build artifacts and the isolated session root under
`/tmp`; no foreign user path was deleted.
