# CNA-Swift structural verifier

`verify.py` maps the pinned Microsoft XNA 4.0 Windows runtime contract through
`mapping-rules.json`, then compares it to Swift's compiler-emitted Symbol Graph.
Namespace markers and documented Swift synthesis are language infrastructure,
not XNA types or members. Enum raw values are the only compiler-Symbol-Graph
gap in the selected slice; the verifier supplements them from the same compiled
Swift source declarations and reports a category as unmeasured if that evidence
is unavailable.

Run from the repository root:

```text
python3 tools/api_compat/verify.py --self-test
swift package dump-symbol-graph
python3 tools/api_compat/verify.py \
  --symbol-graph .build/x86_64-pc-linux-gnu/symbolgraph/CNA.symbols.json \
  --output docs/generated/api-compat-report.json
```

The command exits nonzero in normal strict mode while future XNA types remain
missing. `--leak-only` fails only public implementation/pointer/FFI leaks.

`dependency_graph.py` builds the public-signature dependency graph from the same
pinned contract and the generated strict report. `--type NAME` reports one
type's direct and transitive reverse consumers; without it the tool ranks every
still-missing type whose XNA dependencies are strict-complete, which is the
deterministic selection input for the next milestone.

```text
python3 tools/api_compat/dependency_graph.py \
  --report docs/generated/api-compat-report.json \
  --output docs/generated/dependency-graph.json
```

## Reference assembly registration

`pinned_assembly_audit.py` decides whether a Microsoft XNA assembly may be used
as a behavior authority. The retained contract is the public-shape authority;
an assembly earns behavior authority only by machine-comparing its public
metadata against every contract entry it declares.

```text
python3 tools/api_compat/pinned_assembly_audit.py \
  --assembly-dir /path/to/xna/redistributable \
  --require-exact Microsoft.Xna.Framework.dll \
  --require-exact Microsoft.Xna.Framework.Graphics.dll \
  --output docs/generated/pinned-assembly-audit.json
```

The tool disassembles each assembly with `ikdasm`, reconstructs the public
type and member shape in the contract's own schema, and diffs it. It exits
nonzero if a calibration assembly is not reproduced exactly, or if its own
mutation self-tests fail.

`accessor_fallibility.py` derives, from the CIL of the same registered
assemblies, whether each public property accessor has a contract-relevant
failure path. Swift cannot express a throwing setter, so that per-accessor
answer decides whether a CLR property projects to `var P { get set }` or to
`var P { get }` plus a `SetP(_:) throws` writer method.

```text
python3 tools/api_compat/accessor_fallibility.py \
  --assembly-dir /path/to/xna/redistributable \
  --output tools/api_compat/reference/xna40-accessor-fallibility.json \
  --markdown docs/generated/accessor-fallibility-inventory.md
```

The emitted JSON is a *pinned reference*, hash-checked by `verify.py` against
`accessorFallibilitySha256` in `mapping-rules.json`, so the verifier still runs
with no Microsoft binary present. Every fallible verdict carries the shortest
call chain that reaches the throw and the exception types constructed there.
Thirty-four self-tests run on every invocation, including five mutations that
must flip a verdict and a two-sided bound proving that merging same-named
overloads by arity changes no accessor verdict.

`return_nullability.py` derives, from the CIL of the same registered
assemblies, whether each public reference-typed return position can normally
return null. A CLR reference return and a CLR failure are independent facts, so
this answer is orthogonal to `accessor_fallibility.py`'s: Optional and `throws`
are decided separately and all four combinations occur.

```text
python3 tools/api_compat/return_nullability.py \
  --assembly-dir /path/to/xna/redistributable \
  --output tools/api_compat/reference/xna40-reference-return-nullability.json \
  --markdown docs/generated/return-nullability-inventory.md
```

