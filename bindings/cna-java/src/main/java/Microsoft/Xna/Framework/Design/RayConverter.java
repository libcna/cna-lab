package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.Ray;
import Microsoft.Xna.Framework.Vector3;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link Ray}. */
public class RayConverter extends MathTypeConverter {

    public RayConverter() {
        supportStringConvert = false;
        propertyDescriptions = properties("Position", Vector3.class, "Direction", Vector3.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof Ray ray && destinationType == Expression.class) {
            return construction(Ray.class, new Vector3(ray.Position), new Vector3(ray.Direction));
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new Ray(
                requiredObject(values, "Position", Vector3.class),
                requiredObject(values, "Direction", Vector3.class));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof Ray ray)) {
            throw new IllegalArgumentException("value must be a Ray.");
        }
        return values(
                "Position", new Vector3(ray.Position),
                "Direction", new Vector3(ray.Direction));
    }
}
