# Storage evidence

**Updated:** 2026-08-23

**Authority:** Microsoft XNA Framework 4.0 Windows runtime metadata and IL

**Runtime evidence:** CNA ABI 0.7.0, Linux x86-64, HEADLESS renderer, isolated XDG storage root

This document records the optional Storage milestone completed after Media/Video. XNA metadata and
IL define the Java public contract and deterministic behavior. CNA-C# supplied comparative
ownership and test evidence, but did not override XNA. The qualified CNA ABI 0.7 artifact was
retested directly because older CNA-C# evidence observed a missing native `Disposing` callback.

## Status vocabulary

- `STRICT_COMPLETE`: the entire selected-profile mapped public contract is present.
- `MANAGED_VERIFIED`: deterministic Java validation, fake-async, event, or lifecycle behavior has
  direct assertions.
- `NATIVE_VERIFIED`: the named CNA ABI 0.7 route executed against the qualified artifact.
- `PLATFORM_PENDING`: a real host/device transition was not available to produce the event.
- `CNA_GAP`: the current C route lacks a semantic that Java must preserve explicitly.

## Strict result at the Storage milestone boundary

All three mapped Storage types are complete as one dependency group:

```text
Microsoft.Xna.Framework.Storage.StorageContainer
Microsoft.Xna.Framework.Storage.StorageDevice
Microsoft.Xna.Framework.Storage.StorageDeviceNotConnectedException
```

```text
REFERENCE_TYPES=257
REFERENCE_MEMBERS=2964
EXPECTED_JAVA_TYPES=265
EXPECTED_JAVA_MEMBERS=3206
TARGET_TYPES=251
TARGET_MEMBERS=3150
TOTAL_DIAGNOSTICS=14
MISSING_TYPE=14
MISSING_MEMBER=0
ALLOWLIST_ENTRIES=0
```

Every mismatch, unexpected-type/member, internal-leak, and mapping-drift category remained zero.
At this historical boundary the 14 remaining whole types were thirteen Design converters and
`GamerServicesComponent`; both later milestones are now complete and the project-wide state is
265/265 target types, 3206/3206 members, and zero diagnostics. The mapped expected-member total
decreased by one at Storage because the protected
CLR serialization constructor on `StorageDeviceNotConnectedException` has no Java serialization-
protocol equivalent and is excluded by its exact signature, like the existing
`ContentLoadException` rule.

## Per-type ledger

| Type | Strict | Managed | Native and remaining boundary |
|---|---|---|---|
| StorageDevice | STRICT_COMPLETE | MANAGED_VERIFIED all four completed-result shapes, synchronous callback/state, one-End rule, exact size/directory validation, parent identity, static event snapshots | NATIVE_VERIFIED all four selectors, connected/free/total properties, delete, process event subscription, Game shutdown destroy; actual OS device-change origin PLATFORM_PENDING |
| StorageContainer | STRICT_COMPLETE | MANAGED_VERIFIED display/device identity, null/empty/disposed behavior, normalized paths, XNA containment, one-shot/reentrant/mutable/throwing Disposing handlers, reverse child close | NATIVE_VERIFIED open/display, CRUD, patterns, stream operations, dispose callback, unsubscribe/destroy, wrong-thread refusal/retry; native path containment is a documented CNA_GAP |
| StorageDeviceNotConnectedException | STRICT_COMPLETE | MANAGED_VERIFIED three public constructors, message, and mapped inner exception | No native route is required; Java throws it when a Game-scoped device facade has been invalidated |

The supporting CLR carriers are mapped to `System.IAsyncResult`, `System.AsyncCallback`,
`System.IO.FileMode`, `FileAccess`, `FileShare`, `SeekOrigin`, and the existing `System.IO.Stream`.
They are compatibility types outside the selected XNA type count.

## Authoritative fake-async behavior

XNA IL does not select a device or open a container during `Begin`. It creates a result whose wait
state is already signaled, reports `CompletedSynchronously=true` and `IsCompleted=true`, invokes a
non-null callback before returning, and defers object construction to `End`. `End` may be called
once. A null or foreign result fails, and a container result is accepted only by the device that
created it. The Java result stores the original state and operation arguments and follows that
shape exactly.

XNA validates `PlayerIndex` and negative `sizeInBytes` at `Begin`, but does not reject a negative
`directoryCount`; Java normalizes that value to zero only when calling CNA because CNA 0.7 adds a
native validation XNA did not have. The JNI route supplies a private completion counter and
requires CNA to invoke its completion callback exactly once before returning, so the C fake-async
contract is also executed rather than assumed.

## Ownership and Game teardown

`StorageDevice` has no public XNA disposal contract. CNA-Java therefore registers every owned
native device with the current one-live `Game`. A device strongly owns its opened containers; a
container strongly retains its device and owns every stream it returns. Game shutdown performs:

```text
streams (reverse order)
    -> container dispose / Disposing / unsubscribe / destroy
    -> device destroy
    -> Media and Audio roots
    -> native Game destroy
```

Explicit stream/container close removes the child from its owner. Destruction changes Java state
only after CNA accepts the release, so result 8 from a wrong thread leaves the stream/container
and parent bookkeeping live for owner-thread retry. Container close and every native release are
idempotent after success. A live stream at Game shutdown is closed before its container; a live
container is disposed before the non-disposable public device facade is invalidated.

