# The blocked list, re-measured against CNA 0.21.0

`NEXT.md`'s **BLOCKED** section, written at Foundation 57, listed
`GraphicsAdapter`, `DisplayMode`, `GraphicsDeviceInformation`,
`PreparingDeviceSettingsEventArgs`, `GameWindow`, `GraphicsDevice.Present`,
`Reset` and `GetBackBufferData`, and six deferred `Texture2D` messages waiting
on `GraphicsDevice.GraphicsProfile`. Every one of those is re-measured here
against the qualified HEADLESS artifact, because `NEXT.md`'s own rule 4 says a
blocker is worth exactly what its last measurement is worth, and three of these
had never been measured at all — they were inferred from "HEADLESS has no
window".

`build-probe/f60_devicecaps.c` and `build-probe/f60_getters.c`, against
`~/deps/cna-c-abi-0.21.0/libcna_c_api.so`, inside a real `LoadContent` callback.

```text
cna_graphics_device_get_graphics_profile        -> 0   profile = 0 (Reach)
cna_graphics_device_get_adapter_index           -> 0   index   = 0
cna_graphics_device_get_display_mode            -> 0   800x480, format 0
cna_graphics_device_get_presentation_parameters -> 0   back buffer 800x480
cna_graphics_device_get_backbuffer_info         -> 0
cna_graphics_device_get_vertex_buffer_count     -> 0   count = 0
cna_graphics_device_get_render_target_count     -> 0   count = 0
cna_graphics_device_get_texture(pixel, slot 0)  -> 0   bound = false
cna_graphics_device_present                     -> 0
cna_graphics_device_reset                       -> 0

cna_graphics_device_get_graphics_profile(device, NULL)   -> 1  INVALID_ARGUMENT
cna_graphics_device_get_graphics_profile(bad, &profile)  -> 2  INVALID_HANDLE
```

**Six things on the blocked list are not blocked.** The device publishes its
profile, its adapter index, its display mode, its presentation parameters, its
bound-buffer and render-target counts, and its texture slots, and it accepts
`present` and `reset`. Route existence is not capability and none of this is
claimed until it is bound, projected against the pinned IL and qualified — but
none of it is *blocked*, and the six deferred `Texture2D` profile messages in
`recorded-message-absences.json` name `GraphicsDevice.GraphicsProfile` as their
only blocker.

## One thing on the blocked list is still blocked

Back-buffer readback. The size query answers and the read does not:

```text
cna_graphics_device_get_backbuffer_data_rgba8(device, NULL, 0, &needed)
    -> 14 (CAPACITY)  needed = 384000        (= 800 x 480)
cna_graphics_device_get_backbuffer_data_rgba8(device, pixels, 384000, &written)
    -> 6 (NOT_SUPPORTED)
```

and it stays `NOT_SUPPORTED` after a `cna_game_clear` that itself succeeds. So
Foundation 53's first bounding fact holds exactly as written: **no test here
can assert that a pixel ended up anywhere.** What is new is only that the
*size* of the thing that cannot be read is knowable.

## A probe that lied, and why it is recorded

The first version of `f60_devicecaps.c` printed the call's result and the
out-parameter in one `printf`:

```c
printf("get_graphics_profile -> %u profile=%u\n",
       (unsigned)cna_graphics_device_get_graphics_profile(device, &profile),
       (unsigned)profile);
```

C does not order the evaluation of argument expressions, and this compiler read
`profile` **before** making the call, so three getters appeared to return
`CNA_RESULT_SUCCESS` while leaving the caller's variable untouched — which
would have been a serious upstream defect. `f60_getters.c` calls each route on
its own line into two variables with different sentinels, and every one of them
writes.

A probe is a measuring instrument, and this one was miscalibrated. The rule it
earns: **never read an out-parameter in the same expression that fills it.**

## What this changes

