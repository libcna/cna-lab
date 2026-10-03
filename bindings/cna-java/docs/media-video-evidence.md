# Media/Video evidence

**Updated:** 2026-08-23

**Authority:** Microsoft XNA Framework 4.0 Windows runtime metadata and IL

**Runtime evidence:** CNA ABI 0.7.0, Linux x86-64, HEADLESS renderer, NULL audio

This document separates public-contract completeness from managed behavior, native route
execution, and environment-dependent capability. CNA-C# supplied ownership and blocker evidence;
it did not override XNA metadata or IL. CNA-Java does not scan arbitrary host directories, invent
media records, fabricate decoded video frames, or turn a borrowed frame into an owning texture.

The strict and 679-function figures below intentionally capture the Media/Video milestone boundary.
The figures below intentionally preserve the Media/Video milestone boundary. Storage later moved
the project to 251 target types, 14 missing types, and 720 routes; the completed Design and
GamerServices work now places the project-wide selected profile at 265/265 target types, zero
diagnostics, and 723 routes. See `plan.md`. No Media/Video qualification claim changed.

## Status vocabulary

- `STRICT_COMPLETE`: the entire selected-profile mapped public contract is present and passes the
  structural verifier.
- `MANAGED_VERIFIED`: deterministic Java validation, caching, collection, value, or lifecycle
  behavior has direct assertions.
- `NATIVE_VERIFIED`: the named CNA ABI 0.7 route executed against the qualified artifact.
- `BACKEND_BLOCKED`: the HEADLESS backend or current C API cannot supply the stronger behavior.
- `ASSET_PENDING`: no suitable video/catalog asset was available to qualify the successful path.
- `PLATFORM_PENDING`: behavior requires a real platform media catalog, picture store, or device.

## Strict result

All 24 mapped Media/Video types are `STRICT_COMPLETE`. The seven collection `iterator()` methods
are formal Java `Iterable<T>` bridge obligations, so the mapped Java member total increases by
seven relative to the pre-milestone estimate.

```text
REFERENCE_TYPES=257
REFERENCE_MEMBERS=2964
EXPECTED_JAVA_TYPES=265
EXPECTED_JAVA_MEMBERS=3207
TARGET_TYPES=248
TARGET_MEMBERS=3115
TOTAL_DIAGNOSTICS=17
MISSING_TYPE=17
MISSING_MEMBER=0
ALLOWLIST_ENTRIES=0
```

Every mismatch, unexpected-type/member, internal-leak, and mapping-drift category is zero. The 17
remaining diagnostics are exactly three Storage types, thirteen Design types, and one
GamerServices type.

## Per-type ledger

