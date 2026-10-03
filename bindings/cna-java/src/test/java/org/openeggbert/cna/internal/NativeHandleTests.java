package org.openeggbert.cna.internal;

import org.junit.jupiter.api.Test;

import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.*;

final class NativeHandleTests {

    @Test
    void ZeroHandleIsRejectedBeforeOwnershipBegins() {
        assertThrows(IllegalArgumentException.class,
                () -> new NativeHandle(0, NativeHandle.Ownership.OWNED, ignored -> { }));
    }

    @Test
    void OwnedAndAdoptedReleaseExactlyOnce() {
        for (NativeHandle.Ownership ownership : new NativeHandle.Ownership[] {
                NativeHandle.Ownership.OWNED, NativeHandle.Ownership.ADOPTED}) {
            AtomicInteger releases = new AtomicInteger();
            NativeHandle handle = new NativeHandle(42, ownership, ignored -> releases.incrementAndGet());
            handle.close();
            handle.close();
            assertEquals(1, releases.get());
            assertTrue(handle.isClosed());
            assertThrows(IllegalStateException.class, handle::requireValue);
        }
    }

    @Test
    void BorrowedAndParentOwnedNeverRelease() {
        for (NativeHandle.Ownership ownership : new NativeHandle.Ownership[] {
                NativeHandle.Ownership.BORROWED, NativeHandle.Ownership.PARENT_OWNED}) {
            AtomicInteger releases = new AtomicInteger();
            NativeHandle handle = new NativeHandle(7, ownership, ignored -> releases.incrementAndGet());
            handle.close();
            assertEquals(0, releases.get());
            assertTrue(handle.isClosed());
        }
    }

    @Test
    void FailedReleaseRemainsOpenForExplicitRetry() {
        AtomicInteger attempts = new AtomicInteger();
        NativeHandle handle = new NativeHandle(9, NativeHandle.Ownership.OWNED, ignored -> {
            if (attempts.incrementAndGet() == 1) throw new IllegalStateException("busy child");
        });
        assertThrows(IllegalStateException.class, handle::close);
        assertFalse(handle.isClosed());
        handle.close();
        assertTrue(handle.isClosed());
        assertEquals(2, attempts.get());
    }
}
