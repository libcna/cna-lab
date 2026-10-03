# Foundation 70 — `SpriteFont`, and a BCL family that is not admitted by needing it

```text
COMPLETE_TYPES   178          MISSING_TYPES  73 -> 72
TOTAL_DIAGNOSTICS 115 -> 116  MISSING_MEMBER 38 -> 36
BOUND_FUNCTIONS  303 -> 312   LAYOUTS 50 -> 54   ABI_MISMATCHES=0
MESSAGES_REPRODUCED 229 -> 230  RESOURCE_STRINGS_REPRODUCED 72 -> 73
tests            756 -> 773   PROJECTION_MUTATIONS 241 -> 263
```

`SpriteFont`'s five projected members and three of `SpriteBatch.DrawString`'s
six overloads. **Four members are deliberately absent**, and that is the
milestone's first result rather than its footnote.

## `System.Text.StringBuilder` is not admitted, so four members are not written

XNA declares `MeasureString(StringBuilder)` and three `DrawString` overloads
over a `StringBuilder`. This project admits a BCL family by *measuring* it:
authority established in `bcl-authorities.json`, the family's full public shape
pinned in `bcl40-selected-shape.json`, a Swift support class written and
compared against that shape. `System.Collections.Generic.List<T>`,
`ReadOnlyCollection<T>`, `Dictionary<K,V>` and fifteen more went through it.

`StringBuilder` declares sixty-five public members, most of them `Append` and
`Insert` overloads, and it is its own contract with its own semantics.
Projecting the four overloads over `String` instead is not an option: XNA's
take a *mutable buffer*, and a caller holding one is not served by a projection
that quietly copies it.

**A first draft of this section gave the wrong reason**, and it is corrected
here rather than left standing, because it would have read as a rule. It
claimed that writing the four members an XNA signature needs would be
"admitting a family by fabricating it" — that admission requires projecting
every pinned member. It does not. `CNAList` projects **sixteen** of `List<T>`'s
pinned fifty-two: the pinned shape is the authority record of what the family
*is*, and `bclSupportContract` measures the Swift class structurally, not
member for member. Left uncorrected, that claim would have told a future
session no family can ever be admitted.

The real reason the four members are absent here is scope: `StringBuilder` is a
milestone, not a paragraph of one, and it is the next one.

So the four are recorded `MISSING_MEMBER`, the family is recorded in
`availableButNotAdmitted` with the reason, and admitting `StringBuilder` is
named as its own piece of work. `SpriteFont` stays `PARTIAL` and says why.

## `System.Char` is a UTF-16 code unit, and Swift has no type for that

`Characters` is a `ReadOnlyCollection<Char>` and `DefaultCharacter` a
`Nullable<Char>`, and this is the first place the contract needed
`System.Char`. It maps to **`UInt16`**.

Swift's `Character` is a grapheme cluster and `Unicode.Scalar` is a code point;
a CLR `char` is neither. It is one UTF-16 code unit, and a font's character map
is a sorted list of them — `'\u{1F600}'` occupies two entries in XNA and one
`Character` in Swift. Mapping to either Swift type would silently change what
`Characters` contains and what `DefaultCharacter` accepts. `UInt16` has the CLR
type's exact representation and range, which is what the projection needs, and
`CNA_Char16` is the same decision made on the native side — the header calls it
"one UTF-16 code unit matching the native XNA `char` representation".

## A font with no content pipeline, and why that matters

`cna_sprite_font_create` takes a texture and a complete glyph table, so a font
can be built out of nothing but a `Texture2D` this host can already create.
`build-probe/f70_font.c` builds one from three invented glyphs and the tests
build the same one, which turns the whole measurement into arithmetic a test
can compute by hand:

| char | cropping height | kerning (left, width, right) |
|---|---:|---|
| `A` | 6 | (-2, 5, 1) |
| `B` | 7 | (1, 4, 2) |
| `C` | 9 | (0.5, 3, 0) |

`A`'s left bearing is negative on purpose, the cropping heights all differ, and
so do the right bearings — each one makes a different branch of the measurement
observable.

