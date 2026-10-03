package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.function.Function;

/**
 * Dispatches a {@code .cnb} document to the loader that may read it.
 *
 * <p><strong>This is a Java registry, not a projection of CNA's.</strong> CNA's registry takes a C
 * function pointer and hands back a {@code void*}, and its own header is explicit that "only a
 * loader registered from C produces something C can hold" -- its built-in loaders construct C++
 * objects, and the invoke route answers {@code NOT_SUPPORTED} rather than returning a pointer
 * nothing could name. A Java loader is a Java function returning a Java object, so there is no
 * pointer to pass and nothing to receive. See {@code docs/cnb-loader-registry-decision.md}.
 *
 * <p>What is worth keeping is CNA's <em>identity rule</em>, which this implements exactly.
 * A built-in asset type's number is authoritative: CNA assigns those and they are frozen, so a
 * match proves identity. A custom one's is not -- a custom identifier is a 31-bit hash, so two
 * unrelated games' types can legitimately collide -- and a custom-typed file must therefore also
 * carry a canonical type name equal to the registered one. A file whose number matches and whose
 * name does not is refused, because decoding it would silently misinterpret someone else's
 * content. Getting that wrong is the whole reason this is not a {@code HashMap} in user code.
 *
 * <p>The registry is per-instance rather than process-wide, which is the one place it deliberately
 * differs from CNA's: a Java game that wants one registry can hold one, and a tool that loads two
 * games' content in one process is not forced to share a table with itself.
 *
 * @param <T> what this registry's loaders produce
 */
public final class CnbLoaders<T> {

    private final Map<Integer, Registration<T>> loaders = new HashMap<>();

    /** Creates an empty registry. */
    public CnbLoaders() {
    }

    /**
     * Registers a loader for one of CNA's built-in asset types.
     *
     * @param assetType the built-in type this loader reads
     * @param loader the function that turns a document into an object
     * @return this registry
     * @throws IllegalArgumentException when the type is a custom one, which needs its name
     */
    public CnbLoaders<T> register(CnbAssetType assetType, Function<CnbDocument, T> loader) {
        Objects.requireNonNull(assetType, "assetType");
        if (assetType.isCustom()) {
            throw new IllegalArgumentException(
                    "a custom asset type must be registered with its canonical name, because its "
                    + "identifier is a hash two unrelated types can share");
        }
        return put(assetType, "", loader);
    }

    /**
     * Registers a loader for a game's own asset type.
     *
     * @param canonicalTypeName the type's canonical name, exactly the string its identifier was
     *        minted from; it is compared against the name the file itself carries before dispatch
     * @param loader the function that turns a document into an object
     * @return this registry
     */
    public CnbLoaders<T> registerCustom(
            String canonicalTypeName, Function<CnbDocument, T> loader) {
        Objects.requireNonNull(canonicalTypeName, "canonicalTypeName");
        if (canonicalTypeName.isEmpty()) {
            throw new IllegalArgumentException("a canonical type name must not be empty");
        }
        return put(CnbAssetType.custom(canonicalTypeName), canonicalTypeName, loader);
    }

    /**
     * Reads a document with whichever registered loader may read it.
     *
     * @param document the document to load
     * @return what that loader produced
     * @throws ContentNotSupportedException when nothing is registered for the document's type
     * @throws CnbFormatException when the type is custom and the file's canonical name is absent
     *         or disagrees with the registered one
     */
    public T load(CnbDocument document) {
        Objects.requireNonNull(document, "document");
        CnbAssetType assetType = document.getAssetType();
        Registration<T> registration = loaders.get(assetType.Id());
        if (registration == null) {
            throw new ContentNotSupportedException(
                    "no loader is registered for asset type " + assetType.Id());
        }
        if (assetType.isCustom()) {
            String declared = document.getMetadata().AssetTypeName();
            if (declared.isEmpty()) {
                throw new CnbFormatException(
                        "this file's asset type is custom and it carries no canonical type name, "
                        + "so there is nothing to check its identifier against");
            }
            if (!declared.equals(registration.canonicalTypeName())) {
                throw new CnbFormatException("this file says it is '" + declared
                        + "' and the loader registered under that identifier reads '"
                        + registration.canonicalTypeName()
                        + "'; the two collide on a 31-bit hash and are different types");
            }
        }
        return registration.loader().apply(document);
    }

    /**
     * Reports whether this registry has a loader for one asset type.
     *
     * @param assetType the type to ask about
     * @return true when one is registered
     */
    public boolean isRegistered(CnbAssetType assetType) {
        Objects.requireNonNull(assetType, "assetType");
        return loaders.containsKey(assetType.Id());
    }

    /**
     * Reports whether <strong>CNA's own</strong> registry has a loader for one asset type.
     *
     * <p>A different question from {@link #isRegistered}, and worth asking: it says whether the
     * native side can read a file, which is what decides whether a document needs a Java loader at
     * all. It is a query only -- CNA's loaders build C++ objects that no Java call could receive.
     *
     * @param assetType the type to ask about
     * @return true when CNA has a loader for it
     */
    public static boolean isRegisteredWithCna(CnbAssetType assetType) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(assetType, "assetType");
        // Curve and AnimationClip are the two built-ins that need nothing but their own codec, so
        // they are the two a process can have without constructing a content manager. Asking for
        // them installs them; the call is idempotent.
        CnbExtension.check("CnbLoaders.isRegisteredWithCna",
                NativeCnbRoutes.cnbLoaderRegistryRegisterBuiltins());
        boolean[] registered = new boolean[1];
        CnbExtension.check("CnbLoaders.isRegisteredWithCna", NativeCnbRoutes
                .cnbLoaderRegistryIsRegistered(assetType.Id(), registered));
        return registered[0];
    }

    /**
     * Returns the canonical type name CNA's own registry holds for an asset type.
     *
     * @param assetType the type to ask about
     * @return the registered name, empty when nothing is registered for it
     */
    public static String getCnaRegisteredTypeName(CnbAssetType assetType) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(assetType, "assetType");
        CnbExtension.check("CnbLoaders.getCnaRegisteredTypeName",
                NativeCnbRoutes.cnbLoaderRegistryRegisterBuiltins());
        return CnbExtension.text("CnbLoaders.getCnaRegisteredTypeName",
                bytes -> NativeCnbRoutes
                        .cnbLoaderRegistryGetRegisteredTypeNameSize(assetType.Id(), bytes),
                (destination, bytes) -> NativeCnbRoutes
                        .cnbLoaderRegistryCopyRegisteredTypeName(
                                assetType.Id(), destination, bytes));
    }

    private CnbLoaders<T> put(
            CnbAssetType assetType, String canonicalTypeName, Function<CnbDocument, T> loader) {
        Objects.requireNonNull(loader, "loader");
        Registration<T> existing = loaders.get(assetType.Id());
        if (existing != null && !existing.canonicalTypeName().equals(canonicalTypeName)) {
            // CNA's rule, kept: letting the second registration win would mean loading one game
            // type's file with another's loader.
            throw new IllegalStateException("asset type " + assetType.Id()
                    + " is already registered as '" + existing.canonicalTypeName()
                    + "', and '" + canonicalTypeName + "' hashes to the same identifier");
        }
        loaders.put(assetType.Id(), new Registration<>(canonicalTypeName, loader));
        return this;
    }

    private record Registration<T>(String canonicalTypeName, Function<CnbDocument, T> loader) {
    }
}
