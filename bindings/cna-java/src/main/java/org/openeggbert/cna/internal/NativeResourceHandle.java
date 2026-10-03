package org.openeggbert.cna.internal;

import java.util.function.LongConsumer;

/** Owned native child resource that must be released before its parent game. */
final class NativeResourceHandle extends NativeHandle {

    NativeResourceHandle(long value, LongConsumer releaser) {
        super(value, Ownership.OWNED, releaser);
    }

    NativeResourceHandle(long value, Ownership ownership, LongConsumer releaser) {
        super(value, ownership, releaser);
    }
}
