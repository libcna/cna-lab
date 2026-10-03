package org.openeggbert.cna.extensions.content;

/**
 * One draw range of a CNA model mesh, as a value.
 *
 * <p>The effect's type name is CNA's own runtime type name -- the same identity XNB records and
 * the managed reader turns into a Java {@code BasicEffect} or {@code SkinnedEffect}. It is
 * reported rather than turned into an object here, because the effect belongs to the model:
 * handing out a Java facade for it would create a second owner for something CNA frees with the
 * model.
 *
 * @param NumVertices how many vertices the range covers
 * @param PrimitiveCount how many primitives it draws
 * @param StartIndex where in the index buffer it starts
 * @param VertexOffset the offset added to every index
 * @param EffectTypeName the runtime type name of the effect the part draws with, empty when the
 *     part has none
 * @param HasVertexBuffer whether the part has a vertex buffer
 * @param HasIndexBuffer whether the part has an index buffer
 */
public record CnaModelMeshPart(
        int NumVertices,
        int PrimitiveCount,
        int StartIndex,
        int VertexOffset,
        String EffectTypeName,
        boolean HasVertexBuffer,
        boolean HasIndexBuffer) {
}
