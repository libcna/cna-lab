package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;

import java.util.NoSuchElementException;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

final class ModelGraphTests {

    @Test
    void modelGraphPreservesIdentityReadOnlyCollectionsAndTransformSnapshots() {
        ModelBone root = new ModelBone("Root", Matrix.CreateTranslation(1.0f, 0.0f, 0.0f), 0);
        ModelBone child = new ModelBone("Child", Matrix.CreateTranslation(0.0f, 2.0f, 0.0f), 1);
        root.setParentAndChildren(null, new ModelBone[]{child});
        child.setParentAndChildren(root, new ModelBone[0]);

        ModelMeshPart firstPart = new ModelMeshPart(0, 3, 0, 1, "part-0");
        ModelMeshPart secondPart = new ModelMeshPart(3, 3, 3, 1, "part-1");
        ModelMesh mesh = new ModelMesh(
                "Body", child, new BoundingSphere(new Vector3(1.0f, 2.0f, 3.0f), 4.0f),
                new ModelMeshPart[]{firstPart, secondPart}, "mesh-tag");
        Model model = new Model(
                new ModelBone[]{root, child}, new ModelMesh[]{mesh}, root, "model-tag");

        assertSame(root, model.getRoot());
        assertSame(root, model.getBones().get(0));
        assertSame(root, model.getBones().get("root"));
        assertSame(child, model.getBones().TryGetValue("CHILD"));
        assertNull(model.getBones().TryGetValue("missing"));
        assertThrows(NoSuchElementException.class, () -> model.getBones().get("missing"));
        assertThrows(NullPointerException.class, () -> model.getBones().TryGetValue(null));
        assertThrows(IllegalArgumentException.class, () -> model.getBones().TryGetValue(""));
        assertEquals(2, model.getBones().size());
        assertThrows(UnsupportedOperationException.class, () -> model.getBones().add(root));

        assertSame(mesh, model.getMeshes().get(0));
        assertSame(mesh, model.getMeshes().get("body"));
        assertSame(mesh, model.getMeshes().TryGetValue("BODY"));
        assertNull(model.getMeshes().TryGetValue("missing"));
        assertSame(firstPart, mesh.getMeshParts().get(0));
        assertSame(secondPart, mesh.getMeshParts().get(1));
        assertEquals(0, mesh.getEffects().size());
        assertSame(child, mesh.getParentBone());

        BoundingSphere sphere = mesh.getBoundingSphere();
        sphere.Center.X = 99.0f;
        sphere.Radius = 100.0f;
        assertEquals(1.0f, mesh.getBoundingSphere().Center.X);
        assertEquals(4.0f, mesh.getBoundingSphere().Radius);

        Matrix childTransform = child.getTransform();
        childTransform.M42 = 99.0f;
        assertEquals(2.0f, child.getTransform().M42);
        Matrix[] local = new Matrix[2];
        model.CopyBoneTransformsTo(local);
        assertEquals(1.0f, local[0].M41);
        assertEquals(2.0f, local[1].M42);
        local[0].M41 = 88.0f;
        assertEquals(1.0f, root.getTransform().M41);

        Matrix[] absolute = new Matrix[2];
        model.CopyAbsoluteBoneTransformsTo(absolute);
        assertEquals(1.0f, absolute[1].M41);
        assertEquals(2.0f, absolute[1].M42);
        Matrix replacementRoot = Matrix.CreateTranslation(5.0f, 6.0f, 7.0f);
        Matrix replacementChild = Matrix.CreateScale(2.0f);
        model.CopyBoneTransformsFrom(new Matrix[]{replacementRoot, replacementChild});
        replacementRoot.M41 = -1.0f;
        assertEquals(5.0f, root.getTransform().M41);
        assertEquals(2.0f, child.getTransform().M11);
        assertThrows(NullPointerException.class, () -> model.CopyBoneTransformsTo(null));
        assertThrows(IllegalArgumentException.class,
                () -> model.CopyBoneTransformsTo(new Matrix[1]));
        assertThrows(IllegalArgumentException.class,
                () -> model.CopyAbsoluteBoneTransformsTo(new Matrix[1]));
        assertThrows(IllegalArgumentException.class,
                () -> model.CopyBoneTransformsFrom(new Matrix[1]));

        ModelBoneCollection.Enumerator boneCursor = model.getBones().GetEnumerator();
        assertTrue(boneCursor.hasNext());
        assertTrue(boneCursor.MoveNext());
        assertSame(root, boneCursor.getCurrent());
        ModelBoneCollection.Enumerator copiedCursor =
                new ModelBoneCollection.Enumerator(boneCursor);
        assertSame(child, copiedCursor.next());
        assertFalse(copiedCursor.hasNext());
        copiedCursor.close();

        ModelMeshCollection.Enumerator meshCursor = model.getMeshes().GetEnumerator();
        assertSame(mesh, meshCursor.next());
        assertThrows(NoSuchElementException.class, meshCursor::next);
        ModelMeshPartCollection.Enumerator partCursor = mesh.getMeshParts().GetEnumerator();
        assertSame(firstPart, partCursor.next());
        assertSame(secondPart, partCursor.next());
        assertFalse(partCursor.MoveNext());
        ModelEffectCollection.Enumerator effectCursor = mesh.getEffects().GetEnumerator();
        assertFalse(effectCursor.MoveNext());

        assertEquals("model-tag", model.getTag());
        model.setTag("new-model-tag");
        assertEquals("new-model-tag", model.getTag());
        assertEquals("mesh-tag", mesh.getTag());
        assertEquals("part-0", firstPart.getTag());
        firstPart.setTag("new-part-tag");
        assertEquals("new-part-tag", firstPart.getTag());
        assertNotSame(model.getBones().GetEnumerator(), model.getBones().GetEnumerator());
    }
}
