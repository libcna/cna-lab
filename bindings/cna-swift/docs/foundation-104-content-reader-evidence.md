# Foundation 104: the XNA ContentReader closure

## 1. Start state

The selected work began from `722d9255a9240fb6a67ee02c02b712250e4cd9fd`
on local `develop`, equal to `origin/develop`. The writable template was on
`402cbd0`; the read-only dependency repositories were inspected but never
edited. The baseline strict report had 233 target types, 225 complete types, 8
partial types, 24 missing types and 43 diagnostics.

## 2. Baseline

The live report reproduced the five requested missing types and the two absent
protected `ContentManager` members. The selected scope was explicitly limited
to the runtime reader family; neither Design/`InstanceDescriptor` nor XACT
asset policy was reopened.

## 3. Filesystem safety

A dedicated session temp root was created before the work and passed as the
isolated `HOME`, `XDG_CACHE_HOME`, `XDG_DATA_HOME` and module-cache roots.
Individual tests create uniquely named children beneath that root and remove
only the exact child they created. Repository-owned `build-asan`, `build-tsan`
and consumer build
directories are documented build artifacts. Foreign paths touched: none.
Foreign deletions: 0.

## 4. Content-reader dependency closure

`docs/generated/content-reader-closure.json` is the machine-readable closure.
It reads all five type shapes from the pinned XNA contract, classifies every
public/protected BCL member extracted from the admitted .NET Framework 4.0 IL,
records private behavioural dependencies, route relevance and fixture needs,
and refuses an unclassified member. Its public-Encoding and BinaryReader
constructor negative controls execute on every regeneration.

The five XNA types declare 32 members in total. `ContentReader` declares 20 and
inherits 25 selected members from `BinaryReader`; `ContentTypeReader` declares
6; `ContentTypeReader<T>` 3; `ContentTypeReaderManager` 1; and
`ResourceContentManager` 2. The tiny transitive public support closure is
`IDisposable`, `Action<T>`, `EndOfStreamException` and `FormatException`.

## 5. BinaryReader authority

Authority is .NET Framework 4.0 `mscorlib.dll`, assembly version 4.0.0.0,
SHA-256 `5634668d4775b0113f08ea31093b281fea69bfc4e99227f5ca761b4ed98acc63`.
The authority exposes 28 relevant declarations. Twenty-six are selected: the
one-argument Stream constructor for internal base construction plus 25
public/protected members inherited by `ContentReader`. Two are refused:
`BinaryReader(Stream, Encoding)` because constructors are not inherited and no
selected signature names Encoding, and `ReadDecimal` because no selected XNA
member reaches Decimal. Public Encoding dependencies are zero.

## 6. BinaryReader Swift projection

`CNABinaryReader` is an open Swift class that owns a real Foundation
`InputStream`; `ContentReader` is its real subclass. It preserves BaseStream
identity, opens lazily, tracks buffered bytes without claiming seek support,
decodes every integer explicitly little-endian, implements IEEE single/double,
seven-bit UTF-8 strings and UTF-16 character units, and distinguishes EOF from
legal short array reads. Close/Dispose are idempotent, close the underlying
stream, and every later read refuses use.

## 7. ContentTypeReader

The non-generic type is an open abstract-in-CLR base with the exact measured
TargetType, TypeVersion, Initialize, CanDeserializeIntoExistingObject and
erased Read surface. Swift has no abstract class declaration, so its widened
construction/hooks throw a deliberate unsupported error if a subclass fails
to override Read; no object is fabricated.

## 8. ContentTypeReader<T>

`ContentTypeReaderOfT<T>` is a real generic subclass. `TargetType` is the
stable `T.self` metatype, the typed Read hook keeps `T?` existing-instance
state, and the erased override is only an internal dispatch bridge. A wrong
existing type is refused, and a reader asked to fill an existing reference may
not silently return a different instance.

## 9. ContentTypeReaderManager

Pinned Windows XNA metadata exposes exactly one selected public member,
`GetTypeReader(Type)`; it exposes no public `AddTypeCreator`. The replacement
for CLR reflection is therefore private implementation state, not invented
XNA API. Its recursive lock, case-sensitive name keys, duplicate refusal,
rollback after failed initialization and target-type identity make the process
registry deterministic and independent of test ordering. Creators are invoked
once for one registered reader identity.

