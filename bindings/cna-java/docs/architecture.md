# Architecture

## Public and implementation boundaries

```text
Microsoft.Xna.Framework.*
        strict, machine-verified Java projection
                    ↓
org.openeggbert.cna.internal.*
        private adapters, ownership, errors, JNI entry points
                    ↓
src/main/c/cna_java_jni.c
        dynamically resolved CNA stable C ABI only
                    ↓
CNA C++
```

The strict projection contains no raw address, JNI wrapper, CNA handle, or
implementation class in public/protected signatures. `tools/api-compat` checks
that boundary from compiled class metadata rather than Java source text.
When XNA exposes no public/protected constructor but Java facades in another
namespace must be created, the internal `FacadeFactory` invokes a package-private
constructor reflectively. This narrow friend-construction adapter avoids adding
an unexpected public constructor to the strict compiled contract.

There is deliberately no invented `CNA.Framework` layer. Opt-in CNA-specific
Java APIs will live under `org.openeggbert.cna.extensions` unless they project a
real native `CNA::...` public concept.

## Why JNI

The retained Java baseline is 17 and Android remains a future target. Stable
Foreign Function & Memory API is not available on Java 17, while JNI is present
on Java 17 desktop and Android. One C adapter is therefore the implemented
backend. It resolves only unmangled `cna_*` functions from the CNA C ABI; it
never binds the C++ ABI. A second backend is not justified until this path has
broader functional coverage.

Library discovery is explicit and portable:

- `cna.java.jniLibrary` or `CNA_JNI_LIBRARY` selects the adapter;
- `cna.native.library` or `CNA_NATIVE_LIBRARY` selects a CNA library;
- `CNA_NATIVE_DIR` supplies a directory containing the platform library name;
- the platform loader name is the final fallback.

The adapter owns callback global references, attaches/detaches callback threads,
copies UTF-8 at the boundary, converts fixed-width ABI types, and turns CNA
results into Java exceptions.

XNA's input, MediaPlayer, and process-wide SoundEffect entry points are static, while every
corresponding CNA C ABI route requires a game handle. CNA-Java therefore records the most recently
created live `Game` as the process-current game and clears it on successful
destruction, matching XNA's one-game-per-process model. Static `Keyboard` and
`Mouse` calls without a live current game fail deterministically. Keyboard
state crosses JNI as a copied four-word POD snapshot and mouse state as copied
canonical scalar/button fields; no state retains the native parent. Opaque
window tokens issued by CNA-Java are registered internally so `Mouse` can round
trip them without exposing their numeric address.

## Ownership

Internal handles record one of four modes: owned, borrowed, parent-owned, or
adopted/transferred. Only owned/adopted wrappers destroy native objects. Close is
explicit, deterministic, retryable after a failed native release, and
idempotent after success. Raw handle values never reach application code.

`Game` owns its content/device facades and native CNA game. Textures and
SpriteBatches are owned child handles registered against that game; explicit
resource close removes them from the registry, while game teardown closes any
remaining children in reverse creation order before destroying the parent.
Texture and draw inputs are copied from mutable Java value objects at each JNI
boundary. Content is unloaded while the native parent is still live, and
borrowed Java facades are invalidated afterward. Deprecated finalization is not
used.

Audio uses the same transactional ownership rule. A SoundEffect owns one native
effect and closes its independently owned instances first; every instance
strongly retains its parent so CNA cannot observe a dead effect. A dynamic
instance owns its callback registration until CNA accepts unsubscribe. Java
handles and listener contexts remain live after a refused wrong-thread release,
so owner-thread retry is safe. Game teardown closes registered Audio roots before
destroying the CNA game.

The XACT graph has one dependency root. AudioEngine owns its engine and category
handles; WaveBank and SoundBank retain/register with the engine; Cue retains the
engine and participates in SoundBank bookkeeping. Engine teardown walks cues and
banks before categories and the native root. Disposal events are raised once
after successful native destruction, and listener failures are aggregated only
after cleanup. Authored-bank runtime qualification remains separate from this
implemented ownership graph because no redistributable XGS/XSB/XWB fixture is
available.

Native Audio callbacks enter Java only through registered global references.
Dynamic BufferNeeded and microphone BufferReady listeners run on CNA's
FrameworkDispatcher owner-thread pump. User exceptions are captured in Java and
surface on a later owner call/close; they never unwind across a JNI/native frame.
Failed unsubscribe re-enables and retains the registration rather than freeing a
context CNA may still call.

MediaLibrary is a native platform facade, not a host-filesystem adapter. Its seven collection
facades are cached per library, are read-only, retain order, and cache each native object wrapper
per index. Empty collections on the qualified HEADLESS Linux runtime are valid platform results;
the binding never invents media records. Library-owned object relationships release only their C
wrapper handles, while owned Song values from `Song.FromUri` and their queue aliases are tracked so
there is no second native owner. Native destruction is transactional: a refused wrong-thread
library or player close leaves the handle live and retryable.

MediaPlayer state is process-global and uses the process-current Game. Its queue facade is stable
within a Game lifetime and invalidated before the native Game is destroyed. Native callbacks own a
per-registration JNI context, attach when necessary, and only enqueue an event kind. The existing
FrameworkDispatcher/automatic successful-frame pump drains that queue on the owner thread. A
throwing `Update` therefore skips delivery; listener exceptions are contained until the CNA Game
callback barrier reports them, and later listeners still run. Shutdown disables and clears the
queue before native destruction, then the next Game begins with a fresh queue generation.

Video separates managed XNB metadata from native decoder ownership. ContentManager owns the
hidden native Video handle and destroys it during reverse content teardown. VideoPlayer is a Game
child and closes before the native Game. Loop, mute, and volume are Java-cached as XNA observes,
including after player disposal; NaN volume is intentionally accepted while finite values outside
`[0,1]` fail.