| Previously recorded | Now |
|---|---|
| `GraphicsDevice.GraphicsProfile` unavailable, six `Texture2D` messages deferred behind it | the route answers; the messages are ordinary work |
| `GraphicsAdapter` blocked by a HEADLESS host | the device names its adapter index, and `cna_graphics_adapter_*` queries take it |
| `DisplayMode` blocked | the device answers 800x480 |
| `GraphicsDevice.Present`, `Reset` blocked | both accepted |
| `GraphicsDevice.PresentationParameters` has no infallible source | a route answers; whether it can serve an `IL_NO_FAILURE_PATH` getter is a separate question about *when* it is read, not about whether the value exists |
| `TextureCollection` blocked by a missing kind discriminator | CNA's own header prescribes the fix: *"cache what you bind and answer from the cache, and use `bound` to tell 'something else owns this slot now' from 'the slot is empty'"* — which is what XNA's `DeviceResourceManager` cache does |
| back-buffer readback blocked | still blocked, and now measured rather than inferred |

## `GameWindow` is not blocked either

`NEXT.md` records it as blocked because "no window exists under HEADLESS". CNA
publishes nineteen `cna_game_window_*` routes, every one of them taking the
**game** handle rather than a separate window handle, and the header states the
headless answer rather than leaving it to be guessed: *"A session with no native
window answers the bounds the window object was last given, which in a headless
tree is the empty rectangle it started with."*

`build-probe/f60_window.c`:

```text
get_client_bounds               -> 0   (0,0 0x0)
get_allow_user_resizing         -> 0   false
set_allow_user_resizing(true)   -> 0
get_allow_user_resizing         -> 0   true          <- round trips
get_current_orientation         -> 0   0 (Default)
copy_title                      -> 0   ""
cna_game_set_window_title       -> 0
copy_title                      -> 0   "probe"       <- round trips
copy_screen_device_name         -> 0   ""
copy_type_name                  -> 0   "Microsoft.Xna.Framework.GameWindow"
begin_screen_device_change      -> 0
end_screen_device_change        -> 0
```

The window object exists, names itself, carries state that survives a write,
and reports the empty client rectangle a headless session has. That is not a
missing capability; it is a window with no pixels, which is what HEADLESS
means. `GameWindow` and `Game.Window` are ordinary work.

The three window events — `ClientSizeChanged`, `OrientationChanged`,
`ScreenDeviceNameChanged` — have `cna_game_window_subscribe` behind them.
Whether HEADLESS ever *raises* one is a separate measurement, and the honest
answer for a size that never changes is likely no; a subscription that is made
and released is still verifiable, exactly as `RenderTarget2D.ContentLost` is.

**The first run of this probe repeated the mistake recorded above**, reading
`allowUserResizing` and `orientation` in the same `printf` that filled them,
and reported both untouched. One call per statement is the rule, and it took
two probes to learn it once.

## Every remaining missing type, against CNA's route inventory

4,076 distinct `cna_*` symbols are declared in the canonical headers. Mapping
each of the 100 missing types onto its route family
(`docs/generated/cna-route-map.txt`) leaves **no family without native
support** except the ones that need none:

| Missing type family | Routes |
|---|---|
| `Effect` and the effect graph | 140 |
| `Model` and its collections | 134 |
| `Media` (`Picture`, `Song`, `MediaPlayer`, `Album`, …) | 25-45 each |
| `SoundEffect`, `AudioEngine` and XACT | 20-42 |
| `ContentManager`, `ContentReader` | 34, 20 |
| `Mouse`, `TouchPanel` | 27, 21 |
| `StorageContainer`, `StorageDevice` | 24, 14 |
| `GameWindow` | 19 |
| `BasicEffect` and the four stock effects | 17-21 each |
| `GraphicsDeviceInformation` | 4 |
| `TextureCube`, `RenderTargetCube`, `VertexBufferBinding` | 1 family each |
| the 13 `Design` converters, the four `Enumerator`s, `IEffectLights`, `RenderTargetBinding`, `TextureCollection`, `PreparingDeviceSettingsEventArgs`, `ResourceContentManager`, `ContentTypeReader<T>` | **none needed** — pure managed |

`Texture3D`'s routes are `cna_texture_volume_*` and `TextureCollection`'s are
`cna_graphics_device_{get,set,unbind}_texture`, which the naming sweep above
does not match on name alone.
