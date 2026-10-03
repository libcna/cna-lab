# The header pin, and the day the headers moved

`tools/native_abi/verify.py` reads two things and requires them to agree: the
`CNA_ABI_VERSION` compiled out of the **headers** it is pointed at, and the
value `cna_get_abi_version` returns from the **library** it loads. On
2026-09-04 they stopped agreeing:

```text
library ABI 5376 disagrees with the canonical header ABI 5632
        5376 = 0.21.0   the qualified artifact
        5632 = 0.22.0   cnanext's working tree
```

Nothing in this repository changed. `cnanext`'s `modules/c-api/include` moved
to ABI 0.22.0 in `75847b7f2`, merged at `44f0cd16c` — between Foundation 68's
verification and Foundation 69's. Earlier runs in the same session reported
`ABI_MISMATCHES=0` because the working tree still held 0.21.0 headers when they
ran.

## What actually changed, and what it touched

The 0.22.0 delta is four files and is entirely additive:

| file | change |
| --- | --- |
| `abi.h` | `CNA_ABI_VERSION_MINOR` 21 → 22 |
| `graphics.h` | ten new renderer identifiers; `RENDERER_MAXIMUM` raised to `NANOVG` |
| `net_sessions.h` | one new route, `cna_network_session_replace_session_properties` |
| `devices.h` | a documentation comment about the browser target's device type |

**No prototype of any of the 306 bound routes differs between the two trees**,
no mirrored struct differs, and no pinned constant differs — this binding pins
no renderer identifier, so the raised maximum is invisible to it. Every
Foundation up to 68 is unaffected in substance.

## The fix is to stop reading a moving tree

The session now reads `~/deps/cna-c-abi-0.21.0/include` — the header set that
shipped with the qualified library — rather than `cnanext`'s working tree:

```text
BOUND_FUNCTIONS=306  LAYOUTS=50  CONSTANTS=225  ABI_MISMATCHES=0
```

That is what "qualified boundary" has meant all along: a *pair*. Measuring a
0.21.0 library against whatever headers happen to be checked out today was
always the weaker reading, and it held only by luck for sixty-eight
milestones. A dependency's working tree is not a pin.

**The equality check was not relaxed, and must not be.** It is the only thing
standing between this binding and a header that documents a route the loaded
library does not implement the same way. It did its job here: it caught a
dependency moving underneath a running session, which is exactly the event it
exists to catch.

Re-qualifying against 0.22.0 is a deliberate act — a new library artifact, a
re-run of the package qualification, and a new pinned header set — not a
consequence of someone else's merge.
