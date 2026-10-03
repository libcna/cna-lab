# Foundation 59 — `SaveAsPng`, `SaveAsJpeg`, the resizing `FromStream`, and three disposal links

Five members and one overload closed; three defects in already-implemented
code found by reading the IL and fixed; one mapping rule measured rather than
assumed; two upstream CNA divergences characterised.

```text
MISSING_MEMBER              63 -> 58
OVERLOAD_MAPPING_MISMATCH    6 -> 5
TOTAL_DIAGNOSTICS          169 -> 165
BOUND_FUNCTIONS             87 -> 90
tests                      609 -> 622
```

## `System.IO.Stream` is direction-mapped, and the direction is read out of the IL

`mapping-rules.json` mapped `System.IO.Stream` to `Foundation.InputStream`
globally, which is right for `Texture2D.FromStream` and makes `SaveAsPng`
**unimplementable**: nothing can be written to an `InputStream`. The CLR has
one `Stream` that reads and writes; Foundation has two classes and no common
one that does either.

The direction is not a judgement call — it is in the IL, one position at a
time. `Texture2D::SaveAsImage`, which both save members forward to, calls
`System.IO.Stream::get_CanWrite` and then
`System.IO.Stream::Write(uint8[], int32, int32)` on its argument, and never
reads it. `FromStream`'s argument goes to `get_CanSeek` and to the decoder.

`streamDirectionParameters` in `mapping-rules.json` overrides exactly the
positions the CIL proves are written, each carrying its evidence. The verifier
**refuses** an override that names a position whose CLR type is not
`System.IO.Stream`, and its self-tests check that the default stays
`InputStream`, that an overridden position becomes `OutputStream`, that every
entry names a position the contract declares, and that every entry carries
evidence. All four were planted and all four failed the gate.

The two positions overridden are `SaveAsPng(stream)` and `SaveAsJpeg(stream)`.
Every other `System.IO.Stream` in the contract is a read position or a return,
and none is touched.

## `SaveAsImage`, from the IL

```text
if (stream == null)   throw new ArgumentNullException("stream", NullNotAllowed);
if (!stream.CanWrite) throw new ArgumentException("stream");
if (format != Jpeg && format != Png) throw new ArgumentException("format");
Color[] colors = <the surface, converted to Color>;
for (int i = 0; i < colors.Length; i++)
    if (colors[i].A == 0) colors[i] = Color.Transparent;
using (ImageStream image = ImageStream.FromColors(colors, _width, _height,
                                                  format, width, height))
    stream.Write(new BinaryReader(image).ReadBytes((int)image.Length), 0, ...);
```

`SaveAsPng` pushes `ldc.i4.2` and `SaveAsJpeg` `ldc.i4.0` for `format`;
otherwise the two are the same eleven bytes.

Four things in that body decided the projection.

**Both `ArgumentException`s carry a parameter name as their *message*.**
`newobj ArgumentException::.ctor(string)` is the one-argument overload, so
`Message` is literally `"stream"` and `"format"` and `ParamName` is null. That
is XNA's own slip; it is reproduced rather than corrected, and asserted with
class, message, `ParamName` and `HResult` together.

**`CanWrite` has no Foundation counterpart.** An `OutputStream` is a writing
stream by construction; what it can still be is closed or failed, which is what
`CanWrite == false` describes, so `.closed` and `.error` raise and nothing else
does. The null check is unreachable — a Swift `OutputStream` parameter cannot
be nil — and the `format` check is unreachable too, because the only two
callers each push a constant.

**Alpha-zero texels are rewritten to `Color.Transparent`, and CNA does not do
it.** `build-probe/f59_encode.c` authors `(200,100,50,0)`, encodes it through
`cna_texture2d_copy_encoded`, and an independent decode reads
`(200,100,50,0)` straight back where XNA writes `(0,0,0,0)`. The rewrite is
therefore performed in the projection, on a copy — which is also XNA's own
structure, since `ImageStream.FromColors` encodes the rewritten array and never
the texture.

**The intermediate is a CPU-only texture.** `cna_texture2d_create_cpu_only_rgba8`
takes no device and no callback scope, which matches a `SaveAsPng` XNA lets a
caller make at any time. It is created, encoded and destroyed inside the call
and is never reachable — the direct counterpart of the `ImageStream` XNA
disposes in its `finally`. Its header comment offers it for "headless/test
integration"; it is a published canonical route with defined semantics, and it
is the only route that turns a colour array into something encodable, which is
the whole of what this needs.

