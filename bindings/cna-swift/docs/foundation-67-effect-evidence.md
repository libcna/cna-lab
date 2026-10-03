# Foundation 67 — the `Effect` core, and what the IL said about `Apply`

```text
COMPLETE_TYPES   161 -> 170     MISSING_TYPES  90 -> 81
TOTAL_DIAGNOSTICS 141 -> 132    TARGET_TYPES 167 -> 176
TARGET_MEMBERS  2069 -> 2165    MEMBERS_READ 1329 -> 1403
BOUND_FUNCTIONS  125 -> 191     LAYOUTS 42 -> 48     ABI_MISMATCHES=0
MESSAGES_REPRODUCED 189 -> 195  RESOURCE_STRINGS_REPRODUCED 62 -> 64
tests            704 -> 716     PROJECTION_MUTATIONS 189 -> 203
```

Nine mutually dependent types and ninety-six members: `Effect`,
`EffectParameter`, `EffectAnnotation`, `EffectPass`, `EffectTechnique` and their
four collections. They land together because they refer to each other —
`Effect.Parameters` returns an `EffectParameterCollection` whose elements return
`EffectAnnotationCollection`s — so any subset would leave the verifier reporting
every one of them partial.

## Three things `EffectPass.Apply` does that a reasonable design would not

The projection's first draft validated the pass's own handle, applied
unconditionally, and called `OnApply` from a whole-effect entry point. The IL
says otherwise, in eleven instructions:

```text
IL_001c: Helpers.CheckDisposed(effect, effect.pComPtr);
IL_0022: if (effect._currentTechnique != this._technique)
IL_002f:     throw new InvalidOperationException(NotCurrentTechnique);
IL_003b: effect.OnApply();
         ... the native apply
```

* **The disposal check is on the *effect*.** A pass whose effect has been
  disposed raises `ObjectDisposedException` naming **`Effect`**, not the pass.
* **A pass of a non-current technique is refused.** `NotCurrentTechnique` —
  *"Cannot Apply an EffectPass that is not from the CurrentTechnique."*
* **`OnApply` is called from here**, not from any effect-level entry point. The
  derived hook fires once per *pass* application.

All three needed a structural change: an `EffectPass` has to know its
`EffectTechnique` and a technique its `Effect`, both weakly, because the effect
owns the collection that owns the technique that owns the collection that owns
the pass. XNA has the same two back-references as fields.

**A test caught it, and it was a test that asserted something obvious.** "A pass
of a disposed effect refuses" failed with `accepted`, which is what sent the
reading to the IL. The other two came out of the same eleven instructions.

## The collections are unlike every other collection in this binding

**An out-of-range index is `nil`, not a throw.**

```text
get_Item(int32 index):
    if (index < 0 || index >= pPass.Count) { ldnull; ret }
    return pPass[index];
```

Pinned `IL_NO_FAILURE_PATH` and `PROVEN_NULLABLE_SUCCESS` — where
`SamplerStateCollection` and `TextureCollection` both raise
`ArgumentOutOfRangeException("index")`. So all four effect collections project
`Item` as a Swift **`subscript`** returning an Optional, and they are the first
infallible indexed accessors in the project: every existing one is a throwing
method because every existing one is fallible. The general accessor rule had
that branch in its self-test table and nothing had exercised it.

**The element objects are cached by index**, because CNA's `get_at` hands back a
*fresh owned view handle* on every call. Without the cache
`effect.Techniques !== effect.Techniques` and
`technique.Passes[0] !== technique.Passes[0]`; XNA holds a `List<T>` built once
and indexes into it, so identity holds there and the cache is what reproduces
it.

**The five `..._collection_find` routes are deliberately not bound.** CNA
publishes them, and using them would have been the obvious thing — but `find`
answers a *second* owned view of the element rather than the one `get_at` gave,
so `parameters["X"] === parameters[0]` would be false where XNA has it true.
XNA's own `Item[String]` is a managed scan over its `List<T>` comparing `Name`,
and that is what this does. A route with no consuming member is not bound.

## Fifty-one members over six routes

`EffectParameter` declares twenty `SetValue`/`SetValueTranspose`, twenty-three
`GetValue*` and eight properties, because XNA writes an overload per type. CNA
does not repeat that shape — it publishes one **tagged** pair:

```text
cna_effect_parameter_get_value (parameter, CNA_EffectValueType, void* out)
cna_effect_parameter_set_value (parameter, CNA_EffectValueType, const void*)
cna_effect_parameter_get_values(parameter, type, requested, dst, cap, &n)
cna_effect_parameter_set_values(parameter, type, values, count)
```

