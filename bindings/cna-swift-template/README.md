# CNA Swift desktop canary

This is one deliberately small XNA-shaped Swift canary. It subclasses
`Microsoft.Xna.Framework.Game`, and CNA's canonical C ABI drives the lifecycle.
It opens the project-owned `Content/logo.png` through the mapped
`Texture2D.FromStream` route, reads the native viewport, clears the real device,
submits a rotating/scaling/moving sprite through SpriteBatch, polls native
keyboard state, and requests native Game exit at an exact Draw callback count.

It also creates one `RenderTarget2D`, holds it as the `Texture2D` it derives
from, binds it and restores the backbuffer -- a public-API demonstration that
the inheritance is real. No pixel is claimed by the canary itself; back-buffer
readback on a windowed renderer is asserted in the binding's own suite.

It also reads the adapter
list and the device's own adapter, the window snapshot, and the two members that
**refuse** -- `GraphicsDevice.Dispose`, because the running game owns the device
and a borrowed handle may not destroy it, and the three-argument
`GraphicsDevice.Present`, because this runtime's only presentation route takes
no rectangle and no window handle. A consumer most needs to see refusals
working, so the canary asserts them rather than avoiding them.

It reads `Game.Content` too, and the three facts a consumer meets first: the
game's manager is **cached** rather than rebuilt per read, a caller can install
their own through `SetContent`, and the two ways a load fails are told apart --
`kindRefused` is an asset kind this runtime has no route for, `missingRefused`
is an asset that is simply not there. **No asset is loaded**, because this
template ships no `.xnb` and a canary that needed one would be testing the
fixture instead of the binding.

The manager's root reads back **empty**: `Game.Content` uses XNA's one-argument
constructor, whose root directory is the empty string. `Content` is also not
Optional -- read outside a Game callback it traps rather than answering nil,
which is why the canary reads it in `LoadContent`.

It runs one real `OcclusionQuery` -- begin, end, wait, read -- and then the two
rules **the binding enforces because CNA does not**: a second `Begin` before the
previous result has been looked at is refused, and a count read from a query
that never finished is refused. This runtime answers a count for a query that
was never begun, so without those two refusals a consumer would read a number
that measured nothing.

It builds and plays a `SoundEffect` too, **from bytes this file writes** -- a
tenth of a second of silence -- because XNA's PCM16 constructor takes a buffer
and this template ships no audio asset. Nothing is audible; what it shows is
that the sound is built, played, controlled through an instance and released,
and that two rules XNA enforces and this runtime does not are enforced by the
binding: the loop flag is fixed at the first `Play`, and a buffer that is not a
whole number of PCM16 frames is refused rather than decoded as a shorter sound.

It also builds a streaming `DynamicSoundEffectInstance` -- from a sample rate
and a channel count, no buffer and no file -- and fills its queue. **This
runtime accepts buffers without limit; XNA refuses past 64**, and the canary
shows the binding enforcing that: `queued=64 pending=64 limitRefused=true`. A
queue that grows without bound fails later and somewhere else.

The last thing it opens is a `Song`, the one Media type reachable without a
media library. **The canary writes its own file and deletes it**: `Song.FromUri`
requires the file to exist -- a URL naming nothing is refused, and the canary
shows that refusal too -- so it writes a minimal PCM16 WAV, opens a song through
it and removes it again. Still no asset ships with this template.

It then opens a `MediaLibrary`, which is the door into the rest of Media, and
reads four of its collections plus the enumeration of media sources. The music
side of this machine's store is empty and the picture side is not, which is
exactly the point: the counts are whatever the machine holds, and what is shown
is that every collection answers. `ownSourceNil=true` is not a failure --
CNA publishes a library's source only as a name, and a name is not a
`MediaSource`, so nil is the honest answer.

Finally it plays the song it wrote through `MediaPlayer`, reads the queue back
and stops. **The queue is process-wide** -- CNA's own wording -- so it outlives
the game that filled it; `queued=1` is the canary's own track and nothing else.

**The canary reads; it never writes.** `MediaLibrary.SavePicture` is not
called, and in this binding it refuses with `NotSupportedException`: the CNA
route it would use writes into the user's own photo album, and CNA publishes no
route to remove the file again. The song it plays is a file it
wrote itself and deletes. And a song
built from a file has **no library context**: CNA reports that as an ordinary
answer, the binding turns it into a refusal because the return is proven
non-null, and `noContextReported=true` is that refusal arriving.

Last it reads the `TouchPanel`, which has no touch device on this host and
answers anyway -- `connected=false maxTouches=0`. It is also the one place a
consumer meets this binding's **only settable properties**: `DisplayWidth` and
`DisplayHeight` have setters that cannot refuse, so they stay properties where
every other fallible setter became a `Set…` method.

