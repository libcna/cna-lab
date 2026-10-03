package Microsoft.Xna.Framework.Input.Touch;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.NoSuchElementException;
import java.util.Objects;

/** Fixed-size, read-only XNA touch snapshot with a Java {@link List} bridge. */
public final class TouchCollection implements List<TouchLocation> {

    private static final int MAX_TOUCHES = 8;

    private final boolean connected;
    private final TouchLocation[] locations;

    public TouchCollection() {
        connected = false;
        locations = new TouchLocation[0];
    }

    public TouchCollection(TouchCollection value) {
        TouchCollection source = Objects.requireNonNull(value, "value");
        connected = source.connected;
        locations = copy(source.locations);
    }

    public TouchCollection(TouchLocation[] touches) {
        Objects.requireNonNull(touches, "touches");
        if (touches.length > MAX_TOUCHES) {
            throw new IndexOutOfBoundsException("touches contains more than eight locations");
        }
        connected = true;
        locations = copy(touches);
    }

    private TouchCollection(boolean connected, TouchLocation[] touches) {
        this.connected = connected;
        locations = copy(touches);
    }

    public int getCount() {
        return locations.length;
    }

    public boolean getIsConnected() {
        return connected;
    }

    public boolean getIsReadOnly() {
        return true;
    }

    @Override
    public TouchLocation get(int index) {
        checkIndex(index);
        return new TouchLocation(locations[index]);
    }

    @Override
    public TouchLocation set(int index, TouchLocation item) {
        throw readOnly();
    }

    public void Add(TouchLocation item) {
        throw readOnly();
    }

    public void Clear() {
        throw readOnly();
    }

    public boolean Contains(TouchLocation item) {
        return IndexOf(Objects.requireNonNull(item, "item")) >= 0;
    }

    public void CopyTo(TouchLocation[] array, int arrayIndex) {
        Objects.requireNonNull(array, "array");
        if (arrayIndex < 0 || (long) arrayIndex + locations.length > array.length) {
            throw new IndexOutOfBoundsException("arrayIndex");
        }
        for (int index = 0; index < locations.length; index++) {
            array[arrayIndex + index] = new TouchLocation(locations[index]);
        }
    }

    public FindResult FindById(int id) {
        for (TouchLocation location : locations) {
            if (location.getId() == id) {
                return new FindResult(true, location);
            }
        }
        return new FindResult(false, new TouchLocation());
    }

    public Enumerator GetEnumerator() {
        return new Enumerator(this);
    }

    public int IndexOf(TouchLocation item) {
        Objects.requireNonNull(item, "item");
        for (int index = 0; index < locations.length; index++) {
            if (locations[index].operatorEquals(item)) {
                return index;
            }
        }
        return -1;
    }

    public void Insert(int index, TouchLocation item) {
        throw readOnly();
    }

    public boolean Remove(TouchLocation item) {
        throw readOnly();
    }

    public void RemoveAt(int index) {
        throw readOnly();
    }

    @Override
    public int size() {
        return locations.length;
    }

    @Override
    public boolean isEmpty() {
        return locations.length == 0;
    }

    @Override
    public boolean contains(Object item) {
        return item instanceof TouchLocation location && IndexOf(location) >= 0;
    }

    @Override
    public Iterator<TouchLocation> iterator() {
        return new Enumerator(this);
    }

    @Override
    public Object[] toArray() {
        Object[] result = new Object[locations.length];
        for (int index = 0; index < locations.length; index++) {
            result[index] = new TouchLocation(locations[index]);
        }
        return result;
    }

    @Override
    @SuppressWarnings("unchecked")
    public <T> T[] toArray(T[] array) {
        Objects.requireNonNull(array, "array");
        T[] result = array.length >= locations.length
                ? array
                : (T[]) java.lang.reflect.Array.newInstance(
                        array.getClass().getComponentType(), locations.length);
        for (int index = 0; index < locations.length; index++) {
            result[index] = (T) new TouchLocation(locations[index]);
        }
        if (result.length > locations.length) {
            result[locations.length] = null;
        }
        return result;
    }

    @Override
    public boolean add(TouchLocation item) {
        throw readOnly();
    }

    @Override
    public boolean remove(Object item) {
        throw readOnly();
    }

