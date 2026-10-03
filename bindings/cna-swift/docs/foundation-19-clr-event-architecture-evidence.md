# Foundation 19 — CLR event architecture

Foundation 19 decides and implements the general CLR event/delegate projection,
migrates `System.EventArgs` from a support struct to an open class with a
**measured** base, and proves both on the first five XNA types that depend on
them.

## What was decided

| Question the handoff left open | Decision |
|---|---|
| Event shape | One get-only property per CLR event, keeping the XNA name, of type `CNAEvent<TArgs>`. No `add_`/`remove_`/`raise_` identity. |
| Removal identity | `Add` returns an opaque `CNAEventSubscription`; `Remove` matches on it. Recorded as a `LANGUAGE_PROJECTION` that differs from CLR delegate identity, not as equivalence. |
| Raise encapsulation | A separate `CNAEventSource<TArgs>`, **composed** with `CNAEvent<TArgs>` over private storage, never derived from it. |
| `System.EventArgs` | `open class CNAEventArgs`, not `Sendable`, with `Empty` as one shared instance. Its base is measured. |

The full rules are in `docs/xna-swift-mapping.md`.

## Types completed

Five XNA types, carrying 15 mapped Swift XNA identities:

| Type | Assembly | Identities |
|---|---|---|
| `Microsoft.Xna.Framework.IUpdateable` | Game.dll | 5 |
| `Microsoft.Xna.Framework.IDrawable` | Game.dll | 5 |
| `Microsoft.Xna.Framework.GameComponentCollectionEventArgs` | Game.dll | 2 |
| `Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs` | Graphics.dll | 1 |
| `Microsoft.Xna.Framework.Graphics.ResourceDestroyedEventArgs` | Graphics.dll | 2 |

Each was re-derived from the registered, hash-matched IL rather than from
remembered MonoGame or FNA definitions. `IDrawable` in particular does **not**
extend `IUpdateable`: the two interfaces are independent in the pinned metadata.

The two `Graphics` event-argument types have `assembly`-accessible constructors
only, so they are `final class` projections with no public initializer,
following the `DisplayModeCollection` precedent. Completing them claims no
graphics capability: their only XNA producers are members of the untouched
`GraphicsDevice` partial.

## Types deliberately deferred

| Type | Blocker |
|---|---|
| `PreparingDeviceSettingsEventArgs` | `GraphicsDeviceInformation` → `GraphicsAdapter`, which needs real adapter enumeration. |
| `GameComponentCollection` | Base is `System.Collections.ObjectModel.Collection<IGameComponent>`. |
| `GameComponent` | Depends on the `Game` runtime partial. |
| `DrawableGameComponent` | Depends on the `Game` and `GraphicsDevice` runtime partials. |

`GameComponentCollection` is the one worth spelling out, because its **XNA**
dependencies are now all complete and the dependency graph reports it as
`dependencyComplete=True`. It was still not implemented, for a concrete reason:
the pinned XNA IL supplies only the four `protected` overrides
(`InsertItem`, `RemoveItem`, `SetItem`, `ClearItems`) and the two events. Every
public collection identity a consumer would actually use — `Add`, `Clear`,
`Contains`, `CopyTo`, `Count`, `GetEnumerator`, `IndexOf`, `Insert`, `Item`,
`Remove`, `RemoveAt` — is inherited from `Collection<T>` in mscorlib, which is
not a registered XNA reference assembly. Its ordering, duplicate and
enumeration-invalidation behaviour cannot be derived from XNA IL, and the
project's rule is not to guess conventional .NET collection behaviour. A strict
projection of only the declared members would be a collection with no way to add
to or enumerate it.

That deferral is now **enforced rather than merely intended** — see below.

## Verifier

### Event projection

`System.EventHandler<TArgs>` maps to `CNAEvent<TArgs>` in `map_clr_type`, and an
event's expected mutability is pinned to get-only. Event mismatches route to
`EVENT_MAPPING_MISMATCH` for kind, static/instance identity, projected type and
writability, and a scan reports any `add_`/`remove_`/`raise_` name in the strict
XNA surface as a leaked CLR accessor.

`EVENT_MAPPING_MISMATCH` was a measured category that had never been exercised,
because every event in the contract lived in a missing or partial type. It is
now exercised by 49 declared event projections and 26 self-test mutations.

### Support types

The four support types are parsed from the same Symbol Graph and measured
against a pinned shape in `mapping-rules.json` — kind, `final`/`open`, generic
arity, absence of a superclass, required members with exact signatures, and
forbidden members. Two of those checks carry the architecture:

- `CNAEvent` must not expose `Raise`;
- `CNAEventSource` must not inherit from `CNAEvent`.

Omitting a support type from measurement is itself reported, as
`UNMEASURED_STRUCTURAL_CATEGORY`.

### Measured and undecided bases

