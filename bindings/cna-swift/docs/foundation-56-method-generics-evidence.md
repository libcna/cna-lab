# Foundation 56: the verifier could not express a generic method

## What was wrong

CIL spells a **type's** generic parameter `!N` and a **method's** `!!N`. The
contract records both, and `map_clr_type` substituted only the first — from the
owner type's `genericParameters` list, which for a non-generic owner is empty.
So every generic method in the pinned contract expected a parameter type that
kept the raw CIL placeholder:

```text
Microsoft.Xna.Framework.Graphics.Texture2D.SetData(_:[!!0])
Microsoft.Xna.Framework.Graphics.Texture2D.GetData(_:Int32,rect:…,data:[!!0],…)
```

No Swift declaration can produce `[!!0]`. `Texture2D.SetData` and `GetData` —
six members — were therefore not merely unimplemented but **unimplementable**:
any projection of them would have been reported as a `MISSING_MEMBER` plus an
`UNEXPECTED_MEMBER`, whatever it looked like.

That is a gate defect, not a projection defect, and it had been silently
shaping the roadmap: six members sat in the missing list behind a diagnostic
nothing could satisfy.

## The fix

`map_clr_type` takes the member's own generic parameter names alongside the
owner's, and `expected_member` passes them — read from the contract's
`genericParameters`, ordered by `position`, so the name is the contract's own:

```text
before   SetData(_:[!!0])
after    SetData(_:[T])
```

Nothing else moved: `TOTAL_DIAGNOSTICS` is 175 before and after, because the
six members are still absent. What changed is that they can now be written.

## Falsifiability

Five self-test assertions, and the fix is demonstrated by removing it:

```text
$ (substitution removed)
self-test failures:
  a method's own generic parameter is not substituted into its parameter types: ('[!!0]', 'Int32')
  a method placeholder is not substituted inside Nullable
  a method's generic parameter is not substituted into its return type
```

The two directions are checked separately, because the two lists are different
lists:

* `map_clr_type("!!0[]", rules, ("TOwner",))` must stay `[!!0]` — a **method**
  placeholder must not be filled from the **owner's** list.
* `map_clr_type("!0[]", rules, ("TOwner",), ("TMethod",))` must be `[TOwner]` —
  and a type placeholder must not be filled from the method's.

Both would pass a naive implementation that substituted from whichever list was
non-empty, which is why they are stated as separate assertions.

## What this does not do

It does not implement `SetData` or `GetData`. Their shape is now expressible;
the transfer itself is measured and designed in the next milestone, including
the two facts that decide it — CNA's typed transfer round-trips through
`get_data_rgba8`, and it **refuses** a byte-typed transfer into a Color texture
(`build-probe/f56_texdata.c`), so XNA's "any struct whose size divides the
format's" cannot be projected as a raw byte blit.
