package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Rectangle;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link Rectangle}. */
public class RectangleConverter extends MathTypeConverter {

    public RectangleConverter() {
        supportStringConvert = false;
        propertyDescriptions = properties(
                "X", int.class, "Y", int.class, "Width", int.class, "Height", int.class);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Rectangle rectangle && destinationType == Expression.class) {
            return construction(
                    Rectangle.class, rectangle.X, rectangle.Y, rectangle.Width, rectangle.Height);
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Rectangle(
                requiredInteger(values, "X"),
                requiredInteger(values, "Y"),
                requiredInteger(values, "Width"),
                requiredInteger(values, "Height"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Rectangle rectangle)) {
            throw new IllegalArgumentException("value must be a Rectangle.");
        }
        return values(
                "X", rectangle.X,
                "Y", rectangle.Y,
                "Width", rectangle.Width,
                "Height", rectangle.Height);
    }
}
