package Microsoft.Xna.Framework.Net;

import org.openeggbert.cna.internal.NativeDeferredRelease;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;

import java.util.AbstractList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

/**
 * The optional integers a title uses to describe or to search for a session.
 *
 * <p>XNA declares {@code IList<int?>} of exactly eight slots: an unset slot is null, a search
 * matches a session on the slots the search actually set, and the list's size never changes --
 * {@code Add}, {@code Insert}, {@code Remove}, {@code RemoveAt} and {@code Clear} throw
 * {@code NotSupportedException}, which is {@link UnsupportedOperationException} here. Java's
 * {@link Integer} carries the null distinction, so nothing is adapted for it.
 *
 * <p>CLR's base is {@code Object} with {@code IList<T>} as an interface, so this implements
 * {@link List} rather than extending {@link AbstractList}. The derived list operations are
 * delegated to a private {@code AbstractList} view over the same native list, which is what
 * keeps them consistent with {@code get}, {@code set} and {@code size} without restating them.
 */
public class NetworkSessionProperties implements List<Integer> {

    private final long handle;
    private final List<Integer> view = new AbstractList<>() {

        @Override
        public Integer get(int index) {
            return NetworkSessionProperties.this.item(index);
        }

        @Override
        public Integer set(int index, Integer element) {
            return NetworkSessionProperties.this.item(index, element);
        }

        @Override
        public int size() {
            return NetworkSessionProperties.this.count();
        }
    };

    /**
     * Creates an empty property list, as a game does before searching for a session.
     *
     * <p>The native list this owns is released once this object is unreachable, on the thread
     * that created it, the next time that thread pumps the framework dispatcher. XNA's type is
     * not disposable, so there is nothing for a game to close and nothing it has to remember;
     * CNA refuses the release from any other thread, which is why it waits for this one rather
     * than happening on a cleaner thread.
     */
    // The cleaner registration hands `this` out of the constructor, and this type is not final
    // because XNA's is not sealed. The escape is inert: Cleaner.register only takes a phantom
    // reference and never calls a method on the object, and the cleaning action captures the
    // handle and the thread by value rather than the object -- capturing the object would keep
    // it reachable from its own cleaner and it could never be collected at all.
    @SuppressWarnings("this-escape")
    public NetworkSessionProperties() {
        long[] properties = new long[1];
        NativeGamerServices.check("NetworkSessionProperties",
                NativeNetworkRoutes.networkSessionPropertiesCreate(properties));
        handle = properties[0];
        NativeDeferredRelease.onOwningThread(this, handle,
                NativeNetworkRoutes::networkSessionPropertiesDestroy,
                "cna_network_session_properties_destroy");
    }

    /**
     * Adopts a list a session owns.
     *
     * <p>No release is registered: the session owns it and frees it, and a second owner would be
     * a double free.
     */
    NetworkSessionProperties(long handle) {
        this.handle = handle;
    }

    long handle() {
        return handle;
    }

    /** Returns XNA's enumerator over the property slots. */
    public final Iterator<Integer> GetEnumerator() {
        return view.iterator();
    }

    public final int getCount() {
        return count();
    }

    @Override
    public final Integer get(int index) {
        return item(index);
    }

    @Override
    public final Integer set(int index, Integer item) {
        return item(index, item);
    }

    @Override
    public final int size() {
        return count();
    }

    @Override
    public final boolean isEmpty() {
        return view.isEmpty();
    }

    /**
     * Reports whether one value is in the list.
     *
     * <p>Asked of CNA in one call rather than walked slot by slot, and {@code null} is a real
     * question here: an unset slot is what {@code int?} means, so asking whether the list has one
     * is asking whether anything is still unset.
     */
    @Override
    public final boolean contains(Object item) {
        if (item != null && !(item instanceof Integer)) {
            return false;
        }
        boolean[] contains = new boolean[1];
        NativeGamerServices.check("NetworkSessionProperties.contains",
                NativeNetworkRoutes.networkSessionPropertiesContains(
                        handle, new byte[3], optional((Integer) item), contains));
        return contains[0];
    }

    @Override
    public final Iterator<Integer> iterator() {
        return view.iterator();
    }

