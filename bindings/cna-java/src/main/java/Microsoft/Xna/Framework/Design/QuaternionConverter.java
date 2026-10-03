package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Quaternion;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link Quaternion}. */
public class QuaternionConverter extends MathTypeConverter {

    public QuaternionConverter() {
        propertyDescriptions = properties(
                "X", float.class, "Y", float.class, "Z", float.class, "W", float.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        if (value instanceof String) {
            String[] parts = components(culture, value, 4);
            return new Quaternion(
                    single(parts[0], culture),
                    single(parts[1], culture),
                    single(parts[2], culture),
                    single(parts[3], culture));
        }
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Quaternion quaternion) {
            if (destinationType == String.class) {
                return format(culture, quaternion.X, quaternion.Y, quaternion.Z, quaternion.W);
            }
            if (destinationType == Expression.class) {
                return construction(
                        Quaternion.class, quaternion.X, quaternion.Y, quaternion.Z, quaternion.W);
            }
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Quaternion(
                requiredSingle(values, "X"),
                requiredSingle(values, "Y"),
                requiredSingle(values, "Z"),
                requiredSingle(values, "W"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Quaternion quaternion)) {
            throw new IllegalArgumentException("value must be a Quaternion.");
        }
        return values(
                "X", quaternion.X, "Y", quaternion.Y, "Z", quaternion.Z, "W", quaternion.W);
    }
}
