# Removed scaffold public-surface audit

`CURRENT_KIND` and behavior refer to initial commit `db329d8`. `XNA_KIND`
comes from the pinned XNA contract. Every row was removed or replaced; no old
flat alias remains callable.

| TYPE | MEMBER | CURRENT_KIND | XNA_KIND | CURRENT_BEHAVIOR | REAL | PLACEHOLDER | WRONG_SHAPE | DECISION |
|---|---|---|---|---|---:|---:|---:|---|
| CnaError | nativeUnavailable | enum/case | CNA support, not XNA | unused generic error | no | yes | yes | REPLACE with `CNAError` outside XNA |
| GameTime | elapsed,total,init | flat struct | XNA class | fabricated `Double` seconds | no | yes | yes | REPLACE with strict class and exact `Duration` ticks |
| Game | type | flat protocol | XNA class | protocol defaults | no | yes | yes | REPLACE with strict `open class` |
| Game | Initialize | protocol/default | protected virtual method | no-op | no | yes | yes | REPLACE with callback-driven open method |
| Game | LoadContent | protocol/default | protected virtual method | no-op | no | yes | yes | REPLACE with callback-driven open method |
| Game | Update | protocol/default | protected virtual method | no-op | no | yes | yes | REPLACE with native GameTime callback |
| Game | Draw | protocol/default | protected virtual method | no-op | no | yes | yes | REPLACE with native GameTime callback |
| Game | UnloadContent | protocol/default | protected virtual method | no-op | no | yes | yes | REPLACE with shutdown callback |
| Game | Exit | protocol/default | method | called `exit(0)` | no | yes | yes | REPLACE with `cna_game_request_exit` |
| Vector2 | X,Y,init | flat struct | XNA struct | stored values | partial | no | yes | REPLACE under namespace; measured partial |
| Vector2 | zero | static property | `Zero` | correct value, wrong name/path | partial | no | yes | REPLACE with `Zero` |
| Vector3 | all | flat struct | XNA struct | tiny incomplete shell | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| Matrix | m,init,identity | flat struct/array | XNA struct/M11…M44 | public nested array, identity | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| Matrix | createScale | static method | `CreateScale` overload | only one overload | partial | yes | yes | REMOVE_UNTIL_COMPLETE CLOSURE |
| Matrix | createRotationX | static method | `CreateRotationX` | identity | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| Matrix | createRotationY | static method | `CreateRotationY` | identity | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| Matrix | createTranslation | static method | `CreateTranslation` | identity | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| Matrix | createLookAt | static method | `CreateLookAt` | identity | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| Matrix | createPerspectiveFieldOfView | static method | same PascalCase | identity | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| Matrix | operator * | operator | XNA operator | identity | no | yes | no | REMOVE_UNTIL_IMPLEMENTED |
| Color | R,G,B,A,init | flat struct | XNA struct | stored bytes | partial | no | yes | REPLACE with packing/clamping and strict path; partial |
| Color | white,black,cornflowerBlue | static property | PascalCase properties | right common values, wrong names | partial | no | yes | REPLACE with `White`, `Black`, `CornflowerBlue` |
| GraphicsCapability | type/threeD | enum | no XNA type | invented capability | no | no | yes | REMOVE |
| Viewport | X,Y,Width,Height | flat struct | XNA struct | caller/synthetic values | no | yes | yes | REPLACE with strict partial native value |
| GraphicsDevice | Viewport | flat class/property | XNA class/property | hard-coded 1280x720 | no | yes | yes | REPLACE with callback-borrowed native query |
| GraphicsDevice | Clear | method | XNA method | no-op | no | yes | yes | REPLACE with CNA clear |
| GraphicsDevice | SupportsCapability | method | no XNA member | always true | no | yes | yes | REMOVE |
| GraphicsDeviceManager | GraphicsDevice | flat synthetic property | XNA class/property | constructed fake device | no | yes | yes | REPLACE with Game-owned native manager/device |
| GraphicsDeviceManager | init(game:) | init | XNA constructor | ignored Game | no | yes | yes | REPLACE with CNA manager create |
| GraphicsDeviceManager | ApplyChanges | method | XNA method | no-op | no | yes | no | REPLACE with CNA apply |
| SpriteBatch | init(device:) | init | XNA constructor | ignored device | no | yes | yes | REPLACE with native create |
| SpriteBatch | Begin | method | XNA overload family | no-op | no | yes | no | REPLACE selected overload with CNA begin |
| SpriteBatch | End | method | XNA method | no-op | no | yes | no | REPLACE with CNA end |
| SpriteBatch | Draw | method | XNA overload | no-op | no | yes | yes | REPLACE two exact real overloads; type remains partial |
| SpriteBatch | DrawRect | method | no XNA member | no-op | no | yes | yes | REMOVE |
| Texture2D | Width,Height | flat class/properties | inherited/XNA properties | constants 1x1 | no | yes | yes | REPLACE with native decoded info |
| Texture2D | init() | init | no matching constructor | synthetic object | no | yes | yes | REMOVE; add real `FromStream` path |
| ContentManager | init | flat class | XNA class | empty shell | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| ContentManager | Load<T> | generic method | XNA generic method | always nil | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| BasicEffect | World,View,Projection | flat class/properties | XNA properties | identity storage | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| BasicEffect | TextureEnabled,Texture | properties | XNA properties | fake Swift state | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| BasicEffect | init(device:),Apply | init/method | XNA constructor / no public Apply | ignored/no-op | no | yes | yes | REMOVE_UNTIL_IMPLEMENTED |
| Keys | escape | flat enum case | XNA enum `Escape=27` | invented ordinal 0 | no | yes | yes | REPLACE with all 160 exact enum constants |
| KeyboardState | IsKeyDown | flat struct/method | XNA struct/method | always false | no | yes | yes | REPLACE with real native bitset/value behavior |
| Keyboard | GetState | flat static method | XNA static method | always empty | no | yes | yes | REPLACE with CNA input query |
| global | Run(game:) | free function | no XNA member | printed one line | no | yes | yes | REMOVE; `Game.Run()` calls CNA |

The empty implementations on `Game` now are only XNA's real base lifecycle
defaults. They are reached by CNA callbacks and are override points, not fake
runtime operations.
