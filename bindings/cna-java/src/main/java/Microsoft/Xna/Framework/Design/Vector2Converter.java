package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Vector2;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link Vector2}. */
public class Vector2Converter extends MathTypeConverter {

    public Vector2Converter() {
        propertyDescriptions = properties("X", float.class, "Y", float.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        if (value instanceof String) {
            String[] parts = components(culture, value, 2);
            return new Vector2(single(parts[0], culture), single(parts[1], culture));
        }
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Vector2 vector) {
            if (destinationType == String.class) {
                return format(culture, vector.X, vector.Y);
            }
            if (destinationType == Expression.class) {
                return construction(Vector2.class, vector.X, vector.Y);
            }
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Vector2(requiredSingle(values, "X"), requiredSingle(values, "Y"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Vector2 vector)) {
            throw new IllegalArgumentException("value must be a Vector2.");
        }
        return values("X", vector.X, "Y", vector.Y);
    }
}
