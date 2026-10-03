# CNA-Swift GamePad native qualification

This runner keeps canonical CNA observations separate from the pure
XNA-derived managed behavior corpus. It exercises the four state modes,
capabilities, safe zero-vibration, current-generation reuse, and wrong-thread
preflight against a CNA C ABI 0.35 library.

```text
python3 tools/gamepad_native/run.py \
  --library /absolute/path/to/libcna_c_api.so \
  --swift-test /absolute/path/to/swift-test \
  --output docs/generated/gamepad-native-report.json
```

On a host without controller hardware, successful disconnected CNA snapshots
prove the real negative routes. Positive state, capability, and rumble behavior
remain `HARDWARE_PENDING`.
