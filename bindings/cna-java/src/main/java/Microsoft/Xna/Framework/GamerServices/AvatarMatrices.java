package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Matrix;

/**
 * Converts between XNA's {@code Matrix} and {@code AvatarExpression} values and the flat
 * arrays the generated CNA boundary uses.
 *
 * <p>The row-major order here is the order CNA's {@code CNA_Matrix} declares its fields in,
 * which is the same M11..M44 order XNA's struct uses.
 */
final class AvatarMatrices {

    private AvatarMatrices() {
    }

    static Matrix of(float[] values) {
        Matrix matrix = new Matrix();
        matrix.M11 = values[0];
        matrix.M12 = values[1];
        matrix.M13 = values[2];
        matrix.M14 = values[3];
        matrix.M21 = values[4];
        matrix.M22 = values[5];
        matrix.M23 = values[6];
        matrix.M24 = values[7];
        matrix.M31 = values[8];
        matrix.M32 = values[9];
        matrix.M33 = values[10];
        matrix.M34 = values[11];
        matrix.M41 = values[12];
        matrix.M42 = values[13];
        matrix.M43 = values[14];
        matrix.M44 = values[15];
        return matrix;
    }

    static float[] values(Matrix matrix) {
        return new float[] {
            matrix.M11, matrix.M12, matrix.M13, matrix.M14,
            matrix.M21, matrix.M22, matrix.M23, matrix.M24,
            matrix.M31, matrix.M32, matrix.M33, matrix.M34,
            matrix.M41, matrix.M42, matrix.M43, matrix.M44,
        };
    }

    static AvatarExpression expression(long[] values) {
        AvatarExpression expression = new AvatarExpression();
        expression.setMouth(AvatarMouth.values()[(int) values[0]]);
        expression.setLeftEye(AvatarEye.values()[(int) values[1]]);
        expression.setRightEye(AvatarEye.values()[(int) values[2]]);
        expression.setLeftEyebrow(AvatarEyebrow.values()[(int) values[3]]);
        expression.setRightEyebrow(AvatarEyebrow.values()[(int) values[4]]);
        return expression;
    }

    static long[] values(AvatarExpression expression) {
        return new long[] {
            expression.getMouth().ordinal(),
            expression.getLeftEye().ordinal(),
            expression.getRightEye().ordinal(),
            expression.getLeftEyebrow().ordinal(),
            expression.getRightEyebrow().ordinal(),
        };
    }
}
