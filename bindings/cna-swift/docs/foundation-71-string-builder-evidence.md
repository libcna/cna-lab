# Foundation 71 — `StringBuilder`, admitted rather than approximated

```text
COMPLETE_TYPES   178 -> 179    PARTIAL_TYPES 7 -> 6
TOTAL_DIAGNOSTICS 116 -> 108   MISSING_MEMBER 36 -> 32
OVERLOAD_MAPPING_MISMATCH 8 -> 4   UNEXPECTED_MEMBER=0
BCL_AUTHORITY_TYPES 28 -> 29   BCL_AUTHORITY_MEMBERS 254 -> 319
BCL_SUPPORT_TYPE_MEASUREMENTS 18 -> 19  BCL_MANIFEST_CHECKS 51 -> 62
BCL_RESOURCE_STRING_PROJECTIONS 19 -> 29  API_COMPAT_SELF_TESTS 2426 -> 2443
MESSAGES_REPRODUCED 230 -> 233
tests 773 -> 791   PROJECTION_MUTATIONS 263 -> 278
```

`System.Text.StringBuilder` admitted as a BCL support family, and the four XNA
members that were waiting for it: `SpriteFont.MeasureString(StringBuilder)` and
`SpriteBatch.DrawString`'s three `StringBuilder` overloads. `SpriteFont` is
complete, and `OVERLOAD_MAPPING_MISMATCH` is back to the four it stood at
before Foundation 70 opened these.

## What admission actually required, corrected once already

Foundation 70 deferred this and gave the wrong reason — that admission means
projecting every pinned member. It does not, and the correction was committed
before this milestone started: `CNAList` projects sixteen of `List<T>`'s pinned
fifty-two. The pinned shape is the **authority record of what the family is**;
`bclSupportContract` measures the Swift class structurally.

So `bcl40-selected-shape.json` gains all **sixty-five** of `StringBuilder`'s
visible members — six constructors, fifty-five methods, four properties — and
`CNAStringBuilder` projects fourteen. `AppendFormat` is in the pinned record
and in `forbiddenMembers`: it brings .NET composite formatting with it, no XNA
member reaches it, and a half-built one would be worse than its absence.

## The store is `[UInt16]`, and that is the whole design

Every index in this family's API — `Length`, `Item`, `Insert`, `Remove`,
`ToString(startIndex:length:)` — is a **UTF-16 code-unit** index. Swift's
`Character` is a grapheme cluster and `Unicode.Scalar` a code point; a CLR
`char` is neither, which is why Foundation 70 mapped `System.Char` to `UInt16`
and why this class stores code units rather than a `String`.

The test that pins it uses an astral scalar:

```text
"a\u{1F600}b".count      == 3    // three Swift Characters
builder.Length           == 4    // four UTF-16 code units
builder.Item(1)          == 0xD83D
builder.Item(2)          == 0xDE00
```

A `String`-backed builder answers 3 and cannot address either half of the
surrogate pair — which is exactly what a CLR `char` indexer does address.

## Three refusal shapes read out of the IL, none of them guessable

**The indexer's two accessors raise different exception types.**

```text
get_Chars:  newobj IndexOutOfRangeException::.ctor()          -- bare, twice
set_Chars:  ArgumentOutOfRangeException("index",
                ArgumentOutOfRange_Index)                     -- named, twice
```

One indexer, two exception types, for the same kind of mistake. It is the first
thing a reimplementation gets wrong, and two mutations transpose them.

**Every growing member is fallible for a reason its own body does not show.**
`Append(String)` has no refusal of its own; `ExpandByABlock` and `MakeRoom`
raise `ArgumentOutOfRangeException("requiredLength", ArgumentOutOfRange_SmallCapacity)`
when the result would pass `MaxCapacity`. A plain append reaches it, so
`Append`, `AppendLine` and `Insert` all throw — measured, not assumed, and the
parameter name is one no caller ever passed.

**One message is formatted, and its argument is the parameter name.**
`ArgumentOutOfRange_MustBePositive` is `'{0}' must be greater than zero.`, so
the two-argument constructor's third refusal reads
`ArgumentOutOfRangeException("capacity", "'capacity' must be greater than
zero.")` — the argument named twice, once in `ParamName` and once inside the
sentence.

**Two of those messages were transcribed wrong in the first draft** and caught
by extracting them rather than trusting the draft:
`ArgumentOutOfRange_MustBePositive` was written "Positive number required."
and `ArgumentOutOfRange_SmallMaxCapacity` "MaxCapacity must be greater than
zero." — where mscorlib says "MaxCapacity must be one or greater." That is the
failure `registered-assemblies.json` already warns about: *"Transcribing one
from memory is a mapping mismatch even when it looks right."*

