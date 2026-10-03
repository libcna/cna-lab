# Foundation 69 — the five stock effects, and a gate that could not see an explicit implementation

```text
COMPLETE_TYPES   170 -> 178     MISSING_TYPES  81 -> 73
TOTAL_DIAGNOSTICS 123 -> 115    TARGET_TYPES 176 -> 184
TARGET_MEMBERS  2165 -> 2296    MEMBERS_READ 1403 -> 1605
BOUND_FUNCTIONS  197 -> 303     LAYOUTS 50   CONSTANTS 225   ABI_MISMATCHES=0
MESSAGES_REPRODUCED 223 -> 229  RESOURCE_STRINGS_REPRODUCED 69 -> 72
ACCESSOR_SELF_TESTS 34 -> 41    THROWING_SETTERS 113 -> 114
tests            730 -> 756     PROJECTION_MUTATIONS 215 -> 241
```

Eight types and a hundred and twenty-nine members: `BasicEffect`,
`AlphaTestEffect`, `DualTextureEffect`, `EnvironmentMapEffect`,
`SkinnedEffect`, `EffectMaterial`, `DirectionalLight` and `IEffectLights`.

## The gate was wrong before the code was

`IEffectLights.LightingEnabled` is implemented three ways. `BasicEffect`
implements it implicitly with a field-backed setter that cannot fail.
`EnvironmentMapEffect` and `SkinnedEffect` implement it **explicitly** and
refuse:

```text
EnvironmentMapEffect::IEffectLights.get_LightingEnabled:  ldc.i4.1; ret
EnvironmentMapEffect::IEffectLights.set_LightingEnabled:
    ldarg.1; brtrue.s IL_0032
    ... newobj NotSupportedException(
            Format(CantDisableLighting, GetType().Name)); throw
    IL_0032: ret
```

`accessor_fallibility.py` reported that interface setter **infallible**, and
the reason is one line:

```python
candidate = (subtype, name, arity)      # ("…SkinnedEffect", "set_LightingEnabled", 1)
```

An explicit implementation's IL name is the interface member's
*fully-qualified* name — `Microsoft.Xna.Framework.Graphics.IEffectLights.set_LightingEnabled`
— so a search by member name finds only `BasicEffect`, and an accessor decided
from its implementors was decided from one third of them. The projection that
followed would have been `LightingEnabled { get set }` on the protocol, and
neither always-lit effect could have reported the refusal XNA reports.

The repair is to index what the CLR actually records. An explicit
implementation names its interface method in an `.override` directive, and
that directive is now read into `CallGraph.explicit` and consulted by both
`implementors` and `_overrides` — the second of which had the same blindness,
so a `callvirt` on an interface member could not reach an explicit
implementation either. One accessor changed verdict across all eight hundred
and forty; `THROWING_SETTERS` goes 113 → 114 and nothing else moved, which is
what a surgical fix looks like.

**Seven self-tests hold it**, including two controls: the *getter* of the same
property must stay infallible (both explicit bodies are `ldc.i4.1; ret`), and
a `callvirt` on the interface member must reach `SkinnedEffect`'s explicit
body. Reverting `implementors` to its old lookup fails two of them.

CNA reached the same verdict independently and refuses the same call in its
own words — *"This stock effect's lighting cannot be disabled."* — which is
corroboration, not authority. The message a caller sees is
`CantDisableLighting`.

## Two Swift members for one CLR accessor, and a rule extended to say so

The interface's setter is fallible; `BasicEffect`'s is not. Swift projects the
requirement as `{ get }` plus a throwing writer, and `BasicEffect`'s own
property as `{ get set }` — and a property cannot witness a method
requirement. So `BasicEffect` spells `SetLightingEnabled` a second time.

