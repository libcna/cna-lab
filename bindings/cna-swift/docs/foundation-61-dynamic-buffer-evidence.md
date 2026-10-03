# Foundation 61 — `DynamicVertexBuffer` and `DynamicIndexBuffer`

The buffer family closed.

```text
COMPLETE_TYPES   154 -> 156      MISSING_TYPES  97 -> 95
TOTAL_DIAGNOSTICS 160 -> 158     BOUND_FUNCTIONS 100 -> 106
CALLBACKS          4 -> 6        ABI_MISMATCHES=0
tests            641 -> 647
```

## What `SetDataOptions` actually does

The two extra `SetData` overloads each end in the same `CopyData` the base
overloads reach, with the option carried in:

```text
SetData(data, startIndex, elementCount, options)
    -> SetData(0, data, startIndex, elementCount, 0, options)
    -> CopyData(offsetInBytes, data, startIndex, elementCount, vertexStride,
                ConvertXnaSetDataOptionsToDx(options), isSetting: true)
```

Inside `CopyData` the option has exactly **one** managed consequence:

```text
if ((options & (Discard | NoOverwrite)) == 0)
    ... if this buffer is currently bound to the device, throw ResourceInUse
```

Everything else it does is a D3D lock flag — a driver hint with no defined
observable effect. So the managed half is what must be reproduced, and it is;
the hint is forwarded where CNA takes it.

## Which route takes the hint, measured

`build-probe/f60_options.c`:

```text
static  vertex  set_data_raw_at                      -> 0
static  vertex  set_data_raw_at_with_options(0, 1, 2) -> 6  NOT_SUPPORTED
dynamic vertex  set_data_raw_at                      -> 0
dynamic vertex  set_data_raw_at_with_options(0, 1, 2) -> 0

static  index   set_data / set_data_at, None          -> 0
static  index   ... Discard, NoOverwrite              -> 6
dynamic index   set_data      with Discard/NoOverwrite -> 0
dynamic index   set_data_at   with Discard/NoOverwrite -> 6
```

Two consequences, and both are in the code rather than in a comment.

**Vertex.** `None` goes through the option-free upload, which both kinds
accept; a real option goes through the one only a dynamic buffer has. Sending
an option-taking upload to a static buffer would fail outright, for `None` as
much as for `Discard`, which is why the base's `SetData` cannot simply pass
`None` to it.

**Index.** A dynamic index buffer takes an option through the whole-buffer
route and refuses it through the windowed one. So the option rides the
whole-buffer route when the write covers the whole buffer, and is dropped
otherwise. Dropping it changes nothing observable: the flag is a lock hint, and
the one thing XNA lets a caller see about it is the bound-buffer test, which is
managed and reproduced.

## `IsContentLost` is false here, and that is measured twice

```text
if (!_contentLost) _contentLost = _parent.IsDeviceLost;
return _contentLost;
```

The latch from the device has no counterpart — `GraphicsDevice.IsDeviceLost` is
not projected — and the native event never fires either: CNA's own header says
it raises `ContentLost` "on the renderers whose API can actually lose a device
(DirectX9, Direct2D, Skia); families that cannot lose one never raise it", and
`CNA_VertexBufferInfo.is_content_lost` is documented as "currently always
false". HEADLESS is one of the families that cannot.

So what is asserted is the **subscription**, not the event:
`cna_vertex_buffer_subscribe_content_lost` and its index counterpart create a
registration, the tests assert it is non-zero while the buffer lives and zero
after disposal, and the flag stays false. That is the same shape
`RenderTarget2D.ContentLost` already carries, and it is the honest one — a test
that waited for the event would wait forever.

Disposal releases the subscription **before** the base releases the handle, on
both the disposing and the finalizer path, so no native callback can address a
released Swift box. `deinit` releases it too, as a backstop and not as the
mechanism.

## The shim mirrors CNA's typed handles

The first attempt gave both callbacks a plain `CNASwift_Handle` parameter and
the ABI gate reported six mismatches: CNA spells the buffer callbacks with
`CNA_VertexBufferHandle` and `CNA_IndexBufferHandle`, and their registrations
with `CNA_VertexBufferEventRegistrationHandle` and its index counterpart, where
the render-target callback uses the plain `CNA_Handle`.

Both halves are now right, and deliberately by different means:

* the shim declares `CNASwift_VertexBufferHandle` and `CNASwift_IndexBufferHandle`
  as aliases of `CNASwift_Handle` and uses them in the callbacks, so the
  callback wall stays a **textual** mirror of the canonical declaration and the
  probe's `__builtin_types_compatible_p` proves each alias is the same
  underlying type;
* the two event-registration typedefs join the alias table used by the separate
  type-compatibility comparison, which needs the underlying type and leaves the
  manifest holding CNA's own spelling.

`CALLBACKS` 4 → 6, `SCALAR_FACTS` unchanged, `ABI_MISMATCHES=0`,
`NATIVE_ABI_MUTATIONS=14 CAUGHT=14`.

## Constructors

XNA's dynamic constructors differ from the static ones only in the D3D usage
and pool they pass to `CreateBuffer`; the `vertexCount`/`indexCount` and
`vertexDeclaration` checks are the same, in the same order, with the same
messages. The projection shares the one creation path and passes `dynamic`
true, which is the same distinction `CNA_VertexBufferCreateInfo.dynamic` and
`CNA_IndexBufferCreateInfo.dynamic` carry.

The `Type`-taking constructors inherit Foundation 60's language-mapping limit
unchanged, and for the same reason.

## Falsifiability

Six mutations, each planted and each caught: the option-taking upload used for
`None` (which a static buffer refuses), a streaming option forwarded to the
windowed index upload (which CNA refuses), a `DynamicVertexBuffer` created with
the static flag, a disposal that leaves the native registration alive, a
dynamic index buffer that never registers, and an `IsContentLost` that answers
true.

One of the six **survived first**, and the survival was a real test gap rather
than an unfalsifiable mutation: `dynamic-vertex-buffer-created-static` changes
the *declaration-taking* constructor, and every option test went through the
`Type`-taking one. The two constructors carry `dynamic: true` separately, so a
projection that got one of them wrong would have shipped. The test now builds
through both, and the mutation is caught.

Total `PROJECTION_MUTATIONS=143`; the full run is repeated before the final
handoff.
