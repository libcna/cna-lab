package org.openeggbert.cna.extensions.content;

/**
 * One renderable part's numeric state, without its bytes.
 *
 * <p>The vertex and index payloads travel separately, through
 * {@link CnbModelData#readPartVertexBytes} and {@link CnbModelData#readPartIndexBytes}, because a
 * part's geometry is the large thing and its description is the small one.
 *
 * <p>CNA's encoder checks the two against each other: {@code VertexStride * VertexCount} has to be
 * the vertex byte count, and {@code IndexElementSize * IndexCount} the index byte count. A model
 * whose description and payload disagree is refused at encode time rather than written.
 *
 * @param VertexStride bytes per vertex
 * @param VertexCount how many vertices the part has
 * @param IndexCount how many indices the part has
 * @param IndexElementSize bytes per index, two or four
 * @param PrimitiveTopology CNA's own topology identifier
 * @param PrimitiveCount how many primitives the indices describe
 * @param EffectKind which effect the part expects
 * @param VertexColorEnabled whether the vertices carry colours
 * @param Unlit whether the part is drawn without lighting
 */
public record CnbModelPart(
        int VertexStride,
        int VertexCount,
        int IndexCount,
        int IndexElementSize,
        int PrimitiveTopology,
        int PrimitiveCount,
        CnbEffectKind EffectKind,
        boolean VertexColorEnabled,
        boolean Unlit) {
}
