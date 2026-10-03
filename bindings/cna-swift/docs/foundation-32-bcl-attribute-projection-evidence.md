# Foundation 32 — `System.Attribute`, where the base carries identity rather than members

The five `Microsoft.Xna.Framework.Content.ContentSerializer*` types are all
declared on `System.Attribute`. Unlike the collection, exception and dictionary
families, almost nothing usable is inherited here — what the base carries is
the **identity**. Dropping it would have left five ordinary classes a consumer
could not treat uniformly as attributes, and it would have been the silent
base-dropping the Foundation 27 rule exists to stop.

```text
System.Attribute  ->  CNAAttribute
```

## What a CLR attribute mostly is, and why almost none of it is projected

`System.Attribute`'s public surface is overwhelmingly reflection: eight
`GetCustomAttribute` overloads, sixteen `GetCustomAttributes` overloads and
eight `IsDefined` overloads over `MemberInfo`, `Assembly`, `Module` and
`ParameterInfo`, plus `Equals`, `GetHashCode` and `Match` — which compare two
attributes field by field *through reflection* — and `TypeId`, which returns
`GetType()`.

None of that is reconstructible without `System.Reflection` and `System.Type`,
so every one is **forbidden** by the verifier rather than answered with
something plausible, exactly as on the exception and dictionary families. A
`GetHashCode` that hashed the Swift object identity would compile and would be
a different function presented as the same member.

What is left, and what is projected, is the constructor and
`IsDefaultAttribute()`, whose base body is `ldc.i4.0; ret` — a plain `false`.
None of the five XNA types overrides it.

## The one widening, recorded rather than hidden

`mscorlib` declares `System.Attribute` **abstract** with a **protected**
constructor: a consumer may derive from it but may not construct one. Swift has
neither `abstract` nor `protected`, so `CNAAttribute()` compiles where
`new Attribute()` does not.

That is stated rather than papered over with an invented runtime trap, and it
is measured: the support contract carries `clrAbstract`, the verifier records
`swiftConstructionWidened` on the type's evidence, and
`BCL_ABSTRACT_BASE_WIDENINGS=1` counts it. A test asserts the widening exists,
so it is a stated fact rather than something a reader discovers.

## The five XNA types

Re-derived from `Microsoft.Xna.Framework.dll` (`38e7093f…a130`). All five are
`sealed`, so all five are Swift `final`.

| type | shape |
|---|---|
| `ContentSerializerAttribute` | 6 settable knobs, a derived name, a flag, a `Clone` |
| `ContentSerializerCollectionItemNameAttribute` | one validating constructor, one get-only name |
| `ContentSerializerIgnoreAttribute` | a constructor and nothing else |
| `ContentSerializerRuntimeTypeAttribute` | one validating constructor, one get-only name |
| `ContentSerializerTypeVersionAttribute` | one constructor that validates nothing |

### The `CollectionItemName` trio is three different things

The most interesting behaviour in the family, and the one a plausible
reimplementation collapses:

```text
get_CollectionItemName   IsNullOrEmpty(field) ? "Item" : field
set_CollectionItemName   IsNullOrEmpty(value) -> ArgumentNullException("value")
                         field = value
get_HasCollectionItemName !IsNullOrEmpty(field)
```

- the **getter** substitutes a default for an unset field, so it never returns
  null or empty — which is why it projects as a non-Optional `String`;
- the **setter** *refuses* the very values the getter substitutes for;
- `HasCollectionItemName` reports the **raw field**, so it is `false` on a
  fresh instance even though `CollectionItemName` already answers `"Item"`.

Setting `"Item"` explicitly is a real store, so the flag flips while the
reported name does not change. There is a test for exactly that.

Deriving this getter is what turned up the nullability analyser's
`IsNullOrEmpty` blind spot, fixed in its own commit: the member was
`PROVEN_NULLABLE` and would have been projected `String?` for a state XNA
cannot produce.

### `Clone` copies fields, not properties

```text
Clone()  new ContentSerializerAttribute(); copy the six FIELDS directly
```

So a clone of an instance whose `collectionItemName` was never set carries the
null field on and its `HasCollectionItemName` is `false` too. Cloning through
the properties — the obvious implementation — would have turned the substituted
`"Item"` into a stored value and flipped the flag on the copy.

### Defaults

`.ctor()` sets `allowNull = true` **before** calling `Attribute::.ctor()` and
leaves every other field at its zero value. `AllowNull` therefore starts `true`
and `FlattenContent`, `Optional` and `SharedResource` start `false` — a default
a reimplementation gets wrong by making the four uniform.

### Validation

`ContentSerializerCollectionItemNameAttribute` and
`ContentSerializerRuntimeTypeAttribute` both raise `ArgumentNullException` for a
null **or empty** argument before storing it, which is why both their
properties project non-Optional. `ContentSerializerTypeVersionAttribute`
validates nothing at all, so a negative version is accepted exactly as XNA
accepts it.

`ContentSerializerRuntimeTypeAttribute.RuntimeType` carries the
assembly-qualified type NAME as a string and nothing here resolves it: doing
that would need `System.Type`, and inventing a resolution would be worse than
carrying the string XNA carries.

`CNAError` gained `argumentNull`, which reports the parameter name only. The
CLR composes `ArgumentNullException.Message` from two resource strings around
`Environment.NewLine`; reproducing that would mean asserting the reference
platform's newline, and the exception **class** is what the named payload
milestone will project.

## What the audit gained

```text
BCL_AUTHORITY_TYPES     18  ->  19
BCL_AUTHORITY_MEMBERS  167  -> 205
BCL_SENTINEL_CHECKS    294  -> 314
BCL_MUTATION_SELF_TESTS 320 -> 346
BCL_AUTHORITY_STATUS=PASS
```

Two generic mutators were also fixed while adding the attribute ones: both
`change_parameter_type` and `change_property_type` substituted `System.Object`
without checking whether the target was **already** `System.Object`. On
`System.Attribute` — where `Equals(object)` and `TypeId` are exactly that — the
mutation changed nothing and was correctly reported as undetected. They now
skip a no-op target, which strengthens every subject and not only this one.

## Structural scoreboard

```text
TARGET_TYPES=140                     (135 -> 140)
TARGET_MEMBERS=1747                  (1731 -> 1747)
TOTAL_DIAGNOSTICS=274                (279 -> 274)
COMPLETE_TYPES=133                   (128 -> 133)
PARTIAL_TYPES=7                      unchanged
MISSING_TYPE=117                     (122 -> 117)
MISSING_MEMBER=132                   unchanged
every mismatch/leak category unchanged
UNMEASURED_STRUCTURAL_CATEGORY=0
ALLOWLIST_ENTRIES=0

BCL_SUPPORT_TYPE_MEASUREMENTS=10     (9 -> 10)
BCL_BASE_PROJECTIONS=19              (14 -> 19)
PROJECTED_BCL_BASE_TYPES=15          (10 -> 15)
PENDING_BCL_BASE_TYPES=4             unchanged
BCL_INHERITED_MEMBER_PROJECTIONS=77  (72 -> 77, one IsDefaultAttribute each)
BCL_ABSTRACT_BASE_WIDENINGS=1        new
```

## Behaviour

```text
PURE_XNA_DERIVED  1968/1968/0    (1923 -> 1968)
PURE_BCL_DERIVED   208/208/0     unchanged
DEBUG_TESTS=375 PASS             (366 -> 375)
API_COMPAT_SELF_TESTS=2385       (2378 -> 2385)
```
