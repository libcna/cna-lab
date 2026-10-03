package org.openeggbert.cna.internal;

import java.util.Objects;
import java.util.function.LongConsumer;

/** Internal ownership-aware wrapper; raw addresses never cross into strict XNA signatures. */
class NativeHandle implements AutoCloseable {

    enum Ownership {
        OWNED,
        BORROWED,
        PARENT_OWNED,
        ADOPTED
    }

    private long value;
    private final Ownership ownership;
    private final LongConsumer releaser;

    NativeHandle(long value, Ownership ownership, LongConsumer releaser) {
        if (value == 0L) {
            throw new IllegalArgumentException("A native handle must not be zero");
        }
        this.value = value;
        this.ownership = Objects.requireNonNull(ownership, "ownership");
        this.releaser = Objects.requireNonNull(releaser, "releaser");
    }

    synchronized long requireValue() {
        if (value == 0L) {
            throw new IllegalStateException("Native resource is already closed");
        }
        return value;
    }

    public synchronized boolean isClosed() {
        return value == 0L;
    }

    Ownership getOwnership() {
        return ownership;
    }

    /**
     * Forgets the handle without releasing it.
     *
     * <p>For a native object that has been handed to something that now owns it: releasing it
     * here as well would be a double free, and keeping it would let a later close become one.
     */
    synchronized void surrender() {
        value = 0L;
    }

    @Override
    public synchronized void close() {
        if (value == 0L) {
            return;
        }
        long closing = value;
        if (ownership == Ownership.OWNED || ownership == Ownership.ADOPTED) {
            releaser.accept(closing);
        }
        value = 0L;
    }
}

