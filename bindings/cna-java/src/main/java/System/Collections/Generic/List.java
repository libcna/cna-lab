package System.Collections.Generic;

import java.util.Iterator;
import java.util.NoSuchElementException;

/** Minimal carrier for CLR's concrete {@code List<T>.Enumerator} return type. */
public final class List {

    private List() {
    }

    /** Snapshot iterator used by mapped XNA collection {@code GetEnumerator()} methods. */
    public static final class Enumerator<T> implements Iterator<T> {

        private final java.util.List<T> values;
        private int index;

        public Enumerator(java.util.List<T> values) {
            this.values = java.util.List.copyOf(values);
        }

        @Override
        public boolean hasNext() {
            return index < values.size();
        }

        @Override
        public T next() {
            if (!hasNext()) {
                throw new NoSuchElementException();
            }
            return values.get(index++);
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException("XNA effect collections are read-only");
        }
    }
}