## The measurement is reproduced, not forwarded

CNA publishes `cna_sprite_font_measure_utf8`, and the probe measures that it
agrees with `SpriteFont.InternalMeasure` on every case tried: `""` → `(0, 0)`,
`"A"` → `(6, 10)`, `"AB"` → `(14.5, 10)`, `"A\nB"` → `(7, 20)`, and an unknown
character falling back to the default → `(7, 10)`.

It is still not called, for Foundation 69's reason: `MeasureString`'s answer is
XNA behaviour, and forwarding would make a native library the authority for it.
The glyph table is CNA's — `cna_sprite_font_copy_glyphs` is documented as "the
inverse of `cna_sprite_font_create`" — and the arithmetic over it is XNA's.

Three details a plausible reimplementation gets wrong, all of them asserted:

* **A carriage return is skipped before every other test.** `"A\r\nB"` and
  `"A\nB"` measure identically, and `"\r"` alone measures `(0, 10)` — *not*
  `Vector2.Zero`, because the length test at the top already passed.
* **Only the first glyph of a line has its left bearing clamped at zero.** That
  clamp is what stops a negative bearing pulling the first glyph off the left
  edge; applying it to every glyph, or to none, are two different mutations and
  both are caught.
* **The height is the running max of the *cropping* heights**, not of the atlas
  rectangles, plus one `lineSpacing` per newline seen.

And the width of a multi-line string is `max(last line, widest earlier line)` —
`"A\nB"` is 7 because the second line's 5 plus its trailing bearing of 2 beats
the first line's 6, which is the sort of thing that looks like an off-by-one
until the IL is read.

## Three exception shapes for one message

`CharacterNotInFont` is raised twice in `SpriteFont`, and the two are not the
same exception:

```text
GetIndexForCharacter:   ArgumentException(Format(...), "character")   -- named
set_DefaultCharacter:   ArgumentException(Format(...))                -- unnamed
```

Its second placeholder is `0x{1:x4}`, so the numeric argument is four lowercase
hex digits — `The character 'Z' (0x005a) is not available…` — and the test
asserts the padding, because `%x` and `%04x` differ only for the characters
below U+1000 and every ASCII character is one of them.

`GetIndexForCharacter`'s fallback is **one step deep**: the default character is
tried unless the missing character already *is* the default. Without that guard
a font whose default is itself absent recurses for ever rather than reporting
the character, and the mutation that removes it is caught by a font built with
no default at all.

## A deferred write, for the third time

`LineSpacing` and `Spacing` are `stfld` in XNA and cannot fail; CNA's
`cna_sprite_font_set_line_spacing` can. A Swift setter cannot report that, so
the write is deferred to `DrawString` — the moment the native font is actually
used, and a member that is allowed to throw. It is the same shape the stock
effects' `OnApply` has, and a test reads the native `SpriteFontInfo` back
before and after a draw to prove the value actually arrives.

## `DrawString`'s begin/end check is conditional, and XNA is why

XNA's `DrawString` tests its two arguments for null and calls
`SpriteFont.InternalDraw`, which loops over the characters and calls
`SpriteBatch.InternalDraw` once per glyph. `BeginMustBeCalledBeforeDraw` is
raised *there* — so a string that produces no glyph raises nothing and succeeds
outside a begin/end pair.

CNA's route answers `INVALID_STATE` outside an interval whatever the text —
measured — so forwarding an empty string would refuse a call XNA accepts. The
check is reproduced with XNA's condition: the pair is required only when the
text would produce a glyph, and a string of nothing but `\r` and `\n` produces
none.

Every character is validated before anything is submitted, because XNA's loop
raises from the glyph it reaches while CNA would refuse the whole string with a
message of its own.

## What a passing draw means

What it meant in Foundation 68. `cna_graphics_device_get_backbuffer_data_window`
still answers `NOT_SUPPORTED`, so a draw that returns is a draw the device
accepted and nothing here claims a glyph appeared anywhere. The measurement is
the part with pixel-level evidence, and it is managed arithmetic.

