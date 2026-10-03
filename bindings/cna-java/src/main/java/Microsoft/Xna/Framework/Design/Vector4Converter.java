package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Vector4;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link Vector4}. */
public class Vector4Converter extends MathTypeConverter {

    public Vector4Converter() {
        propertyDescriptions = properties(
                "X", float.class, "Y", float.class, "Z", float.class, "W", float.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        if (value instanceof String) {
            String[] parts = components(culture, value, 4);
            return new Vector4(
                    single(parts[0], culture),
                    single(parts[1], culture),
                    single(parts[2], culture),
                    single(parts[3], culture));
        }
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Vector4 vector) {
            if (destinationType == String.class) {
                return format(culture, vector.X, vector.Y, vector.Z, vector.W);
            }
            if (destinationType == Expression.class) {
                return construction(Vector4.class, vector.X, vector.Y, vector.Z, vector.W);
            }
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Vector4(
                requiredSingle(values, "X"),
                requiredSingle(values, "Y"),
                requiredSingle(values, "Z"),
                requiredSingle(values, "W"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Vector4 vector)) {
            throw new IllegalArgumentException("value must be a Vector4.");
        }
        return values("X", vector.X, "Y", vector.Y, "Z", vector.Z, "W", vector.W);
    }
}
