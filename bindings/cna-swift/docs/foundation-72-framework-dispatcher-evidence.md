# Foundation 72 — `FrameworkDispatcher`, and a mutation aimed at the only observable half

One type, one member, one route. It is here because it is the cheapest thing
left that unblocks something bigger: Media and the dynamic audio instances both
need a pump, and both milestones would otherwise have to invent one.

## What a passing test here means

**The pump was accepted.** Nothing asserts that it did any work, and the reason
is measurable rather than cautious: XNA's `Update` drains a private static
queue into `MediaPlayer.OnActiveSongChanged`, `MediaPlayer.OnMediaStateChanged`,
`Microphone.AllMicrophones`, `DynamicSoundEffectInstance
.RaiseBufferNeededOnInstance` and `FrameworkCallbackLinker
.OnStorageDeviceChanged`. On this host no song plays, no storage device changes,
and **no microphone is opened** — opening one would need a new explicit
authorization, which this milestone did not have and did not seek.

## `Update` throws nothing, and that is from the IL

290 bytes, and **zero `throw` instructions**. It calls `PollForEvents`, takes
`Monitor.Enter` on the pending-call list, copies it, releases, and dispatches.
Every branch of the dispatch is a call into a subsystem the native runtime owns.

So there is no managed half to reproduce. This member is a forward, and the
projection says so rather than dressing it up.

## The one place XNA and CNA disagree, and how it is resolved

XNA's `FrameworkDispatcher.Update()` is genuinely static and takes nothing.
`cna_framework_dispatcher_update` takes a game handle, and the header explains
why:

```text
The canonical dispatcher is static and exists for applications that do not run
the game loop; a game handle is taken here only for thread affinity. Calling it
while the loop runs is harmless and does the work twice.
```

A handle taken *only* for thread affinity is not a parameter the XNA surface may
grow, so it is fetched the way every other static XNA member fetches it —
`RuntimeRegistry.current()`, the same route `Keyboard.GetState()` takes. The
consequence is a refusal XNA does not have: outside a lifecycle callback there
is no runtime, so `Update()` raises `callbackOutsideGameLifecycle`. That is the
binding being honest about a capability token it does not hold, rather than
reporting a pump that never happened.

## Falsifiability, and a mutation that was replaced before it was scored

The obvious mutation — skip the route, return success — **would have survived**,
and noticing that before running it is the point. The tests observe that
`Update()` was *accepted*, not that the framework did work, so a projection that
quietly did nothing would pass every assertion. Scoring that mutation as CAUGHT
would have been scoring the harness, not the code.

What *is* observable is that the route was handed the runtime's real handle:
CNA validates it, so a wrong handle turns an accepted pump into a refused one.
The mutation is aimed there.

```text
dispatcher-handle-not-carried
    runtime.functions.frameworkDispatcherUpdate(runtime.gameHandle)
 -> runtime.functions.frameworkDispatcherUpdate(0)
```

**And it caught the test, not the code, on its first run.** The verdict came
back `HUNG` rather than `CAUGHT`: the probe game exited on a *pump* counter, so
a dispatcher that refuses never advances it, `Exit` is never reached, and the
game ran until the harness's 600-second deadline killed it. The projection was
right and the test was the defect — it would have reported a real regression
through a timeout instead of an assertion, ten minutes later. The exit is driven
by frames now, and the same mutation comes back `CAUGHT` in seconds.

That deadline was added two milestones ago after two runs hung. This is the
first time it has paid for itself, and what it found was a bad test.

This is the same shape as Foundation 63's withheld draws and Foundation 67's
`effect-string-ignores-the-written-length`: when a member's *effect* is not
observable, aim at the argument that reaches the route, and say plainly that
the effect is not what is being claimed.

## `TitleContainer` was expected to land here too, and did not

It was sized as the other one-member type. Reading the IL says otherwise:
`OpenStream` is 213 bytes over `GetCleanPath` (256 bytes), `IsCleanPathAbsolute`
(87) and `CollapseParentDirectory` (40) — a complete path-normalisation routine
that is managed XNA behaviour and must be reproduced, not delegated to CNA's own
path handling.

It also needs things this binding does not have yet:

* **A `FileNotFoundException` family.** `OpenStream` raises
  `FileNotFoundException` for the not-found branch, and the exception hierarchy
  here has no `System.IO` member at all. That is a BCL admission, with the
  process that implies.
* **Three resource strings**, now extracted mechanically from the
  hash-registered assembly and ready to admit:
  `InvalidTitleContainerName` = *"Invalid filename. TitleContainer.OpenStream
  requires a relative URI."*, `OpenStreamNotFound` = *"Error loading \"{0}\".
  File not found."*, `OpenStreamError` = *"Error loading \"{0}\". Cannot open
  file."*
* **A decision about a narrowing CNA states outright.** There is no title stream
  handle; `cna_title_container_read_ext` delivers the whole file through a
  count/copy pair, and it reports *every* open failure as `CNA_RESULT_IO`.
  XNA splits that: `FileNotFoundException`/`DirectoryNotFoundException`/
  `ArgumentException` become `FileNotFoundException(OpenStreamNotFound)`, and
  anything else — an access denial, for instance — becomes
  `InvalidOperationException(OpenStreamError)`. **CNA cannot tell them apart**,
  so the projection will have to pick one and record why, and picking silently
  would be the wrong way to do it.

Sized properly it is its own milestone, and it is the next one.