## CNA does not sort a glyph table, and XNA's search requires it

`GetIndexForCharacter` **binary-searches** `characterMap`, which is safe in XNA
because the content pipeline writes it sorted and nothing else builds one.
`cna_sprite_font_create` takes a caller-supplied array, and
`build-probe/f70_sorted.c` measures what it does with a bad one:

```text
create with an UNSORTED glyph table -> 0
  copy_characters order: CAB
  measure 'A' -> 0 width=5      (CNA's own measurement copes)
create with a DUPLICATE character -> 0
```

It neither sorts nor refuses, and it accepts a duplicated character. A binary
search over `CAB` answers *"not available in this SpriteFont"* for a character
that is in it — a wrong answer wearing the shape of a legitimate refusal, which
is the worst form a defect takes.

Reproducing XNA means reproducing the binary search, so the projection keeps it
and **checks the invariant it depends on, once, at construction**. No public
route can reach the bad state — `SpriteFont` has no public constructor, so
every font a consumer holds came from content — but the check is what makes the
day that changes loud instead of silent. **`ContentManager` inherits a specific
question**: whether a compiled `.xnb`'s character table arrives sorted.

## A native rule XNA does not have, again

```text
Texture2D.Dispose -> CNA_RESULT_INVALID_OPERATION
    "The Texture2D is retained by an active SpriteBatch, SpriteFont, effect,
     model or render-target scope."
```

The atlas cannot be disposed while the font holds it — CNA's header says as
much ("the source texture … cannot be destroyed until this SpriteFont is
destroyed") and the probe confirms it. XNA has no such rule. Asserted, not
worked around, exactly as the same refusal was in Foundation 69.

## Falsifiability

Twenty-four mutations, eighteen caught on the first round. The six survivors
are the milestone's most useful output, because three of them were **the
fixture being too kind** and the other three split the way the rules say they
should.

**The fixture could not show three of its own rules.** `A` is the only glyph
with a negative left bearing and `A` is first in every string the first tests
measured, so "clamp the first glyph" and "clamp every glyph" produced identical
answers. No glyph had a negative *right* bearing, so the trailing clamp was
invisible. And every cropping height was under the line spacing, so a height
read from the atlas rectangle instead of the cropping one changed nothing —
the running maximum starts at `LineSpacing` and swallowed the difference.

The fixture is now configurable and three tests supply their own glyphs: `"BA"`
puts a negative bearing in second place, a two-glyph font gives one of them a
right bearing of `-1`, and a font with `LineSpacing = 4` lets a cropping height
of 9 win against an atlas height of 8. All three mutations are caught, and the
tests are better for reasons that have nothing to do with mutation scores.

**`layout-flush-does-not-clear-its-flag` was a no-op and was replaced.**
Leaving the flag set re-pushes the same value on the next draw and no answer
changes. Its replacement crosses the two flags, which loses one property when
only the other is set — and that needed a test that sets one alone, which
there was not.

**The harness itself lost an edit, and now cannot.** A file edited while a run
holds it was silently reverted: `originals` is snapshotted before the first
mutation and restored unconditionally, so anything that changed meanwhile is
overwritten. The tree lock stops a second harness and can stop nothing else.
The restore now writes the snapshot back only when the file still holds exactly
what the harness put there, and reports the collision otherwise — losing a
mutation's score is recoverable, losing someone's work is not.

**Two were withdrawn with the reason written where they stood.**
`character-fallback-recurses-without-a-guard` removes the
`defaultCharacter != character` guard, which matters only for a font whose
default is itself absent from the glyph table. `build-probe/f70_default.c`
measures that CNA refuses to build one — *"defaultCharacter is not present in
characters"* — and refuses the same change afterwards, so no route this
projection has reaches the state. `draw-string-uniform-scale-widened-wrongly`
falls to Foundation 65's rule and Foundation 68's: a submitted sprite is not
readable back, there is no query for what a batch was given, and no scale the
route refuses. Both guards stay; what is recorded is that neither can be
scored.

`PROJECTION_MUTATIONS=263`; the full run is repeated before the final handoff.
