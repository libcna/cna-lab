# Foundation 76 — `SpriteBatch.Begin`'s two effect overloads

```text
TOTAL_DIAGNOSTICS 105 -> 101   MISSING_MEMBER 32 -> 30
OVERLOAD_MAPPING_MISMATCH 4 -> 2   BOUND_FUNCTIONS 317 -> 318
tests 814 -> 816   PROJECTION_MUTATIONS 290 -> 292   ABI_MISMATCHES=0
```

Two members on a type that already stood — the kind of work the roadmap does not
name, because every milestone in it is named after a missing *type*. They were
found by listing the 32 members missing from standing types.

## The identity matrix is XNA's, not this binding's

The six-argument `Begin` is **twenty-one bytes** of IL: it loads its five
arguments, pushes `Matrix.Identity`, and calls the seven-argument overload. So
the five-argument overload that already existed passes `Identity` too, and all
five entry points now funnel into one core — which is exactly XNA's shape rather
than a convenience.

## `effect` is not Optional, and that is a rule rather than a reading

The pinned signature is `Effect`, not `Effect?`. The registered CIL does not
prove XNA accepts null there, and this projection is deliberately non-Optional
until it does. The nulls the other three overloads pass are internal.

## Which route carries a batch

`cna_sprite_batch_begin_with_effect` takes the four state descriptors, an effect
handle and a matrix, and its header says `CNA_INVALID_HANDLE` selects the
default sprite effect — so it could serve every batch. It does not: a batch with
no effect keeps using `cna_sprite_batch_begin_with_states`, the route Foundation
54's probe measured against the device's blend state. Changing the route under
the four overloads that already worked would have put a measured behaviour at
risk to save a branch.

## Falsifiability

Two mutations, both caught.

`begin-effect-skips-the-pair-check` removes the `inBeginEndPair` guard from the
shared core. XNA's very first instruction is `ldfld inBeginEndPair`, ahead of
any state work, and the test that catches it nests a second `Begin` inside the
first through the *effect* overload — so the guard is asserted on the new path,
not merely on the old one.

`begin-six-argument-invents-a-matrix` replaces the pushed `Matrix.Identity` with
a zeroed matrix. It was expected to survive, on the assumption that a transform
is not observable on this host; it is caught, because CNA validates the matrix
it is handed. Worth recording: the assumption was wrong in the direction that
costs nothing, and the mutation was kept rather than replaced.
