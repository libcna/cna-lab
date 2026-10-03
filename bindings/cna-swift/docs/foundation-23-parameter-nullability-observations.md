# Reference *parameter* nullability — what the return analysis incidentally showed

Foundation 23 decided return positions. The repository's selective
`optionalReferenceParameters` policy was deliberately **not** reopened: return
nullability being measured is not a reason to make every reference parameter
Optional. But the same IL evidence answers the parameter question too, and the
milestone was asked to record any clear inconsistency it turned up rather than
act on it. This is that record.

## The existing rule

> Reference parameters become Optional only where null is an observable
> selected operation rather than merely an immediate invalid argument.

Nine parameters are selected today, all on `CurveKey` and `CurveKeyCollection`.

## What was measured

Every public reference-typed parameter of every member on an implemented type
was checked against its own IL: is the parameter null-tested with `brfalse` /
`brtrue`, and where does the null branch go — to a normal `ret`, or to a
`throw`?

- **Null branch reaches a `throw`.** Immediate invalid argument; the parameter
  stays non-Optional under the current rule. Every `sourceArray` /
  `destinationArray` on `Vector2`/`Vector3`/`Vector4`, `TouchCollection..ctor`,
  `TouchCollection.CopyTo`, `SpriteBatch.DrawString`, `Texture2D..ctor` and
  `GraphicsDeviceManager..ctor` are in this class. No change.
- **Null branch reaches a normal `ret`.** Null is an observable operation. Two
  sub-cases follow.

## No inconsistency on any implemented member

### `Equals(object obj)` — seven value types

`VertexElement`, `Ray`, `GamePadButtons`, `GamePadDPad`, `GamePadState`,
`GamePadThumbSticks` and `GamePadTriggers` all null-test `obj` and return
`false`. The parameter is `System.Object`, which the type mapping already
projects as `Any?`, so these are Optional today and were never at risk. Covered
by a rule, not by an omission.

### `KeyboardState..ctor(Keys[] keys)` — checked, and correctly non-Optional

The constructor zeroes all eight state words first, then

```text
IL_004e:  ldarg.1
IL_004f:  brfalse.s  IL_0068          // null: skip the loop entirely
IL_0055:  ...        AddPressedKey    // otherwise fold every element in
IL_0068:  ret
```

A null array and an empty array therefore produce **bit-identical** state: all
keys up. `KeyboardState([])` in Swift is indistinguishable from
`new KeyboardState(null)` in XNA, so the non-Optional `[Keys]` projection loses
nothing. This one looked like a candidate and is not.

## One real gap, on members that are still absent

`GraphicsDevice` has four members where passing null is a *documented XNA
idiom* and the IL confirms it is a normal selected operation:

```text
SetRenderTarget(RenderTarget2D renderTarget)
  IL_0000:  ldarg.1
  IL_0001:  brfalse.s  IL_0016
  IL_0003:  ...        new RenderTargetBinding(renderTarget); SetRenderTargets(&binding, 1)
  IL_0016:  ldarg.0
  IL_0017:  ldc.i4.0
  IL_0018:  ldc.i4.0
  IL_0019:  call       GraphicsDevice::SetRenderTargets(RenderTargetBinding*, int32)
  IL_001e:  ret
```

Null binds *zero* render targets — that is how a caller restores the back
buffer. `SetVertexBuffer` has the identical shape against `SetVertexBuffers`,
and `SetRenderTargets` / `SetVertexBuffers` accept a null array with the same
meaning.

| Member | Parameter | Null means |
|---|---|---|
| `GraphicsDevice.SetRenderTarget` | `renderTarget` | unbind, restore the back buffer |
| `GraphicsDevice.SetRenderTarget` | `renderTarget` (cube overload) | unbind, restore the back buffer |
| `GraphicsDevice.SetRenderTargets` | `renderTargets` | unbind all |
| `GraphicsDevice.SetVertexBuffer` | `vertexBuffer` | unbind the vertex buffer |
| `GraphicsDevice.SetVertexBuffers` | `vertexBuffers` | unbind all |

All five are `MISSING_MEMBER` on the `GraphicsDevice` runtime partial, so there
is nothing to correct today and no diagnostic moves. The record exists so that
whoever implements them projects the parameter as Optional under the *existing*
rule — null there is an observable selected operation, exactly like
`CurveKeyCollection.Remove(nil)` — rather than discovering it afterwards.

Nothing in the selected nine was found inconsistent, and no parameter rule was
changed.


## Addendum, Foundation 54: four more selected parameters

`SpriteBatch.Begin`'s `blendState`, `samplerState`, `depthStencilState` and
`rasterizerState` join the registry, taking `optionalReferenceParameters` from
26 entries to 30. They pass the same test the four `GraphicsDevice` members
passed: `SetRenderState` `brfalse`-tests each one and the null branch reaches a
normal `ldsfld` of a preset, not a `throw`.

```text
IL_0001:  ldfld      SpriteBatch::blendState
IL_0006:  brfalse.s  IL_001b
IL_0008:  …          _parent.BlendState = this.blendState
IL_0019:  br.s       IL_002b
IL_001b:  …          _parent.BlendState = BlendState.AlphaBlend
```

Null selects a different, well-defined state, and a caller can see which one
the device ends up with — which is exactly the line this rule draws.