`System.EventArgs → CNAEventArgs` joins XNA bases as a measured base. Separately,
any **undecided** non-XNA base — one that is neither a CLR root nor a decided
support projection — now reports `UNMEASURED_STRUCTURAL_CATEGORY` if a type
carrying it is implemented. Twenty-one still-missing types are guarded this way,
`GameComponentCollection` among them, so none of them can be quietly projected
as a base-less Swift class to improve the scoreboard.

### Self-tests

`API_COMPAT_SELF_TESTS` rose from 1,994 to 2,119. The 125 new checks were each
confirmed to bite by disabling the corresponding rule and observing the failure:

| Rule disabled | Self-tests that failed |
|---|---|
| event accessor leak scan | 3 |
| measured support bases | 7 |
| event return-type measurement | 5 |
| event get-only mutability | 2 |
| support-type base check | 1 |
| undecided-base guard | 1 |

Mutations cover: event missing, event renamed, event projected as a bare
closure, as an array of closures, as `CNAEventSource`, as a callback pointer;
writable event property; wrong `TArgs`; wrong static identity; `add_`, `remove_`
and `raise_` leaks; support source leaked as the event property; each support
type omitted from measurement; `CNAEvent` exposing `Raise`; `CNAEventSource`
subclassing `CNAEvent`; wrong token type on both `Add` and `Remove`; a public
native handle and a public raw pointer inside the subscription token; missing
`CNAEventArgs` base; `AnyObject` or `Object` in its place; a struct where the CLR
declares a class; a wrong support base; an extra incompatible base projection.

## Runtime qualification

26 event-runtime tests cover zero/one/multiple handlers, registration order,
duplicate registration with independent tokens, removing the first and the
second duplicate, repeated removal, foreign-event token removal, subscription
and removal during dispatch, snapshot behaviour, self-removal during dispatch,
a throwing first handler, a throwing middle handler, only-the-first-error
propagation, an intact registration list after a thrown dispatch, sender
identity, exact args object identity, `CNAEventArgs.Empty` identity, token
`deinit` not unsubscribing, strong handler retention, and the composition
boundary.

These are Swift/BCL projection facts, so they are deliberately **not** counted
in the pure XNA-derived behaviour corpus. What the corpus gained is the six
XNA-derived contract tests transcribed from the registered IL, including the
`GameComponent` setter pattern — compare, store, then raise with `(this, Empty)`,
and raise nothing when the value is unchanged.

## External conformance

The isolated consumer canary is the part that matters most for this
architecture, because the in-module tests use `@testable import`. It defines
real user types **outside** CNA that own private `CNAEventSource` instances,
publish only `CNAEvent` views, satisfy every `IUpdateable` and `IDrawable`
requirement, raise their own events, subscribe, unsubscribe, mutate state and
observe the results — plus an external subclass of `CNAEventArgs`, proving the
migrated base is open across a module boundary.

A Swift 6.0.3 compiler bug was found and worked around while building it:
`swift-frontend` asserts in IRGen while mangling the debugger type for
`any F.IUpdateable` when the existential is written through a typealias.
Spelling the protocol in full avoids it; the projection itself is unaffected.

## Scoreboard

```text
                        before   after
COMPLETE_TYPES             108     113
PARTIAL_TYPES                5       5
MISSING_TYPE               144     139
TARGET_TYPES               113     118
TARGET_MEMBERS            1641    1656
TOTAL_DIAGNOSTICS          295     290
MISSING_MEMBER             131     131
EVENT_MAPPING_MISMATCH       0       0
BASE_MAPPING_MISMATCH        2       2
INTERFACE_MAPPING_MISMATCH   1       1
PROPERTY_MAPPING_MISMATCH    1       1
OVERLOAD_MAPPING_MISMATCH   16      16
RAW_HANDLE_LEAK              0       0
PUBLIC_NATIVE_FFI_LEAK       0       0
UNMEASURED_STRUCTURAL_CAT    0       0
ALLOWLIST_ENTRIES            0       0
API_COMPAT_SELF_TESTS     1994    2119
DEBUG_TESTS                186     218
BEHAVIOR_ASSERTIONS       1595    1628
EVENT_PROJECTIONS            —      49
EVENT_SUPPORT_TYPE_MEAS      —       4
MEASURED_SUPPORT_BASES       —       4
NONPUBLIC_CONSTRUCTION       5       7
```

`MISSING_MEMBER` held at 131 exactly as required: no event was added to `Game`,
`GraphicsDeviceManager` or `GraphicsDevice`. Those events need real lifecycle and
native raising, and an event that never fires is not implemented.

## ABI

Unchanged. This milestone is pure Swift support machinery and adds no native
surface:

```text
BOUND_FUNCTIONS=29 PROTOTYPE_TYPE_POSITIONS=91 C_SWIFT_MEASUREMENTS=91
LAYOUTS=18 CALLBACKS=2 CONSTANTS=214
MISSING_HEADER_SYMBOLS=0 MISSING_LIBRARY_SYMBOLS=0 ABI_MISMATCHES=0
```
