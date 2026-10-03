# The XNA 4.0 Content Pipeline, measured — and the decision

**Task:** `JAVA-XNA-006`
**Measured:** 2026-08-31, against the seven SHA-256-pinned build-time assemblies in
`tools/api-compat/profiles/xna40-content-pipeline.json`
**Decision:** **partial / interop.** CNA-Java consumes compiled content and does not
reimplement Microsoft's build-time pipeline. The reasoning is below, and it is about
architecture rather than about size.

## What the profile contains

```text
PROFILE=XNA 4.0 Content Pipeline build-time
REFERENCE_TYPES=128        REFERENCE_MEMBERS=743
EXPECTED_JAVA_TYPES=134    EXPECTED_JAVA_MEMBERS=925
MISSING_TYPE=125           already present=3
abstract members a Java implementor would have to supply = 42
```

By namespace:

| Namespace | Types | What it is |
|---|---:|---|
| `…Content.Pipeline.Graphics` | 47 | the content value graph: nodes, meshes, bitmaps, materials |
| `…Content.Pipeline` | 32 | the framework: importers, contexts, identity, logging, discovery |
| `…Content.Pipeline.Processors` | 28 | the stock processors and their outputs |
| `…Content.Pipeline.Serialization.Intermediate` | 7 | the XML intermediate format |
| `…Content.Pipeline.Serialization.Compiler` | 5 | writing `.xnb` |
| `…Content.Pipeline.Audio` | 5 | audio content and conversion |
| `…Content.Pipeline.Tasks` | 4 | MSBuild tasks |

The 331 `UNEXPECTED_TYPE` diagnostics in the raw report are an artefact of measuring the whole
runtime projection against a build-time-only profile, exactly as the selected runtime profile
needed a subset rule before its zero meant anything. They are not a finding.

## Why size is not the argument

125 missing types is a large but ordinary amount of work; this binding has projected more than
that twice already. Four measured facts decide it instead.

**1. Two namespaces require types the pinned corpus does not contain.** The contract references
`Microsoft.Build.Framework.ITaskItem` 15 times and derives four types from
`Microsoft.Build.Utilities.Task`; `IntermediateSerializer` takes `System.Xml.XmlReader` and
`XmlWriter`. Those are MSBuild and the BCL, not XNA. Projecting them would mean inventing Java
stand-ins for types outside the reference corpus — the one thing the strict projection may never
do, because a stand-in has no authority to be measured against. `Tasks` (4 types) and
`Serialization.Intermediate` (7) are unreachable on those grounds alone, and they are precisely
the two that make the pipeline a *build system* rather than a library.

**2. The importers wrap Windows-only native technology whose output cannot be reproduced.** Ten
concrete importers ship in the profile. `FbxImporter` is Autodesk's FBX SDK. `EffectImporter`
is the legacy Direct3D HLSL compiler, and its output is compiled shader bytecode for a specific
D3D9-era compiler; a Java implementation would produce different bytes for the same source and
would therefore not be the same importer. `WmaImporter`, `WmvImporter` and `Mp3Importer` are
Windows Media. `XImporter` reads the DirectX `.x` format. A Java pipeline could offer importers
with the same *names* and different *results*, which is worse than not offering them: a game
built through it would not be the game XNA's pipeline builds.

**3. The output format's identity is a CLR assembly-qualified type name.** The one abstract
member every `ContentTypeWriter` must implement is
`String GetRuntimeReader(TargetPlatform)`, and what it returns is the assembly-qualified name of
the CLR reader the runtime will resolve. A Java pipeline emitting `.xnb` would have to write CLR
names for a CLR runtime it is not — and CNA-Java's own runtime already maps those names to Java
readers, so the interoperable direction is the one that already works: **read** the `.xnb` that
Microsoft's pipeline produced. Emitting Java type names instead would produce files no XNA
runtime reads, including this one.

**4. CNA already ships the content tooling this binding should consume.** CNA has its own
compiled format and its own compiler: `cna_cnb_compile_cnj` builds a `.cnb` from a `.cnj`
project, `cna_cnb_build_model_from_cnj` builds a model, and the encoders write textures, fonts,
sounds, songs, videos, curves and animation clips. A second, Java-side reimplementation of a
*different* vendor's build system would compete with the one the runtime underneath already has.

## What CNA-Java does instead

The consuming half, which is where a Java game's value is, exists and is verified:

- `ContentManager.Load` reads Microsoft's `.xnb` through the managed reader, including the
  shared-resource graph and the effect subclass XNB names.
- `org.openeggbert.cna.extensions.content` reads and **writes** CNA's own `.cnb`: the container,
  the checked byte reader, the texture data, and a Texture2D that a game draws with. A Java build
  step that wants to produce compiled content produces CNA's format with CNA's own writer.
- `CnaModel` gives CNA's runtime the XNA model graph, so the parts of the modern model layer that
  do not need Microsoft's pipeline are reachable.

## What would change the decision

Only one thing: an XNA-shaped build-time API that a Java game actually needs and that neither
CNA's own tooling nor the runtime readers can serve. The likeliest candidate is the value graph
in `…Content.Pipeline.Graphics` (47 types, no MSBuild or BCL dependency, no native importer)
as a *data model* for a Java tool that builds content through CNA's compiler rather than through
Microsoft's. That is a separate product with its own consumers, and it should be started when
one of them exists, not because the type count is reachable.

Until then `JAVA-XNA-006` is answered, and the profile stays pinned and unmeasured against the
runtime target — a build-time contract is not a runtime regression gate, and conflating them
would make the runtime profile's zero mean less than it does.