| Type | Strict | Managed | Native and remaining boundary |
|---|---|---|---|
| Album | STRICT_COMPLETE | MANAGED_VERIFIED stable artist/genre/song child identity, streams, equality/disposal graph | ABI/export qualified; Song relationship handle exercised; populated catalog metadata PLATFORM_PENDING |
| AlbumCollection | STRICT_COMPLETE | MANAGED_VERIFIED count, index bounds, order, iterator, stable index identity, read-only/disposal | NATIVE_VERIFIED empty HEADLESS count/iteration/disposal; non-empty catalog PLATFORM_PENDING |
| Artist | STRICT_COMPLETE | MANAGED_VERIFIED stable album/song children, equality/disposal graph | ABI/export qualified; populated catalog PLATFORM_PENDING |
| ArtistCollection | STRICT_COMPLETE | MANAGED_VERIFIED collection contract | NATIVE_VERIFIED empty HEADLESS collection; non-empty catalog PLATFORM_PENDING |
| Genre | STRICT_COMPLETE | MANAGED_VERIFIED stable album/song children, equality/disposal graph | ABI/export qualified; populated catalog PLATFORM_PENDING |
| GenreCollection | STRICT_COMPLETE | MANAGED_VERIFIED collection contract | NATIVE_VERIFIED empty HEADLESS collection; non-empty catalog PLATFORM_PENDING |
| MediaLibrary | STRICT_COMPLETE | MANAGED_VERIFIED constructor validation, stable children/source, reverse close, disposed access, retryable close | NATIVE_VERIFIED default library/source/scan/dispose and wrong-thread refusal; picture token/save PLATFORM_PENDING |
| MediaPlayer | STRICT_COMPLETE | MANAGED_VERIFIED static facade, null/range behavior, queue generation, event semantics | NATIVE_VERIFIED properties, transport, URI Song play, queue, visualization, canonical event raises, Game recreation |
| MediaQueue | STRICT_COMPLETE | MANAGED_VERIFIED stable per-Game facade and per-index Song identity, bounds, active index/song, invalidation | NATIVE_VERIFIED empty and one-Song queue, active item, MoveNext/MovePrevious/Stop, Game shutdown |
| MediaSource | STRICT_COMPLETE | MANAGED_VERIFIED immutable source values and unmodifiable source list | NATIVE_VERIFIED HEADLESS enumeration/default local source; other device types PLATFORM_PENDING |
| MediaSourceType | STRICT_COMPLETE | MANAGED_VERIFIED exact numeric values | Native POD layout/signature verified; device availability PLATFORM_PENDING |
| MediaState | STRICT_COMPLETE | MANAGED_VERIFIED exact enum order/default transitions | NATIVE_VERIFIED MediaPlayer and VideoPlayer state conversions |
| Picture | STRICT_COMPLETE | MANAGED_VERIFIED cached album relation, Instant/stream mapping, equality/disposal graph | ABI/export qualified; real image/thumbnail/token object PLATFORM_PENDING |
| PictureAlbum | STRICT_COMPLETE | MANAGED_VERIFIED parent/albums/pictures identity and disposal graph | Root route executed where available; populated hierarchy PLATFORM_PENDING |
| PictureAlbumCollection | STRICT_COMPLETE | MANAGED_VERIFIED collection contract | ABI/export qualified; populated hierarchy PLATFORM_PENDING |
| PictureCollection | STRICT_COMPLETE | MANAGED_VERIFIED collection contract | NATIVE_VERIFIED empty Pictures/SavedPictures; image content PLATFORM_PENDING |
| Playlist | STRICT_COMPLETE | MANAGED_VERIFIED stable Song collection and disposal graph | ABI/export qualified; platform playlist metadata PLATFORM_PENDING |
| PlaylistCollection | STRICT_COMPLETE | MANAGED_VERIFIED collection contract | NATIVE_VERIFIED empty HEADLESS collection; populated catalog PLATFORM_PENDING |
| Song | STRICT_COMPLETE | MANAGED_VERIFIED URI/null behavior, cached album/artist/genre facades, equality/disposal | NATIVE_VERIFIED owned FromUri WAV, properties, MediaPlayer Play, queue alias, Stop and Game shutdown; protected/catalog metadata PLATFORM_PENDING |
| SongCollection | STRICT_COMPLETE | MANAGED_VERIFIED collection contract and Play overload validation | NATIVE_VERIFIED library collection lifetime; non-empty authored collection PLATFORM_PENDING |
| Video | STRICT_COMPLETE | MANAGED_VERIFIED authoritative boxed VideoReader layout, metadata, content cache/unload/path resolution | NATIVE_VERIFIED metadata create/destroy; successful decode ASSET_PENDING and BACKEND_BLOCKED |
| VideoPlayer | STRICT_COMPLETE | MANAGED_VERIFIED defaults, validation including NaN, cached values after disposal, failure timing, frame invalidation | NATIVE_VERIFIED create/property/control/dispose routes; successful Play/frame BACKEND_BLOCKED and ASSET_PENDING |
| VideoSoundtrackType | STRICT_COMPLETE | MANAGED_VERIFIED exact enum order | Native POD layout/signature verified; decoded soundtrack ASSET_PENDING |
| VisualizationData | STRICT_COMPLETE | MANAGED_VERIFIED stable read-only 256-value views and native replacement | NATIVE_VERIFIED canonical visualization route; real spectrum output requires playback/backend |

“ABI/export qualified” means the exact header declaration, JNI descriptor, C signature, layout,
and library symbol are verified. It does not mean a populated platform object was available to
exercise every property.

## Collections and MediaLibrary

All seven collection types are read-only facades, not exposed mutable `List` instances. They retain
native order, check negative and upper-bound indices, cache one Java wrapper per native index, and
use a snapshot upper bound for iteration. Empty iteration terminates normally and `next()` then
throws. Closing a library walks cached children in reverse order. Collection elements and object
relationships have stable Java identity without duplicate native ownership.

The default MediaLibrary and MediaSource routes executed on the qualified artifact. The result may
be an empty library on HEADLESS Linux. `SavePicture` and `GetPictureFromToken` call only canonical
CNA routes; they do not fall back to arbitrary host-file writes. Those operations remain
`PLATFORM_PENDING` because no qualified platform picture store/token corpus was present.

## Process-global MediaPlayer and events

MediaPlayer has no Java instance. Every operation resolves the one process-current live Game. The
queue facade is stable for that Game, its per-index Song facades remain stable, and shutdown
releases the queue before native Game destruction. The next Game receives a new queue generation;
stale pending events and stale native handles are not retained.

Both MediaPlayer events use one native registration and the existing FrameworkDispatcher owner-
thread path. The JNI callback owns a per-registration context and only appends an event kind; it
does not invoke user code. A successful dispatcher pump or successful Update-to-BeginDraw path
drains events in order. Evidence covers subscription, one-at-a-time duplicate removal,
self-removal, stable dispatch snapshots, 100 queued callbacks, player close inside a callback,
throwing handlers, later-handler continuation, a throwing Game Update that skips delivery, and no
delivery into the next Game. The arbitrary-native-thread attach/detach branch is implemented and
audited, but this HEADLESS run did not originate the event from a backend worker thread.

## Video and frame ownership

VideoReader was derived from XNA IL. It reads all six fields through `ReadObject<T>`, including the
boxed numeric values, and resolves the embedded loose-video path relative to the XNB asset and
Content root. ContentManager owns the hidden CNA Video handle, preserves cache identity, rolls back
failure, and destroys it before Game teardown.

