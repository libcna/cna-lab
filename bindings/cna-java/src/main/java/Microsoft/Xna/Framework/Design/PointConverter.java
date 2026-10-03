package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Point;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link Point}. */
public class PointConverter extends MathTypeConverter {

    public PointConverter() {
        propertyDescriptions = properties("X", int.class, "Y", int.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        if (value instanceof String) {
            String[] parts = components(culture, value, 2);
            return new Point(integer(parts[0], culture), integer(parts[1], culture));
        }
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Point point) {
            if (destinationType == String.class) {
                return format(culture, point.X, point.Y);
            }
            if (destinationType == Expression.class) {
                return construction(Point.class, point.X, point.Y);
            }
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Point(requiredInteger(values, "X"), requiredInteger(values, "Y"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Point point)) {
            throw new IllegalArgumentException("value must be a Point.");
        }
        return values("X", point.X, "Y", point.Y);
    }
}
