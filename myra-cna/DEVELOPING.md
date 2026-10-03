# Developing Myra-CNA

Myra-CNA's own library, test, and example targets require C++23 with compiler
extensions disabled. They compile with `/W4` on MSVC or `-Wall -Wextra` on
GCC/Clang, and warnings are errors by default. A local diagnostic build may set
`-DMYRA_CNA_WARNINGS_AS_ERRORS=OFF`; changes are expected to pass again with the
default enabled before hand-off.

Never use more than three concurrent build workers:

```bash
cmake -S . -B build
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure --parallel 3
```

For linked or sanitised validation against the current modular CNA checkout,
configure CNA as the top-level project with
`CMAKE_PROJECT_CNA_INCLUDE=.../cmake/AddMyraCnaToCnaBuild.cmake`, then run CTest
from the generated `_myra_cna` subdirectory. The exact SOFTWARE,
SDL_RENDERER, and sanitizer commands are maintained in [README.md](README.md).
`MYRA_CNA_LINK_CNA=ON` is retained only for compatible legacy CNA layouts.

## Formatting

The repository root `.clang-format` is authoritative for C++ sources. When a
supported `clang-format` executable is installed, the default configuration
adds two explicit developer targets:

```bash
cmake --build build --target myra_cna_check_format
cmake --build build --target myra_cna_format
```

The check target is non-mutating. The format target rewrites all project-owned
`.hpp` and `.cpp` files under `include`, `src`, `tests`, and `examples`, so
review its diff before keeping the result. Set
`-DMYRA_CNA_ENABLE_FORMAT_TARGETS=OFF` to skip tool discovery. A missing
formatter does not prevent ordinary consumer builds.

## Include-what-you-use

Include-what-you-use is an opt-in audit because it is a developer tool rather
than a consumer dependency:

```bash
cmake -S . -B build-iwyu -DMYRA_CNA_ENABLE_IWYU=ON
cmake --build build-iwyu --parallel 3
```

Configuration fails immediately when the option is enabled but the executable
is unavailable. The audit applies to the `MYRA_CNA` library sources; findings
in sibling CNA or sharp-runtime targets are outside this repository's scope.
