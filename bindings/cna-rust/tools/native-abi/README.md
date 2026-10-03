# CNA native ABI verification

`bindings.json` inventories the reviewed declarations used by the implemented
Rust projection in `cna-rust-sys`. The verifier uses canonical
CNA headers plus Clang's AST as the C prototype authority and compares every
reviewed function with its Rust function-pointer alias. It checks return and
parameter types, count, scalar width/signedness, pointer depth/constness where
representable, callback and structure pointer types, and boolean/enum/flag
representation.

It also checks canonical header declarations, optional Linux ELF exports, and
the exact native ABI version. Independently compiled C and Rust executables
probe `sizeof`, alignment, field offsets, scalar/boolean representation,
constants, and callback signatures; the C compiler is the layout authority.

```bash
python3 tools/native-abi/verify.py \
  --cna-root /path/to/cna \
  --library /path/to/libcna_c_api.so
```

`generate.py --linked --all-manifest-symbols` re-derives every direct-link
declaration from Clang, one process per symbol, which at three thousand symbols
takes roughly twenty-five minutes. A slice that adds a few dozen routes can use
`splice-linked.py` instead, which generates only the new ones and appends them:

```bash
python3 tools/native-abi/splice-linked.py cna_some_new_route cna_another_one
```

The two produce the same file -- measured by removing declarations from a freshly
generated `linked.rs`, splicing them back and diffing -- and the verifier reads
`linked.rs` either way, so a splice that went wrong is caught there rather than
trusted.

Current reviewed ABI-0.35 evidence is:

```text
reviewed functions                  2398
prototype functions checked         2398
prototype type measurements         8672
layout types                         163
callback signatures                   36
constants                            834
all C/Rust measurements             2664
symbol acquisitions                 2397
symbol type mismatches                 0
mismatches                             0
unaudited declarations                 0
```

Measured 2026-09-30 against `~/deps/cna-c-abi-0.35.0` (HEADLESS), built from
`libcna/cna` `next` `5b4edd6cc25d656e8eaf0aee304e75ae7ee5a90d`.

The 2,398-function slice covers every one of CNA's 3,202 exported C functions
that the projection binds; the other 804 are deliberate non-bindings, each
classified in `tools/c-api-inventory/classification.json`, and the census gates
on there being no route without one. Every new safe native facade route
must add its raw declaration and enter this manifest. The version gate follows
CNA's own `0.x` policy from `docs/c-api/ABI_VERSIONING.md`: the checked library
must report exactly minor 35, because a `0.x` minor bump may be a breaking
change -- 0.30, 0.33 and 0.34 were.
Current ELF/runtime evidence is Linux x86-64 only.

`symbol acquisitions` is a separate gate from `prototype functions checked`. It
derives the expected function alias from each acquired symbol's own name and
proves the table field it fills is typed with *that* route's prototype, so a
field wired to a neighbouring route's alias fails even though both aliases exist
and both prototypes are individually correct.