Two of the values it prints are worth reading twice. On HEADLESS the window
reports a client area of **0x0** while the device reports a **800x480**
viewport: that is what headless means here, and a canary that quietly used the
window's size instead of the viewport's would draw nothing and say nothing. On
OPENGLES3 the window is real (`client=800x480`, a non-zero handle). The adapter
names itself `\\.\DISPLAY1`; its mode list is the host's (1 under SDL's dummy
video driver, 40 on the private Xwayland display).

The game asks its `GraphicsDeviceManager` for **HiDef** in its constructor,
because XNA constructs an `OcclusionQuery` only under HiDef; under Reach the
query is refused at construction.

There is no XNB, BasicEffect, cube, capability guess, fake banner, synthetic
texture, or Swift-owned frame loop.

One line, printed at exit, is the whole verdict:

```text
CNA_SWIFT_CANARY requested=600 updates=600 draws=600 viewport=800x480
texture=128x128 offscreen=64x64 adapters=1 name=\\.\DISPLAY1 default=true
modes=1 reach=true window=handle=0 client=0x0 resizing=false
device=isDisposed=false disposeRefused=true presentRefused=true content=root=
cached=true installed=true kindRefused=true missingRefused=true query=pixels=1
waited=0 rearmRefused=true earlyCountRefused=true audio=played=true ms=100
state=Stopped queued=64 pending=64 limitRefused=true loopRefused=true
badBufferRefused=true master=1.0 song=name=canary track track=0
missingRefused=true readAfterDisposeRefused=true noContextReported=true
librarySongs=0 libraryArtists=0 libraryPlaylists=0 libraryPictures=89
ownSourceNil=true mediaSources=1 queued=1 playerState=Playing
gameHasControl=true touch=connected=false maxTouches=0 touches=0
display=800x480 gestures=1
```

**`updates` is the one field that is not reproducible, and this file used to
print it as though it were.** Three runs of the same binary at `--frames 600`
answered 600, 601 and 602. `draws` is always exactly `requested`; `updates` is
`requested` or a little more, because CNA runs a fixed time step and catches up
with extra `Update` calls when a frame overruns, without a matching `Draw`.
HEADLESS rarely overruns (600/600 on 2026-09-30); OPENGLES3 on the private
display presents with vsync and does, measured at 85 updates for 60 draws and
910 for 600. Read the line for `updates >= requested`, not for a number.

The canary also **exits 0**. Until Foundation 101 it did not: every run ended

```text
CNA Swift canary failed: CNA operation cna_game_destroy failed with result 3:
All owned C child resources must be destroyed before the game.
```

printed on stderr, after the verdict line, with exit status 1 -- and this file
documented the verdict without mentioning it. The cause was in the binding, not
here: `ContentManager` was the only owned type whose handle was not released
when its Swift object went away, and this canary reads `Game.Content`, installs
its own through `SetContent`, and lets the first one go. That is an ordinary
consumer pattern, which is what made the canary worth running.

`pixels` and `waited` are host facts, not scene measurements: HEADLESS answers
`pixels=1 waited=0` at once, while OPENGLES3 waits on the real GPU for a
begin/end pair with nothing drawn between them and counts `pixels=0`. Either way
the round trip reaches the renderer and comes back.

## Qualified boundary

Linux x86-64 with Swift 6.0.3 and an external CNA C ABI 0.35.0 library built
from CNA `next` is the qualified boundary. The binding admits CNA's own
published consumer window — major `0` exactly, minor `35` or later — so an
earlier generation is rejected by name. macOS, iOS, tvOS, visionOS, Windows,
and Web/Wasm are unqualified.

Re-measured on 2026-09-30 against CNA `next` 5b4edd6cc and CNA-Swift `b24d81b`,
`--frames 60` and `--frames 600`, every run exit 0 with `draws == requested`:

| Consumed as | HEADLESS (SDL dummy drivers) | OPENGLES3 (private Weston + Xwayland) |
|---|---|---|
| sibling path `../cna-swift` | 60/60, 600/600 updates/draws | 87/60, 931/600 updates/draws |
| the binding's exact source archive, extracted read-only | 60/60, 600/600 | 86/60, 912/600 |

A windowed renderer must run on a private display, never the desktop. From
the CNA checkout:
`tools/platform/run_gpu_tests_private.sh --exec env -u WAYLAND_DISPLAY SDL_VIDEODRIVER=x11 SDL_AUDIO_DRIVER=dummy CNA_NATIVE_LIBRARY=... HelloGame --frames 600`.

From this repository root, provide the external library explicitly:

```bash
export SDL_VIDEODRIVER=dummy SDL_AUDIO_DRIVER=dummy   # HEADLESS: no display at all
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so swift run HelloGame --frames 60
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so swift run HelloGame --frames 600
```

The command prints actual Update and Draw callback counts plus native viewport
and decoded texture dimensions. The Swift package dependency is a sibling path
for template development only; the table's second row is the same template
built against the binding's exact `swift package archive-source` output, and
the binding's `tools/package_qualification` builds its own independent consumer
from that archive too.
