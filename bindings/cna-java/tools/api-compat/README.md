# XNA-to-Java contract verifier

This tool compares compiled metadata, not Java source text. Its pipeline is:

```text
pinned XNA CLR assemblies -> XnaContractExtractor -> neutral JSON
neutral JSON + mapping-rules.json -> expected Java contract
compiled cna-java classes -> ClassContractReader -> actual Java contract
expected vs actual -> categorized diagnostics + strict leak findings
```

`profiles/xna40-windows-runtime.json` names and SHA-256-pins the seven canonical
runtime assemblies. A missing or byte-different assembly is rejected before extraction. The local
user supplies these legally obtained assemblies through `XNA_REFERENCE_DIR`; they are not stored or
redistributed here. Mono's `mcs`/`mono`, Python 3, and a JDK 17 toolchain are required.

Run the measurable, green report mode with:

```bash
XNA_REFERENCE_DIR=/path/to/xna4/windows ./gradlew apiCompatReport
```

Run the deliberately red completeness gate with `apiCompatCheck`. It exits nonzero while any
diagnostic remains. `leakGuard` needs no XNA binaries and rejects internal/native implementation
types or raw handles in strict public/protected signatures. `apiCompatToolTests` protects mapping,
hash-validation, leak-detection, and class-reader behavior.

Reports distinguish reference CLR counts, the transformed expected Java counts, compiled target
counts, and each diagnostic kind. The mapping allowlist is required to remain empty. Necessary Java
language adaptations belong in the normative mapping and machine-readable rules, not in broad
suppression entries. The first complete report remains in `baselines/`; current exact counts and
deltas are recorded in the repository `plan.md` and `NEXT.md`.