### The encoder, measured

`build-probe/f59_encode.c`, against the qualified HEADLESS library:

```text
cna_texture2d_create_cpu_only_rgba8            -> 0        (no device needed)
get_encoded_byte_count(png,  2x2)              -> 0 bytes=79
copy_encoded(png,  2x2)                        -> 0 written=79
get_encoded_byte_count(jpeg, 2x2)              -> 0 bytes=829
get_encoded_byte_count(png,  4x4)              -> 0 bytes=131   (resize works)
get_encoded_byte_count(png,  0x0)              -> 1  INVALID_ARGUMENT
get_encoded_byte_count(fmt=7, 2x2)             -> 1  INVALID_ARGUMENT
copy_encoded(png, capacity=4)                  -> 14 need=79, destination untouched
```

An independent decode of the 2×2 PNG returns the authored texels exactly, and
of the 4×4 the bilinear resample of them.

## The output is read by a decoder that has never heard of CNA

`Tests/CNATests/PortableNetworkGraphicsDecoder.swift`: the PNG container,
DEFLATE (stored, fixed and dynamic Huffman) and the five scanline filters, from
the format specification. Handing `SaveAsPng`'s output back to
`Texture2D.FromStream` would have been a round trip through the same encoder
and decoder, and a transposed dimension, a swapped channel or a dropped alpha
rewrite survives that perfectly.

The decoder is itself checked before anything depends on it, against a PNG this
test builds byte by byte — a stored DEFLATE block, one `Sub`-filtered row and
one `Up`-filtered row, carrying texels the test knows.

## `FromStream(GraphicsDevice, Stream, Int32, Int32, Boolean)`

Twenty-three bytes of IL:

```text
XnaImageOperation op = zoom ? (Scale | Crop) : Scale;   // 3 : 1
return new Texture2D(graphicsDevice, stream, width, height, op);
```

`XnaImageOperation` is `Nothing = 0, Scale = 1, Crop = 2` in the registered
`Microsoft.Xna.Framework.dll`, so `zoom` is exactly *fit inside width×height*
against *cover width×height and crop*. The two-argument overload passes
`Nothing` with the profile's `MaxTextureSize` for both dimensions, which is why
it resizes nothing.

`CNA_Texture2DDecodeInfo` carries `width`, `height` and a `zoom` boolean
documented as *"true to cover-and-crop; false to fit while preserving aspect
ratio"* — the same two operations under the same two names.

**What is authoritative and what is not.** XNA's own resize happens inside
`UnsafeNativeMethods.DecodeStreamToTexture`, unmanaged code that is not in the
registered assembly, so **there is no pinned authority for the arithmetic**.
What the IL does decide is reproduced: which operation the flag selects, and
that the decoder's granted dimensions become the texture's `Width` and
`Height`. The granted numbers the tests assert are CNA's, recorded as native
evidence, and the evidence says so.

### CNA's decode-resize, characterised

`build-probe/f59_decode.c`, sixteen targets under both settings, from a 4×2
source:

```text
zoom=0   4x4 -> 4x2    8x2 -> 8x4    2x8 -> 2x1    6x3 -> 6x3
         3x6 -> 3x1    5x5 -> 5x2    4x8 -> 4x2    3x3 -> 3x1
         2x2 -> 2x1   12x6 ->12x6   16x8 ->16x8    1x1 -> INVALID_ARGUMENT
zoom=1   4x4 -> 4x4    6x3 -> 6x3    3x6 -> 3x6    5x5 -> 5x5
         4x8 -> 4x8    1x2 -> 1x2    1x1 -> 1x1   16x8 ->16x8
         8x2 -> INVALID_ARGUMENT     2x8 -> INVALID_ARGUMENT
```

Two upstream findings, stated as measurements rather than as bugs this binding
can fix:

* **`zoom = false` ignores the requested height.** The output is the requested
  width with the height derived from the source's aspect ratio, so a target of
  8×2 on a 2:1 source grants 8×4 — taller than what was asked for. XNA's
  `Scale` fits inside *both* dimensions. A correction is not available here
  without inventing a resampler, and inventing one would replace a measured
  native operation with an unmeasured managed one.
* **`zoom = true` refuses some targets.** 8×2 and 2×8 answer
  `CNA_RESULT_INVALID_ARGUMENT` where 4×4, 5×5, 3×6 and 16×8 succeed. The rule
  behind the refusal was not determined; the refusal reaches the caller on the
  runtime channel.

