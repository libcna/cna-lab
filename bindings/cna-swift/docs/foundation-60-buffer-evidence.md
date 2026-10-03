# Foundation 60 — `VertexBuffer`, `IndexBuffer`, `VertexBufferBinding`

Three types projected, one defect shipped in Foundation 59 found and fixed, and
the gate that should have caught it taught to.

```text
COMPLETE_TYPES             150 -> 154
PARTIAL_TYPES                7 -> 6
MISSING_TYPES              100 -> 97
TOTAL_DIAGNOSTICS          165 -> 160
BOUND_FUNCTIONS             90 -> 100      ABI_MISMATCHES=0
XNA_RESOURCE_STRING_PROJECTIONS  38 -> 46
tests                      622 -> 641
```

`Texture2D` is the fourth type that became complete, for the reason in the next
section.

## Foundation 59 shipped two live disagreements

The Foundation 59 commit carried `PARAMETER_MAPPING_MISMATCH=2`:

```text
Texture2D.SaveAsPng(_:Foundation.OutputStream,width:Int32,height:Int32)
  expected [('_', 'Foundation.OutputStream'), …], found [('_', 'OutputStream'), …]
Texture2D.SaveAsJpeg(…)   the same
```

The compiler emits Foundation's stream classes unqualified and the mapping names
them qualified. `normalize_swift_type` had a special case for `InputStream` and
none for `OutputStream`, so the two new members were reported as disagreeing —
correctly — and the report was committed with them in it.

**Why nothing stopped it.** `TOTAL_DIAGNOSTICS=165` was read as
`100 + 58 + 5 + 2`, and the two were not decomposed. The status gate did not
catch it either, because the prose claim it polices —

> every category that would mean DISAGREEMENT with XNA: 0

— names the categories *without their values*, so no `KEY=VALUE` token existed
for a gate that compares tokens.

**The fix, in both places.** `normalize_swift_type` now normalizes both stream
directions. And the status gate no longer relies on prose for this: it reads the
strict report and reports every summary key ending in `_MISMATCH` or `_LEAK`, or
named `UNEXPECTED_TYPE`, `UNEXPECTED_MEMBER`, `UNMEASURED_STRUCTURAL_CATEGORY`
or an allowlist count, whose value is not zero. `OVERLOAD_MAPPING_MISMATCH` is
the single exclusion and it is an exclusion of *meaning*: every entry it can
carry reads "required overload is absent", which is an absence. **A category
this gate has never heard of is policed the moment it is non-zero** — one of the
seven new self-tests checks exactly that.

A gate that polices a sentence can only be as complete as the sentence.

## What CNA's raw transfer can express, measured

`build-probe/f60_stride.c`, ten triples against a four-vertex buffer whose
declaration stride is 16:

```text
bytes=64 count=4 stride=16   -> 0   the whole buffer
bytes=32 count=2 stride=16   -> 0   a shorter buffer, then a 64-byte read fails
bytes=64 count=2 stride=32   -> 1   INVALID_ARGUMENT
bytes=64 count=16 stride=4   -> 1
bytes=64 count=8 stride=8    -> 1
bytes=48 count=4 stride=16   -> 1   byte count is not count * stride
bytes=80 count=5 stride=16   -> 1   past the end
bytes=48 count=4 stride=12   -> 1
bytes=0  count=0 stride=16   -> 0

at offset 16, 1 vertex        -> 0   leaves the other three alone
at offset  8, 1 vertex        -> 1   not a multiple of the stride
at offset 48, 2 vertices      -> 1   past the end
get_data_raw(offset 16, 2)    -> 0   returns exactly those 32 bytes
```

So the stride must equal the buffer's own, the byte count must be exactly
`count * stride`, and a windowed offset must be stride-aligned.

XNA's `CopyData<T>` is looser: it writes `elementCount` elements of `sizeof(T)`
bytes **spaced `vertexStride` apart** and requires only
`vertexStride >= sizeof(T)`; the buffer's declaration stride is never compared
against it. Every transfer whose destination is a contiguous run of whole
vertices — which is every transfer with `vertexStride` 0 or equal to
`sizeof(T)`, and so every ordinary use of the six overloads — is expressible
exactly. One that leaves a gap between elements is not, and is refused on the
runtime channel.

**The refusal comes last.** Every XNA validation runs first, so a call XNA would
have rejected is rejected with XNA's own exception and not with this binding's
limit. `testAStridedPartialWriteIsRefusedAndNotGuessed` asserts the refusal;
`testTheCopyParameterRefusalsKeepTheirOwnNames` asserts the order.

## Which upload route, and why it is not the obvious one

`build-probe/f60_options.c`:

