# Pure behavior corpus

The corpus executes only deterministic `PURE_XNA_DERIVED` observations. CNA is
not a golden source for these value semantics, and no native library is needed.
Selected exact Float results are compared by binary32 bit pattern; semantic
predicates cover NaN and value/object behavior where a single bit pattern is
not the reference contract.

GamePad value groups cover both enums, the Int32 OptionSet, physical button and
DPad values, trigger/thumbstick special floats, both state constructors, every
physical and virtual button query, combinations, connection/packet equality,
all 26 managed capability properties, and XNA vibration input quantization.
Canonical controller observations are intentionally emitted by the separate
`tools/gamepad_native` runner and never counted as pure XNA-derived behavior.

The `DISPLAY_ORIENTATION` group records only the pinned XNA flags, Int32
underlying type, and four literal values. Swift `OptionSet` union,
intersection, arbitrary-bit, and copy qualification lives in a separate test
case and is not counted as XNA runtime behavior.

The `DEPTH_FORMAT` group records only the pinned non-flags enum, Int32
underlying type, and complete four-value literal table. Swift raw-value
initialization, unknown-value rejection, and copy qualification remain in a
separate projection test and are not counted as XNA runtime behavior.

The `RENDER_TARGET_USAGE` group records only the pinned non-flags enum, Int32
underlying type, and complete three-value literal table `DiscardContents=0`,
`PreserveContents=1`, and `PlatformContents=2`. Swift raw-value initialization,
unknown-value rejection, and copy qualification remain in a separate projection
test and are not counted as XNA runtime behavior. The literal names describe
XNA's render-target preservation policy and are not evidence that any discard,
preserve, or platform-defined content behavior is implemented.

The `SURFACE_FORMAT` group likewise records only the pinned non-flags enum,
Int32 underlying type, and complete twenty-value literal table. Swift raw-value
initialization, unknown-value rejection, and copy qualification remain in a
separate projection test and are not counted as XNA runtime behavior.

The four `DISPLAY_MODE_*` groups record only pinned XNA facts for the managed
`DisplayMode` descriptor: verbatim width/height/format storage through the
non-public constructor, the exact binary32 `AspectRatio` bit patterns including
the zero-dimension short circuit, the unmodified Windows `TitleSafeArea`
rectangle, and the exact `ToString` strings for every pinned SurfaceFormat
literal. Swift class-reference identity, internal-only construction, immutable
public state, and Rectangle value semantics remain in a separate projection
test and are not counted as XNA runtime behavior.

## The BCL half is counted separately

`Foundation27ContractTests` observes the two `System.Collections.ObjectModel`
collection families, and its authority is the admitted Microsoft .NET Framework
4.0 `mscorlib` recorded in `tools/api_compat/bcl-authorities.json` — **not** an
XNA assembly. Those observations run in the same suite and are held to the same
standard, but they are reported as `BCL_OBSERVATIONS` under the separate
`PURE_BCL_DERIVED` authority and are counted in no XNA total. Folding them into
`OBSERVATIONS` would relabel BCL behaviour as XNA behaviour.

`Foundation28ContractTests` is the opposite case and belongs in the XNA corpus:
`GameComponentCollection`'s constructor, its four overrides, its two events and
both of its exception messages were read from the registered
`Microsoft.Xna.Framework.Game.dll`, the messages out of that assembly's own
`Microsoft.Xna.Framework.Resources.resources` blob.