    @Override
    public boolean containsAll(Collection<?> collection) {
        Objects.requireNonNull(collection, "collection");
        for (Object item : collection) {
            if (!contains(item)) {
                return false;
            }
        }
        return true;
    }

    @Override
    public boolean addAll(Collection<? extends TouchLocation> collection) {
        throw readOnly();
    }

    @Override
    public boolean addAll(int index, Collection<? extends TouchLocation> collection) {
        throw readOnly();
    }

    @Override
    public boolean removeAll(Collection<?> collection) {
        throw readOnly();
    }

    @Override
    public boolean retainAll(Collection<?> collection) {
        throw readOnly();
    }

    @Override
    public void clear() {
        throw readOnly();
    }

    @Override
    public void add(int index, TouchLocation item) {
        throw readOnly();
    }

    @Override
    public TouchLocation remove(int index) {
        throw readOnly();
    }

    @Override
    public int indexOf(Object item) {
        return item instanceof TouchLocation location ? IndexOf(location) : -1;
    }

    @Override
    public int lastIndexOf(Object item) {
        if (!(item instanceof TouchLocation location)) {
            return -1;
        }
        for (int index = locations.length - 1; index >= 0; index--) {
            if (locations[index].operatorEquals(location)) {
                return index;
            }
        }
        return -1;
    }

    @Override
    public ListIterator<TouchLocation> listIterator() {
        return snapshot().listIterator();
    }

    @Override
    public ListIterator<TouchLocation> listIterator(int index) {
        return snapshot().listIterator(index);
    }

    @Override
    public List<TouchLocation> subList(int fromIndex, int toIndex) {
        return Collections.unmodifiableList(
                new ArrayList<>(snapshot().subList(fromIndex, toIndex)));
    }

    static TouchCollection fromNative(boolean connected, TouchLocation[] locations) {
        return new TouchCollection(connected, locations);
    }

    private List<TouchLocation> snapshot() {
        List<TouchLocation> result = new ArrayList<>(locations.length);
        for (TouchLocation location : locations) {
            result.add(new TouchLocation(location));
        }
        return Collections.unmodifiableList(result);
    }

    private void checkIndex(int index) {
        if (index < 0 || index >= locations.length) {
            throw new IndexOutOfBoundsException("index");
        }
    }

    private static TouchLocation[] copy(TouchLocation[] source) {
        TouchLocation[] result = new TouchLocation[source.length];
        for (int index = 0; index < source.length; index++) {
            result[index] = new TouchLocation(
                    Objects.requireNonNull(source[index], "touches element"));
        }
        return result;
    }

    private static UnsupportedOperationException readOnly() {
        return new UnsupportedOperationException("TouchCollection is read-only");
    }

    /** Immutable Java carrier for XNA's Boolean-plus-out-value method. */
    public static final class FindResult {

        private final boolean succeeded;
        private final TouchLocation touchLocation;

        private FindResult(boolean succeeded, TouchLocation touchLocation) {
            this.succeeded = succeeded;
            this.touchLocation = new TouchLocation(touchLocation);
        }

        public boolean getSucceeded() {
            return succeeded;
        }

        public TouchLocation getTouchLocation() {
            return new TouchLocation(touchLocation);
        }
    }

    /** XNA cursor plus the reviewed Java {@link Iterator} bridge. */
    public static final class Enumerator implements Iterator<TouchLocation> {

        private final TouchCollection collection;
        private int position;

        public Enumerator() {
            collection = new TouchCollection();
            position = 0;
        }

        public Enumerator(Enumerator value) {
            Enumerator source = Objects.requireNonNull(value, "value");
            collection = new TouchCollection(source.collection);
            position = source.position;
        }

        private Enumerator(TouchCollection collection) {
            this.collection = new TouchCollection(collection);
            position = -1;
        }

        public TouchLocation getCurrent() {
            return collection.get(position);
        }

        public boolean MoveNext() {
            position++;
            if (position >= collection.getCount()) {
                position = collection.getCount();
                return false;
            }
            return true;
        }

        public void Dispose() {
        }

        public void close() {
            Dispose();
        }

        @Override
        public boolean hasNext() {
            return position + 1 < collection.getCount();
        }

        @Override
        public TouchLocation next() {
            if (!MoveNext()) {
                throw new NoSuchElementException();
            }
            return getCurrent();
        }
    }
}
