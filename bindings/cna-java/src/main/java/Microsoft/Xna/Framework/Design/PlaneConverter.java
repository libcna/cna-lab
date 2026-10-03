package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Plane;
import Microsoft.Xna.Framework.Vector3;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link Plane}. */
public class PlaneConverter extends MathTypeConverter {

    public PlaneConverter() {
        supportStringConvert = false;
        propertyDescriptions = properties("Normal", Vector3.class, "D", float.class);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Plane plane && destinationType == Expression.class) {
            return construction(Plane.class, new Vector3(plane.Normal), plane.D);
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Plane(
                requiredObject(values, "Normal", Vector3.class),
                requiredSingle(values, "D"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Plane plane)) {
            throw new IllegalArgumentException("value must be a Plane.");
        }
        return values("Normal", new Vector3(plane.Normal), "D", plane.D);
    }
}
