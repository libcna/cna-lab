package Microsoft.Xna.Framework;

/** XNA's internal Johnson-distance GJK simplex solver. */
final class Gjk {

    private static final int[] BITS_TO_INDICES = {
        0, 1, 2, 17, 3, 25, 26, 209, 4, 33, 34, 273, 35, 281, 282, 2257
    };

    private Vector3 closestPoint = new Vector3();
    private final Vector3[] y = new Vector3[4];
    private final float[] yLengthSquared = new float[4];
    private final Vector3[][] edges = new Vector3[4][4];
    private final float[][] edgeLengthSquared = new float[4][4];
    private final float[][] determinant = new float[16][4];
    private int simplexBits;
    private float maxLengthSquared;

    Gjk() {
        for (int i = 0; i < 4; i++) {
            y[i] = new Vector3();
            for (int j = 0; j < 4; j++) {
                edges[i][j] = new Vector3();
            }
        }
    }

    boolean isFullSimplex() { return simplexBits == 15; }
    float getMaxLengthSquared() { return maxLengthSquared; }
    Vector3 getClosestPoint() { return new Vector3(closestPoint); }

    void reset() {
        simplexBits = 0;
        maxLengthSquared = 0.0f;
    }

    boolean addSupportPoint(Vector3 newPoint) {
        int index = (BITS_TO_INDICES[simplexBits ^ 0xF] & 7) - 1;
        y[index] = new Vector3(newPoint);
        yLengthSquared[index] = newPoint.LengthSquared();
        for (int bits = BITS_TO_INDICES[simplexBits]; bits != 0; bits >>= 3) {
            int oldIndex = (bits & 7) - 1;
            Vector3 edge = Vector3.Subtract(y[oldIndex], newPoint);
            edges[oldIndex][index] = edge;
            edges[index][oldIndex] = new Vector3(-edge.X, -edge.Y, -edge.Z);
            float lengthSquared = edge.LengthSquared();
            edgeLengthSquared[index][oldIndex] = lengthSquared;
            edgeLengthSquared[oldIndex][index] = lengthSquared;
        }
        updateDeterminant(index);
        return updateSimplex(index);
    }