Every method body is abstractly interpreted over `NONNULL < UNKNOWN < NULLABLE`
on a simulated evaluation stack, with an environment for arguments, locals and
field reads so that a null guard refines exactly what it tests; method and
field summaries reach a least fixpoint together, and a field is decided by its
whole construction-and-store lifecycle rather than by one read. The emitted
JSON is a *pinned reference*, hash-checked by `verify.py` against
`returnNullabilitySha256` in `mapping-rules.json`, and records only CLR facts —
no Swift spelling, no mapping rule, no machine-local path — so it is a function
of the pinned contract and the seven registered binaries alone. It regenerates
byte-identically. Self-tests run on every invocation, including mutations that
must flip a verdict and a two-sided per-overload bound proving the arity merge
decides every in-scope method's fallibility.

`verify.py --graph-self-test --symbol-graph <path>` is the reader's own
negative-fixture suite: seventeen mutations of the Symbol Graph the compiler
actually emitted, each of which must introduce a diagnostic the unmutated graph
does not already carry. Five of them break the reference-return nullability
projection — losing an Optional, gaining one, gaining a `throws` beside an
Optional, moving the Optional to the wrong generic level, and applying one to a
CLR value type that is not `System.Nullable<T>`.

`--require-exact` is the calibration gate. The reconstruction's correctness is
not asserted, it is demonstrated on the assemblies whose provenance was already
established, and only then trusted for the others. No Microsoft binary is
stored in the repository; only the SHA-256 of each registered assembly is
retained, in `docs/xna-swift-mapping.md`.

## BCL behaviour authority

`bcl_authority_audit.py` decides whether a Microsoft .NET Framework BCL
assembly may be used as a behaviour authority. It is deliberately **separate**
from `pinned_assembly_audit.py`: that gate earns authority by reproducing the
pinned XNA contract, and a BCL assembly declares zero XNA contract types, so it
would pass vacuously. The registry is `bcl-authorities.json` and the
calibration is `reference/bcl40-selected-shape.json`.

```text
python3 tools/api_compat/bcl_authority_audit.py \
  --assembly mscorlib.dll=/path/to/mscorlib.dll \
  --cross-check \
  --negative-control mscorlib.dll=/usr/lib/mono/4.5/mscorlib.dll \
  --negative-control mscorlib.dll=/usr/lib/mono/4.0/mscorlib.dll \
  --negative-control mscorlib.dll=/usr/lib/mono/2.0-api/mscorlib.dll \
  --negative-control mscorlib.dll=/path/to/another/net4/mscorlib.dll \
  --output docs/generated/bcl-authority-audit.json
```

Authority is earned by five independent things, each counted separately:
exact identity including a **recomputed** strong-name token
(`BCL_IDENTITY_CHECKS`), reproduction of the pinned selected-shape manifest
(`BCL_MANIFEST_CHECKS`), sentinel facts stated independently of the extractor
(`BCL_SENTINEL_CHECKS`), mutation self-tests that must each be detected
(`BCL_MUTATION_SELF_TESTS`), and agreement from `monodis`, an independent
The fourth control is the one that matters most: a **genuine Microsoft**
`mscorlib.dll` from a different .NET 4.0 install, refused by 2 of 21 checks
where the Mono binaries are refused by 12 to 16. A control that only just fails
tells you where the boundary actually is; three that fail obviously do not.
The specific control binaries are not pinned — this machine's Mono packages
have been upgraded at least once, retiring the digests an earlier session
recorded — so reproduce the count, not the files.

metadata reader (`BCL_CROSS_CHECKS`). `--negative-control NAME=PATH` offers a
binary that must be **refused**, which is what keeps the gate honest; the Mono
`mscorlib` builds are the standing controls. Authority is demand-driven: only
the families the binding actually derives behaviour from are admitted, each
with a recorded reason. `--write-manifest` regenerates the manifest, which is
byte-identical with and without an IL cache. The BCL counters share no name
with any XNA counter, and no XNA measurement changes when a BCL assembly is
admitted. No Microsoft binary is stored in the repository.