VideoPlayer executes CNA create, state, position, loop, mute, volume, Play/Pause/Resume/Stop,
texture-query, dispose, and destroy routes as applicable. Loop, mute, and volume are deliberately
cached and remain observable after disposal. Finite volume outside `[0,1]` throws; NaN is accepted
and passed through unchanged. A failed native Play does not replace the cached Video.

For a nonzero frame handle, `GetTexture` creates a `PARENT_OWNED` Texture2D facade. Closing that
facade only invalidates the Java handle. The player invalidates any previous facade before its next
native operation, next frame query, Stop, disposal, or Game teardown. The qualified HEADLESS run
returned no decoded frame; it did not manufacture pixels.

## Native boundary

The milestone adds 192 reviewed Media/Video functions to the 487-function baseline.

```text
STARTING_BOUND_FUNCTIONS=487
FINAL_BOUND_FUNCTIONS=679
HEADER_ABI=0.7.0
MANIFEST_JNI_BINDING_CHECK=PASS
LAYOUT_SIGNATURE_PROBE=PASS
LIBRARY_ABI=0.7.0
LIBRARY_SYMBOL_CHECK=PASS (679/679)
```

The layout probe additionally checks the three Media enum widths, `CNA_VisualizationData` size and
array offsets, and `CNA_VideoInfo` size/field offsets. JNI compiles as C11 with `-Wall -Wextra
-Werror`. ABI 0.8 is not accepted.

## Exact upstream/platform blockers

### Frame identity and generation

```text
AFFECTED_XNA_API=VideoPlayer.GetTexture
CURRENT_CNA_ROUTE=cna_video_player_get_texture
CURRENT_JAVA_SAFE_BEHAVIOR=return null when CNA has no frame; otherwise return one non-owning
  parent-owned Texture2D facade and invalidate it before the next player operation/frame query
MISSING_CNA_SEMANTIC=XNA stable/two-buffer texture identity plus an observable frame generation or
  lifetime boundary
OWNERSHIP_REQUIREMENT=the player remains sole native owner; Java must never destroy frame memory
CALLBACK/THREAD_REQUIREMENT=frame advancement must atomically identify which aliases became stale
WHAT_WOULD_UNBLOCK_JAVA=a CNA ABI route that returns stable frame identity and generation/lifetime
  metadata, or an explicit retained frame object with a documented release contract
```

### Decode and frame availability

```text
AFFECTED_XNA_API=VideoPlayer.Play/Pause/Resume/Stop/State/PlayPosition/GetTexture
CURRENT_CNA_ROUTE=cna_video_create_with_metadata and cna_video_player_*
CURRENT_JAVA_SAFE_BEHAVIOR=execute the real control path, preserve native failure timing, and return
  no frame when HEADLESS supplies none
MISSING_CNA_SEMANTIC=a qualified non-HEADLESS decoder/presenter path for this artifact
OWNERSHIP_REQUIREMENT=Video remains Content-owned; VideoPlayer and frames remain ordered Game
  children
CALLBACK/THREAD_REQUIREMENT=decoder state/frame publication must obey the player owner thread or
  expose a synchronization contract
WHAT_WOULD_UNBLOCK_JAVA=a provenance-qualified CNA ABI-0.7 video backend plus a legal deterministic
  video asset and successful frame-generation evidence
```

### Platform media catalog and pictures

```text
AFFECTED_XNA_API=MediaLibrary collections, Picture/PictureAlbum, GetPictureFromToken, SavePicture
CURRENT_CNA_ROUTE=cna_media_source_*, cna_media_library_*, cna_picture_*, and collection/object routes
CURRENT_JAVA_SAFE_BEHAVIOR=expose the canonical native platform result, including an empty library;
  propagate unsupported/native errors and never emulate with arbitrary filesystem directories
MISSING_CNA_SEMANTIC=a populated, provenance-qualified Linux platform catalog with XNA-compatible
  token and picture-persistence behavior
OWNERSHIP_REQUIREMENT=library owns catalog objects; Java releases wrappers without disposing the
  canonical platform record
CALLBACK/THREAD_REQUIREMENT=library and picture operations remain on the native Game owner thread
WHAT_WOULD_UNBLOCK_JAVA=a qualified platform provider and deterministic catalog/token/save fixture
  that can be reset between tests
```

## Behavior and stress

The normalized reference-backed corpus grows from 117 to 127 observations: 94 math/geometry, 23
input, and 10 deterministic Media observations. Backend timing and native identity are kept in
direct assertions instead of golden text.

Direct native assertions cover real URI Song playback, a non-empty queue, all static MediaPlayer
properties, NaN volume, visualization views, callbacks, library lifecycle, Video metadata/player
control, and disposal behavior. Stress covers 40 MediaLibrary/VideoPlayer/Game recreation cycles
and 100 callback deliveries. Wrong-thread MediaLibrary and VideoPlayer destruction returns CNA
result 8, keeps both wrappers live, and succeeds when retried on the owner thread. No native crash,
observed use-after-free, or double-free occurred. No sanitizer-built compatible runtime was used,
so allocator leak freedom is not claimed.
