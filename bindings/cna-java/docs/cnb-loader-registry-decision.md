# The `.cnb` loader registry: a Java registry, not a projection

**Decided:** 2026-08-31, against CNA ABI 0.21.0.
**Backlog:** `JAVA-EXT-003`.

## The question

CNA has a loader registry: a process-wide table mapping a `.cnb` asset type identifier to the
code that turns such a file into a runtime object. Ten routes publish it. The obvious move was to
project all ten and be done.

## What the headers say

Two of the ten decide it, and `cnb.h` says both outright rather than leaving them to be
discovered.

`cna_cnb_loader_registry_register` takes a `CNA_CnbLoaderCallback` and a `void* context` that
"must outlive the registration, which is process-wide". Projecting it means a JNI callback that
outlives every content manager, holding a global reference to a Java object for the life of the
process, with a C-to-Java exception barrier on a path CNA calls during content loading.

`cna_cnb_loader_invoke` returns the loaded object through a `void**`, and the header is explicit
about what that can be:

> Only a loader registered from C produces something C can hold. CNA's own built-in loaders
> construct C++ objects -- a `Curve`, a `Texture2D` -- and this route says so rather than handing
> back a pointer whose type nothing in C could name.

A Java loader returns a Java object. There is no pointer to give CNA and none to receive back. The
`void**` is not an obstacle to work around; it is the boundary saying this table is for C loaders.

## The decision

**`org.openeggbert.cna.extensions.content.CnbLoaders` is a Java registry that implements CNA's
dispatch rule, and CNA's own registry is projected as a query only.**

What is worth keeping from CNA's registry is not its storage -- a map is a map -- but its
**identity rule**, which is subtle and which a hand-rolled `HashMap` gets wrong:

- A **built-in** asset type's number is authoritative. CNA assigns those and they are frozen, so a
  match is a proof of identity and the file's canonical type name is not consulted.
- A **custom** one's number is not. A custom identifier is a 31-bit hash of the type's name, so two
  unrelated games' types can legitimately collide. A custom-typed file must therefore also carry a
  canonical type name, and that name must equal the registered one. A file whose number matches and
  whose name does not is refused, because decoding it would silently misinterpret someone else's
  content.
- Registering the same identifier twice under different names is refused for the same reason:
  letting the second win would load one game type's file with another's loader.

`CnbLoaders` implements all three. Every fact it needs is already readable from a `CnbDocument`:
the asset type identifier from the header, the canonical name from the `CMET` chunk.

Two static queries do reach CNA's own table, because "can the native side read this?" is a real
question a tool asks and Java cannot answer for itself: `isRegisteredWithCna` and
`getCnaRegisteredTypeName`. Both are queries. Neither can invoke, and the Javadoc says why.

## What is deliberately not projected

| Route | Why |
|---|---|
| `cna_cnb_loader_registry_register` | Needs a C callback and a process-lifetime context |
| `cna_cnb_loader_invoke` | Returns a `void*` no Java call could receive |
| `cna_cnb_loader_registry_find` | Its own header calls it "the wrong entry point for loading a file"; the resolve route is the right one, and that too can only produce a loader Java cannot invoke |
| `cna_cnb_loader_registry_resolve_for_document` | Same: it resolves a loader that only C can call |
| `cna_cnb_loader_registry_clear` / `_remove` | Withdrawing a process-wide registration a Java caller never made |
| `cna_cnb_loader_destroy` | Releases a loader handle nothing here obtains |

## One place this differs from CNA on purpose

CNA's registry is process-wide, matching how the `.xnb` reader table already works.
`CnbLoaders` is per-instance. A Java game that wants one registry holds one; a tool that loads two
games' content in one process is not forced to share a table with itself. Nothing about the
identity rule depends on the table being global, and a global table in Java would be a static
mutable singleton with no way to reset it -- the thing CNA's own `_clear` exists to work around.