## Three defects in already-implemented code

None was found by a failing test. All three were found by reading the IL of a
member that was already green.

### 1. `Dispose(false)` announced a disposal to everyone

`GraphicsResource.Dispose(bool)` is two paths, not one:

```text
if (disposing) { ~GraphicsResource(); }        // flag, then Disposing
else { try { !GraphicsResource(); }            // flag only
       finally { Object.Finalize(); } }
```

The projection raised `Disposing` on both. `Dispose(false)` is the finalizer
path — a finalizer must not reach other managed objects — and Swift's lack of
`protected` puts it within any consumer's reach, so the event was observable
where XNA raises it to nobody. One `guard disposing else { return }`, and both
paths are asserted in both directions.

### 2. A disposed texture failed on the wrong channel

`Texture2D::CopyData`, which every public `SetData` and `GetData` forwards to
unchanged, opens with `Helpers.CheckDisposed(this, pComPtr)` →
`ObjectDisposedException(GetType().Name)`. The projection read its handle
through the *storage*, which raises `CNAError.disposedObject` on the runtime
channel. `plan.md` rule 8 is explicit that a consumer using a resource it
disposed is a CLR failure; `GraphicsResource.validatedHandle` is the accessor
that says so, and it was not the one being used.

### 3. …and it failed in the wrong order

`CheckDisposed` is `CopyData`'s **first** instruction, ahead of
`GetAndValidateSizes`, `GetAndValidateRect` and `ValidateTotalSize`. The
projection ran all three validations first and read the handle last, so a
disposed texture handed a wrong-sized array reported the array. The test
asserts the order, not just the class: a disposed texture and a deliberately
wrong-sized array together must still report the disposal.

## Members closed

| Member | Route |
|---|---|
| `Texture2D.SaveAsPng(OutputStream, Int32, Int32)` | `cna_texture2d_create_cpu_only_rgba8`, `_get_encoded_byte_count`, `_copy_encoded` |
| `Texture2D.SaveAsJpeg(OutputStream, Int32, Int32)` | the same three, JPEG constant |
| `Texture2D.FromStream(_, _, width:height:zoom:)` | `cna_texture2d_create_from_encoded_memory` with a decode block |
| `Texture2D.Dispose(Boolean)` | managed |
| `SpriteBatch.Dispose(Boolean)` | managed |

`cna_texture2d_save_file` exists and is **not** bound: no XNA member takes a
path, and a route with no consuming member is not a route this binding has.

Both `Dispose(Boolean)` overrides forward to the base and say why in their own
documentation. XNA's `Texture2D` override releases the COM pointer and drops
`_savedData`, the CPU copy it keeps to recreate a D3D9 texture after a device
loss; this projection's handle is owned by `GraphicsResource` and released in
the same position relative to the event, and CNA neither exposes nor needs a
recreation cache. XNA's `SpriteBatch` override disposes its own sprite shader
and its two dynamic buffers, all three of which are inside CNA's sprite batch
and released with it. Each override exists because XNA declares it and a
consumer's subclass must reach that link of the chain — which
`testRenderTargetDisposeWalksTheWholeChain` exercises, three deep.

## Falsifiability

Nine projection mutations added, each aimed at a decision above:

```text
disposing-flag-ignored-on-the-finalizer-path
alpha-zero-texels-not-rewritten
encode-dimensions-transposed
jpeg-saved-through-the-png-format
fromstream-zoom-ignored
fromstream-decode-dimensions-transposed
setdata-disposal-check-on-the-runtime-channel
getdata-disposal-check-after-the-arguments
closed-stream-accepted-by-save
```

One existing mutation, `disposing-raised-before-release`, had its site drift
when the `disposing` guard was inserted and was updated to the new body rather
than being allowed to report a survivor forty minutes later.

Six verifier self-tests for the stream-direction rule, four of them
demonstrated against planted defects.

## What is still absent here

`SaveAsPng` on a `RenderTarget2D` reaches `GetData`, and render-target readback
answers `CNA_RESULT_NOT_SUPPORTED` on this artifact (Foundation 53). The member
is implemented and the failure is the environment's, reported on the runtime
channel.

`SurfaceFormat` other than `Color` cannot be created here (Foundation 55), so
`SaveAsImage`'s other nineteen switch arms and its `DxtDecoder` arm are
unreachable and are refused with a producer-invariant rather than written as
code that cannot run.