```text
static  vertex  set_data_raw_at                     -> 0
static  vertex  set_data_raw_at_with_options(0,1,2) -> 6  NOT_SUPPORTED
dynamic vertex  set_data_raw_at                     -> 0
dynamic vertex  set_data_raw_at_with_options(0,1,2) -> 0

static  index   set_data / set_data_at, options None -> 0
static  index   ... Discard, NoOverwrite             -> 6
dynamic index   set_data with Discard/NoOverwrite    -> 0
dynamic index   set_data_at with Discard/NoOverwrite -> 6
```

The option-taking vertex upload refuses a static buffer for **every** option
value, `CNA_SET_DATA_NONE` included. So `VertexBuffer.SetData` uses
`cna_vertex_buffer_set_data_raw_at`, and the `_with_options` route beside it
belongs to `DynamicVertexBuffer`, where XNA's `SetDataOptions` overloads live.
The last line is the one to remember when the dynamic buffers land: a dynamic
index buffer accepts `Discard` through the whole-buffer route and refuses it
through the windowed one.

## `CopyData`, from the IL

```text
Helpers.CheckDisposed(this, pComPtr);
if (data == null || data.Length == 0)
    throw new ArgumentNullException("data", NullNotAllowed);
if ((options & (Discard | NoOverwrite)) == 0)
    for (i = 0; i < device.currentVertexBufferCount; i++)
        if (device.currentVertexBuffers[i]._vertexBuffer == this)
            throw GetExceptionFromResult(E_ABORT);      // ResourceInUse
if (!isSetting && (usage & WriteOnly) == WriteOnly)
    throw new NotSupportedException(WriteOnlyGetNotSupported);
Helpers.ValidateCopyParameters(data.Length, startIndex, elementCount);
int bytes = sizeof(T) * elementCount;
int slack = 0;
if (vertexStride != 0) {
    slack = vertexStride - sizeof(T);
    if (slack < 0)
        throw new ArgumentOutOfRangeException("vertexStride", VertexStrideTooSmall);
    if (elementCount > 1) bytes += (elementCount - 1) * slack;
}
if (bytes + offsetInBytes > _size)
    throw new InvalidOperationException(ResourceDataMustBeCorrectSize);
```

Three details are reproduced that a reasonable reimplementation would tidy away.

**An empty array is an `ArgumentNullException`.** `IL_001c: ldlen;
IL_001d: brfalse IL_02be` branches to the same throw the null test uses, so a
zero-length array reports `"data"` as null.

**`ValidateCopyParameters` names `dataIndex`, not the caller's `startIndex`.**
It is a shared helper reporting its own parameter, and that is what reaches the
caller. Its order matters as much: an index past the end of the array is blamed
on the index, and only a *window* past the end on the count.

**The `WriteOnly` refusal is managed.** CNA refuses it natively too —
`cna_vertex_buffer_get_data_raw` answers `NOT_SUPPORTED` on a write-only buffer
— but the check is made before the call, because XNA's carries a class and a
message that a native result code does not.

The bound-buffer test has no counterpart yet: nothing can bind a vertex buffer
until `GraphicsDevice.SetVertexBuffer` is projected, so no input reaches the
branch. `E_ABORT` resolves through `Helpers.GetExceptionFromResult` to
`InvalidOperationException(ResourceInUse)`, and that is the first thing the draw
milestone must add.

## Two constructors Swift cannot fully serve

`VertexBuffer(GraphicsDevice, Type, Int32, BufferUsage)` needs
`Activator.CreateInstance`, and `IndexBuffer(GraphicsDevice, Type, …)` needs
`Marshal.SizeOf` — on a `System.Type`. Swift has neither for a metatype:
a metatype cannot be instantiated, `MemoryLayout<T>` needs a static `T`, and
`IVertexType.VertexDeclaration` is an *instance* member. Adding an `init()`
requirement to the projected `IVertexType` would put a member on it that XNA's
interface does not have.

So the four vertex structs XNA ships are registered by identity with the static
declaration each of them returns — **the same object** their own
`VertexDeclaration` property answers, which is the object XNA's `FromType` cache
would hold, so two buffers of one vertex type share one declaration exactly as
in XNA — and the four CLR index widths are recognised the same way. Everything
else reaches the `VertexDeclaration`- and `IndexElementSize`-taking constructors
beside them, which have no such limit.

`FromType`'s five tests: the null type and the null declaration are unreachable
through Swift's type system and are recorded; the value-type test, the
`IVertexType` test and the **size-against-declared-stride** test are reproduced.
That last one is not decoration — `testEveryVertexTypesSizeMatchesItsDeclaredStride`
compiler-measures all four structs against the strides their declarations claim,
so a layout that drifted would fail there rather than corrupt a transfer.

A type that *is* an `IVertexType` but is not one of the four gets the
language-limitation message on the runtime channel, not
`VertexTypeNotIVertexType`: saying it does not implement the interface would be
false.

The type named in those messages is the **Swift** type — `Swift.Int32`, not
`System.Int32`. XNA formats `System.Type.ToString()`; a CLR name for a type that
is not the CLR's would be an invention.

