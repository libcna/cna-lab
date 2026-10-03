package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Graphics.IndexBuffer;
import Microsoft.Xna.Framework.Graphics.ModelMeshPart;
import Microsoft.Xna.Framework.Graphics.VertexBuffer;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeModelExtensionRoutes;

import java.util.Objects;

/**
 * CNA's own view of one mesh part, which a skinned model draws.
 *
 * <p>A CNA extension. The XNA {@link ModelMeshPart} a Java game has is a managed object with no
 * native side -- the managed content reader built it, and CNA never saw it -- so a skinned model
 * that draws it needs CNA's own. This is that, made from the same four numbers and two buffers
 * XNA's part carries.
 *
 * <p><strong>The part is always the caller's, and the close order is a chain.</strong> Measured
 * rather than assumed, because the wording invites the other reading:
 * {@link CnaSkinnedModel#addPart} <em>retains</em> the part, it does not take it. A model that has
 * released everything it owns still leaves the part alive, and the part still retains the vertex
 * and index buffers it was made from -- so destroying a buffer first is refused with
 * {@code INVALID_STATE}, naming the part that holds it.
 *
 * <p>The order is therefore: close the model, then the part, then the buffers. In a
 * try-with-resources that means declaring them the other way round, and getting it wrong is what
 * found the rule.
 */
public final class CnaModelMeshPartHandle implements AutoCloseable {

    static {
        // extensions.graphics's LodGroup holds levels made of these, and a handle that is
        // package-private cannot be handed to another package any other way. The reader is
        // registered from here so the handle never appears in a signature.
        org.openeggbert.cna.internal.ExtensionHandles.registerMeshPartReader(
                part -> ((CnaModelMeshPartHandle) part).value());
    }

    private final long handle;

    /** Whether closing this frees the part, which an alias a model lent back does not. */
    private final boolean owned;
    private boolean closed;

    CnaModelMeshPartHandle(long handle, boolean owned) {
        this.handle = handle;
        this.owned = owned;
    }