`VideoPlayer.GetTexture` is an explicit borrowed boundary. A nonzero CNA frame handle becomes a
parent-owned `Texture2D` facade whose `close()` never destroys native frame memory. The player
invalidates that facade before every subsequent player operation, frame query, Stop, disposal, or
Game teardown. CNA ABI 0.7 does not expose the XNA two-buffer identity or a frame generation token,
so CNA-Java does not claim stable frame identity. HEADLESS may return no frame and no pixels are
fabricated. The exact missing native contract is recorded in `media-video-evidence.md`.

Storage uses the same one-live-Game ownership root because XNA exposes no public disposal member on
`StorageDevice` while CNA returns an owned handle. A device owns its containers, each container
owns its streams and disposal registration, and Game shutdown closes streams and containers in
reverse order before destroying devices. The completed `Begin` results do not own native work:
they retain arguments/state, invoke the callback synchronously, and create the device or container
once at `End`, matching XNA IL. A refused wrong-thread close leaves all handles and owner lists
intact for retry.

Container `Disposing` has a two-stage callback boundary. The CNA callback only records native
delivery in Java; after that native frame returns, `StorageContainer.close()` invokes a stable
listener snapshot, contains handler failures until cleanup, and permits recursive close without a
second event or destroy. Static `StorageDevice.DeviceChanged` has one process-owned native
registration and enqueues into the existing FrameworkDispatcher owner-thread pump. Pending work is
discarded at Game shutdown, so no callback targets a dead Game generation.

XNA canonicalizes every file/directory path and checks that it remains below the container root.
The qualified CNA ABI 0.7 artifact did not enforce that rule for parent traversal, so Java performs
portable absolute/drive/traversal rejection before the native operation. This native semantic gap
and the platform-pending origin of a real DeviceChanged event are recorded explicitly in
`storage-evidence.md`.

The Design converter family is a managed value-conversion subsystem and deliberately does not
enter this ownership graph. `MathTypeConverter` is the compact Java base contract. Ordered
`LinkedHashMap` metadata/decomposition replaces CLR property descriptors, `Map<String,Object>`
supplies reconstruction values, `java.beans.Expression` replaces `InstanceDescriptor`, and
`Locale` replaces `CultureInfo`. Context and attribute-filter parameters with no Java designer-
host meaning are omitted by exact formal mapping. Every mutable struct component is snapshotted;
no reflection over public fields is used as the converter contract and no native route is needed.

`GamerServicesComponent` is the sole selected-profile bridge into CNA GamerServices. It remains a
normal managed `GameComponent`: initialization copies the opaque Game window token and borrows the
live native Game for the canonical dispatcher initialization; each enabled component update pumps
the canonical no-argument dispatcher route. It allocates no native component handle and therefore
adds nothing to Game teardown. The separate Gamer/Guide/Avatar/Network object profiles are not
projected, and no native stub gamer becomes a Java public object.

Managed XNB resources use the same graph rather than a parallel handle system.
`ContentManager` records successfully constructed disposable resources once and
unloads them in reverse construction order. Thus a SpriteFont native object is
released before its atlas, and a Model's effects and buffers retain the native
ownership already assigned when their shared-resource fixups were resolved.
Partial reader failure rolls back only resources constructed by that load.
`Model`, bones, meshes, and mesh parts are stable managed identity nodes; they
do not become additional native owners and Model is not made disposable when
XNA does not expose that contract.

## Managed XNB pipeline

```text
ContentManager.Load(Class<T>, asset)
        ↓
Windows XNB v5 framing
        ↓ optional XNA LZX frame decompression
reader table
        ↓
internal ContentTypeReader registry
        ↓
managed metadata/raw-byte parsing
        ↓
existing GraphicsDevice resource creation and upload
```

Compressed framing validates the XNB decompressed-size field, XNA short and
extended frame headers, the 32-KiB frame bound, exact block availability, and
canonical end padding. One stateful LZX decoder retains its 64-KiB window across
frames. The decompressed bytes then enter exactly the same reader-table pipeline
as uncompressed assets; reader failures and resource rollback are therefore not
a separate content mode.

The registry is type-reader driven; it does not dispatch on asset names or call
CNA's loose-file loader for XNB. Texture2D currently preserves uncompressed
`SurfaceFormat.Color` payloads and mip levels exactly. Formats for which CNA
cannot create/upload the matching native surface fail explicitly, so compressed
payloads are never relabeled as RGBA. SpriteFont is assembled from its loaded
atlas and copied glyph table. Model resolves shared vertex/index/effect
resources through the normal XNB fixup mechanism and retains stable facade
identity across its bone, mesh, and part graph.

VideoReader follows XNA's boxed reader layout: file name, duration milliseconds, width, height,
frames-per-second, and soundtrack type are each read through `ReadObject<T>`. The embedded video
path is resolved relative to the XNB asset and Content root. Managed metadata stays observable even
when the qualified HEADLESS backend cannot decode the referenced file.

LZ4 and unknown compression dialects remain explicit failures. Neither framing
path silently changes a texture format or falls back to CNA's loose-file loader.

## Contract layers

Compatibility is tracked independently as:

1. the authoritative XNA CLR reference contract;
2. the deterministic XNA Java projection produced by the mapping rules;
3. CNA-backed behavior coverage of each mapped API.

A member can therefore be structurally complete without its platform-dependent runtime capability
being available. The selected XNA 4.0 Windows runtime projection is now structurally strict at
zero diagnostics, while `docs/runtime-capabilities.json` records managed/native evidence and the
remaining upstream, backend, asset, hardware, platform, and language-mapping boundaries. Strict
zero is never promoted to universal runtime completeness or to completion of other historical XNA
profiles.
