package org.openeggbert.cna.internal;

import java.util.Objects;

/**
 * Reads the native handle out of one extension object, for another extension package.
 *
 * <p><strong>Why this exists.</strong> A CNA extension object's handle is package-private, which
 * is what keeps a raw handle out of every public signature. That works until two extension
 * packages have to hand one to the other -- {@code extensions.graphics}'s {@code LodGroup} holds
 * levels made of {@code extensions.content}'s mesh parts -- and Java has no way to widen access
 * to one package without widening it to everyone.
 *
 * <p>So the carrier registers a reader for itself, once, from its own static initialiser, and the
 * handle stays where it was: this class is in {@code org.openeggbert.cna.internal} beside
 * {@link NativeBindings#nativeResourceHandle}, and no public or protected signature outside that
 * package returns what it reads. A second registration is refused rather than replacing the
 * first, so a handle reader cannot be swapped for another one after the fact.
 */
public final class ExtensionHandles {

    /** Reads a carrier's live native handle. Implemented by the carrier's own class. */
    @FunctionalInterface
    public interface Reader {

        /**
         * Returns the carrier's live native handle.
         *
         * @param carrier the extension object to read
         * @return its handle
         */
        long handleOf(Object carrier);
    }

    private static volatile Reader meshParts;

    private ExtensionHandles() {
    }

    /**
     * Registers how a CNA mesh part's handle is read. Called once, by that class.
     *
     * @param reader the reader
     * @throws IllegalStateException when one is already registered
     */
    public static void registerMeshPartReader(Reader reader) {
        Objects.requireNonNull(reader, "reader");
        synchronized (ExtensionHandles.class) {
            if (meshParts != null) {
                throw new IllegalStateException("a mesh-part handle reader is already registered");
            }
            meshParts = reader;
        }
    }

    /**
     * Returns the live handle behind one CNA mesh part.
     *
     * @param part the mesh part, whose class has registered its reader
     * @return the handle
     */
    public static long meshPart(Object part) {
        Reader reader = meshParts;
        if (reader == null) {
            throw new IllegalStateException(
                    "no mesh-part handle reader is registered; the mesh part class was never "
                            + "initialised, which cannot happen for a part a caller holds");
        }
        return reader.handleOf(part);
    }
}
