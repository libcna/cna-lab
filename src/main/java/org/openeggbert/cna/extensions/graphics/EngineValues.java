package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.BoundingBox;
import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;

import java.util.List;
import java.util.Objects;

/**
 * Flattens XNA's value types into the arrays the generated native boundary takes.
 *
 * <p>Not public and not an API: the generator projects a struct as its scalar leaves in
 * declaration order, so every extension family needs the same handful of conversions and
 * writing them once means one place can be wrong. The leaf order here is CNA's own
 * {@code CNA_Matrix}, {@code CNA_BoundingBox}, {@code CNA_BoundingSphere} and {@code CNA_Color}
 * declaration order, which is what the adapter reads.
 */
final class EngineValues {

    /** A matrix is sixteen floats, row-major, as CNA declares them. */
    static final int MATRIX_LEAVES = 16;

    /** A box is its two corners. */
    static final int BOX_LEAVES = 6;

    /** A sphere is its centre and its radius. */
    static final int SPHERE_LEAVES = 4;

    private EngineValues() {
    }

    static float[] floats(Matrix matrix, String name) {
        Objects.requireNonNull(matrix, name);
        return new float[] {
            matrix.M11, matrix.M12, matrix.M13, matrix.M14,
            matrix.M21, matrix.M22, matrix.M23, matrix.M24,
            matrix.M31, matrix.M32, matrix.M33, matrix.M34,
            matrix.M41, matrix.M42, matrix.M43, matrix.M44,
        };
    }

    static float[] floats(Vector3 value, String name) {
        Objects.requireNonNull(value, name);
        return new float[] {value.X, value.Y, value.Z};
    }

    static float[] floats(BoundingBox box, String name) {
        Objects.requireNonNull(box, name);
        return new float[] {
            box.Min.X, box.Min.Y, box.Min.Z,
            box.Max.X, box.Max.Y, box.Max.Z,
        };
    }

    static float[] floats(BoundingSphere sphere, String name) {
        Objects.requireNonNull(sphere, name);
        return new float[] {
            sphere.Center.X, sphere.Center.Y, sphere.Center.Z, sphere.Radius,
        };
    }

    static long[] channels(Color color, String name) {
        Objects.requireNonNull(color, name);
        return new long[] {color.getR(), color.getG(), color.getB(), color.getA()};
    }

    /** Packs a list of matrices end to end, rejecting a null element rather than sending zeros. */
    static float[] matrices(List<Matrix> values, String name) {
        Objects.requireNonNull(values, name);
        float[] packed = new float[Math.multiplyExact(values.size(), MATRIX_LEAVES)];
        for (int index = 0; index < values.size(); index++) {
            Matrix value = Objects.requireNonNull(values.get(index), name + "[" + index + "]");
            System.arraycopy(floats(value, name), 0, packed, index * MATRIX_LEAVES,
                    MATRIX_LEAVES);
        }
        return packed;
    }

    /** Packs a list of boxes end to end. */
    static float[] boxes(List<BoundingBox> values, String name) {
        Objects.requireNonNull(values, name);
        float[] packed = new float[Math.multiplyExact(values.size(), BOX_LEAVES)];
        for (int index = 0; index < values.size(); index++) {
            BoundingBox value = Objects.requireNonNull(values.get(index),
                    name + "[" + index + "]");
            System.arraycopy(floats(value, name), 0, packed, index * BOX_LEAVES, BOX_LEAVES);
        }
        return packed;
    }

    /** Packs a list of spheres end to end. */
    static float[] spheres(List<BoundingSphere> values, String name) {
        Objects.requireNonNull(values, name);
        float[] packed = new float[Math.multiplyExact(values.size(), SPHERE_LEAVES)];
        for (int index = 0; index < values.size(); index++) {
            BoundingSphere value = Objects.requireNonNull(values.get(index),
                    name + "[" + index + "]");
            System.arraycopy(floats(value, name), 0, packed, index * SPHERE_LEAVES,
                    SPHERE_LEAVES);
        }
        return packed;
    }

    /** Reads one matrix back out of a packed array. */
    static Matrix matrix(float[] packed, int index) {
        return matrixAt(packed, Math.multiplyExact(index, MATRIX_LEAVES));
    }

    /**
     * Reads one matrix out of flat leaves at a given offset.
     *
     * <p>The sibling of {@link #matrix(float[], int)} for a structure whose matrices are not the
     * only thing in the array: a cascade state's four transforms start one leaf in, after the
     * blend band, so counting in matrices would read the wrong sixteen floats.
     *
     * @param packed the leaves
     * @param base the offset of the first element
     * @return the matrix
     */
    static Matrix matrixAt(float[] packed, int base) {
        return new Matrix(
                packed[base], packed[base + 1], packed[base + 2], packed[base + 3],
                packed[base + 4], packed[base + 5], packed[base + 6], packed[base + 7],
                packed[base + 8], packed[base + 9], packed[base + 10], packed[base + 11],
                packed[base + 12], packed[base + 13], packed[base + 14], packed[base + 15]);
    }
}
