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
import org.junit.jupiter.api.Test;

import java.beans.Expression;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

final class DesignConverterTests {

    @Test
    void MathTypeConverterExposesTheMappedBaseContract() {
        MathTypeConverter converter = new MathTypeConverter();

        assertTrue(converter.CanConvertFrom(String.class));
        assertFalse(converter.CanConvertFrom(Integer.class));
        assertTrue(converter.CanConvertTo(String.class));
        assertTrue(converter.CanConvertTo(Expression.class));
        assertFalse(converter.CanConvertTo(Integer.class));
        assertTrue(converter.GetCreateInstanceSupported());
        assertTrue(converter.GetPropertiesSupported());
        assertNull(converter.GetProperties(new Object()));
    }

    @Test
    void EveryConverterPublishesTheReferencePropertyOrderAndMappedTypes() {
        assertProperties(new PointConverter(), "X", "int", "Y", "int");
        assertProperties(new RectangleConverter(),
                "X", "int", "Y", "int", "Width", "int", "Height", "int");
        assertProperties(new Vector2Converter(), "X", "float", "Y", "float");
        assertProperties(new Vector3Converter(), "X", "float", "Y", "float", "Z", "float");
        assertProperties(new Vector4Converter(),
                "X", "float", "Y", "float", "Z", "float", "W", "float");
        assertProperties(new QuaternionConverter(),
                "X", "float", "Y", "float", "Z", "float", "W", "float");
        assertProperties(new ColorConverter(),
                "R", "int", "G", "int", "B", "int", "A", "int");
        assertProperties(new BoundingBoxConverter(),
                "Min", Vector3.class.getTypeName(), "Max", Vector3.class.getTypeName());
        assertProperties(new BoundingSphereConverter(),
                "Center", Vector3.class.getTypeName(), "Radius", "float");
        assertProperties(new PlaneConverter(),
                "Normal", Vector3.class.getTypeName(), "D", "float");
        assertProperties(new RayConverter(),
                "Position", Vector3.class.getTypeName(), "Direction", Vector3.class.getTypeName());

        MatrixConverter matrix = new MatrixConverter();
        assertEquals(List.of(
                "Translation", "M11", "M12", "M13", "M14", "M21", "M22", "M23", "M24",
                "M31", "M32", "M33", "M34", "M41", "M42", "M43", "M44"),
                List.copyOf(matrix.propertyDescriptions.keySet()));
        assertEquals(Vector3.class, matrix.propertyDescriptions.get("Translation"));
        for (String name : matrix.propertyDescriptions.keySet()) {
            if (!name.equals("Translation")) {
                assertEquals(float.class, matrix.propertyDescriptions.get(name));
            }
        }
    }

    @Test
    void StringCapableConvertersUseCultureListAndDecimalSeparators() {
        assertEquals("1, -2", new PointConverter().ConvertTo(
                Locale.ROOT, new Point(1, -2), String.class));
        assertEquals("1; -2", new PointConverter().ConvertTo(
                Locale.GERMANY, new Point(1, -2), String.class));
        assertEquals("1.25, -2.5, 3.75", new Vector3Converter().ConvertTo(
                Locale.ROOT, new Vector3(1.25f, -2.5f, 3.75f), String.class));
        assertEquals("1,25; -2,5; 3,75", new Vector3Converter().ConvertTo(
                Locale.GERMANY, new Vector3(1.25f, -2.5f, 3.75f), String.class));
        assertEquals("NaN, Infinity, -Infinity, 0", new Vector4Converter().ConvertTo(
                Locale.ROOT,
                new Vector4(Float.NaN, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY, -0.0f),
                String.class));
        assertEquals("NaN; +unendlich; -unendlich; 0", new Vector4Converter().ConvertTo(
                Locale.GERMANY,
                new Vector4(Float.NaN, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY, -0.0f),
                String.class));
        assertEquals("1E-30, 3.402823E+38", new Vector2Converter().ConvertTo(
                Locale.ROOT, new Vector2(1.0e-30f, Float.MAX_VALUE), String.class));
        assertEquals("0, 255, 10, 40", new ColorConverter().ConvertTo(
                Locale.ROOT, new Color(0, 255, 10, 40), String.class));
    }

