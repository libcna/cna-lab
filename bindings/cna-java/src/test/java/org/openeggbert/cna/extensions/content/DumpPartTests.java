package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Graphics.BufferUsage;
import Microsoft.Xna.Framework.Graphics.IndexBuffer;
import Microsoft.Xna.Framework.Graphics.IndexElementSize;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.VertexBuffer;
import Microsoft.Xna.Framework.Graphics.VertexPositionColor;
import Microsoft.Xna.Framework.Matrix;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.util.List;
import java.util.Map;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class DumpPartTests {
    private static void step(String what, Runnable body) {
        try {
            body.run();
            System.out.println("DUMP ok      " + what);
        } catch (RuntimeException e) {
            System.out.println("DUMP REFUSED " + what + ": " + e.getMessage());
        }
    }

    @Test
    void dump() {
        CnaSkinnedModelProbe.run(device -> {
            VertexBuffer vertices = new VertexBuffer(device,
                    VertexPositionColor.VertexDeclaration, 3, BufferUsage.None);
            IndexBuffer indices = new IndexBuffer(device, IndexElementSize.SixteenBits, 3,
                    BufferUsage.None);
            Texture2D texture = new Texture2D(device, 2, 2);
            CnaSkeleton skeleton = new CnaSkeleton(List.of(-1),
                    List.of(Matrix.getIdentity()), List.of(Matrix.getIdentity()), List.of());
            CnaSkinnedModel model = CnaSkinnedModel.of(skeleton, Map.of());
            CnaModelMeshPartHandle part = CnaModelMeshPartHandle.create(
                    vertices, indices, 3, 1, 0, 0);
            step("addPart", () -> model.addPart("body", vertices, indices, part, texture));
            step("getPart+close", () -> {
                CnaSkinnedModel.Part read = model.getPart(0);
                read.close();
            });
            step("model.close", model::close);
            step("part.close", part::close);
            step("texture.close", texture::close);
            step("indices.close", indices::close);
            step("vertices.close", vertices::close);
        });
    }
}