## Refusal order is behaviour, and it is reproduced

Each member's guards run in the IL's order, because a call wrong in two ways
reports whichever XNA tests first:

| member | order |
|---|---|
| `.ctor(capacity:maxCapacity:)` | capacity > maxCapacity, maxCapacity < 1, capacity < 0 |
| `SetCapacity` | negative, above maxCapacity, below `Length` |
| `SetLength` | negative, above maxCapacity |
| `Remove` | negative length, negative start, range past the end |
| `ToString(_:_:)` | negative start, start past `Length`, negative length, range past the end |

`Remove`'s third names **`"index"`**, not `"startIndex"` — a parameter the
caller never passed — and `ToString`'s reordering is a mutation of its own.

Two smaller ones, both from the IL and both tested: a zero capacity becomes
`Math.Min(16, maxCapacity)`, so a builder with a ceiling of 4 does not start at
16; and `Clear()` is literally `this.Length = 0`, three instructions, so **the
capacity survives** — which is the only reason to clear rather than rebuild.

## Where XNA's `StringProxy` went

XNA needs a `StringProxy` struct so one measurement body can serve a `String`
and a `StringBuilder`. The proxy has nothing to abstract over here: both
overloads hand `measure` the same `[UInt16]`, and `SpriteBatch.DrawString`'s
six overloads funnel into one `drawText` over code units. The `StringBuilder`
overloads read the buffer's contents where the `String` ones read theirs, so a
caller holding a builder is not made to materialise a string first — which was
the reason not to project these over `String` in the first place.

## The messages are policed now, and were not

`bclResourceStringProjections` pins nineteen BCL messages and compares each
against the Swift source. `CNAStringBuilder` arrived with eleven message
constants and **none of them were in it** — unpoliced, in exactly the set where
two had already been transcribed wrong and caught only because they happened to
be re-extracted.

Ten are admitted now, with their values read out of mscorlib's embedded table
and their reasons recorded. `BCL_RESOURCE_STRING_PROJECTIONS` goes 19 -> 29,
`BCL_RESOURCE_CHECKS` 21 -> 31, `BCL_MANIFEST_CHECKS` 52 -> 62. The eleventh,
`Environment.NewLine`, is not a resource string; `ArgumentOutOfRange_Index` was
already pinned.

The check is demonstrated to fail on the real mistake: restoring the original
typo — "MaxCapacity must be greater than zero." for
`ArgumentOutOfRange_SmallMaxCapacity` — turns a green tree into

```text
BASE_MAPPING_MISMATCH resourceString.ArgumentOutOfRange_SmallMaxCapacity
    the Swift support source does not reproduce the pinned value
    'MaxCapacity must be one or greater.' read from the admitted assembly's
    own embedded resources
```

which is the difference between catching a wrong message by luck and catching
it by construction.

## Falsifiability

Fifteen mutations, twelve caught on the first round and all fifteen after. All three survivors earned
their place.

**`measure-string-builder-reads-a-copy` found that nothing tested the four XNA
members at all.** They were landed, the counters moved, the gate went green,
and no test exercised one of them — which is the failure the mutation harness
exists to catch and which no coverage number would have shown. There are three
device-scoped tests now, and the sharpest measures a builder holding
`U+1F600` against a font with glyphs at **both surrogate halves**:

```text
builder.Length                    == 3       'A' plus the pair
font.MeasureString(builder).X     == 10      5 + 3 + 2
                                             a rendered String answers 5
```

That test is only possible because a `SpriteFont`'s character map is UTF-16
code units, so a glyph at `0xD83D` is as legitimate as one at `0x0041`. It is
the difference between measuring the buffer and measuring a copy of it, and
nothing else in the milestone could tell them apart.

**`builder-to-string-range-refusals-reordered` was a genuine test gap.** Each
existing `ToString` test was wrong in exactly one way, and any permutation of
the four guards passes all of them. Pinning an *order* needs a call wrong in
**two** ways: `ToString(-1, -1)` must report `startIndex`, because that is what
XNA tests first.

**`builder-repeat-count-written-before-testing` was a no-op and was replaced.**
Moving the append before the guard changes nothing when the count is negative:
`max(0, -1)` writes zero elements and the guard still throws. Its replacement
refuses `repeatCount: 0`, which XNA accepts and writes nothing for — a value
the tests already covered.

`PROJECTION_MUTATIONS=278`; the full run over all of them is the handoff's.
