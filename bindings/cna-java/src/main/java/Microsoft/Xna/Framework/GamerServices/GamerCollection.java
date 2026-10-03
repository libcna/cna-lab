package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.util.AbstractList;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.function.IntFunction;
import java.util.function.IntSupplier;
import java.util.function.LongFunction;

/**
 * A read-only collection of gamers.
 *
 * <p>XNA derives this from {@code ReadOnlyCollection<T>} and additionally declares
 * {@code IEnumerable<Gamer>} so a caller can walk any gamer collection as {@code Gamer}. Java
 * forbids one class from implementing two parameterizations of one interface, so the
 * projection bounds {@code T} by {@link Gamer} instead: a {@code GamerCollection<T>} is an
 * {@code Iterable<? extends Gamer>}, which is what those call sites wanted.
 *
 * <p>A collection is a live view, never a snapshot: reading it twice can give two different
 * answers, exactly as XNA's does when a gamer joins or signs out in between.
 *
 * @param <T> the kind of gamer this collection holds
 */
public class GamerCollection<T extends Gamer> extends AbstractList<T> {

    private final long handle;
    private final IntFunction<T> elementAt;
    private final IntSupplier elementCount;

    GamerCollection(long handle, LongFunction<T> factory) {
        this.handle = handle;
        this.elementAt = index -> {
            long[] gamer = new long[1];
            NativeGamerServices.check("GamerCollection.get",
                    NativeGamerServicesRoutes.gamerCollectionGetAt(handle, index, gamer));
            return factory.apply(gamer[0]);
        };
        this.elementCount = () -> {
            int[] count = new int[1];
            NativeGamerServices.check("GamerCollection.size",
                    NativeGamerServicesRoutes.gamerCollectionGetCount(handle, count));
            return count[0];
        };
    }

    /**
     * Builds a collection over a caller-supplied live view.
     *
     * <p>CLR builds gamer collections from {@code Microsoft.Xna.Framework.Net} through
     * assembly-internal access, which Java has no equivalent for across packages. Two ordinary
     * functional interfaces replace it: the collection stays a live view rather than becoming a
     * snapshot, and no native handle appears in a protected signature.
     */
    protected GamerCollection(IntFunction<T> elementAt, IntSupplier elementCount) {
        this.handle = 0L;
        this.elementAt = elementAt;
        this.elementCount = elementCount;
    }

    long handle() {
        return handle;
    }

    /** Returns XNA's enumerator over this collection. */
    public final GamerCollectionEnumerator<T> GetEnumerator() {
        if (handle == 0L) {
            return new GamerCollectionEnumerator<>(this);
        }
        long[] enumerator = new long[1];
        NativeGamerServices.check("GamerCollection.GetEnumerator",
                NativeGamerServicesRoutes.gamerCollectionCreateEnumerator(handle, enumerator));
        return new GamerCollectionEnumerator<>(enumerator[0], this);
    }

    @Override
    public T get(int index) {
        return elementAt.apply(index);
    }

    @Override
    public int size() {
        return elementCount.getAsInt();
    }

    /**
     * XNA's enumerator over a gamer collection.
     *
     * <p>It is a struct in XNA, so it is a value carrier here, and it is also a Java
     * {@link Iterator} so the collection works in a for-each loop. {@code MoveNext} and
     * {@code getCurrent} are the XNA members; {@code hasNext} and {@code next} are the Java
     * bridge over the same cursor.
     *
     * @param <T> the kind of gamer this enumerator yields
     */
    public static final class GamerCollectionEnumerator<T extends Gamer> implements Iterator<T> {

        private final long handle;
        private final GamerCollection<T> owner;
        private int position = -1;
        private boolean hasCurrent;

        GamerCollectionEnumerator(long handle, GamerCollection<T> owner) {
            this.handle = handle;
            this.owner = owner;
        }

        GamerCollectionEnumerator(GamerCollection<T> owner) {
            this(0L, owner);
        }

        public GamerCollectionEnumerator() {
            this(0L, null);
        }

        public GamerCollectionEnumerator(GamerCollectionEnumerator<T> value) {
            this(value == null ? 0L : value.handle, value == null ? null : value.owner);
        }

        public void Dispose() {
            if (handle != 0L) {
                NativeGamerServices.check("GamerCollectionEnumerator.Dispose",
                        NativeGamerServicesRoutes.gamerEnumeratorDestroy(handle));
            }
        }

        public void close() {
            Dispose();
        }

        public boolean MoveNext() {
            if (owner == null) {
                return false;
            }
            if (handle == 0L) {
                hasCurrent = ++position < owner.size();
                return hasCurrent;
            }
            boolean[] present = new boolean[1];
            NativeGamerServices.check("GamerCollectionEnumerator.MoveNext",
                    NativeGamerServicesRoutes.gamerEnumeratorMoveNext(handle, present));
            hasCurrent = present[0];
            position++;
            return hasCurrent;
        }

        public T getCurrent() {
            if (!hasCurrent) {
                throw new IllegalStateException(
                        "Current is undefined before the first MoveNext and after the last");
            }
            if (handle == 0L) {
                return owner.get(position);
            }
            long[] gamer = new long[1];
            NativeGamerServices.check("GamerCollectionEnumerator.Current",
                    NativeGamerServicesRoutes.gamerEnumeratorGetCurrent(handle, gamer));
            return owner.get(position);
        }

        @Override
        public boolean hasNext() {
            return owner != null && position + 1 < owner.size();
        }

        @Override
        public T next() {
            if (!hasNext()) {
                throw new NoSuchElementException();
            }
            position++;
            hasCurrent = true;
            return owner.get(position);
        }
    }
}
