# Foundation 36 — `System.Type` is the Swift metatype, and `Game.Services` follows

`GameServiceContainer`'s whole state is one `Dictionary<System.Type, object>`,
so it could not be implemented until both halves of that generic argument had a
decided projection. Foundation 31 decided the dictionary; this decides
`System.Type`.

## The decision

```text
System.Type  ->  Any.Type
```

A **language projection**, in the same sense as `System.IntPtr -> Int` and
`System.Object -> Any?`: the Swift type that carries the same value, which here
is a runtime type token.

What made it decidable rather than a guess is a measurement. The pinned
contract names `System.Type` in **twenty-four** positions — `ContentTypeReader`,
the fourteen `Design` converters, three `GameServiceContainer` methods and four
buffer constructors — and **calls a member on it in none of them**. Every one
passes it, compares it or returns it. The selected surface of the family is
therefore identity and assignability, and a Swift metatype has both:

| CLR | Swift |
|---|---|
| `Type` equality | `==` on metatypes |
| `Type.IsAssignableFrom` | `_openExistential(type) { value is $0 }` |

The second was probed before the decision was made, not after: it answers true
for the type itself, for any base class and for any protocol the value
conforms to, and false otherwise — which is exactly what the CLR asks.

What a Swift metatype does **not** have is `System.Type`'s reflection surface —
`Name`, `FullName`, `Assembly`, `GetMethods`. Nothing in the contract needs one.
A wrapper class would have had to either invent those members or be an empty
box; neither is better than the language type, which is why the alternative was
not close.

## `GameServiceContainer`

```text
.ctor()               services = new Dictionary<Type, object>()
AddService(t, p)      t == null -> ArgumentNullException("type", …)
                      p == null -> ArgumentNullException("provider", …)
                      ContainsKey(t) -> ArgumentException(ServiceAlreadyPresent)
                      !t.IsAssignableFrom(p.GetType())
                                -> ArgumentException(ServiceMustBeAssignable …)
                      services.Add(t, p)
RemoveService(t)      t == null -> ArgumentNullException; services.Remove(t)
GetService(t)         t == null -> ArgumentNullException
                      ContainsKey(t) ? services[t] : null
```

The null-**type** branches are unreachable through a non-Optional Swift
metatype. The null-**provider** branch is not, because `System.Object` projects
to `Any?`, so it is reproduced. `RemoveService` discards `Dictionary.Remove`'s
result, so removing an absent type is a no-op. `GetService` returns null rather
than raising, which is why it is Optional.

The assignability check is the one that matters: registering an implementation
under an **interface** type is what every real use of this container does, and
identity alone would have refused it. A test registers one object under a
protocol, a base class and its own type, and refuses two mismatches.

The internal dictionary supplies an explicit `CNAEqualityComparer` — `==` for
equality, `ObjectIdentifier` for the hash — because a metatype is not
`Hashable`. It is a private field, so the comparer is invisible; only the
identity rule it implements is observable, and that rule is the CLR's. It is
also the first real use of the comparer machinery Foundation 31 built.

## One divergence, in a message and not in behaviour

XNA formats `ServiceMustBeAssignable` with `provider.GetType().FullName` and
`type.GetType().FullName`. Note the second: `Type.GetType()` is
`Object.GetType()`, so **XNA's second argument is always `"System.RuntimeType"`**
rather than the service type's name — a defect in XNA.

Neither name is reconstructible here, because `FullName` is not part of the
`Any.Type` projection. The template is reproduced verbatim and pinned; both
arguments carry the Swift type names, which is what the sentence plainly
intends. XNA's own second argument is deliberately **not** reproduced: hard
coding a CLR internal type name into a Swift message would be meaningless.

`ServiceProviderCannotBeNull` is deliberately **not** registered as a
reproduced string. XNA raises the two-argument `ArgumentNullException`, whose
`Message` composes that resource with the parameter name around
`Environment.NewLine`; only the parameter name is carried here, as at every
other `ArgumentNullException` site, and the exception **class** is what the
named payload milestone will project.

## `Game.Services`

`Game..ctor` allocates exactly one container and `get_Services` is a bare field
read, so it is the same object on every read. **Nothing registers anything into
it here**: XNA's own registration happens in `GraphicsDeviceManager..ctor`,
which is a separate member, and an empty container is what a `Game` with no
manager has in XNA too.

It is what `RunGame` asks for the `IGraphicsDeviceManager` before creating the
device, and what `DrawableGameComponent` will ask for the graphics device
service — so this is the member that was blocking that type.

## Scoreboard

```text
TARGET_TYPES=142                     (141 -> 142)
TARGET_MEMBERS=1767                  (1762 -> 1767)
TOTAL_DIAGNOSTICS=269                (271 -> 269)
COMPLETE_TYPES=135                   (134 -> 135)
MISSING_TYPE=115                     (116 -> 115)
MISSING_MEMBER=129                   (130 -> 129, Game.Services resolved)
every mismatch/leak category unchanged
UNMEASURED_STRUCTURAL_CATEGORY=0     ALLOWLIST_ENTRIES=0
XNA_RESOURCE_STRING_PROJECTIONS=4    (2 -> 4)

DEBUG_TESTS=406 PASS                 (399 -> 406)
PURE_XNA_DERIVED 2002/2002/0         (1980 -> 2002)
```