over nine value types, plus a texture pair over four overload identities. So the
fifty-one members are fifty-one *names* over six routes, each one line naming
its tag — and that is also where a defect would hide, because a member naming
the wrong tag would compile, run, and quietly read or write the wrong storage.
The tags are named constants rather than literals at fifty call sites.

`SetValueTranspose` and `GetValueMatrixTranspose` are **not** a second storage
location: they are `MATRIX_TRANSPOSE` against the same parameter. The `BASE`
texture identity is setter-only in CNA — *"no corresponding native getter
exists"* — and XNA agrees, its `GetValue` overloads being `Texture2D`,
`Texture3D` and `TextureCube` and never plain `Texture`.

## What this host can actually build

`cna_effect_create_empty` — *"the minimal concrete adapter for the native
abstract Effect base class"* — gives an effect with **no parameters, one
technique named `Default`, one pass named `P0`**, and a native type name that is
literally `Microsoft.Xna.Framework.Graphics.Effect`. `has_renderer` and
`is_compiled` are both false on HEADLESS and neither prevents `apply`.

So `EffectParameter`'s fifty-one members have no parameter to read on this
artifact. What is asserted is the structure, the identity rules, the collection
semantics above, the constructor's two managed tests, and the three things
`Apply` does.

The byte-array constructor's own tests split cleanly:

| test | where it lives |
| --- | --- |
| `effectCode == null \|\| Length == 0` → `ArgumentNullException("effectCode")` | reproduced |
| `Length % 4 != 0` → `ArgumentException(ArrayMultipleFour, "effectCode")` | reproduced |
| `MustUserShaderCode` | **native-owned** — a verdict about bytecode this binding does not parse |
| `ProfileVertexShaderModel`, `ProfilePixelShaderModel` | **native-owned** — the profile limit is pinned, the shader model inside the bytecode is not readable here |

## The draws are the next milestone, and the evidence is already in

`build-probe/f67_effect.c` measures the whole chain: with an effect applied and
a vertex buffer written, `cna_graphics_device_draw_primitives` returns **0**,
where the same call without an apply answers `CNA_RESULT_INTERNAL` with *"no
effect has been applied"*. Foundation 63's decision to withhold the draws is
vindicated rather than merely cautious.

The draw members are **not** projected here, so
`cna_graphics_device_draw_primitives` has no consuming member and is not bound —
which is why the draw half of that evidence lives in the probe and this document
rather than in a test. Foundation 63 read its own refusal the same way.

The probe also found a native rule XNA does not have, and found it only because
it was wrong first: **CNA counts vertices written, not capacity allocated.** A
64-vertex buffer with a renderer, bound, with an effect applied, refuses every
draw until `SetData` has been called. Enlarging the buffer changed nothing;
uploading data made the identical call succeed. The draw milestone has to decide
what to do about that explicitly.

## Falsifiability

Fourteen mutations, each planted and each caught — after two rounds.

`effect-collection-index-throws-instead-of-nil` was a **no-op**: relaxing the
bound from `<` to `<=` lets one-past-the-end through the guard, and `get_at`
then fails, so the answer is still nil. Replaced, not scored, with one that
returns element zero for an out-of-range index — which changes the answer.

`effect-string-ignores-the-written-length` survived while the test only asserted
that a name was **non-empty**: a name read back with its buffer padding attached
still matches itself, so the name lookup found it and nothing noticed. The test
now asserts the exact values the probe measured — `Default` and `P0` — which is
the difference between checking that a string reader ran and checking that it
read the right bytes.

One existing native-ABI mutation had to be re-aimed, and the reason is worth
recording: `wrong-parameter-width` matched `"char* destination", "uint64_t
capacity"`, and **nine** of the seventy new effect routes copy a name into a
caller buffer with exactly that parameter pair. The harness reported it as a
*survivor* rather than a stale site, which is the failure the pre-check exists
to make loud — a mutation that matches nine places mutates none of them. It now
names its route as well as its parameters.

And the status gate itself had to be fixed to survive that repair. Its
`len(MUTATIONS)` reader counted brackets from the assignment until the depth
returned to zero — which is wrong the moment a mutation's own text contains one.
The re-aimed site is `cParameters: ["CNA_Handle texture", ...`, a legal Python
string with an unbalanced `[`, and the scanner ran off the end of the file and
crashed the entire gate with an `IndexError`. It parses the module with `ast`
now, which a string's contents cannot fool.

`PROJECTION_MUTATIONS=203`; the full run is repeated before the final handoff.
