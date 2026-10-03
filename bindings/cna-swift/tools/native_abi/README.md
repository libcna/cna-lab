# CNA-Swift native ABI verifier

The verifier compiles `probe.c` against canonical CNA headers and the package's
Clang shim, generates compiler type-compatibility assertions for every symbol
in the Swift native manifest, compares every Swift `@convention(c)` position to
the manifest, checks all `Keys` constants, and audits loaded-library exports and
the exact ABI version.

```text
python3 tools/native_abi/verify.py \
  --cna-include ../../cna/modules/c-api/include \
  --library /absolute/path/to/libcna_c_api.so \
  --output docs/generated/native-abi-report.json
```
