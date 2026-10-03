package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;
import System.IO.Stream;

import java.io.ByteArrayInputStream;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

/**
 * The title-defined columns of a leaderboard entry, keyed by name.
 *
 * <p>XNA declares this as an {@code IDictionary<string, object>} with a typed getter and a
 * typed setter per supported value kind. Java's projection keeps both: the XNA-named
 * {@code GetValue*} and {@code SetValue} members are the ones a ported title calls, and the
 * lower-cased {@link Map} members are the Java bridge that makes the declared interface real.
 * Both reach the same native dictionary, so a value written through one is read by the other.
 */
public final class PropertyDictionary implements Map<String, Object> {

    private static final int KIND_UNKNOWN = 0;
    private static final int KIND_DATE_TIME = 1;
    private static final int KIND_DOUBLE = 2;
    private static final int KIND_INT32 = 3;
    private static final int KIND_INT64 = 4;
    private static final int KIND_OUTCOME = 5;
    private static final int KIND_SINGLE = 6;
    private static final int KIND_STREAM = 7;
    private static final int KIND_STRING = 8;
    private static final int KIND_TIME_SPAN = 9;

    private final long handle;

    PropertyDictionary(long handle) {
        this.handle = handle;
    }

    public boolean ContainsKey(String key) {
        boolean[] contains = new boolean[1];
        NativeGamerServices.check("PropertyDictionary.ContainsKey",
                NativeGamerServicesRoutes.propertyDictionaryContainsKey(
                        handle, utf8(key), contains));
        return contains[0];
    }

    /**
     * Returns XNA's enumerator over the dictionary's entries.
     *
     * <p>The order is the one CNA reports, which is by key. CLR's
     * {@code Dictionary<string, object>} does not specify an order either, so no order
     * is invented here to match one.
     */
    public Iterator<Map.Entry<String, Object>> GetEnumerator() {
        return snapshot().entrySet().iterator();
    }

    public Instant GetValueDateTime(String key) {
        long[] ticks = new long[1];
        NativeGamerServices.check("PropertyDictionary.GetValueDateTime",
                NativeGamerServicesRoutes.propertyDictionaryGetDateTimeTicks(
                        handle, utf8(key), ticks));
        return NativeGamerServices.instant(ticks[0]);
    }

    public double GetValueDouble(String key) {
        double[] value = new double[1];
        NativeGamerServices.check("PropertyDictionary.GetValueDouble",
                NativeGamerServicesRoutes.propertyDictionaryGetDouble(handle, utf8(key), value));
        return value[0];
    }

    public int GetValueInt32(String key) {
        int[] value = new int[1];
        NativeGamerServices.check("PropertyDictionary.GetValueInt32",
                NativeGamerServicesRoutes.propertyDictionaryGetInt32(handle, utf8(key), value));
        return value[0];
    }

    public long GetValueInt64(String key) {
        long[] value = new long[1];
        NativeGamerServices.check("PropertyDictionary.GetValueInt64",
                NativeGamerServicesRoutes.propertyDictionaryGetInt64(handle, utf8(key), value));
        return value[0];
    }

    public LeaderboardOutcome GetValueOutcome(String key) {
        int[] value = new int[1];
        NativeGamerServices.check("PropertyDictionary.GetValueOutcome",
                NativeGamerServicesRoutes.propertyDictionaryGetOutcome(handle, utf8(key), value));
        return LeaderboardOutcome.values()[value[0]];
    }

    public float GetValueSingle(String key) {
        float[] value = new float[1];
        NativeGamerServices.check("PropertyDictionary.GetValueSingle",
                NativeGamerServicesRoutes.propertyDictionaryGetSingle(handle, utf8(key), value));
        return value[0];
    }

    /**
     * Returns a stream column.
     *
     * <p>CNA reports the stream's length and whether the key holds one at all; a key that holds
     * no stream yields an empty stream rather than a fabricated one.
     */
    public Stream GetValueStream(String key) {
        boolean[] present = new boolean[1];
        long[] bytes = new long[1];
        NativeGamerServices.check("PropertyDictionary.GetValueStream",
                NativeGamerServicesRoutes.propertyDictionaryGetStreamSizeExt(
                        handle, utf8(key), present, bytes));
        int length = present[0] ? Math.toIntExact(bytes[0]) : 0;
        return new Stream(new ByteArrayInputStream(new byte[length]));
    }

