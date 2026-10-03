# Foundation 73 — `TitleContainer`, and the one branch CNA cannot tell apart

One public member. Four fifths of it is managed, and that is the milestone:
`OpenStream` is 213 bytes of IL over `GetCleanPath` (256), `IsCleanPathAbsolute`
(87) and `CollapseParentDirectory` (40) — a complete path-normalisation routine,
reproduced here rather than delegated to CNA's own path handling, because
**which names XNA refuses is XNA behaviour** and a native runtime that happened
to agree would still not be the authority for it.

It was sized as a one-member type in Foundation 72's handoff and deliberately
deferred there. That sizing was wrong in the useful direction: reading the IL
before starting is what turned it into its own milestone instead of a rushed
appendix to another one.

## An exception family had to be admitted first

The hierarchy had no `System.IO` member at all. `System.IO.IOException` and
`System.IO.FileNotFoundException` are admitted from the pinned .NET 4.0
`mscorlib`, cross-checked, with three Mono binaries rejected as negative
controls (16, 12 and 14 of 21 checks failed respectively).

`CNAIOException` is admitted **for the chain, not for a member** — nothing here
raises a bare one — and carries `COR_E_IO` `0x80131620`.

`CNAFileNotFoundException` is a **measured subset, and the boundary is exact**.
The CLR type has six constructors; three are projected. The three that carry a
`fileName` are not, and `SetMessageField` is the reason: for such an instance
with no message of its own, `Message` comes from
`FileLoadException.FormatFileLoadExceptionMessage`, an internal call into the
CLR. Reproducing them would mean inventing a message the authority does not
give. So `FileName` answers `nil` on every instance this binding can produce —
which is exactly what the projected constructors leave it, and what XNA's own
path produces, since `OpenStream` uses `.ctor(String message)`.

Its HResult is `0x80070002`, a **Win32 facility code**, not the `0x8013…` the
rest of this hierarchy uses. That is asserted rather than assumed.

Five resource strings were admitted, all read mechanically out of embedded
string tables: three from the hash-registered XNA assembly
(`InvalidTitleContainerName`, `OpenStreamNotFound`, `OpenStreamError`) and two
from `mscorlib` (`Arg_IOException`, `IO.FileNotFound`).

## What the probe measured, and why it decided the design

`build-probe/f73_title.c` asked the route five questions:

```text
title path = …/build-probe/
absent file                -> result=5 bytes=0
empty name                 -> result=5 bytes=0
absolute existing          -> result=5 bytes=0
unreadable (perm)          -> result=5 bytes=0
a directory                -> result=5 bytes=0
```

**Every failure is the same code.** `CNA_RESULT_IO`, for a file that is missing,
one that exists but cannot be read, and a directory alike.

XNA splits exactly there. `File.OpenRead` raising `FileNotFoundException`,
`DirectoryNotFoundException` or `ArgumentException` becomes
`FileNotFoundException(OpenStreamNotFound)`; anything else — an access denial —
becomes `InvalidOperationException(OpenStreamError)` carrying the original as
its inner exception.

So the split cannot be reproduced, and the projection says which way it went and
why: `CNA_RESULT_IO` maps to the **not-found** branch, because that is the
failure CNA's own header names for this route and the one a title asset actually
meets. **An unreadable-but-present file therefore reports `FileNotFoundException`
where XNA reports `InvalidOperationException`.** That divergence is measured, not
assumed, and no observation available on this host can close it.

The other narrowing is CNA's and it is stated outright in the header: this ABI
has no stream handle for title content, so the count/copy pair delivers the
**whole file**. The `InputStream` returned here is over bytes already in memory,
and *incremental reads over a title stream are not available* — a caller opening
a large asset pays for all of it at once, where XNA would not.

## Three things the IL says that a reasonable implementation would not

**Normalisation runs before the absoluteness test.** A name is judged on what it
collapses to, not what it was written as: `"./../x"` is refused because it
becomes `"..\x"`. The test asserts the ordering rather than the outcome alone.

**`badCharacters` has seven entries and none of them are separators.** They are
`: * ? " < > |`, read out of the 14-byte static array blob the `.cctor` hands to
`RuntimeHelpers.InitializeArray`. A name containing one is reported as
*absolute*, which is XNA's word for a test that has nothing to do with rooting.

**A trailing `"\.."` leaves the separator behind.** `GetCleanPath("a\b\..")` is
`"a\"`, not `"a"`. `CollapseParentDirectory` is called with position 3, finds the
previous separator at index 1 so `start` is 2, and removes
`position - start + removeLength` = 4 characters from index 2 — `"b\.."` —
leaving `"a\"`.

That one was asserted as `"a"` first, because that is what the shape of the
operation suggests. **The projection was right and the test was wrong**, which is
the third time in four milestones that a failing assertion turned out to be the
assertion's fault. The rule that keeps paying: when a test disagrees with the
projection, read the IL before touching either.

## What the tests can and cannot claim

Fourteen tests. The success path is real: a fixture written next to the test
executable — which the probe measured to be the title path CNA reports — is read
back byte for byte and removed afterwards. It is project-controlled; no user
document is read or written anywhere in this milestone.

Every managed refusal is reached **before** `RuntimeRegistry.current()`, and one
test exists only to pin that: a consumer outside a lifecycle callback still gets
XNA's `ArgumentException` for a bad name rather than a capability failure. Only
a well-formed name needs the runtime.

`CONSTANTS` gains three: `CNA_RESULT_SUCCESS`, `CNA_RESULT_IO` and
`CNA_RESULT_BUFFER_TOO_SMALL` are pinned with `_Static_assert`s for the reason
`CubeMapFace`'s six were — they are compared as bare numbers here, so a
renumbering would be invisible to every observation this host can make.

## Falsifiability

Eight mutations, seven caught on the first round.

The survivor is the useful one. `clean-path-strips-leading-dot-once` turns the
leading-`.\` **loop** into a single `if`, and the test that was supposed to
cover it — `GetCleanPath(".\.\a") == "a"` — did not, because the `"\.\"` → `"\"`
replacement two steps earlier already collapses that input to `".\a"`, where one
strip finishes the job. The assertion looked like it exercised the loop and
tested nothing of the sort.

The input that actually needs a second iteration has to survive that
replacement, which is non-overlapping:

```text
"./././a" -> "\" swap    -> ".\.\.\a"
          -> "\.\" swap  -> ".\.\a"      (one match, not two)
          -> strip ".\"  -> ".\a"
          -> strip ".\"  -> "a"          <- the iteration the `if` loses
```

With that assertion added the mutation is caught. Same lesson as Foundation 71's
`ToString` survivor: a test can name the right behaviour and still not reach it,
and only a mutation finds out.

The other seven cover the trailing-dot short arm, `CollapseParentDirectory`'s
floor of 1, both halves of the absoluteness test, the empty-name refusal, the
`CNA_RESULT_IO` branch choice, and the second read being told the size the first
one reported.

`PROJECTION_MUTATIONS=287`; the full run is repeated before the final handoff.
