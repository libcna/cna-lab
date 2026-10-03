package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Color;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for byte-component {@link Color} values. */
public class ColorConverter extends MathTypeConverter {

    public ColorConverter() {
        propertyDescriptions = properties(
                "R", int.class, "G", int.class, "B", int.class, "A", int.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        if (value instanceof String) {
            String[] parts = components(culture, value, 4);
            return new Color(
                    unsignedByte(parts[0], culture),
                    unsignedByte(parts[1], culture),
                    unsignedByte(parts[2], culture),
                    unsignedByte(parts[3], culture));
        }
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Color color) {
            if (destinationType == String.class) {
                return format(culture, color.getR(), color.getG(), color.getB(), color.getA());
            }
            if (destinationType == Expression.class) {
                return construction(
                        Color.class, color.getR(), color.getG(), color.getB(), color.getA());
            }
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Color(
                requiredByte(values, "R"),
                requiredByte(values, "G"),
                requiredByte(values, "B"),
                requiredByte(values, "A"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Color color)) {
            throw new IllegalArgumentException("value must be a Color.");
        }
        return values(
                "R", color.getR(), "G", color.getG(), "B", color.getB(), "A", color.getA());
    }
}