    @Test
    void ParsingPreservesInt32AndBinary32Edges() {
        Point point = assertInstanceOf(Point.class,
                new PointConverter().ConvertFrom(Locale.ROOT, " 2147483647 , -2147483648 "));
        assertEquals(Integer.MAX_VALUE, point.X);
        assertEquals(Integer.MIN_VALUE, point.Y);

        Vector3 vector = assertInstanceOf(Vector3.class,
                new Vector3Converter().ConvertFrom(Locale.ROOT, "-0, 1e-30, 3.40282347E+38"));
        assertEquals(0x80000000, Float.floatToRawIntBits(vector.X));
        assertEquals(0x0DA24260, Float.floatToRawIntBits(vector.Y));
        assertEquals(0x7F7FFFFF, Float.floatToRawIntBits(vector.Z));

        Vector3 german = assertInstanceOf(Vector3.class,
                new Vector3Converter().ConvertFrom(Locale.GERMANY, "1,5; -2,25; 3,75"));
        assertEquals(new Vector3(1.5f, -2.25f, 3.75f), german);

        Vector3 nonFinite = assertInstanceOf(Vector3.class,
                new Vector3Converter().ConvertFrom(Locale.GERMANY, "NaN; +unendlich; -unendlich"));
        assertTrue(Float.isNaN(nonFinite.X));
        assertEquals(Float.POSITIVE_INFINITY, nonFinite.Y);
        assertEquals(Float.NEGATIVE_INFINITY, nonFinite.Z);
    }

    @Test
    void MalformedStringsAndOutOfRangeComponentsAreRejected() {
        Vector3Converter vector = new Vector3Converter();
        assertThrows(IllegalArgumentException.class, () -> vector.ConvertFrom(Locale.ROOT, ""));
        assertThrows(IllegalArgumentException.class, () -> vector.ConvertFrom(Locale.ROOT, "1,2"));
        assertThrows(IllegalArgumentException.class, () -> vector.ConvertFrom(Locale.ROOT, "1,2,3,4"));
        assertThrows(IllegalArgumentException.class, () -> vector.ConvertFrom(Locale.ROOT, "1,,3"));
        assertThrows(IllegalArgumentException.class, () -> vector.ConvertFrom(Locale.ROOT, "3.5e38,0,0"));
        assertThrows(IllegalArgumentException.class,
                () -> vector.ConvertFrom(Locale.GERMANY, "1.5; -2.25; 3.75"));
        assertThrows(IllegalArgumentException.class,
                () -> new PointConverter().ConvertFrom(Locale.ROOT, "2147483648,0"));
        assertThrows(IllegalArgumentException.class,
                () -> new PointConverter().ConvertFrom(Locale.ROOT, "1.0,2"));
        assertThrows(IllegalArgumentException.class,
                () -> new ColorConverter().ConvertFrom(Locale.ROOT, "-1,0,0,0"));
        assertThrows(IllegalArgumentException.class,
                () -> new ColorConverter().ConvertFrom(Locale.ROOT, "256,0,0,0"));
        assertThrows(IllegalArgumentException.class,
                () -> new ColorConverter().ConvertFrom(Locale.ROOT, "Red"));
    }

    @Test
    void NonStringConvertersRetainXnaBaseFormattingAndRejectParsing() {
        RectangleConverter rectangle = new RectangleConverter();
        assertFalse(rectangle.CanConvertFrom(String.class));
        assertEquals("{X:1 Y:2 Width:3 Height:4}", rectangle.ConvertTo(
                Locale.GERMANY, new Rectangle(1, 2, 3, 4), String.class));

        MatrixConverter matrix = new MatrixConverter();
        assertFalse(matrix.CanConvertFrom(String.class));
        assertEquals(Matrix.getIdentity().toString(), matrix.ConvertTo(
                Locale.GERMANY, Matrix.getIdentity(), String.class));

        assertThrows(UnsupportedOperationException.class,
                () -> new BoundingBoxConverter().ConvertFrom(Locale.ROOT, "1,2"));
        assertThrows(UnsupportedOperationException.class,
                () -> new BoundingSphereConverter().ConvertFrom(Locale.ROOT, "1,2"));
        assertThrows(UnsupportedOperationException.class,
                () -> new RayConverter().ConvertFrom(Locale.ROOT, "1,2"));
    }