## `VertexBufferBinding` is pure managed

Three constructors, a conversion operator and three getters, none of which
reaches CNA. `cna_vertex_buffer_binding_init` exists and is deliberately
**unbound**: it fills a native structure only `SetVertexBuffers` will need, and
a route with no consuming member is not a route this binding has.

Its validation order is asserted rather than assumed: the offset is checked
before the frequency, so a binding wrong in both blames the offset; and the
bound is `bge.un`, **unsigned**, so a negative offset is caught by the explicit
`>= 0` half rather than by wrapping.

## `VertexDeclaration` still has no native handle

`cna_vertex_buffer_create` takes a declaration handle and copies the declaration
into the buffer, so one is created for the call and released immediately after.
Foundation 43 left `VertexDeclaration` managed because nothing in its public
surface needed a handle; nothing here changes that.

The explicit-stride route is the one used, because XNA's declaration carries a
stride a caller may have supplied rather than one computed from the elements.

## Enums that needed no map

`BufferUsage`, `SetDataOptions`, `IndexElementSize`, `PrimitiveType`,
`VertexElementFormat` and `VertexElementUsage` all carry CNA's own values, read
out of `graphics3d.h` and compared case by case against the projected enums.
None needs the kind of explicit translation `BlendFunction.Min`/`.Max` needs, and
that is stated because it was checked, not because it was assumed.

## Messages

Eight resource strings added to the registered selection and pinned, read out of
`Microsoft.Xna.Framework.dll`'s own string table:
`MustBeValidIndex`, `VertexStrideTooSmall`, `ResourceDataMustBeCorrectSize`,
`WriteOnlyGetNotSupported`, `IndexBuffersMustBeSizedCorrectly`,
`VertexTypeNotValueType`, `VertexTypeNotIVertexType`, `VertexTypeWrongSize`.
`RESOURCE_STRINGS_REPRODUCED` 38 → 46, `CALIBRATION_STATUS=PASS`.

Three deferrals added, and their reason has changed since Foundation 57:
`VertexBuffer.ctor` and `IndexBuffer.ctor` raise `ProfileTooBig`, and
`IndexBuffer.ctor` raises `ProfileNoIndexElementSize32`, from the same
`GraphicsProfile` capability family six `Texture2D` messages already wait
behind. What changed is that the fact is no longer *unknowable*:
`cna_graphics_device_get_graphics_profile` answers `Reach` on the qualified
artifact. These are deferred behind ordinary work — projecting
`GraphicsDevice.GraphicsProfile` and reading the pinned `ProfileCapabilities`
table out of the IL — rather than behind a missing host fact.
`MESSAGES_DEFERRED` 10 → 13.

## Falsifiability

Fourteen projection mutations, each aimed at a decision above: the copy
parameters' name and order, the empty-array quirk, the `WriteOnly` guard, the
stride floor, the size comparison, both offsets, the index element width, the
`Type` overload's width map, the windowed index write, the binding's signed
comparison and its check order, and `FromType`'s size test.

**Two of the first fourteen survived, and both were no-op mutations.** The run
reported `PROJECTION_MUTATIONS=137 CAUGHT=135 SURVIVORS=2`:

* `copy-parameters-checked-out-of-order` swapped two tests that both raise
  `ArgumentOutOfRangeException("elementCount", MustBeValidIndex)`. There is no
  input that distinguishes them, so the mutation measured nothing.
* `from-type-skips-the-size-test` deleted a guard that never fires while all
  four vertex structs are correct, so deleting it changed nothing either.

The rule is that a no-op mutation is **replaced, not scored**, and both were:

* `copy-parameters-blame-the-count-not-the-index` moves the window test ahead of
  the *index* test, which does change which parameter is blamed;
* `from-type-size-test-reads-the-wrong-size` plants the defect the guard exists
  to catch — a registered vertex type's measured size four bytes off its
  declared stride — which must make a correct call fail.

Both replacements were then planted by hand and both were caught. The full
137-mutation run is repeated before the final handoff, so the committed verdict
is `CAUGHT=135` from the run plus two demonstrated by hand, and not a `137/137`
this tree has not yet earned.

Seven new status-gate self-tests for the disagreement rule, including a category
the gate has never seen.

### Two things the harness itself got wrong

Its list of files to snapshot and restore was **written out by hand** beside the
mutation list. Foundation 60 added mutations in three new files and the run died
on a `KeyError` instead of reporting a stale site — the one failure the
pre-flight site check exists to prevent. The set is now derived from
`MUTATIONS`, so a file with a mutation cannot be forgotten and a file without
one is not read.

And checking that every mutation site still resolves is **only valid while the
harness is not running**: doing it mid-run reads a tree with one file
deliberately mutated, and reports a false stale site. That cost one confused
minute in `SpriteBatch.swift`.
