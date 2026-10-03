package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.NoSuchElementException;
import java.util.Objects;

/**
 * The achievements a title defines, addressable by index and by key.
 *
 * <p>XNA declares {@code IList<Achievement>} and reports the list read-only, so every mutating
 * member of the Java {@link List} bridge refuses with {@link UnsupportedOperationException} --
 * the same refusal the CLR list raises. The XNA-named {@code GetEnumerator}, {@code Count} and
 * the key indexer are the members a ported title calls.
 */
public final class AchievementCollection implements List<Achievement>, AutoCloseable {

    private final long handle;
    private boolean disposed;

    AchievementCollection(long handle) {
        this.handle = handle;
    }

    public void Dispose() {
        synchronized (this) {
            if (disposed) {
                return;
            }
            disposed = true;
        }
        NativeGamerServices.check("AchievementCollection.Dispose",
                NativeGamerServicesRoutes.achievementCollectionDestroy(handle));
    }

    @Override
    public void close() {
        Dispose();
    }

    /** Returns XNA's enumerator over this collection. */
    public Iterator<Achievement> GetEnumerator() {
        return iterator();
    }

    public int getCount() {
        return size();
    }

    public boolean getIsDisposed() {
        if (disposed) {
            return true;
        }
        boolean[] value = new boolean[1];
        NativeGamerServices.check("AchievementCollection.IsDisposed",
                NativeGamerServicesRoutes.achievementCollectionGetIsDisposed(handle, value));
        return value[0];
    }

    /** Returns the achievement with this key, or {@code null} when the title defines none. */
    public Achievement get(String achievementKey) {
        long[] achievement = new long[1];
        NativeGamerServices.check("AchievementCollection.get",
                NativeGamerServicesRoutes.achievementCollectionGetByKey(handle,
                        NativeGamerServices.utf8(
                                Objects.requireNonNull(achievementKey, "achievementKey")),
                        achievement));
        return achievement[0] == 0L ? null : new Achievement(achievement[0]);
    }

    @Override
    public Achievement get(int index) {
        long[] achievement = new long[1];
        NativeGamerServices.check("AchievementCollection.get",
                NativeGamerServicesRoutes.achievementCollectionGetAt(handle, index, achievement));
        return new Achievement(achievement[0]);
    }

    @Override
    public int size() {
        int[] count = new int[1];
        NativeGamerServices.check("AchievementCollection.size",
                NativeGamerServicesRoutes.achievementCollectionGetCount(handle, count));
        return count[0];
    }

    @Override
    public boolean isEmpty() {
        return size() == 0;
    }

    /**
     * Reports whether the collection holds one achievement.
     *
     * <p>CNA answers this in one call, so it is asked rather than derived from
     * {@link #indexOf(Object)}: the collection is native, and walking it to find out would
     * cross the boundary once per element for a question it already knows.
     */
    @Override
    public boolean contains(Object item) {
        if (!(item instanceof Achievement achievement)) {
            return false;
        }
        boolean[] contains = new boolean[1];
        NativeGamerServices.check("AchievementCollection.contains",
                NativeGamerServicesRoutes.achievementCollectionContains(
                        handle, achievement.handle(), contains));
        return contains[0];
    }

    @Override
    public Iterator<Achievement> iterator() {
        return listIterator();
    }

    @Override
    public Object[] toArray() {
        int count = size();
        Object[] values = new Object[count];
        for (int index = 0; index < count; index++) {
            values[index] = get(index);
        }
        return values;
    }

    @Override
    @SuppressWarnings("unchecked")
    public <T> T[] toArray(T[] array) {
        int count = size();
        T[] values = array.length >= count ? array
                : (T[]) java.lang.reflect.Array.newInstance(
                        array.getClass().getComponentType(), count);
        for (int index = 0; index < count; index++) {
            values[index] = (T) get(index);
        }
        if (values.length > count) {
            values[count] = null;
        }
        return values;
    }

    @Override
    public boolean containsAll(Collection<?> collection) {
        for (Object item : collection) {
            if (!contains(item)) {
                return false;
            }
        }
        return true;
    }

    @Override
    public int indexOf(Object item) {
        if (!(item instanceof Achievement achievement)) {
            return -1;
        }
        int[] index = new int[1];
        NativeGamerServices.check("AchievementCollection.indexOf",
                NativeGamerServicesRoutes.achievementCollectionIndexOf(
                        handle, achievement.handle(), index));
        return index[0];
    }

    @Override
    public int lastIndexOf(Object item) {
        for (int index = size() - 1; index >= 0; index--) {
            if (get(index).equals(item)) {
                return index;
            }
        }
        return -1;
    }

    @Override
    public ListIterator<Achievement> listIterator() {
        return listIterator(0);
    }

    @Override
    public ListIterator<Achievement> listIterator(int index) {
        return new ReadOnlyCursor(index);
    }

    @Override
    public List<Achievement> subList(int fromIndex, int toIndex) {
        if (fromIndex < 0 || toIndex > size() || fromIndex > toIndex) {
            throw new IndexOutOfBoundsException(
                    "fromIndex " + fromIndex + ", toIndex " + toIndex + ", size " + size());
        }
        List<Achievement> values = new java.util.ArrayList<>(toIndex - fromIndex);
        for (int index = fromIndex; index < toIndex; index++) {
            values.add(get(index));
        }
        return java.util.Collections.unmodifiableList(values);
    }

    @Override
    public boolean add(Achievement item) {
        throw readOnly();
    }

    @Override
    public void add(int index, Achievement item) {
        throw readOnly();
    }

    @Override
    public boolean addAll(Collection<? extends Achievement> collection) {
        throw readOnly();
    }

    @Override
    public boolean addAll(int index, Collection<? extends Achievement> collection) {
        throw readOnly();
    }

    @Override
    public void clear() {
        throw readOnly();
    }

    @Override
    public boolean remove(Object item) {
        throw readOnly();
    }

    @Override
    public Achievement remove(int index) {
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
    public Achievement set(int index, Achievement item) {
        throw readOnly();
    }

    private static UnsupportedOperationException readOnly() {
        return new UnsupportedOperationException("An AchievementCollection is read-only");
    }

    private final class ReadOnlyCursor implements ListIterator<Achievement> {

        private int position;

        ReadOnlyCursor(int position) {
            if (position < 0 || position > size()) {
                throw new IndexOutOfBoundsException("index " + position + ", size " + size());
            }
            this.position = position;
        }

        @Override
        public boolean hasNext() {
            return position < size();
        }

        @Override
        public Achievement next() {
            if (!hasNext()) {
                throw new NoSuchElementException();
            }
            return get(position++);
        }

        @Override
        public boolean hasPrevious() {
            return position > 0;
        }

        @Override
        public Achievement previous() {
            if (!hasPrevious()) {
                throw new NoSuchElementException();
            }
            return get(--position);
        }

        @Override
        public int nextIndex() {
            return position;
        }

        @Override
        public int previousIndex() {
            return position - 1;
        }

        @Override
        public void remove() {
            throw readOnly();
        }

        @Override
        public void set(Achievement item) {
            throw readOnly();
        }

        @Override
        public void add(Achievement item) {
            throw readOnly();
        }
    }
}
