# Foundation Milestone 7 — DisplayOrientation evidence

## Exact closure and authority

Foundation Milestone 7 closes exactly one public XNA type:
`Microsoft.Xna.Framework.DisplayOrientation`. It does not add or change
`GraphicsDeviceManager.SupportedOrientations`, `GameWindow`, platform rotation,
screen detection, SDL integration, or another missing XNA type.

Public shape comes from the pinned Microsoft XNA Framework 4.0 Windows runtime
contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
The contract was independently checked against the pinned Microsoft runtime
assembly with SHA-256
`38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`.
Its metadata identifies a sealed `System.Enum`, an `Int32` `value__` field,
`System.FlagsAttribute`, and the four literal values below.

| Identity | CLR value |
|---|---:|
| `value__` | `System.Int32` storage |
| `Default` | 0 |
| `LandscapeLeft` | 1 |
| `LandscapeRight` | 2 |
| `Portrait` | 4 |

The CLR contract therefore contains five identities. The established enum
storage language mapping excludes synthetic `value__`, leaving exactly four
expected Swift XNA identities. There are no declared methods, properties,
constructors, events, or operators.

## Swift flags projection

The established CNA-Swift `[Flags]` rule maps the CLR enum to a Swift
`OptionSet`, exactly as for `Buttons` and `SpriteEffects`. The type is declared
in the root `Microsoft.Xna.Framework` namespace and its `rawValue` and
`init(rawValue:)` use `Int32`, matching the CLR underlying type.

`Default` uses the repository's compiler-warning-free empty `OptionSet`
spelling and has raw value zero. The other three constants retain visually
explicit raw values 1, 2, and 4. The compiler Symbol Graph records the struct,
the `OptionSet` conformance, `rawValue: Int32`, `init(rawValue: Int32)`, and all
four constants.

`rawValue`, the raw-value initializer, and inherited `OptionSet` operations are
Swift language machinery. The verifier recognizes the compiler-synthesized
surface and does not count it as XNA members. Consequently
`UNEXPECTED_MEMBER=0` and no custom flag operator or convenience XNA identity
is introduced.

## Projection qualification

The separate Swift projection test establishes:

- `DisplayOrientation(rawValue: 0)` and `Default` have the same zero bit set;
- unions `1|2`, `1|4`, `2|4`, and `1|2|4` produce 3, 5, 6, and 7;
- intersecting `1|4` with `LandscapeLeft` retains bit 1;
- intersecting `1|4` with `LandscapeRight` produces raw zero;
- arbitrary raw values 8 and -1 retain their exact `Int32` bit patterns;
- assignment and later mutation of a copy preserve ordinary Swift value
  semantics.

These are qualifications of the Swift language projection, not new XNA
runtime observations. No validation mask or normalization is applied, matching
the established `Buttons` policy.

## XNA-derived behavior evidence

The compact `DISPLAY_ORIENTATION` pure group contains only the pinned XNA
contract facts: flags status, `Int32` underlying storage, and literal values
0, 1, 2, and 4. The generic Swift union, intersection, arbitrary-bit, and copy
tests are deliberately outside the `PURE_XNA_DERIVED` counter so they are not
mischaracterized as reference-runtime behavior.

DisplayOrientation is a managed value and all of its tests pass with
`CNA_NATIVE_LIBRARY` unset. It does not load `RuntimeRegistry`,
`NativeFunctions`, `CNAShim`, `Game`, or `GraphicsDevice`.

## Structural result

| Measurement | Result |
|---|---:|
| CLR identities | 5 |
| Expected Swift XNA identities | 4 |
| Target Swift XNA identities | 4 |
| Local diagnostics | 0 |

Every local category is zero: missing member; type kind; base; interface;
field; property; method signature; parameter; return; overload; generic; enum
value; flags; event; operator; ref/out; language mapping; and all leak/safety
categories. The verifier's targeted negative fixtures reject a missing type,
ordinary-enum mapping, `UInt32` raw type, every wrong literal, missing
`Portrait`, wrong Graphics namespace, missing flags metadata, an invented
member, and incorrectly requiring `value__` as Swift public API.

The formal whole-profile mapping counters remain unchanged. In particular,
the contract already contributed its `value__` identity to the 49 enum-storage
exclusions before implementation, so implementing the target does not change
that count.

## Native and runtime boundary

No CNA ABI or native implementation participates. Bound functions, prototype
positions, C/Swift measurements, layouts, callbacks, and constants remain
29, 91, 91, 18, 2, and 214. `CNAShim`, `NativeManifest`, `NativeFunctions`, CNA
source, and the maintained template source are unchanged.

Runtime capability is limited to `DisplayOrientation: VERIFIED_MANAGED`. This
does not claim orientation switching, portrait or landscape display modes,
screen rotation, mobile rotation, `GraphicsDeviceManager` orientation support,
or `GameWindow` orientation support. Those APIs and every platform integration
remain deferred.
