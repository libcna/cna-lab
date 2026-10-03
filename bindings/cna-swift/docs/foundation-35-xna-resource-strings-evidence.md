# Foundation 35 — an XNA message is a resource lookup, and now it is pinned

Foundation 30 established that a BCL default message is read out of the
admitted assembly's embedded string table rather than transcribed. XNA's own
messages are the same kind of fact and were **not** held to that standard: they
were literals someone had written down. Deriving `GameServiceContainer` turned
up that one of them was wrong.

## The defect

`GameComponentCollection.SetItem`'s whole IL body is a throw carrying the
`CannotSetItemsIntoGameComponentCollection` resource string:

```text
pinned:  "Cannot set a value using operator[] on GameComponentCollection.  Use Add/Remove instead."
was:     "XNA does not support setting a value using operator[] on GameComponentCollection. Use Add/Remove instead."
```

A different opening clause, and a single space where XNA has two. Nothing
caught it because nothing was comparing.

Seven more sites were inexact in the same way. `TouchCollection`'s six mutators
and `Collection<T>`'s read-only guard all raise a **parameterless**
`NotSupportedException`, whose message is the `Arg_NotSupportedException`
resource — `"Specified method is not supported."` — and all seven carried
descriptions of this binding's own devising instead.

## The rule, and the machinery

> A user-visible message this binding reproduces is read out of the registered
> binary's own embedded string table, pinned, and compared against the Swift
> source.

The PE and `.resources` readers Foundation 30 built now live in
`pinned_assembly_audit.py`, which `bcl_authority_audit.py` already imports
from, so the XNA audit and the BCL authority audit agree by construction about
what an assembly says. `registered-assemblies.json` gains a
`selectedResourceStrings` list, the audit reads and pins them in
`reference/xna40-selected-resource-strings.json`, and the verifier reads the
literals back out of `Sources/CNA/Xna` and compares
(`XNA_RESOURCE_STRING_PROJECTIONS`). A message that does not match is a
`LANGUAGE_MAPPING_MISMATCH`.

Registration is **demand-driven**, exactly as the BCL authority is: a key
appears once something reproduces it. Four `GameServiceContainer` keys were
read while deriving this and are deliberately not registered yet, because
nothing reproduces them.

The source reader was generalised out of the BCL one, so both halves see
through `"a " + "b"` seams and skip comments. Five self-tests hold the XNA half
non-vacuous: the pinned values are the reference model, a mutated value must be
caught, and unreadable pins or unreadable sources must be reported unmeasured
rather than assumed away.

## `CNAError.notSupported` now reports the message verbatim

It used to compose `"XNA does not support \(operation)"` around a fragment,
which cannot produce a CLR message. It now carries the message itself, and all
eight call sites carry the exact string the assembly that raises them holds.
`Arg_NotSupportedException` is pinned in the BCL set for the seven that share
it.

The exception **class** is still `CNAError` rather than
`NotSupportedException`. That remains the named payload milestone; this one
makes the message exact, which is the half that was silently wrong.

## Scoreboard

```text
TOTAL_DIAGNOSTICS=271                unchanged
LANGUAGE_MAPPING_MISMATCH=0          the new check, green
XNA_RESOURCE_STRING_PROJECTIONS=2    new
BCL_RESOURCE_STRING_PROJECTIONS=8    (7 -> 8, Arg_NotSupportedException)
RESOURCE_STRING_CHECKS=4             new, in the pinned XNA audit
API_COMPAT_SELF_TESTS=2390           (2385 -> 2390)
DEBUG_TESTS=399 PASS                 unchanged
```

One existing test asserted the wrong message and now asserts the pinned one,
double space included.