## 10. ContentReader

The sealed reader consumes bytes through its BinaryReader base. All 20 declared
members are projected: the eight object/raw-object forms, shared and external
references, six math/color helpers, the single/double overrides, ContentManager
and AssetName. Heterogeneous readers are retained without exposing a type-erased
box. Reader markers use XNA's zero-for-null, otherwise one-based encoding.

The canonical routes named `cna_content_reader_*` take
`CNA_StorageStreamHandle` and implement save-game/storage transport. They are
semantically unrelated and deliberately unbound.

## 11. XNB runtime reader path

The smallest faithful managed runtime path validates `XNB`, Windows platform
`w`, XNA version/profile flags and declared file size; reads the type-reader
manifest and versions; reads the primary object; and then reads and fixes up
all shared resources in order. Relative external references are combined with
the source asset directory, normalized through `TitleContainer.GetCleanPath`
and re-enter the same manager/cache.

Uncompressed Windows XNB v5 is supported. The XNA compressed flag is recognized
and fails with the pinned decompression message. LZX is not implemented and
non-XNA LZ4 is not claimed; compression does not block the complete
uncompressed public path.

## 12. Project-authored XNB fixture

`Foundation104ContentReaderTests` writes the envelope, manifest, reader IDs,
primitive fields, string, vector, external-reference name and shared object
byte-by-byte. It downloads nothing and stages no binary fixture. The primary
proof enters through public `ContentManager.Load<T>`; internal reader tests are
additional, not substitutes. The same bytes are returned by a project-owned
ResourceManager subclass for the resource-content proof.

## 13. ContentManager.OpenStream

The widened protected member combines RootDirectory with `<asset>.xnb`, uses
the existing XNA path cleaner, reproduces Windows-invalid-character ordering,
and wraps missing/open failures in `ContentLoadException`. Managed-only or
absolute-root managers use Foundation `InputStream`; a live runtime-relative
manager keeps the existing TitleContainer route. The stream is owned by the
reader and deterministically closed.

## 14. ContentManager.ReadAsset

The widened protected generic member now opens a real stream, creates the real
ContentReader and invokes its XNB path. It preserves exception order, requested
type checks and the caller's disposable-registration callback. Public Load
owns the single normalized, case-insensitive cache; wrong-type hits refuse
rather than reload, so ReadAsset never creates a competing cache.

## 15. Existing CNA typed loaders

The hybrid remains deliberate. Existing built-in XNA asset kinds stay on their
admitted CNA typed backend; custom/public reader paths use managed XNB. Of the
six CNA loader names, only Texture2D currently has a truthful adopted-object
route; the other five remain unbound rather than returning handles their types
cannot faithfully adopt. Load selects exactly one backend before inserting one
cache entry, so an asset is not loaded twice through incompatible systems.

## 16. ResourceManager authority

Authority is the same .NET Framework 4.0 `mscorlib.dll`, not System.dll. Of 26
authority-visible declarations, seven are selected: protected construction,
GetObject(String), GetString(String), ReleaseAllResources, BaseName, IgnoreCase
and ResourceSetType. Nineteen constructors, fields and overloads reaching
Assembly, CultureInfo, ResourceSet, Hashtable, Version,
UltimateResourceFallbackLocation or UnmanagedMemoryStream are refused under the
demand rule.

## 17. ResourceManager Swift projection

`CNAResourceManager` is self-contained Swift runtime code. Its virtual lookup
path has deterministic case-sensitive/default and IgnoreCase behavior, cached
factory values, stable BaseName and ResourceSetType, exact null-name/type-error
exceptions, and observable ReleaseAllResources reload semantics. It needs no
Microsoft assembly at runtime and reads no machine-global application resource.

## 18. ResourceContentManager

The exact constructor and OpenStream override are projected. OpenStream calls
the selected virtual ResourceManager.GetObject with the asset key, validates a
byte-array resource, records its length and returns a stream over those bytes to
the same ContentReader path. CNA's similarly named placeholder manager is not
bound or treated as authority.

