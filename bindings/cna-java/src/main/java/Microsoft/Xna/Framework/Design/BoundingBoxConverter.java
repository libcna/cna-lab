package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.BoundingBox;
import Microsoft.Xna.Framework.Vector3;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link BoundingBox}. */
public class BoundingBoxConverter extends MathTypeConverter {

    public BoundingBoxConverter() {
        supportStringConvert = false;
        propertyDescriptions = properties("Min", Vector3.class, "Max", Vector3.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof BoundingBox box && destinationType == Expression.class) {
            return construction(BoundingBox.class, new Vector3(box.Min), new Vector3(box.Max));
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new BoundingBox(
                requiredObject(values, "Min", Vector3.class),
                requiredObject(values, "Max", Vector3.class));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof BoundingBox box)) {
            throw new IllegalArgumentException("value must be a BoundingBox.");
        }
        return values("Min", new Vector3(box.Min), "Max", new Vector3(box.Max));
    }
}
