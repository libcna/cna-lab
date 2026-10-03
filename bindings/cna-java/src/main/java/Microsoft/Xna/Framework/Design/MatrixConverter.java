package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for the 16 stored components of {@link Matrix}. */
public class MatrixConverter extends MathTypeConverter {

    public MatrixConverter() {
        supportStringConvert = false;
        propertyDescriptions = properties(
                "Translation", Vector3.class,
                "M11", float.class, "M12", float.class, "M13", float.class, "M14", float.class,
                "M21", float.class, "M22", float.class, "M23", float.class, "M24", float.class,
                "M31", float.class, "M32", float.class, "M33", float.class, "M34", float.class,
                "M41", float.class, "M42", float.class, "M43", float.class, "M44", float.class);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Matrix matrix && destinationType == Expression.class) {
            return construction(
                    Matrix.class,
                    matrix.M11, matrix.M12, matrix.M13, matrix.M14,
                    matrix.M21, matrix.M22, matrix.M23, matrix.M24,
                    matrix.M31, matrix.M32, matrix.M33, matrix.M34,
                    matrix.M41, matrix.M42, matrix.M43, matrix.M44);
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Matrix(
                requiredSingle(values, "M11"), requiredSingle(values, "M12"),
                requiredSingle(values, "M13"), requiredSingle(values, "M14"),
                requiredSingle(values, "M21"), requiredSingle(values, "M22"),
                requiredSingle(values, "M23"), requiredSingle(values, "M24"),
                requiredSingle(values, "M31"), requiredSingle(values, "M32"),
                requiredSingle(values, "M33"), requiredSingle(values, "M34"),
                requiredSingle(values, "M41"), requiredSingle(values, "M42"),
                requiredSingle(values, "M43"), requiredSingle(values, "M44"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Matrix matrix)) {
            throw new IllegalArgumentException("value must be a Matrix.");
        }
        return values(
                "Translation", matrix.getTranslation(),
                "M11", matrix.M11, "M12", matrix.M12, "M13", matrix.M13, "M14", matrix.M14,
                "M21", matrix.M21, "M22", matrix.M22, "M23", matrix.M23, "M24", matrix.M24,
                "M31", matrix.M31, "M32", matrix.M32, "M33", matrix.M33, "M34", matrix.M34,
                "M41", matrix.M41, "M42", matrix.M42, "M43", matrix.M43, "M44", matrix.M44);
    }
}
