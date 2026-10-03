# The dependency and capability frontier, re-measured on CNA 0.21.0

Every native blocker recorded while the boundary was pinned to CNA C ABI 0.7.0
is re-measured here against the current C API. A blocker that CNA has since
answered is not a blocker; carrying one forward unexamined would be the mistake
this document exists to prevent.

Route names below are read out of
`cnanext/modules/c-api/include/CNA/C/*.h` at HEAD `0a6158e4f`, whose headers are
byte-identical to the admitted artifact's. **Route existence is not capability**:
a route that exists still has to be bound, projected against pinned XNA, and
qualified before anything is claimed.

## Structural frontier

`tools/api_compat/dependency_graph.py` on the current report:

```text
DEPENDENCY_COMPLETE_MISSING_TYPES=16
 reach=50  Graphics.GraphicsAdapter            directMissing=1  directPartial=1
 reach=25  Graphics.EffectAnnotation           directMissing=1
 reach=12  Design.MathTypeConverter            directMissing=12
 reach= 7  Content.ContentManager              directMissing=2  directPartial=1
 reach= 3  Audio.AudioCategory                 directMissing=1
 reach= 3  Audio.RendererDetail                directMissing=1
 reach= 2  Audio.SoundEffectInstance           directMissing=2
 reach= 2  GameWindow                          directPartial=1
 reach= 1  Audio.Cue                           directMissing=1
 reach= 1  Media.MediaSource                   directMissing=1
 reach= 0  Audio.Microphone, FrameworkDispatcher, Graphics.SpriteFont,
           Input.Mouse, Input.Touch.TouchPanel, TitleContainer
```

## Historical native blockers, re-measured

| Blocker, as recorded under CNA 0.7.0 | Current CNA | Classification now |
|---|---|---|
| `Game.Tick` | `cna_game_tick` | ACTIONABLE_LOCAL |
| `Game.SuppressDraw` | `cna_game_suppress_draw` | ACTIONABLE_LOCAL |
| `Game.ResetElapsedTime` | `cna_game_reset_elapsed_time` | ACTIONABLE_LOCAL |
| `Game.IsFixedTimeStep` | `cna_game_get/set_is_fixed_time_step` | ACTIONABLE_LOCAL |
| `Game.TargetElapsedTime` | `cna_game_get/set_target_elapsed_time_ticks` | ACTIONABLE_LOCAL |
| `Game.IsMouseVisible` | `cna_game_get/set_is_mouse_visible` | ACTIONABLE_LOCAL |
| `Game.InactiveSleepTime` | `cna_game_get/set_inactive_sleep_time_ticks` | ACTIONABLE_LOCAL |
| `Game.IsActive` | `cna_game_get_is_active` | ACTIONABLE_LOCAL |
| `Game.Activated/Deactivated/Exiting/Disposed` | `cna_game_subscribe`/`_unsubscribe` over `CNA_GAME_EVENT_ACTIVATED/DEACTIVATED/DISPOSED/EXITING` | ACTIONABLE_LOCAL, after the raise sites are measured |
| `Game.Window` / `GameWindow` | 20 `cna_game_window_*` routes incl. `_subscribe`, `_get_client_bounds`, `_copy_title`, `_get_allow_user_resizing`, the screen-device-change pair | ACTIONABLE_LOCAL for structure; windowed evidence needs a renderer |
| `Game.Content` / `ContentManager` | 32 `cna_content_manager_*` routes plus `cna_game_get/set_content_manager_ext` | ACTIONABLE_LOCAL |
| `RenderTarget2D` | `cna_render_target2d_create`, `cna_render_target_get_info`, `cna_render_target_destroy`, `cna_graphics_device_set_render_target2d`, `cna_render_target_subscribe_content_lost` | ACTIONABLE_LOCAL |
| `IGraphicsDeviceService` producer | `cna_graphics_device_manager_get_graphics_device`, `cna_graphics_device_manager_subscribe`, and `CNA_GRAPHICS_DEVICE_EVENT_DISPOSING/DEVICE_LOST/DEVICE_RESET/DEVICE_RESETTING` | ACTIONABLE_LOCAL, after the two-container question is measured |
| `GraphicsDeviceManager`'s preference surface | 34 `cna_graphics_device_manager_*` routes covering every XNA preference property, `ToggleFullScreen`, `CreateDevice`, `BeginDraw`/`EndDraw` and `PreparingDeviceSettings` | ACTIONABLE_LOCAL |
| `GraphicsAdapter` | 22 `display.h` routes: adapter count, description, device name, display modes, current mode, profile support, format queries | ACTIONABLE_LOCAL |
| `Input.Mouse` | 20 `cna_mouse_*` routes | ACTIONABLE_LOCAL |
| `Graphics.SpriteFont` | 9 `cna_sprite_font_*` routes | ACTIONABLE_LOCAL, needs a font asset |
| Audio (`SoundEffect`, `SoundEffectInstance`, …) | 74 `audio.h` routes; the qualified artifact is built `CNA_AUDIO_PLATFORM=SDL3`, not `NULL` | ACTIONABLE_LOCAL for structure and state; playback needs a legally usable fixture |
| Media / video | 39 `media.h`, 41 `media_player.h`, 42 `video.h`, 148 `media_library.h` routes; the artifact links ffmpeg | ACTIONABLE_LOCAL for structure; playback needs a fixture |
| Canonical CNA HEAD C API build (Foundation 11) | resolved: the C API builds at HEAD and the admitted artifact is byte-identical to that build tree | RESOLVED |
| `NULL` audio backend | resolved: the qualified artifact is SDL3 | RESOLVED |
| Visible window / pixels | still HEADLESS | BLOCKED_RENDERER for this artifact |
| Attached controller | none on this host | BLOCKED_HARDWARE |

## What did not become actionable

- **Serialization constructors** on `ContentLoadException` and
  `StorageDeviceNotConnectedException` remain blocked. `System.Type` is decided,
  but the protected `(SerializationInfo, StreamingContext)` shape needs
  `IDictionary` and a deserialization runtime, and Swift has no `protected`, so
  a projection would be publicly callable and would carry none of the serialized
  state. SWIFT_LANGUAGE_LIMITATION plus BLOCKED_UPSTREAM (no CLR runtime).
- **`Exception.Data`, `StackTrace`, `Source`, `TargetSite`, `ToString`,
  `GetObjectData`** need CLR runtime services and stay forbidden by the
  verifier rather than answered with something plausible.
- **`Audio.RendererDetail.GetHashCode`** still depends on
  `System.String.GetHashCode()`, which is implementation-defined and not
  derivable from IL. DELIBERATE_OUT_OF_SCOPE.
- **Content Pipeline design-time assemblies** stay outside the runtime profile.
