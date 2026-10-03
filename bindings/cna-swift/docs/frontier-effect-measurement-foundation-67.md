# The `Effect` frontier, measured

> **Not a milestone.** This file is named `frontier-…` and not
> `foundation-67-…-evidence.md` deliberately: `tools/status_gate/verify.py`
> derives the current Foundation from the highest evidence file on disk, and
> Foundation 67 is **not complete**. This is what was measured before starting
> it, so the next session begins from evidence rather than from scratch — the
> same role `docs/frontier-remeasurement-foundation-60.md` played.

Foundation 63 withheld the whole draw family with a specific claim:

> Every CNA draw route answers `CNA_ERROR_CATEGORY_INTERNAL` with *"no effect
> has been applied"* while no effect is applied, and XNA's `VerifyCanDraw`
> raises `CannotDrawNoShader` for the same reason. Both agree, so the draw
> members wait for `Effect`.

That claim has now been tested rather than assumed, and it holds in both
directions.

## An effect exists here, and applying one unblocks the draws

`build-probe/f67_effect.c`, on the qualified HEADLESS artifact:

```text
effect_create_empty            -> 0
effect type name bytes         -> 0 39
   type name = Microsoft.Xna.Framework.Graphics.Effect
effect_has_renderer            -> 0 has=0
effect_get_is_compiled_ext     -> 0 compiled=0
effect_get_parameters          -> 0     parameter count -> 0
effect_get_techniques          -> 0     technique count -> 1
effect_get_current_technique   -> 0     (a valid handle)
effect_apply                   -> 0
   pass count -> 1
   pass_apply -> 0
```

`cna_effect_create_empty` builds what the header calls *"the minimal concrete
adapter for the native abstract Effect base class"* — an effect with **no
parameters**, **one technique** and **one pass**, whose native type name is
literally `Microsoft.Xna.Framework.Graphics.Effect`. `has_renderer` is false on
HEADLESS and `is_compiled` is false, and neither prevents `apply` from
succeeding.

With an effect applied, a real draw succeeds:

```text
vertex_declaration_create      -> 0
vertex_buffer_create           -> 0      count=64 stride=16 renderer=1
set_vertex_buffer              -> 0      bound vertex buffers = 1
vertex_buffer_set_data         -> 0
draw_primitives(count=1)       -> 0
draw_primitives(count=2)       -> 0
pass_apply                     -> 0
draw after pass_apply          -> 0
```

So the draw family is **not** blocked, and Foundation 63's decision to wait was
right rather than merely cautious: the refusal it recorded is removed by exactly
the thing it said would remove it.

## A native rule XNA does not have

Before the data upload, every draw was refused:

```text
draw_primitives(count=1)  -> 1
      The requested primitive range exceeds the bound vertex buffer.
      (Parameter 'primitiveCount') Actual value was 1.
```

with a 64-vertex buffer bound, `cna_primitive_type_get_vertex_count(TriangleList, 1)`
answering 3, and `has_renderer` true. **CNA counts vertices that have been
written, not capacity allocated.** Uploading data made the identical call
succeed.

XNA has no such rule — a `VertexBuffer` whose `SetData` was never called draws
undefined contents, it does not raise. This is a divergence to record and to
decide about when the draws land: the projection either forwards CNA's refusal
on the runtime channel, or refuses earlier with something of its own. It must
not be discovered by a test that happened to upload data first.

**The probe found this only because it was wrong the first time.** Its first
draft allocated three vertices, drew one triangle, and read the refusal as a
capacity problem; enlarging the buffer to 64 changed nothing, which is what
showed the rule was about writes rather than size.

## What Foundation 67 has to build

Nine CLR types, **96 members**, and they are mutually dependent — `Effect`
returns an `EffectParameterCollection`, whose elements return
`EffectAnnotationCollection`s — so they land together or the verifier reports
every one of them partial.

| type | members | shape |
| --- | ---: | --- |
| `Effect` | 8 | `open class`, `IGraphicsResource`; two constructors, `Clone`, `OnApply`, `Dispose(Bool)`, three collection properties |
| `EffectParameter` | 51 | `sealed`; 20 `SetValue`/`SetValueTranspose` overloads, 23 `GetValue*`, 8 properties |
| `EffectAnnotation` | 14 | `sealed`; 8 `GetValue*`, 6 properties |
| `EffectParameterCollection` | 5 | `sealed`, `IEnumerable<T>`; `Count`, `Item[Int32]`, `Item[String]`, `GetParameterBySemantic`, `GetEnumerator` |
| `EffectPass` | 3 | `sealed`; `Apply`, `Annotations`, `Name` |
| `EffectTechnique` | 3 | `sealed`; `Annotations`, `Passes`, `Name` |
| `EffectPassCollection`, `EffectTechniqueCollection`, `EffectAnnotationCollection` | 4 each | `sealed`, `IEnumerable<T>`; the same four-member shape |

The four collections are identical in shape and should share one internal
generic implementation, the way `SamplerStateCollection` and `TextureCollection`
share theirs — the second was written from the first and the difference is a
stage and an offset.

### The routes are all there

138 `cna_effect_*` routes. The value model is one **tagged** pair rather than an
overload per type, which is what makes 51 members tractable:

```text
cna_effect_parameter_get_value (parameter, CNA_EffectValueType, void* out)
cna_effect_parameter_set_value (parameter, CNA_EffectValueType, const void*)
cna_effect_parameter_get_values(parameter, type, requested, dst, capacity, &count)
cna_effect_parameter_set_values(parameter, type, values, count)
```

with nine value types — `BOOLEAN INT32 SINGLE MATRIX MATRIX_TRANSPOSE
QUATERNION VECTOR2 VECTOR3 VECTOR4` — and a separate texture pair over four
overload identities (`BASE 2D 3D CUBE`). The base texture identity is
**setter-only**: the header says *"no corresponding native getter exists"*,
which matches XNA, whose `GetValue` overloads are `Texture2D`, `Texture3D` and
`TextureCube` and never plain `Texture`.

`GetValueMatrixTranspose` is not a second storage location: it is
`MATRIX_TRANSPOSE` against the same parameter, exactly as `SetValueTranspose`
is.

### What is not in Foundation 67

The stock effects — `BasicEffect`, `AlphaTestEffect`, `DualTextureEffect`,
`EnvironmentMapEffect`, `SkinnedEffect`, `EffectMaterial` and `IEffectLights` —
are a separate milestone, as the phase order has it. They derive from `Effect`
and cannot start before it exists.

## Ordering, for whoever picks this up

1. The four collections over one shared internal generic.
2. `EffectAnnotation`, then `EffectPass` and `EffectTechnique` (three members
   each, and they only read).
3. `EffectParameter` — the bulk, but mechanical once the tagged-value helper is
   written once.
4. `Effect` itself, which ties them together.
5. The draws, and the vertex-write rule above decided explicitly rather than
   discovered.