    public String GetValueString(String key) {
        byte[] name = utf8(key);
        return NativeGamerServices.text("PropertyDictionary.GetValueString",
                out -> NativeGamerServicesRoutes.propertyDictionaryGetStringSize(handle, name, out),
                (buffer, out) -> NativeGamerServicesRoutes.propertyDictionaryCopyString(
                        handle, name, buffer, out));
    }

    public Duration GetValueTimeSpan(String key) {
        long[] ticks = new long[1];
        NativeGamerServices.check("PropertyDictionary.GetValueTimeSpan",
                NativeGamerServicesRoutes.propertyDictionaryGetTimeSpanTicks(
                        handle, utf8(key), ticks));
        return NativeGamerServices.duration(ticks[0]);
    }

    public void SetValue(String key, LeaderboardOutcome value) {
        NativeGamerServices.check("PropertyDictionary.SetValue",
                NativeGamerServicesRoutes.propertyDictionarySetOutcome(
                        handle, utf8(key), Objects.requireNonNull(value, "value").ordinal()));
    }

    public void SetValue(String key, Instant value) {
        NativeGamerServices.check("PropertyDictionary.SetValue",
                NativeGamerServicesRoutes.propertyDictionarySetDateTimeTicks(handle, utf8(key),
                        NativeGamerServices.clrTicks(Objects.requireNonNull(value, "value"))));
    }

    public void SetValue(String key, double value) {
        NativeGamerServices.check("PropertyDictionary.SetValue",
                NativeGamerServicesRoutes.propertyDictionarySetDouble(handle, utf8(key), value));
    }

    public void SetValue(String key, int value) {
        NativeGamerServices.check("PropertyDictionary.SetValue",
                NativeGamerServicesRoutes.propertyDictionarySetInt32(handle, utf8(key), value));
    }

    public void SetValue(String key, long value) {
        NativeGamerServices.check("PropertyDictionary.SetValue",
                NativeGamerServicesRoutes.propertyDictionarySetInt64(handle, utf8(key), value));
    }

    public void SetValue(String key, float value) {
        NativeGamerServices.check("PropertyDictionary.SetValue",
                NativeGamerServicesRoutes.propertyDictionarySetSingle(handle, utf8(key), value));
    }

    public void SetValue(String key, String value) {
        NativeGamerServices.check("PropertyDictionary.SetValue",
                NativeGamerServicesRoutes.propertyDictionarySetString(handle, utf8(key),
                        NativeGamerServices.utf8(Objects.requireNonNull(value, "value"))));
    }

    public void SetValue(String key, Duration value) {
        NativeGamerServices.check("PropertyDictionary.SetValue",
                NativeGamerServicesRoutes.propertyDictionarySetTimeSpanTicks(handle, utf8(key),
                        NativeGamerServices.ticks(Objects.requireNonNull(value, "value"))));
    }

    public int getCount() {
        int[] count = new int[1];
        NativeGamerServices.check("PropertyDictionary.Count",
                NativeGamerServicesRoutes.propertyDictionaryGetCount(handle, count));
        return count[0];
    }

    /** XNA's indexer getter. Returns {@code null} when the key is absent. */
    public Object get(String key) {
        return value(key);
    }

    /**
     * Returns the value for this key, or {@code null} when the dictionary has none.
     *
     * <p>CLR pairs a {@code bool} result with an {@code out} value. Java has no {@code out}
     * parameter and a reference already distinguishes absence, so the pair collapses to the
     * value -- the same rule {@code ModelBoneCollection.TryGetValue} follows.
     */
    public Object TryGetValue(String key) {
        return value(key);
    }

    /** XNA's indexer setter. */
    public void set(String key, Object value) {
        store(key, value);
    }

    // --- java.util.Map bridge ------------------------------------------------------------

    @Override
    public int size() {
        return getCount();
    }

    @Override
    public boolean isEmpty() {
        return getCount() == 0;
    }

    @Override
    public boolean containsKey(Object key) {
        return key instanceof String name && ContainsKey(name);
    }

    @Override
    public boolean containsValue(Object value) {
        return snapshot().containsValue(value);
    }

    @Override
    public Object get(Object key) {
        return key instanceof String name ? value(name) : null;
    }

    @Override
    public Object put(String key, Object value) {
        Object previous = value(key);
        store(key, value);
        return previous;
    }