    @Test
    void OrderedDecompositionSnapshotsNestedValues() {
        Vector3Converter vectorConverter = new Vector3Converter();
        assertEquals(Map.of("X", 1.0f, "Y", 2.0f, "Z", 3.0f),
                vectorConverter.GetProperties(new Vector3(1, 2, 3)));

        BoundingSphere sphere = new BoundingSphere(new Vector3(1, 2, 3), 4);
        LinkedHashMap<String, Object> sphereValues = new BoundingSphereConverter().GetProperties(sphere);
        assertEquals(List.of("Center", "Radius"), List.copyOf(sphereValues.keySet()));
        Vector3 center = assertInstanceOf(Vector3.class, sphereValues.get("Center"));
        assertNotSame(sphere.Center, center);
        center.X = 99;
        assertEquals(1.0f, sphere.Center.X);

        Matrix matrix = Matrix.getIdentity();
        matrix.setTranslation(new Vector3(7, 8, 9));
        LinkedHashMap<String, Object> matrixValues = new MatrixConverter().GetProperties(matrix);
        assertEquals(new Vector3(7, 8, 9), matrixValues.get("Translation"));
        assertEquals(1.0f, matrixValues.get("M44"));
    }

    @Test
    void CreateInstanceReconstructsAllShapesAndIgnoresExtras() {
        assertEquals(new Point(1, 2), new PointConverter().CreateInstance(
                linked("X", 1, "Y", 2, "Extra", 3)));
        assertEquals(new Rectangle(1, 2, 3, 4), new RectangleConverter().CreateInstance(
                linked("X", 1, "Y", 2, "Width", 3, "Height", 4)));
        assertEquals(new Vector2(1, 2), new Vector2Converter().CreateInstance(
                linked("X", 1.0f, "Y", 2.0f)));
        assertEquals(new Vector3(1, 2, 3), new Vector3Converter().CreateInstance(
                linked("X", 1.0f, "Y", 2.0f, "Z", 3.0f, "Extra", 4.0f)));
        assertEquals(new Vector4(1, 2, 3, 4), new Vector4Converter().CreateInstance(
                linked("X", 1.0f, "Y", 2.0f, "Z", 3.0f, "W", 4.0f)));
        assertEquals(new Quaternion(1, 2, 3, 4), new QuaternionConverter().CreateInstance(
                linked("X", 1.0f, "Y", 2.0f, "Z", 3.0f, "W", 4.0f)));
        assertEquals(new Color(10, 20, 30, 40), new ColorConverter().CreateInstance(
                linked("R", 10, "G", 20, "B", 30, "A", 40)));
        assertEquals(new BoundingBox(new Vector3(1), new Vector3(2)),
                new BoundingBoxConverter().CreateInstance(
                        linked("Min", new Vector3(1), "Max", new Vector3(2))));
        assertEquals(new BoundingSphere(new Vector3(1, 2, 3), 4),
                new BoundingSphereConverter().CreateInstance(
                        linked("Center", new Vector3(1, 2, 3), "Radius", 4.0f)));
        assertEquals(new Plane(new Vector3(1, 2, 3), 4), new PlaneConverter().CreateInstance(
                linked("Normal", new Vector3(1, 2, 3), "D", 4.0f)));
        assertEquals(new Ray(new Vector3(1, 2, 3), new Vector3(4, 5, 6)),
                new RayConverter().CreateInstance(linked(
                        "Position", new Vector3(1, 2, 3),
                        "Direction", new Vector3(4, 5, 6))));
    }

    @Test
    void MatrixCreateInstanceUsesSixteenStoredComponentsAndIgnoresTranslationEntry() {
        LinkedHashMap<String, Object> values = matrixValues();
        values.put("Translation", new Vector3(100, 200, 300));
        Matrix matrix = assertInstanceOf(Matrix.class, new MatrixConverter().CreateInstance(values));

        assertEquals(1.0f, matrix.M11);
        assertEquals(8.0f, matrix.M24);
        assertEquals(13.0f, matrix.M41);
        assertEquals(16.0f, matrix.M44);
        assertEquals(new Vector3(13, 14, 15), matrix.getTranslation());
    }

