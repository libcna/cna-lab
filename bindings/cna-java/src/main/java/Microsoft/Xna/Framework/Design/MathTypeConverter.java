package Microsoft.Xna.Framework.Design;

import java.beans.Expression;
import java.math.BigDecimal;
import java.math.MathContext;
import java.math.RoundingMode;
import java.text.DecimalFormatSymbols;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.regex.Pattern;

/** Shared Java projection of XNA's mathematical design-time converter. */
public class MathTypeConverter {

    protected LinkedHashMap<String, Class<?>> propertyDescriptions;
    protected boolean supportStringConvert;

    public MathTypeConverter() {
        supportStringConvert = true;
    }

    public boolean CanConvertFrom(Class<?> sourceType) {
        return supportStringConvert && sourceType == String.class;
    }

    public boolean CanConvertTo(Class<?> destinationType) {
        return destinationType == String.class || destinationType == Expression.class;
    }

    public boolean GetCreateInstanceSupported() {
        return true;
    }

    public LinkedHashMap<String, Object> GetProperties(Object value) {
        return propertyDescriptions == null ? null : decomposeValue(value);
    }

    public boolean GetPropertiesSupported() {
        return true;
    }

    LinkedHashMap<String, Object> decomposeValue(Object value) {
        throw new IllegalArgumentException("This converter does not describe a concrete value type.");
    }

    static LinkedHashMap<String, Class<?>> properties(Object... definitions) {
        if ((definitions.length & 1) != 0) {
            throw new IllegalArgumentException("Property definitions must contain name/type pairs.");
        }
        LinkedHashMap<String, Class<?>> result = new LinkedHashMap<>();
        for (int index = 0; index < definitions.length; index += 2) {
            result.put((String)definitions[index], (Class<?>)definitions[index + 1]);
        }
        return result;
    }

    static LinkedHashMap<String, Object> values(Object... definitions) {
        if ((definitions.length & 1) != 0) {
            throw new IllegalArgumentException("Property values must contain name/value pairs.");
        }
        LinkedHashMap<String, Object> result = new LinkedHashMap<>();
        for (int index = 0; index < definitions.length; index += 2) {
            result.put((String)definitions[index], definitions[index + 1]);
        }
        return result;
    }

    static String[] components(Locale culture, Object value, int expectedCount) {
        if (!(value instanceof String text)) {
            throw unsupportedSource(value);
        }
        String separator = listSeparator(culture);
        String[] result = text.trim().split(Pattern.quote(separator), -1);
        if (result.length != expectedCount) {
            throw new IllegalArgumentException(
                    "Expected " + expectedCount + " components separated by '" + separator + "'.");
        }
        return result;
    }

    static int integer(String value, Locale culture) {
        String text = value.trim();
        if (!text.matches("[+-]?[0-9]+")) {
            throw new IllegalArgumentException("Invalid Int32 component: " + value);
        }
        try {
            return Integer.parseInt(text);
        } catch (NumberFormatException error) {
            throw new IllegalArgumentException("Int32 component is outside its supported range: " + value, error);
        }
    }

    static int unsignedByte(String value, Locale culture) {
        int result = integer(value, culture);
        if (result < 0 || result > 255) {
            throw new IllegalArgumentException("Byte component is outside 0..255: " + value);
        }
        return result;
    }

    static float single(String value, Locale culture) {
        Locale actualCulture = actualCulture(culture);
        String text = value.trim();
        DecimalFormatSymbols symbols = DecimalFormatSymbols.getInstance(actualCulture);
        String nan = symbols.getNaN();
        String infinity = infinity(actualCulture);
        if (text.equals(nan) || text.equals("NaN")) {
            return Float.NaN;
        }
        if (text.equals(infinity) || text.equals("Infinity") || text.equals("+Infinity")) {
            return Float.POSITIVE_INFINITY;
        }
        if (text.equals(negativeInfinity(actualCulture)) || text.equals("-Infinity")) {
            return Float.NEGATIVE_INFINITY;
        }

        char decimal = symbols.getDecimalSeparator();
        String decimalPattern = Pattern.quote(Character.toString(decimal));
        String numberPattern = "[+-]?(?:[0-9]+(?:" + decimalPattern + "[0-9]*)?"
                + "|" + decimalPattern + "[0-9]+)(?:[eE][+-]?[0-9]+)?";
        if (!text.matches(numberPattern)) {
            throw new IllegalArgumentException("Invalid Single component: " + value);
        }
        String invariant = decimal == '.' ? text : text.replace(decimal, '.');
        try {
            float result = Float.parseFloat(invariant);
            if (Float.isInfinite(result)) {
                throw new IllegalArgumentException("Single component is outside its supported range: " + value);
            }
            return result;
        } catch (NumberFormatException error) {
            throw new IllegalArgumentException("Invalid Single component: " + value, error);
        }
    }