    private static float dot(Vector3 a, Vector3 b) {
        return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z);
    }

    private void updateDeterminant(int newIndex) {
        int newBit = 1 << newIndex;
        determinant[newBit][newIndex] = 1.0f;
        int packedBits = BITS_TO_INDICES[simplexBits];
        int remaining = packedBits;
        int priorCount = 0;
        while (remaining != 0) {
            int oldIndex = (remaining & 7) - 1;
            int oldBit = 1 << oldIndex;
            int pairBits = oldBit | newBit;
            determinant[pairBits][oldIndex] = dot(edges[newIndex][oldIndex], y[newIndex]);
            determinant[pairBits][newIndex] = dot(edges[oldIndex][newIndex], y[oldIndex]);
            int prior = packedBits;
            for (int i = 0; i < priorCount; i++) {
                int thirdIndex = (prior & 7) - 1;
                int thirdBit = 1 << thirdIndex;
                int tripleBits = pairBits | thirdBit;
                int selected = edgeLengthSquared[oldIndex][thirdIndex]
                        < edgeLengthSquared[newIndex][thirdIndex] ? oldIndex : newIndex;
                determinant[tripleBits][thirdIndex] =
                        (determinant[pairBits][oldIndex] * dot(edges[selected][thirdIndex], y[oldIndex]))
                        + (determinant[pairBits][newIndex] * dot(edges[selected][thirdIndex], y[newIndex]));
                selected = edgeLengthSquared[thirdIndex][oldIndex]
                        < edgeLengthSquared[newIndex][oldIndex] ? thirdIndex : newIndex;
                determinant[tripleBits][oldIndex] =
                        (determinant[thirdBit | newBit][thirdIndex]
                                * dot(edges[selected][oldIndex], y[thirdIndex]))
                        + (determinant[thirdBit | newBit][newIndex]
                                * dot(edges[selected][oldIndex], y[newIndex]));
                selected = edgeLengthSquared[oldIndex][newIndex]
                        < edgeLengthSquared[thirdIndex][newIndex] ? oldIndex : thirdIndex;
                determinant[tripleBits][newIndex] =
                        (determinant[oldBit | thirdBit][thirdIndex]
                                * dot(edges[selected][newIndex], y[thirdIndex]))
                        + (determinant[oldBit | thirdBit][oldIndex]
                                * dot(edges[selected][newIndex], y[oldIndex]));
                prior >>= 3;
            }
            remaining >>= 3;
            priorCount++;
        }
        if ((simplexBits | newBit) == 15) {
            int selected = !(edgeLengthSquared[1][0] < edgeLengthSquared[2][0])
                    ? (edgeLengthSquared[2][0] < edgeLengthSquared[3][0] ? 2 : 3)
                    : (edgeLengthSquared[1][0] < edgeLengthSquared[3][0] ? 1 : 3);
            determinant[15][0] =
                    (determinant[14][1] * dot(edges[selected][0], y[1]))
                    + (determinant[14][2] * dot(edges[selected][0], y[2]))
                    + (determinant[14][3] * dot(edges[selected][0], y[3]));
            selected = !(edgeLengthSquared[0][1] < edgeLengthSquared[2][1])
                    ? (edgeLengthSquared[2][1] < edgeLengthSquared[3][1] ? 2 : 3)
                    : (!(edgeLengthSquared[0][1] < edgeLengthSquared[3][1]) ? 3 : 0);
            determinant[15][1] =
                    (determinant[13][0] * dot(edges[selected][1], y[0]))
                    + (determinant[13][2] * dot(edges[selected][1], y[2]))
                    + (determinant[13][3] * dot(edges[selected][1], y[3]));
            selected = !(edgeLengthSquared[0][2] < edgeLengthSquared[1][2])
                    ? (edgeLengthSquared[1][2] < edgeLengthSquared[3][2] ? 1 : 3)
                    : (!(edgeLengthSquared[0][2] < edgeLengthSquared[3][2]) ? 3 : 0);
            determinant[15][2] =
                    (determinant[11][0] * dot(edges[selected][2], y[0]))
                    + (determinant[11][1] * dot(edges[selected][2], y[1]))
                    + (determinant[11][3] * dot(edges[selected][2], y[3]));
            selected = !(edgeLengthSquared[0][3] < edgeLengthSquared[1][3])
                    ? (edgeLengthSquared[1][3] < edgeLengthSquared[2][3] ? 1 : 2)
                    : (!(edgeLengthSquared[0][3] < edgeLengthSquared[2][3]) ? 2 : 0);
            determinant[15][3] =
                    (determinant[7][0] * dot(edges[selected][3], y[0]))
                    + (determinant[7][1] * dot(edges[selected][3], y[1]))
                    + (determinant[7][2] * dot(edges[selected][3], y[2]));
        }
    }

    private boolean updateSimplex(int newIndex) {
        int candidateBits = simplexBits | (1 << newIndex);
        int newBit = 1 << newIndex;
        for (int subset = simplexBits; subset != 0; subset--) {
            if ((subset & candidateBits) == subset && satisfiesRule(subset | newBit, candidateBits)) {
                simplexBits = subset | newBit;
                closestPoint = computeClosestPoint();
                return true;
            }
        }
        if (satisfiesRule(newBit, candidateBits)) {
            simplexBits = newBit;
            closestPoint = new Vector3(y[newIndex]);
            maxLengthSquared = yLengthSquared[newIndex];
            return true;
        }
        return false;
    }

    private Vector3 computeClosestPoint() {
        float sum = 0.0f;
        Vector3 value = new Vector3();
        maxLengthSquared = 0.0f;
        for (int bits = BITS_TO_INDICES[simplexBits]; bits != 0; bits >>= 3) {
            int index = (bits & 7) - 1;
            float weight = determinant[simplexBits][index];
            sum += weight;
            value = Vector3.Add(value, Vector3.Multiply(y[index], weight));
            maxLengthSquared = MathHelper.Max(maxLengthSquared, yLengthSquared[index]);
        }
        return Vector3.Divide(value, sum);
    }

    private boolean satisfiesRule(int xBits, int yBits) {
        for (int bits = BITS_TO_INDICES[yBits]; bits != 0; bits >>= 3) {
            int index = (bits & 7) - 1;
            int bit = 1 << index;
            if ((bit & xBits) != 0) {
                if (determinant[xBits][index] <= 0.0f) {
                    return false;
                }
            } else if (determinant[xBits | bit][index] > 0.0f) {
                return false;
            }
        }
        return true;
    }
}