That is not a new XNA member: it is the same CLR `set_LightingEnabled`
accessor the property's `set` already projects. It is also not the shape the
protocol-witness rule was written for, which was XNA's *explicit*
implementations — members absent from the owner's public surface. The rule now
admits a second shape with its own evidence, and refuses it when it is not
forced: a configured witness whose owner setter is *also* fallible needs no
second writer and is reported rather than recorded. The self-test plants
exactly that.

The parser needed one repair alongside it. A protocol's own `Set<Name>`
requirement is recombined into a single property identity by the accessor
machinery, so it is no longer a member to look up — and every witness for it
was reported as *"sourceOrigin does not resolve to a compiler-emitted
requirement"*. The witness is matched against the recombined property, and its
shape is not re-compared, because the accessor rule has already decided it.

## What is cached and what goes straight through

XNA's stock effects are built the same way twice over, and the accessor
inventory reads the difference out of the IL:

| | reads | writes | verdict |
|---|---|---|---|
| `World`, `Alpha`, `FogStart`, `AlphaFunction`, … | `ldfld` | `stfld` + a dirty bit | infallible |
| `SpecularColor`, `FogColor`, `Texture`, `EnvironmentMap`, … | `param.GetValueX()` | `param.SetValue(…)` | fallible |

The first kind never touches the shader at set time; `OnApply` pushes the
dirty ones. **That is also the only way this projection can work at all**: a
Swift setter cannot throw and every CNA route can fail, so an infallible
property's write has to be deferred to a member that is allowed to throw. XNA
already defers it. The architecture is not a workaround for Swift; Swift
merely makes it compulsory.

## `DirectionalLight` is entirely managed, and had to be

```text
.field private EffectParameter directionParam, diffuseColorParam, specularColorParam
.field private bool enabled
.field private Vector3 cachedDirection, cachedDiffuseColor, cachedSpecularColor
```

Every getter is `ldfld; ret`. Every setter writes a parameter and is fallible.
Every parameter reference is `brfalse.s`-guarded, so a light built with three
nulls is a working value holder — which is why all four constructor parameters
are recorded `optionalReferenceParameters` under the Foundation-23 rule: the
null branch of each selects a different meaningful operation rather than
refusing the argument. `cloneSource` most of all, because null is the ordinary
case and it selects the whole constructor body.

Three behaviours are reproduced exactly and are all directly testable:

* **`set_Enabled` is a no-op when the value has not changed** — `if (enabled
  == value) return;` before anything else.
* **Disabling writes `Vector3.Zero` to the two colour parameters and leaves
  the two caches alone.** `DiffuseColor` still answers what it answered
  before; it is the *shader* that stops seeing it. Re-enabling pushes the
  remembered values back.
* **`Direction` is the one setter that ignores `enabled`.**

`AlphaTestEffect` is what makes the parameter half assertable:
`build-probe/f69_lights.c` measures that a native `BasicEffect` publishes
**no parameters at all** where `AlphaTestEffect` publishes six and
`SkinnedEffect` twelve. So a standalone light is built over a real
`EffectParameter` and every push is observed through `GetValueVector3`.

A stock effect's own lights are the native ones instead, because there are no
parameters to build them from — and they are **fetched once**, because
`cna_effect_lights_get_directional_light` hands back a fresh owned view on
every call:

```text
get_directional_light(0) twice -> 0/0  handles 8589934596 4294967301  same=0
```

## `EnableDefaultLighting` is reproduced, not forwarded

CNA publishes `cna_effect_lights_enable_default`, and `build-probe/f69_default.c`
measures that it produces `EffectHelpers.EnableDefaultLighting`'s ten vectors
**component for component** — three directions, three diffuse colours, three
speculars and the ambient `(0.05333332, 0.09882354, 0.1819608)`.

It is still not called. The values a caller sees have to come from XNA's IL,
not from a native library that happens to agree today; forwarding would make
CNA the authority for XNA's default-lighting table. The route has no consuming
member and is therefore **not bound** — with
`cna_skinned_effect_{get,set}_vertex_color_enabled`, which the header itself
calls "the CNA extension that enables per-vertex color on SkinnedEffect" and
which XNA's `SkinnedEffect` has no property for. `BOUND_FUNCTIONS` is 303, not
306.