    static String format(Locale culture, Object... components) {
        StringBuilder result = new StringBuilder();
        String separator = listSeparator(culture) + ' ';
        for (int index = 0; index < components.length; index++) {
            if (index != 0) {
                result.append(separator);
            }
            Object component = components[index];
            if (component instanceof Float value) {
                result.append(formatSingle(value, culture));
            } else {
                result.append(component);
            }
        }
        return result.toString();
    }

    static String formatSingle(float value, Locale culture) {
        Locale actualCulture = actualCulture(culture);
        if (Float.isNaN(value)) {
            return DecimalFormatSymbols.getInstance(actualCulture).getNaN();
        }
        if (value == Float.POSITIVE_INFINITY) {
            return infinity(actualCulture);
        }
        if (value == Float.NEGATIVE_INFINITY) {
            return negativeInfinity(actualCulture);
        }
        if (value == 0.0f) {
            return "0";
        }

        BigDecimal rounded = new BigDecimal((double)value, new MathContext(7, RoundingMode.HALF_UP));
        int exponent = rounded.precision() - rounded.scale() - 1;
        String invariant;
        if (exponent < -4 || exponent >= 7) {
            BigDecimal mantissa = rounded.movePointLeft(exponent).stripTrailingZeros();
            String exponentText = Integer.toString(Math.abs(exponent));
            if (exponentText.length() < 2) {
                exponentText = '0' + exponentText;
            }
            invariant = mantissa.toPlainString() + 'E' + (exponent >= 0 ? "+" : "-") + exponentText;
        } else {
            invariant = rounded.stripTrailingZeros().toPlainString();
        }
        char decimal = DecimalFormatSymbols.getInstance(actualCulture).getDecimalSeparator();
        return decimal == '.' ? invariant : invariant.replace('.', decimal);
    }

    static Expression construction(Class<?> type, Object... arguments) {
        return new Expression(type, "new", arguments.clone());
    }

    static Object baseConvertFrom(Object value) {
        throw unsupportedSource(value);
    }

    static Object baseConvertTo(Object value, Class<?> destinationType) {
        Objects.requireNonNull(destinationType, "destinationType");
        if (destinationType == String.class && value != null) {
            return value.toString();
        }
        throw new UnsupportedOperationException(
                "Conversion from " + sourceName(value) + " to " + destinationType.getTypeName() + " is not supported.");
    }

    static Map<String, Object> requireValues(Map<String, Object> values) {
        return Objects.requireNonNull(values, "propertyValues");
    }

    static int requiredInteger(Map<String, Object> values, String name) {
        Object value = require(values, name);
        if (!(value instanceof Integer result)) {
            throw wrongPropertyType(name, Integer.class, value);
        }
        return result;
    }

    static int requiredByte(Map<String, Object> values, String name) {
        int result = requiredInteger(values, name);
        if (result < 0 || result > 255) {
            throw new IllegalArgumentException("Property '" + name + "' must be in the range 0..255.");
        }
        return result;
    }

    static float requiredSingle(Map<String, Object> values, String name) {
        Object value = require(values, name);
        if (!(value instanceof Float result)) {
            throw wrongPropertyType(name, Float.class, value);
        }
        return result;
    }

    static <T> T requiredObject(Map<String, Object> values, String name, Class<T> type) {
        Object value = require(values, name);
        if (!type.isInstance(value)) {
            throw wrongPropertyType(name, type, value);
        }
        return type.cast(value);
    }

    private static Object require(Map<String, Object> values, String name) {
        requireValues(values);
        if (!values.containsKey(name) || values.get(name) == null) {
            throw new IllegalArgumentException("Required property '" + name + "' is missing or null.");
        }
        return values.get(name);
    }

    private static IllegalArgumentException wrongPropertyType(String name, Class<?> expected, Object value) {
        return new IllegalArgumentException("Property '" + name + "' must be " + expected.getTypeName()
                + ", not " + value.getClass().getTypeName() + '.');
    }

    private static UnsupportedOperationException unsupportedSource(Object value) {
        return new UnsupportedOperationException("Conversion from " + sourceName(value) + " is not supported.");
    }

    private static String sourceName(Object value) {
        return value == null ? "null" : value.getClass().getTypeName();
    }

    private static Locale actualCulture(Locale culture) {
        return culture == null ? Locale.getDefault() : culture;
    }

    private static String listSeparator(Locale culture) {
        return DecimalFormatSymbols.getInstance(actualCulture(culture)).getDecimalSeparator() == ',' ? ";" : ",";
    }

    private static String infinity(Locale culture) {
        return culture.getLanguage().equals(Locale.GERMAN.getLanguage()) ? "+unendlich" : "Infinity";
    }

    private static String negativeInfinity(Locale culture) {
        return culture.getLanguage().equals(Locale.GERMAN.getLanguage()) ? "-unendlich" : "-Infinity";
    }
}
