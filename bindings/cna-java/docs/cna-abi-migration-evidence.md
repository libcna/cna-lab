# CNA C ABI 0.7.0 -> 0.20.0 migration evidence

**Measured:** 2026-08-30

> Historical record. `cnanext`/`sharp-runtimenext` below were the dependency checkouts of the
> time; they are now the sibling `cna` and `sharp-runtime`, and the binding targets ABI 0.35.0
> (see `NEXT.md` for the 2026-09-30 requalification).

CNA-Java's development dependency is now the live sibling `cnanext` checkout, built against the
live sibling `sharp-runtimenext` checkout. The previous baseline -- an unrelated historical CNA
checkout and a pinned `/tmp` artifact at ABI 0.7.0 -- is gone, and `build.gradle` no longer falls
back to `../cna` or `../../cna`: a missing CNA checkout is now a clear configure failure naming the
header it could not find.

## Qualified native dependency

```text
cnanext HEAD              17b5a90a0878f3f44c23bc8e3197d5d30373dc72 (branch next, clean)
                          72262a33ed5ae7657024c7f1251338748a3feee5 when this was first measured
sharp-runtimenext HEAD    eebebd862121953538e3b84d43384d70a8a1728d (branch next)
                          df1b42abfcdefda030d63e97f16d2f7ea883837f when this was first measured
compiler                  g++ (Debian 14.2.0-19) 14.2.0, C++23, ccache
build type                Debug, Ninja
CNA_SHARP_RUNTIME_ROOT    /rv/data/development/github.com/openeggbert/sharp-runtimenext
CNA_BUILD_C_API           ON
CNA_PLATFORM              HEADLESS
CNA_GRAPHICS_RENDERER     HEADLESS
CNA_AUDIO_PLATFORM        NULL
CNA_CNAEXT                ON
CNA_DEVICES               ON
CNA_ENABLE_NET            ON
CNA_ENABLE_VIDEO          AUTO -> enabled (FFmpeg backend cna_video_ffmpeg)
CNA_BUILD_TESTS           ON
CNA_BUILD_EXAMPLES        OFF
artifact                  cmake-build-javanext/modules/c-api/libcna_c_api.so
reported ABI              0.20.0
exported cna_ symbols     4051
C API inventory SHA-256   e9e0be892dbdce49dedf195dac35f604a9263565a74473195d878ee9e580696d
```

Both dependency HEADs advanced while this work was in progress. The C API did not: the same
4,051 declarations, the same 4,051 exported symbols, the same ABI and the same inventory hash
before and after. The library's own SHA-256 changed with the rebuild, which is why the identity
recorded here is the inventory hash: it is what the binding actually depends on.

The build directory follows `cnanext`'s own mandatory convention -- a stable in-repo
`cmake-build-<variant>/` directory, matched by that repository's `.gitignore`. `git status` in
`cnanext` is clean and no tracked file there was modified.

## What actually changed at the boundary

The header inventory extracted from `modules/c-api/include/CNA/C/*.h` contains **exactly** the 4051
symbols the built library exports -- no header-only declaration and no undeclared export. Against
that inventory:

```text
BOUND_FUNCTIONS                723
BOUND_SYMBOL_REMOVED_UPSTREAM    0
RETURN_TYPE_DRIFT                0
PARAMETER_DRIFT                  0
HEADER_DRIFT                     3  (cna_song_get_album/_artist/_genre moved
                                     media.h -> media_library.h; manifest updated)
UNBOUND_FUNCTIONS             3328
```

Thirteen ABI minor versions moved, and every route CNA-Java binds survived them unchanged in
signature. The version number was therefore **not** the migration; three behaviour contracts were.

## Behaviour contracts that changed, and what Java now asserts

`docs/c-api/ABI_VERSIONING.md` records seven contract changes in ABI 0.9.0. Three of them reach
CNA-Java's tests, and in each case the new CNA behaviour is *closer* to the XNA reference, so the
Java expectation was corrected rather than the behaviour worked around:

| Contract | Before (0.7.0) | Now (0.20.0) | Java change |
|---|---|---|---|
| `SoundEffectInstance.Apply3D` listener count | every count but one refused with `NOT_SUPPORTED` | any positive count accepted; nearest listener decides attenuation, pan and Doppler | the test now exercises two listeners successfully; an empty array still fails |
| Non-finite sprite/transform values | refused with `INVALID_ARGUMENT` | carried into the vertex path, as the reference `SpriteBatch` does -- it validates no float | `SpriteBatch.Begin` with a NaN transform component now succeeds |
| SpriteFont glyph order | unchecked | strictly ascending characters required, because lookup is a binary search | the `.cnj` fixture is ordered `'?'` then `'A'`, and `Characters` reports that order |

## ABI compatibility policy

