# CNA native ABI verification

`bindings.json` is the reviewed manifest of stable CNA C functions used by the JNI adapter,
including ownership notes. `verify.py` enforces four independent facts:

1. every manifest symbol is dynamically loaded by `cna_java_jni.c`, with no unmanifested load;
2. `probe.c` compiles against the selected CNA headers with fixed-width, layout, ABI-version, and
   exact function-pointer signature assertions;
3. when `CNA_NATIVE_LIBRARY` is supplied, every manifest symbol is exported by that library;
4. the loaded library reports a compatible ABI version.

Run it through `./gradlew nativeAbiCheck`. `CNA_ROOT` selects the CNA source/header checkout;
`CNA_NATIVE_LIBRARY` adds the library checks. Evidence is platform-specific: a Linux result is not
treated as Windows or macOS proof. Runtime JNI/lifetime behavior is separately covered by the
conditional JUnit integration suite.
