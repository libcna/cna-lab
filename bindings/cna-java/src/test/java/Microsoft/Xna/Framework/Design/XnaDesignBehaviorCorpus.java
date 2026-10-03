package Microsoft.Xna.Framework.Design;

import Microsoft.Xna.Framework.BoundingBox;
import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Plane;
import Microsoft.Xna.Framework.Point;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Ray;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;

import java.beans.Expression;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/** Deterministic Design observations derived from XNA metadata, IL, and runtime probes. */
final class XnaDesignBehaviorCorpus {

    private XnaDesignBehaviorCorpus() {
    }

    static List<String> capture() {
        ArrayList<String> observations = new ArrayList<>();
        MathTypeConverter math = new MathTypeConverter();
        observations.add("design.math.support="
                + flags(math.CanConvertFrom(String.class), math.CanConvertTo(String.class),
                        math.CanConvertTo(Expression.class), math.GetCreateInstanceSupported(),
                        math.GetPropertiesSupported()));

        addProperties(observations, "point", new PointConverter());
        addProperties(observations, "rectangle", new RectangleConverter());
        addProperties(observations, "vector2", new Vector2Converter());
        addProperties(observations, "vector3", new Vector3Converter());
        addProperties(observations, "vector4", new Vector4Converter());
        addProperties(observations, "quaternion", new QuaternionConverter());
        addProperties(observations, "color", new ColorConverter());
        addProperties(observations, "matrix", new MatrixConverter());
        addProperties(observations, "box", new BoundingBoxConverter());
        addProperties(observations, "sphere", new BoundingSphereConverter());
        addProperties(observations, "plane", new PlaneConverter());
        addProperties(observations, "ray", new RayConverter());

        addSupport(observations, "point", new PointConverter());
        addSupport(observations, "rectangle", new RectangleConverter());
        addSupport(observations, "vector3", new Vector3Converter());
        addSupport(observations, "box", new BoundingBoxConverter());

        PointConverter pointConverter = new PointConverter();
        observations.add("design.point.format.root="
                + pointConverter.ConvertTo(Locale.ROOT, new Point(1, -2), String.class));
        observations.add("design.point.format.de="
                + pointConverter.ConvertTo(Locale.GERMANY, new Point(1, -2), String.class));
        Vector3Converter vectorConverter = new Vector3Converter();
        observations.add("design.vector3.format.root=" + vectorConverter.ConvertTo(
                Locale.ROOT, new Vector3(1.25f, -2.5f, 3.75f), String.class));
        observations.add("design.vector3.format.de=" + vectorConverter.ConvertTo(
                Locale.GERMANY, new Vector3(1.25f, -2.5f, 3.75f), String.class));
        Vector4 special = new Vector4(
                Float.NaN, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY, -0.0f);
        observations.add("design.vector4.format.root="
                + new Vector4Converter().ConvertTo(Locale.ROOT, special, String.class));
        observations.add("design.vector4.format.de="
                + new Vector4Converter().ConvertTo(Locale.GERMANY, special, String.class));
        observations.add("design.color.format=" + new ColorConverter().ConvertTo(
                Locale.ROOT, new Color(0, 255, 10, 40), String.class));

        Rectangle rectangle = new Rectangle(1, 2, 3, 4);
        Matrix identity = Matrix.getIdentity();
        BoundingBox box = new BoundingBox(new Vector3(1), new Vector3(2));
        observations.add("design.fallback.format=" + flags(
                rectangle.toString().equals(new RectangleConverter().ConvertTo(
                        Locale.GERMANY, rectangle, String.class)),
                identity.toString().equals(new MatrixConverter().ConvertTo(
                        Locale.GERMANY, identity, String.class)),
                box.toString().equals(new BoundingBoxConverter().ConvertTo(
                        Locale.GERMANY, box, String.class))));

        Point point = (Point)pointConverter.ConvertFrom(
                Locale.ROOT, "2147483647, -2147483648");
        observations.add("design.point.parse.bounds=" + point.X + ',' + point.Y);
        Vector3 parsed = (Vector3)vectorConverter.ConvertFrom(
                Locale.ROOT, "-0, 1e-30, 3.40282347E+38");
        observations.add("design.vector3.parse.bits="
                + hex(parsed.X) + ',' + hex(parsed.Y) + ',' + hex(parsed.Z));
        Vector3 german = (Vector3)vectorConverter.ConvertFrom(
                Locale.GERMANY, "1,5; -2,25; 3,75");
        observations.add("design.vector3.parse.de="
                + hex(german.X) + ',' + hex(german.Y) + ',' + hex(german.Z));
        Vector3 nonFinite = (Vector3)vectorConverter.ConvertFrom(
                Locale.GERMANY, "NaN; +unendlich; -unendlich");
        observations.add("design.vector3.parse.special="
                + hex(nonFinite.X) + ',' + hex(nonFinite.Y) + ',' + hex(nonFinite.Z));
        Color color = (Color)new ColorConverter().ConvertFrom(Locale.ROOT, "0,255,10,40");
        observations.add("design.color.parse="
                + color.getR() + ',' + color.getG() + ',' + color.getB() + ',' + color.getA());

        observations.add("design.invalid.count="
                + exceptionName(() -> vectorConverter.ConvertFrom(Locale.ROOT, "")) + ','
                + exceptionName(() -> vectorConverter.ConvertFrom(Locale.ROOT, "1,2")) + ','
                + exceptionName(() -> vectorConverter.ConvertFrom(Locale.ROOT, "1,2,3,4")) + ','
                + exceptionName(() -> vectorConverter.ConvertFrom(Locale.ROOT, "1,,3")));
        observations.add("design.invalid.culture="
                + exceptionName(() -> vectorConverter.ConvertFrom(
                        Locale.GERMANY, "1.5; -2.25; 3.75")));
        observations.add("design.invalid.range="
                + exceptionName(() -> vectorConverter.ConvertFrom(Locale.ROOT, "3.5e38,0,0")) + ','
                + exceptionName(() -> pointConverter.ConvertFrom(Locale.ROOT, "2147483648,0")) + ','
                + exceptionName(() -> new ColorConverter().ConvertFrom(Locale.ROOT, "256,0,0,0")));

        observations.add("design.point.create=" + new PointConverter().CreateInstance(
                linked("X", 1, "Y", 2)));
        observations.add("design.vector3.create.extra=" + vectorConverter.CreateInstance(
                linked("X", 1.0f, "Y", 2.0f, "Z", 3.0f, "Extra", 4.0f)));
        LinkedHashMap<String, Object> matrixValues = matrixValues();
        matrixValues.put("Translation", new Vector3(100, 200, 300));
        Matrix rebuiltMatrix = (Matrix)new MatrixConverter().CreateInstance(matrixValues);
        observations.add("design.matrix.create="
                + rebuiltMatrix.M11 + ',' + rebuiltMatrix.M24 + ','
                + rebuiltMatrix.M41 + ',' + rebuiltMatrix.M44);

        BoundingSphere sphere = new BoundingSphere(new Vector3(1, 2, 3), 4);
        Vector3 center = (Vector3)new BoundingSphereConverter().GetProperties(sphere).get("Center");
        center.X = 99;
        observations.add("design.sphere.snapshot=" + sphere.Center.X + ',' + center.X);

        observations.add("design.expressions=" + expressionTypes());
        observations.add("design.map.failures="
                + exceptionName(() -> vectorConverter.CreateInstance(null)) + ','
                + exceptionName(() -> vectorConverter.CreateInstance(linked("X", 1.0f, "Y", 2.0f))) + ','
                + exceptionName(() -> vectorConverter.CreateInstance(
                        linked("X", 1.0, "Y", 2.0f, "Z", 3.0f))) + ','
                + exceptionName(() -> vectorConverter.CreateInstance(
                        linked("X", 1.0f, "Y", null, "Z", 3.0f))));
        observations.add("design.base.failures="
                + vectorConverter.ConvertTo(Locale.ROOT, new Point(), String.class) + ','
                + exceptionName(() -> vectorConverter.ConvertTo(
                        Locale.ROOT, new Vector3(), Integer.class)) + ','
                + exceptionName(() -> new BoundingBoxConverter().ConvertFrom(Locale.ROOT, "1,2")));
        return List.copyOf(observations);
    }