CNA states that ABI `0.x` is experimental and that **an incompatible change requires a
minor-version increment**. A consumer compiled against one `0.x` minor therefore cannot assume the
next one is compatible, whatever the CMake package's `SameMajorVersion` file accepts. CNA-Java's
gate is exact major **and** exact minor, with the patch component free, and the required version is
read from `tools/native-abi/bindings.json` rather than duplicated as a literal.

Regression coverage for the policy lives in `tools/native-abi/test_verify.py`: the exact version is
accepted, a differing patch is accepted, and a different major, a higher minor and a lower minor
are each rejected with their own diagnostic.

## Stale JNI declarations are now a compile error

The dispatch table used to carry 426 hand-written function-pointer typedefs. `dlsym` returns
`void*`, so a typedef that drifted from the real declaration compiled cleanly and crashed at
runtime. Every one of the 723 slots is now declared as

```c
#define CNA_JNI_ROUTE(symbol) __typeof__(&symbol)
...
    CNA_JNI_ROUTE(cna_game_run) game_run;
```

so each slot's type *is* the header's declaration and every call site is type-checked against it.
407 dead typedefs were removed; the 19 still used as local variable types remain. The whole
adapter compiles with `-std=c11 -Wall -Wextra -Werror` against the 0.20.0 headers with no
diagnostic, which is itself the proof that no bound route's signature moved.

`tools/native-abi/verify.py` additionally requires that every `CNA_JNI_ROUTE` slot names the same
symbol the matching `LOAD` line loads, so a slot cannot be declared from one route and filled from
another.

## Verification layers

```text
inventory      4051 header declarations == 4051 exported symbols
manifest       723 bound routes, exact return type and parameter types
jni            LOAD set == manifest set; every slot header-derived and consistent
layout         probe.c compiles: ABI identity, sizes, alignments, field offsets
library        723/723 symbols exported by the built libcna_c_api.so
policy         loaded ABI 0.20.0 satisfies the manifest's stated policy
tool tests     29 checks, each with a negative mutation that must fail
```

## Result

```text
./gradlew check (native enabled)      BUILD SUCCESSFUL
TESTS=156 SUITES=32 FAILURES=0 ERRORS=0 SKIPPED=0
apiCompatCheck (selected profile)     TOTAL_DIAGNOSTICS=0
nativeAbiCheck                        all layers PASS
nativeCoverageCheck                   UNMAPPED_REQUIRES_REVIEW=0
```

Those are the figures **at the migration**, which is what this document is about: 723 bound
routes and 156 tests. The work that followed took them further, and `NEXT.md` carries the
current ones. The migration facts above do not move with them: no bound route's signature
changed, the three behaviour contracts are the three that changed, and the ABI policy is the
policy.


## Addendum: 0.20.0 to 0.21.0, mid-session

CNA moved again while the extension work was in progress. The measurement is the same shape as
the one above, and the answer is much smaller.

```text
cnanext HEAD            17b5a90a -> 599d14e5 (branch next)
header ABI              0.20.0 -> 0.21.0
canonical functions     4051 -> 4054
C API inventory SHA-256 e9e0be89... -> be9a2bf8...
bound routes            1570, unchanged
signature drift         0 of 1570
return-type drift       0 of 1570
header drift            0 of 1570
removed upstream        none
```

Three declarations were added and nothing else moved:
`cna_environment_get_device_type`, and the pair
`cna_object_dictionary_ext_get_runtime_type_name_size` /
`cna_object_dictionary_ext_copy_runtime_type_name`. All three fell under coverage rules that
already existed, so `UNMAPPED_REQUIRES_REVIEW` stayed at zero without a rule being written to
accommodate them -- which is the check working rather than being satisfied.

What the migration cost was therefore three edits: the manifest's `compiledAbi`, its recorded
inventory hash, and the ABI identity `probe.c` pins. The exact-minor policy is why those are
edits rather than nothing: a 0.x minor may be incompatible, so the gate refuses the new library
until a human has looked, and looking is what this addendum records.

The library was rebuilt from the live checkout before any of it was claimed. `cnanext`'s
worktree was clean at `599d14e5`, so the artifact is a committed tree rather than someone's
work in progress; the C API target was rebuilt incrementally in the existing
`cmake-build-javanext` directory, and the result reports ABI 0.21.0 with 4,054 exported
`cna_*` symbols. Qualifying 0.21.0 headers against the 0.20.0 library still on disk would have
described a library nobody ships.

```text
HEADER_ABI=0.21.0   LIBRARY_ABI=0.21.0   LIBRARY_SYMBOL_CHECK=PASS (1570/1570)
TESTS=219 SUITES=50 FAILURES=0 ERRORS=0 SKIPPED=0
selected profile TOTAL_DIAGNOSTICS=0    full profile TOTAL_DIAGNOSTICS=0
UNMAPPED_REQUIRES_REVIEW=0   BOUND_BUT_UNREACHED=0
```