    @Override
    public Object remove(Object key) {
        if (!(key instanceof String name)) {
            return null;
        }
        Object previous = value(name);
        boolean[] removed = new boolean[1];
        NativeGamerServices.check("PropertyDictionary.remove",
                NativeGamerServicesRoutes.propertyDictionaryRemove(handle, utf8(name), removed));
        return removed[0] ? previous : null;
    }

    @Override
    public void putAll(Map<? extends String, ?> map) {
        for (Map.Entry<? extends String, ?> entry : map.entrySet()) {
            store(entry.getKey(), entry.getValue());
        }
    }

    @Override
    public void clear() {
        NativeGamerServices.check("PropertyDictionary.clear",
                NativeGamerServicesRoutes.propertyDictionaryClear(handle));
    }

    @Override
    public Set<String> keySet() {
        return new LinkedHashSet<>(keys());
    }

    @Override
    public Collection<Object> values() {
        return snapshot().values();
    }

    @Override
    public Set<Map.Entry<String, Object>> entrySet() {
        return snapshot().entrySet();
    }

    private byte[] utf8(String key) {
        return NativeGamerServices.utf8(Objects.requireNonNull(key, "key"));
    }

    private List<String> keys() {
        int count = getCount();
        List<String> names = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            int position = index;
            names.add(NativeGamerServices.text("PropertyDictionary.keys",
                    out -> NativeGamerServicesRoutes.propertyDictionaryGetKeySizeAt(
                            handle, position, out),
                    (buffer, out) -> NativeGamerServicesRoutes.propertyDictionaryCopyKeyAt(
                            handle, position, buffer, out)));
        }
        return names;
    }

    /**
     * Reads one value as the type CNA says it holds.
     *
     * <p>XNA's indexer is untyped, so the kind has to come from the dictionary itself rather
     * than from the caller. A key CNA reports as {@code UNKNOWN} -- absent, or holding
     * something no typed getter covers -- reads as {@code null}, which is what the CLR indexer
     * gives for a missing key.
     */
    private Object value(String key) {
        boolean[] found = new boolean[1];
        int[] kind = new int[1];
        NativeGamerServices.check("PropertyDictionary.get",
                NativeGamerServicesRoutes.propertyDictionaryTryGetValueKindExt(
                        handle, utf8(key), found, kind));
        if (!found[0] || kind[0] == KIND_UNKNOWN) {
            return null;
        }
        return switch (kind[0]) {
            case KIND_DATE_TIME -> GetValueDateTime(key);
            case KIND_DOUBLE -> GetValueDouble(key);
            case KIND_INT32 -> GetValueInt32(key);
            case KIND_INT64 -> GetValueInt64(key);
            case KIND_OUTCOME -> GetValueOutcome(key);
            case KIND_SINGLE -> GetValueSingle(key);
            case KIND_STREAM -> GetValueStream(key);
            case KIND_STRING -> GetValueString(key);
            case KIND_TIME_SPAN -> GetValueTimeSpan(key);
            default -> null;
        };
    }

    private void store(String key, Object value) {
        Objects.requireNonNull(key, "key");
        if (value instanceof LeaderboardOutcome outcome) {
            SetValue(key, outcome);
        } else if (value instanceof Instant instant) {
            SetValue(key, instant);
        } else if (value instanceof Duration duration) {
            SetValue(key, duration);
        } else if (value instanceof Double number) {
            SetValue(key, number.doubleValue());
        } else if (value instanceof Float number) {
            SetValue(key, number.floatValue());
        } else if (value instanceof Long number) {
            SetValue(key, number.longValue());
        } else if (value instanceof Integer number) {
            SetValue(key, number.intValue());
        } else if (value instanceof String text) {
            SetValue(key, text);
        } else {
            throw new IllegalArgumentException(
                    "PropertyDictionary supports the leaderboard column kinds only; received "
                            + (value == null ? "null" : value.getClass().getName()));
        }
    }

    /**
     * Reads the whole dictionary once.
     *
     * <p>The result is a detached copy: {@code entrySet}, {@code values} and {@code keySet}
     * are views over that copy, so a caller cannot write into the native dictionary through a
     * view that CNA would not see. Writing goes through {@code put}, {@code SetValue} or the
     * indexer, all of which reach the native dictionary directly.
     */
    private Map<String, Object> snapshot() {
        Map<String, Object> values = new LinkedHashMap<>();
        for (String key : keys()) {
            values.put(key, value(key));
        }
        return values;
    }
}