    private static void addProperties(
            List<String> observations, String name, MathTypeConverter converter) {
        String properties = String.join(",", converter.propertyDescriptions.entrySet().stream()
                .map(entry -> entry.getKey() + ':' + entry.getValue().getSimpleName())
                .toList());
        observations.add("design." + name + ".properties=" + properties);
    }

    private static void addSupport(
            List<String> observations, String name, MathTypeConverter converter) {
        observations.add("design." + name + ".support=" + flags(
                converter.CanConvertFrom(String.class),
                converter.CanConvertFrom(Integer.class),
                converter.CanConvertTo(String.class),
                converter.CanConvertTo(Expression.class),
                converter.GetCreateInstanceSupported(),
                converter.GetPropertiesSupported()));
    }

    private static String expressionTypes() {
        try {
            Expression[] expressions = {
                (Expression)new PointConverter().ConvertTo(
                        Locale.ROOT, new Point(1, 2), Expression.class),
                (Expression)new RectangleConverter().ConvertTo(
                        Locale.ROOT, new Rectangle(1, 2, 3, 4), Expression.class),
                (Expression)new Vector2Converter().ConvertTo(
                        Locale.ROOT, new Vector2(1, 2), Expression.class),
                (Expression)new Vector3Converter().ConvertTo(
                        Locale.ROOT, new Vector3(1, 2, 3), Expression.class),
                (Expression)new Vector4Converter().ConvertTo(
                        Locale.ROOT, new Vector4(1, 2, 3, 4), Expression.class),
                (Expression)new QuaternionConverter().ConvertTo(
                        Locale.ROOT, new Quaternion(1, 2, 3, 4), Expression.class),
                (Expression)new ColorConverter().ConvertTo(
                        Locale.ROOT, new Color(10, 20, 30, 40), Expression.class),
                (Expression)new MatrixConverter().ConvertTo(
                        Locale.ROOT, Matrix.getIdentity(), Expression.class),
                (Expression)new BoundingBoxConverter().ConvertTo(
                        Locale.ROOT, new BoundingBox(new Vector3(1), new Vector3(2)), Expression.class),
                (Expression)new BoundingSphereConverter().ConvertTo(
                        Locale.ROOT, new BoundingSphere(new Vector3(1), 2), Expression.class),
                (Expression)new PlaneConverter().ConvertTo(
                        Locale.ROOT, new Plane(new Vector3(1), 2), Expression.class),
                (Expression)new RayConverter().ConvertTo(
                        Locale.ROOT, new Ray(new Vector3(1), new Vector3(2)), Expression.class)
            };
            ArrayList<String> names = new ArrayList<>();
            for (Expression expression : expressions) {
                names.add(expression.getValue().getClass().getSimpleName());
            }
            return String.join(",", names);
        } catch (Exception error) {
            throw new AssertionError("Construction expression failed.", error);
        }
    }

    private static String flags(boolean... values) {
        StringBuilder result = new StringBuilder();
        for (int index = 0; index < values.length; index++) {
            if (index != 0) {
                result.append(',');
            }
            result.append(values[index] ? '1' : '0');
        }
        return result.toString();
    }

    private static String hex(float value) {
        return String.format(Locale.ROOT, "%08X", Float.floatToRawIntBits(value));
    }

    private static String exceptionName(Runnable operation) {
        try {
            operation.run();
            return "none";
        } catch (RuntimeException exception) {
            return exception.getClass().getSimpleName();
        }
    }

    private static LinkedHashMap<String, Object> linked(Object... entries) {
        LinkedHashMap<String, Object> values = new LinkedHashMap<>();
        for (int index = 0; index < entries.length; index += 2) {
            values.put((String)entries[index], entries[index + 1]);
        }
        return values;
    }

    private static LinkedHashMap<String, Object> matrixValues() {
        LinkedHashMap<String, Object> values = new LinkedHashMap<>();
        int component = 1;
        for (int row = 1; row <= 4; row++) {
            for (int column = 1; column <= 4; column++) {
                values.put("M" + row + column, (float)component++);
            }
        }
        return values;
    }
}