    /**
     * Creates CNA's own mesh part from the same description an XNA one carries.
     *
     * @param vertexBuffer the part's vertices
     * @param indexBuffer the part's indices
     * @param numVertices how many vertices the part draws
     * @param primitiveCount how many primitives it draws
     * @param startIndex where in the index buffer it starts
     * @param vertexOffset what to add to every index
     * @return the part, which the caller owns and closes after any model that retains it
     */
    public static CnaModelMeshPartHandle create(VertexBuffer vertexBuffer,
            IndexBuffer indexBuffer, int numVertices, int primitiveCount, int startIndex,
            int vertexOffset) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(vertexBuffer, "vertexBuffer");
        Objects.requireNonNull(indexBuffer, "indexBuffer");
        long[] created = new long[1];
        CnbExtension.check("CnaModelMeshPartHandle.create",
                NativeModelExtensionRoutes.modelMeshPartCreate(
                        NativeBindings.nativeResourceHandle(vertexBuffer),
                        NativeBindings.nativeResourceHandle(indexBuffer),
                        numVertices, primitiveCount, startIndex, vertexOffset, created));
        return new CnaModelMeshPartHandle(created[0], true);
    }

    /**
     * Creates CNA's own mesh part from an XNA one.
     *
     * @param part the managed part to mirror
     * @return the part, which the caller owns and closes after any model that retains it
     */
    public static CnaModelMeshPartHandle of(ModelMeshPart part) {
        Objects.requireNonNull(part, "part");
        return create(part.getVertexBuffer(), part.getIndexBuffer(), part.getNumVertices(),
                part.getPrimitiveCount(), part.getStartIndex(), part.getVertexOffset());
    }

    /**
     * Returns which primitive the part draws.
     *
     * @return the primitive type
     */
    public Microsoft.Xna.Framework.Graphics.PrimitiveType getPrimitiveType() {
        int[] value = new int[1];
        CnbExtension.check("CnaModelMeshPartHandle.getPrimitiveType",
                NativeModelExtensionRoutes.modelMeshPartGetPrimitiveTypeExt(value(), value));
        return Microsoft.Xna.Framework.Graphics.PrimitiveType.values()[value[0]];
    }

    /**
     * Sets which primitive the part draws.
     *
     * <p>A CNA extension: XNA's {@code ModelMeshPart} always draws a triangle list, and glTF has
     * five more topologies than that.
     *
     * @param type the primitive type
     */
    public void setPrimitiveType(Microsoft.Xna.Framework.Graphics.PrimitiveType type) {
        Objects.requireNonNull(type, "type");
        CnbExtension.check("CnaModelMeshPartHandle.setPrimitiveType", NativeModelExtensionRoutes
                .modelMeshPartSetPrimitiveTypeExt(value(), type.ordinal()));
    }

    /**
     * Returns how one of the part's texture slots is sampled.
     *
     * @param slot which texture slot to ask about
     * @return the sampler state
     */
    public Microsoft.Xna.Framework.Graphics.SamplerState getSamplerState(
            org.openeggbert.cna.extensions.graphics.PbrTextureSlot slot) {
        Objects.requireNonNull(slot, "slot");
        long[] integral = new long[7];
        float[] floating = new float[1];
        CnbExtension.check("CnaModelMeshPartHandle.getSamplerState", NativeModelExtensionRoutes
                .modelMeshPartGetSamplerStateExt(value(), slot.ordinal(), integral, floating));
        Microsoft.Xna.Framework.Graphics.SamplerState state =
                new Microsoft.Xna.Framework.Graphics.SamplerState();
        state.setAddressU(Microsoft.Xna.Framework.Graphics.TextureAddressMode
                .values()[(int) integral[0]]);
        state.setAddressV(Microsoft.Xna.Framework.Graphics.TextureAddressMode
                .values()[(int) integral[1]]);
        state.setAddressW(Microsoft.Xna.Framework.Graphics.TextureAddressMode
                .values()[(int) integral[2]]);
        state.setFilter(Microsoft.Xna.Framework.Graphics.TextureFilter
                .values()[(int) integral[3]]);
        state.setMaxAnisotropy((int) integral[4]);
        state.setMaxMipLevel((int) integral[5]);
        state.setMipMapLevelOfDetailBias(floating[0]);
        return state;
    }

    /**
     * Sets how one of the part's texture slots is sampled.
     *
     * <p>A CNA extension: an XNA {@code ModelMeshPart} has no sampler of its own -- the device's
     * is used -- and glTF states one per texture.
     *
     * @param slot which texture slot to set
     * @param state how to sample it
     */
    public void setSamplerState(org.openeggbert.cna.extensions.graphics.PbrTextureSlot slot,
            Microsoft.Xna.Framework.Graphics.SamplerState state) {
        Objects.requireNonNull(slot, "slot");
        Objects.requireNonNull(state, "state");
        CnbExtension.check("CnaModelMeshPartHandle.setSamplerState", NativeModelExtensionRoutes
                .modelMeshPartSetSamplerStateExt(value(), slot.ordinal(),
                        new long[] {state.getAddressU().ordinal(), state.getAddressV().ordinal(),
                                state.getAddressW().ordinal(), state.getFilter().ordinal(),
                                state.getMaxAnisotropy(), state.getMaxMipLevel(), 0L},
                        new float[] {state.getMipMapLevelOfDetailBias()}));
    }

    /**
     * Returns the morph data the part blends with, or {@code null} when it has none.
     *
     * <p>The handle is the part's own: closing what comes back would take the part's morph data
     * away, so this hands back a view that does not own it.
     *
     * @return the morph data, or null
     */
    public CnaMorphTargetData getMorphTargetData() {
        boolean[] present = new boolean[1];
        long[] data = new long[1];
        CnbExtension.check("CnaModelMeshPartHandle.getMorphTargetData", NativeModelExtensionRoutes
                .modelMeshPartGetMorphTargetDataExt(value(), present, data));
        return present[0] ? CnaMorphTargetData.borrow(data[0]) : null;
    }

    /**
     * Attaches morph data to the part, which the part then retains.
     *
     * @param data the morph data; it stays the caller's to close, after the part
     */
    public void setMorphTargetData(CnaMorphTargetData data) {
        Objects.requireNonNull(data, "data");
        CnbExtension.check("CnaModelMeshPartHandle.setMorphTargetData", NativeModelExtensionRoutes
                .modelMeshPartSetMorphTargetDataExt(value(), data.handle()));
    }

    /**
     * Re-blends the part's vertex buffer from its attached morph data at these weights.
     *
     * <p>The whole point of attaching morph data to a part: one call moves the vertices on the
     * GPU, so a face that smiles is a weight rather than a second mesh.
     *
     * @param weights one weight per morph target
     */
    public void setMorphWeights(float[] weights) {
        Objects.requireNonNull(weights, "weights");
        CnbExtension.check("CnaModelMeshPartHandle.setMorphWeights", NativeModelExtensionRoutes
                .modelMeshPartSetMorphWeightsExt(value(), weights.clone()));
    }

    /** The native handle, for the model that takes it. */
    long value() {
        if (closed) {
            throw new IllegalStateException("this CnaModelMeshPartHandle is closed");
        }
        return handle;
    }

    /**
     * Releases the part, when this handle is the owning one rather than a model's alias.
     *
     * <p>Closing twice is a no-op. Close it <strong>after</strong> every model that retains it
     * and <strong>before</strong> the buffers it was made from.
     */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        if (owned) {
            CnbExtension.check("CnaModelMeshPartHandle.close",
                    NativeModelExtensionRoutes.modelMeshPartDestroy(handle));
        }
    }
}