The process-wide `DeviceChanged` registration is intentionally process-owned. Its JNI context and
global class reference remain live for the loaded process, while per-Game pending work is cleared
and disabled at shutdown and re-enabled for the next Game.

## Callback delivery

The ABI 0.7 container `Disposing` callback was observed exactly once. The JNI callback only sets a
Java observation flag and clears any accidental JNI exception; it never calls user handlers on the
C callback frame. After native dispose returns, Java marks the event one-shot before iterating a
stable listener snapshot. This permits duplicate subscriptions, one-at-a-time removal,
self-removal, recursive `close()` inside a handler, later-handler continuation after an exception,
and cleanup before the listener exception is rethrown.

`StorageDevice.DeviceChanged` uses one process-global CNA subscription and the existing
FrameworkDispatcher/successful-frame owner-thread pump. A callback only enqueues work. The
qualification suite enqueues from a worker thread, verifies deferred duplicate delivery, handler
exceptions, later-handler continuation, and shutdown discard. No actual host device-change event
occurred, so the native subscription is verified but a backend-originated event remains
`PLATFORM_PENDING`.

## Native boundary

Storage adds 41 reviewed canonical routes after the completed 679-function Media/Video state:

```text
STARTING_BOUND_FUNCTIONS=679
FINAL_BOUND_FUNCTIONS=720
HEADER_ABI=0.7.0
MANIFEST_JNI_BINDING_CHECK=PASS
LAYOUT_SIGNATURE_PROBE=PASS
LIBRARY_ABI=0.7.0
LIBRARY_SYMBOL_CHECK=PASS (720/720)
```

The routes cover four selectors, device properties/delete/event/destroy, container open/display/
dispose/event/CRUD/name enumeration/destroy, and owned stream open/read/write/seek/property/flush/
close. The compile-time probe checks every exact function signature plus the 32-bit FileMode,
FileAccess, FileShare, and SeekOrigin identities and the 64-bit device/container/stream handle
aliases. JNI still compiles as C11 with `-Wall -Wextra -Werror`; no C++ ABI is bound and ABI 0.8 is
not accepted.

## Exact CNA gap: relative-path containment

```text
AFFECTED_XNA_API=StorageContainer CreateDirectory/CreateFile/DeleteDirectory/DeleteFile/
  DirectoryExists/FileExists/OpenFile overloads
CURRENT_CNA_ROUTE=cna_storage_container_* path and stream-open routes
CURRENT_JAVA_SAFE_BEHAVIOR=reject null/empty paths, absolute Unix paths, Windows drive/root paths,
  and any normalized parent traversal that escapes the selected container before entering JNI;
  pass valid relative paths unchanged to CNA
MISSING_CNA_SEMANTIC=XNA ValidateArguments combines with the container root, canonicalizes the
  result, and rejects results outside that root; qualified CNA ABI 0.7 accepted ../ traversal
OWNERSHIP_REQUIREMENT=all created directories/files and streams must remain descendants of the
  selected container and streams remain container-owned
CALLBACK/THREAD_REQUIREMENT=validation is synchronous before the owner-thread native operation
WHAT_WOULD_UNBLOCK_JAVA=CNA must enforce and document canonical relative-path containment for every
  container path route, with traversal, absolute-path, Windows-root, and normalized-path tests
```

This gap is not hidden: Java validation is part of the managed evidence and the native route is not
described as providing containment. It prevents a superficially successful operation from writing
outside the XNA container boundary.

## Platform-pending DeviceChanged origin

```text
AFFECTED_XNA_API=StorageDevice.DeviceChanged
CURRENT_CNA_ROUTE=cna_storage_device_subscribe_device_changed
CURRENT_JAVA_SAFE_BEHAVIOR=hold one process registration, enqueue callback work, deliver only on
  the existing owner-thread pump, and discard queued work at Game shutdown
MISSING_CNA_SEMANTIC=none established in the subscription route; no qualified OS/device transition
  occurred to exercise callback origin
OWNERSHIP_REQUIREMENT=registration and JNI context are process-owned; Java listeners remain static
CALLBACK/THREAD_REQUIREMENT=arbitrary native threads may enqueue but must never invoke user code
WHAT_WOULD_UNBLOCK_JAVA=a resettable platform/device-change fixture or a canonical qualification
  raise route that invokes the registered C callback
```

## Behavior and stress

Nine focused Storage tests cover the managed carriers and native integration. The normalized
reference-backed text corpus remains 127 observations (94 math/geometry, 23 input, 10 Media);
Storage filesystem capacity/path results stay in direct assertions because they depend on the
qualified host root.

```text
STORAGE_STRESS_CYCLES=40
STORAGE_CALLBACK_CYCLES=4 qualified DeviceChanged dispatches plus per-container Disposing cycles
NATIVE_CRASHES=0
OBSERVED_UAF=0
DOUBLE_FREE=0
SANITIZER=NOT_RUN
```

The stress loop selects a fresh device, opens a unique container, creates/writes/closes/deletes a
stream, closes/deletes the container, destroys the Game-owned device, and then creates the next
Game. A final fresh Game proves that no dead device/container remains in the ownership registry.
No sanitizer-built compatible CNA runtime was available, so allocator leak freedom is not claimed.