    /**
     * Copies every slot out in one native call.
     *
     * <p>This is the Java spelling of XNA's {@code CopyTo}, so it takes CNA's own bulk copy
     * rather than reading the slots back one at a time.
     */
    @Override
    public final Object[] toArray() {
        return snapshot();
    }

    @Override
    public final <T> T[] toArray(T[] array) {
        return view.toArray(array);
    }

    @Override
    public final boolean add(Integer item) {
        return view.add(item);
    }

    @Override
    public final boolean remove(Object item) {
        return view.remove(item);
    }

    @Override
    public final boolean containsAll(Collection<?> collection) {
        return view.containsAll(collection);
    }

    @Override
    public final boolean addAll(Collection<? extends Integer> collection) {
        return view.addAll(collection);
    }

    @Override
    public final boolean addAll(int index, Collection<? extends Integer> collection) {
        return view.addAll(index, collection);
    }

    @Override
    public final boolean removeAll(Collection<?> collection) {
        return view.removeAll(collection);
    }

    @Override
    public final boolean retainAll(Collection<?> collection) {
        return view.retainAll(collection);
    }

    @Override
    public final void clear() {
        throw new UnsupportedOperationException("NetworkSessionProperties has a fixed size");
    }

    @Override
    public final void add(int index, Integer item) {
        throw new UnsupportedOperationException("NetworkSessionProperties has a fixed size");
    }

    @Override
    public final Integer remove(int index) {
        throw new UnsupportedOperationException("NetworkSessionProperties has a fixed size");
    }

    @Override
    public final int indexOf(Object item) {
        if (item != null && !(item instanceof Integer)) {
            return -1;
        }
        int[] index = new int[1];
        NativeGamerServices.check("NetworkSessionProperties.indexOf",
                NativeNetworkRoutes.networkSessionPropertiesIndexOf(
                        handle, new byte[3], optional((Integer) item), index));
        return index[0];
    }

    @Override
    public final int lastIndexOf(Object item) {
        return view.lastIndexOf(item);
    }

    @Override
    public final ListIterator<Integer> listIterator() {
        return view.listIterator();
    }

    @Override
    public final ListIterator<Integer> listIterator(int index) {
        return view.listIterator(index);
    }

    @Override
    public final List<Integer> subList(int fromIndex, int toIndex) {
        return view.subList(fromIndex, toIndex);
    }

    private static long[] optional(Integer value) {
        return new long[] {value == null ? 0L : 1L, value == null ? 0L : value};
    }

    private Integer item(int index) {
        long[] values = new long[2];
        NativeGamerServices.check("NetworkSessionProperties.get",
                NativeNetworkRoutes.networkSessionPropertiesGetItem(
                        handle, index, new byte[3], values));
        return values[0] != 0L ? (int) values[1] : null;
    }

    private Integer item(int index, Integer value) {
        Integer previous = item(index);
        NativeGamerServices.check("NetworkSessionProperties.set",
                NativeNetworkRoutes.networkSessionPropertiesSetItem(
                        handle, index, new byte[3], optional(value)));
        return previous;
    }

    private int count() {
        int[] count = new int[1];
        NativeGamerServices.check("NetworkSessionProperties.size",
                NativeNetworkRoutes.networkSessionPropertiesGetCount(handle, count));
        return count[0];
    }

    /**
     * Reads every slot through CNA's bulk copy.
     *
     * <p>CNA refuses a destination that cannot hold the whole list and writes no partial copy,
     * so the buffer is sized from the current count first. An unset slot stays null, which is
     * the distinction {@code int?} carries and {@link Integer} keeps.
     */
    private Integer[] snapshot() {
        int size = count();
        long[] values = new long[size * 2];
        long[] copied = new long[1];
        NativeGamerServices.check("NetworkSessionProperties.toArray",
                NativeNetworkRoutes.networkSessionPropertiesCopyTo(
                        handle, new byte[size * 3], values, 0, copied));
        Integer[] result = new Integer[(int) copied[0]];
        for (int index = 0; index < result.length; index++) {
            result[index] = values[index * 2] != 0L ? (int) values[index * 2 + 1] : null;
        }
        return result;
    }
}
