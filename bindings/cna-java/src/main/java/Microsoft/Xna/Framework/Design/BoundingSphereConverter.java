package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Vector3;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/** Design-time conversion for {@link BoundingSphere}. */
public class BoundingSphereConverter extends MathTypeConverter {

    public BoundingSphereConverter() {
        supportStringConvert = false;
        propertyDescriptions = properties("Center", Vector3.class, "Radius", float.class);
    }

    public Object ConvertFrom(Locale culture, Object value) {
        return baseConvertFrom(value);
    }

    public Object ConvertTo(Locale culture, Object value, Class<?> destinationType) {
        if (value instanceof BoundingSphere sphere && destinationType == Expression.class) {
            return construction(BoundingSphere.class, new Vector3(sphere.Center), sphere.Radius);
        }
        return baseConvertTo(value, destinationType);
    }

    public Object CreateInstance(Map<String, Object> propertyValues) {
        Map<String, Object> values = requireValues(propertyValues);
        return new BoundingSphere(
                requiredObject(values, "Center", Vector3.class),
                requiredSingle(values, "Radius"));
    }

    @Override
    LinkedHashMap<String, Object> decomposeValue(Object value) {
        if (!(value instanceof BoundingSphere sphere)) {
            throw new IllegalArgumentException("value must be a BoundingSphere.");
        }
        return values("Center", new Vector3(sphere.Center), "Radius", sphere.Radius);
    }
}