## 19. Ownership, disposal and shared resources

InputStream is closed by BinaryReader/ContentReader; repeated close is safe.
ContentManager owns each recorded disposable once and Unload clears both the
asset and disposable caches after disposal. A ReadAsset callback receives the
same created object. Shared-resource slots deserialize once before all queued
fixups run, preserving duplicate identity and preventing double disposal.
Manager disposal is explicit; Swift deinit remains only a backstop.

## 20. Fallibility, nullability and messages

The generated inventories now cover 840 accessors and 369 reference-return
positions with no unresolved projection disagreement. Content errors cover
missing assets, invalid/truncated/version/platform XNB, unknown/duplicate/wrong-
version readers, wrong requested/existing types, malformed shared indices, EOF,
disposed readers/managers and invalid external paths. Ninety-seven XNA resource
strings are pinned from registered assemblies; message coverage has zero
findings across 2,259 implemented members.

## 21. BCL mutation evidence

The authority audit runs 514 internal mutations, 247 second-disassembler
cross-checks and four negative controls. Foundation 104 additionally kills the
BinaryReader endian, seven-bit string length, BaseStream and read-after-close
defects, plus ResourceManager wrong-key/case/release behavior. The closure's
planted public Encoding parameter and Encoding-constructor demand controls both
fail as required.

## 22. Content mutation evidence

The focused campaign kills all 11 requested source defects: endian swap,
seven-bit off-by-one, zero-based reader index, skipped shared resource,
unnormalized external reference, wrong generic TargetType, wrong creator name,
cache-bypassing ReadAsset, root-ignoring OpenStream,
ResourceContentManager bypass and ResourceManager wrong-key lookup. The public
helper leak is independently planted and caught, with exact source restoration.

## 23. Strict scoreboard

The selected change moves the strict scoreboard from 233 to 238 target types,
225 to 231 complete types, 8 to 7 partial types, 24 to 19 missing types, and 43
to 36 total diagnostics. Missing-type diagnostics move 24 to 19 and missing-
member diagnostics 16 to 14; the three absent-overload diagnostics are
unchanged. Every disagreement category, both leak families, the allowlist and
the unmeasured structural category remain zero. BCL authority grows from 44 to
49 types and from 514 to 577 selected members.

## 24. Remaining non-selected decisions

The Design family still needs the owner's `InstanceDescriptor`/reflection
choice. The XACT family still needs the owner's project-owned `.xgs` and bank
asset policy; remaining microphone/audio capability is outside this selected
managed closure. They were not reopened.

## 25. Qualification

Debug/release and warnings-as-errors builds pass. Debug/release, ASan and TSan
each execute all 978 tests with zero failures and zero skips. Symbol Graph,
strict/leak verification, authority audits, focused behavior and mutations,
native ABI, package qualification, deterministic archive, isolated consumer,
and the final full mutation campaign are recorded by their generated reports.

## 26. Package and consumer

The package canary names BinaryReader, ResourceManager, ContentReader,
ContentTypeReader<T>, ContentTypeReaderManager, ResourceContentManager and
IDisposable from an isolated external package. It checks real inheritance,
metatype identity, a little-endian read and managed-only ContentManager
construction; the full byte fixture stays in package tests rather than turning
the consumer into a showcase.

## 27. Template

The unchanged template remains only a package/runtime canary. Its package
build/import and headless 60- and 600-frame runs are final qualification gates;
no content fixture is added to it.

## 28. Git

The work is kept as coherent local commits on `develop` and is not pushed.
`cnanext` and `sharp-runtimenext` remain unmodified by this session; pre-existing
foreign dependency dirt is reported, never cleaned or staged. No Microsoft
binary or disassembly body is staged.

## 29. Selected-scope stop

The machine-readable closure records zero actionable local work, zero
unreviewed Content members, zero unreviewed BCL closure members, zero selected
missing types, zero selected strict diagnostics and zero public Encoding
dependencies. BinaryReader and ResourceManager authority findings are both
zero.

## 30. Next frontier

Only the still-unselected Design decision and XACT/audio asset/capability
decision remain as context. There is no local ContentReader follow-up.