    @Test
    void CreateInstanceRejectsNullMissingAndIncompatiblePropertyValues() {
        Vector3Converter converter = new Vector3Converter();
        assertThrows(NullPointerException.class, () -> converter.CreateInstance(null));
        assertThrows(IllegalArgumentException.class,
                () -> converter.CreateInstance(linked("X", 1.0f, "Y", 2.0f)));
        assertThrows(IllegalArgumentException.class,
                () -> converter.CreateInstance(linked("X", 1.0, "Y", 2.0f, "Z", 3.0f)));
        assertThrows(IllegalArgumentException.class,
                () -> converter.CreateInstance(linked("X", 1.0f, "Y", null, "Z", 3.0f)));
        assertThrows(IllegalArgumentException.class,
                () -> new ColorConverter().CreateInstance(
                        linked("R", 256, "G", 0, "B", 0, "A", 0)));
        assertThrows(IllegalArgumentException.class,
                () -> new BoundingBoxConverter().CreateInstance(
                        linked("Min", new Point(), "Max", new Vector3())));
    }

    @Test
    void ConstructionExpressionsRoundTripEveryConcreteConverter() throws Exception {
        assertEquals(new Point(1, 2), evaluate(new PointConverter(), new Point(1, 2)));
        assertEquals(new Rectangle(1, 2, 3, 4),
                evaluate(new RectangleConverter(), new Rectangle(1, 2, 3, 4)));
        assertEquals(new Vector2(1, 2), evaluate(new Vector2Converter(), new Vector2(1, 2)));
        assertEquals(new Vector3(1, 2, 3), evaluate(new Vector3Converter(), new Vector3(1, 2, 3)));
        assertEquals(new Vector4(1, 2, 3, 4),
                evaluate(new Vector4Converter(), new Vector4(1, 2, 3, 4)));
        assertEquals(new Quaternion(1, 2, 3, 4),
                evaluate(new QuaternionConverter(), new Quaternion(1, 2, 3, 4)));
        assertEquals(new Color(10, 20, 30, 40),
                evaluate(new ColorConverter(), new Color(10, 20, 30, 40)));
        assertEquals(Matrix.getIdentity(), evaluate(new MatrixConverter(), Matrix.getIdentity()));
        assertEquals(new BoundingBox(new Vector3(1), new Vector3(2)),
                evaluate(new BoundingBoxConverter(), new BoundingBox(new Vector3(1), new Vector3(2))));
        assertEquals(new BoundingSphere(new Vector3(1, 2, 3), 4),
                evaluate(new BoundingSphereConverter(), new BoundingSphere(new Vector3(1, 2, 3), 4)));
        assertEquals(new Plane(new Vector3(1, 2, 3), 4),
                evaluate(new PlaneConverter(), new Plane(new Vector3(1, 2, 3), 4)));
        assertEquals(new Ray(new Vector3(1), new Vector3(2)),
                evaluate(new RayConverter(), new Ray(new Vector3(1), new Vector3(2))));
    }

    @Test
    void WrongInputAndOutputTypesFollowTheMappedBaseBehavior() {
        Vector3Converter converter = new Vector3Converter();
        assertThrows(UnsupportedOperationException.class,
                () -> converter.ConvertFrom(Locale.ROOT, null));
        assertThrows(UnsupportedOperationException.class,
                () -> converter.ConvertFrom(Locale.ROOT, 1));
        assertThrows(NullPointerException.class,
                () -> converter.ConvertTo(Locale.ROOT, new Vector3(), null));
        assertThrows(UnsupportedOperationException.class,
                () -> converter.ConvertTo(Locale.ROOT, new Vector3(), Integer.class));
        assertEquals(new Point().toString(), converter.ConvertTo(
                Locale.ROOT, new Point(), String.class));
    }

    private static Object evaluate(MathTypeConverter converter, Object value) throws Exception {
        Object descriptor;
        if (converter instanceof PointConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof RectangleConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof Vector2Converter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof Vector3Converter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof Vector4Converter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof QuaternionConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof ColorConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof MatrixConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof BoundingBoxConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof BoundingSphereConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof PlaneConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else if (converter instanceof RayConverter concrete) {
            descriptor = concrete.ConvertTo(Locale.ROOT, value, Expression.class);
        } else {
            throw new AssertionError("Unexpected converter: " + converter.getClass());
        }
        return assertInstanceOf(Expression.class, descriptor).getValue();
    }

    private static void assertProperties(MathTypeConverter converter, String... expected) {
        LinkedHashMap<String, String> actual = new LinkedHashMap<>();
        converter.propertyDescriptions.forEach((name, type) -> actual.put(name, type.getTypeName()));
        LinkedHashMap<String, String> wanted = new LinkedHashMap<>();
        for (int index = 0; index < expected.length; index += 2) {
            wanted.put(expected[index], expected[index + 1]);
        }
        assertEquals(wanted, actual);
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
