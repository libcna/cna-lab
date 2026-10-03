package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Vector3;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link Vector3}. */
public class Vector3Converter extends MathTypeConverter {

    public Vector3Converter() {
        propertyDescriptions = properties("X", float.class, "Y", float.class, "Z", float.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        if (value instanceof String) {
            String[] parts = components(culture, value, 3);
            return new Vector3(
                    single(parts[0], culture), single(parts[1], culture), single(parts[2], culture));
        }
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Vector3 vector) {
            if (destinationType == String.class) {
                return format(culture, vector.X, vector.Y, vector.Z);
            }
            if (destinationType == Expression.class) {
                return construction(Vector3.class, vector.X, vector.Y, vector.Z);
            }
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Vector3(
                requiredSingle(values, "X"),
                requiredSingle(values, "Y"),
                requiredSingle(values, "Z"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Vector3 vector)) {
            throw new IllegalArgumentException("value must be a Vector3.");
        }
        return values("X", vector.X, "Y", vector.Y, "Z", vector.Z);
    }
}