The agreement is recorded as native evidence and nothing depends on it.

## Two adjacent tests on one argument, reported two different ways

```text
SetBoneTransforms:
  null or empty  -> ArgumentNullException("boneTransforms", NullNotAllowed)
  Length > 72    -> ArgumentException(Format(SkinnedEffectMaxBones, 72))   // no ParamName
GetBoneTransforms:
  count <= 0     -> ArgumentOutOfRangeException("count")                    // no message
  count > 72     -> ArgumentOutOfRangeException("count", Format(…, 72))
```

The oversized-array refusal really is the one-argument
`ArgumentException(string)`, and the test proves it by asserting the **absence**
of .NET's `\r\nParameter name:` suffix — which the other three carry. Three of
those four assertions were wrong in the first draft, in the direction of
assuming the messages were plainer than the BCL makes them; the projection was
right and the tests were corrected.

`GetBoneTransforms` also **restores `M44` to 1** on every matrix it returns.
XNA stores a bone as three rows, so the fourth column comes back as whatever
the packed form left there. A reader that skipped that loop would hand back
matrices right in twelve components and wrong in the thirteenth, which is why
the test writes `M44 = 0` and asserts it reads back as 1.

## A native rule XNA does not have

```text
Texture2D.Dispose -> CNA_RESULT_INVALID_OPERATION
    "The Texture2D is retained by an active SpriteBatch, SpriteFont, effect,
     model or render-target scope."
```

CNA **retains** a texture assigned to an effect and refuses to dispose it
while the effect still points at it. XNA has no such rule — its
`EffectParameter` stores a raw pointer and the lifetime is the caller's
problem. It was found by a test that disposed a texture still on
`DualTextureEffect`'s second layer, and it is now asserted rather than avoided:
the texture cleared from layer zero disposes, the one still on layer one does
not, and clearing it makes it disposable. A consumer meets this, so it is
written down.

## Falsifiability

Twenty-seven mutations, twenty-four caught on the first round. The harness grew
an `--only` filter to run them: the full set is 241 and one run of it is over
an hour, which is how a mutation ends up merged unrun. The filter narrows only
which mutations are **planted** — every declared site is still checked for
staleness, so a selective run cannot hide a mutation whose site has drifted
away.

The three survivors were the interesting part, and they split three ways.

**`clone-runs-the-setters` survived because the test used nil parameters.** A
clone constructor that assigns through the setters instead of copying the
fields ends at the same four values, so a test that only compares the values
cannot see the difference — what differs is that the setters *write shader
parameters*, which XNA's field-to-field copy deliberately does not. The test
now builds the clone over a real `EffectParameter` and asserts the parameter
is untouched. That is the difference between testing what a constructor
produced and testing what it did.

**`setter-does-not-mark-dirty` survived because nothing observed a push at
all.** Every field-backed property answers its own cache, so a setter that
forgets its dirty bit looks correct from the public surface for ever and never
reaches the shader — and the entire deferred-write architecture was a design
note rather than an assertion. There is now one test that goes behind the
projection and reads the native values back through the routes `OnApply`
writes. It is the only test here that does, and it is the one that makes the
architecture checkable.

**`bone-reader-skips-the-fourth-diagonal` was withdrawn, not scored.**
`build-probe/f69_bones.c` writes a bone with `m44 = 0` and a fourth column of
`(7, 8, 9)`, and CNA answers `m44 = 1` with the column intact: it packs the
bone exactly as XNA does and restores the same diagonal on the way out. The
managed `matrix.M44 = 1` is therefore invisible on this artifact and no test
can be written that its deletion would fail. The line stays — it is what the
IL does, and a CNA that stopped normalising would need it — and the reason it
cannot be scored is written where the line stands.

`PROJECTION_MUTATIONS=241`; the full run is repeated before the final handoff.
