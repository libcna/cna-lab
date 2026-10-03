package Microsoft.Xna.Framework;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;

/** Sorted, mutable collection of curve keys. Equal-position keys preserve insertion order. */
public class CurveKeyCollection implements Collection<CurveKey> {

    private List<CurveKey> keys = new ArrayList<>();
    float timeRange;
    float invTimeRange;
    boolean cacheAvailable = true;

    public CurveKeyCollection() {
    }

    public final CurveKey get(int index) {
        return keys.get(index);
    }

    public final void set(int index, CurveKey value) {
        CurveKey replacement = Objects.requireNonNull(value, "value");
        float position = keys.get(index).position;
        if (position == replacement.position) {
            keys.set(index, replacement);
            return;
        }
        keys.remove(index);
        Add(replacement);
    }

    public final int getCount() {
        return keys.size();
    }

    public final boolean getIsReadOnly() {
        return false;
    }

    public final int IndexOf(CurveKey item) {
        return keys.indexOf(item);
    }

    public final void RemoveAt(int index) {
        keys.remove(index);
        cacheAvailable = false;
    }

    public final void Add(CurveKey item) {
        CurveKey key = Objects.requireNonNull(item, "item");
        int index = binarySearch(key);
        if (index >= 0) {
            while (index < keys.size() && key.position == keys.get(index).position) {
                index++;
            }
        } else {
            index = ~index;
        }
        keys.add(index, key);
        cacheAvailable = false;
    }

    public final void Clear() {
        keys.clear();
        timeRange = 0.0f;
        invTimeRange = 0.0f;
        cacheAvailable = false;
    }

    public final boolean Contains(CurveKey item) {
        return keys.contains(item);
    }

    public final void CopyTo(CurveKey[] array, int arrayIndex) {
        Objects.requireNonNull(array, "array");
        for (int index = 0; index < keys.size(); index++) {
            array[arrayIndex + index] = keys.get(index);
        }
        cacheAvailable = false;
    }

    public final boolean Remove(CurveKey item) {
        cacheAvailable = false;
        return keys.remove(item);
    }

    public final Iterator<CurveKey> GetEnumerator() {
        return iterator();
    }

    public final CurveKeyCollection Clone() {
        CurveKeyCollection result = new CurveKeyCollection();
        result.keys = new ArrayList<>(keys);
        result.invTimeRange = invTimeRange;
        result.timeRange = timeRange;
        result.cacheAvailable = true;
        return result;
    }

    final void computeCacheValues() {
        timeRange = 0.0f;
        invTimeRange = 0.0f;
        if (keys.size() > 1) {
            timeRange = keys.get(keys.size() - 1).position - keys.get(0).position;
            if (timeRange > Float.MIN_VALUE) {
                invTimeRange = 1.0f / timeRange;
            }
        }
        cacheAvailable = true;
    }

    @Override
    public final int size() {
        return keys.size();
    }

    @Override
    public final boolean isEmpty() {
        return keys.isEmpty();
    }

    @Override
    public final boolean contains(Object item) {
        return keys.contains(item);
    }

    @Override
    public final Iterator<CurveKey> iterator() {
        Iterator<CurveKey> source = keys.iterator();
        return new Iterator<>() {
            @Override
            public boolean hasNext() {
                return source.hasNext();
            }

            @Override
            public CurveKey next() {
                return source.next();
            }

            @Override
            public void remove() {
                source.remove();
                cacheAvailable = false;
            }
        };
    }

    @Override
    public final Object[] toArray() {
        return keys.toArray();
    }

    @Override
    public final <T> T[] toArray(T[] array) {
        return keys.toArray(array);
    }

    @Override
    public final boolean add(CurveKey item) {
        Add(item);
        return true;
    }

    @Override
    public final boolean remove(Object item) {
        cacheAvailable = false;
        return keys.remove(item);
    }

    @Override
    public final boolean containsAll(Collection<?> collection) {
        return keys.containsAll(collection);
    }

    @Override
    public final boolean addAll(Collection<? extends CurveKey> collection) {
        Objects.requireNonNull(collection, "collection");
        boolean modified = false;
        for (CurveKey item : collection) {
            Add(item);
            modified = true;
        }
        return modified;
    }

    @Override
    public final boolean removeAll(Collection<?> collection) {
        cacheAvailable = false;
        return keys.removeAll(collection);
    }

    @Override
    public final boolean retainAll(Collection<?> collection) {
        cacheAvailable = false;
        return keys.retainAll(collection);
    }

    @Override
    public final void clear() {
        Clear();
    }

    private int binarySearch(CurveKey item) {
        int low = 0;
        int high = keys.size() - 1;
        while (low <= high) {
            int middle = low + ((high - low) >>> 1);
            int comparison = keys.get(middle).CompareTo(item);
            if (comparison == 0) {
                return middle;
            }
            if (comparison < 0) {
                low = middle + 1;
            } else {
                high = middle - 1;
            }
        }
        return ~low;
    }
}
