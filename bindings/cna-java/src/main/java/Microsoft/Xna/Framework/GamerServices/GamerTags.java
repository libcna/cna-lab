package Microsoft.Xna.Framework.GamerServices;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Keeps the Java objects a title stores in {@code Gamer.Tag}.
 *
 * <p>CNA's tag is a 64-bit value it never dereferences, and a Java reference is not a stable
 * 64-bit value. The token stored natively therefore identifies an entry here, so the object a
 * title reads back is the same object it wrote, with no native pointer ever exposed.
 */
final class GamerTags {

    private static final Map<Long, Object> VALUES = new ConcurrentHashMap<>();
    private static final AtomicLong NEXT = new AtomicLong(1L);

    private GamerTags() {
    }

    static long put(Object value) {
        if (value == null) {
            return 0L;
        }
        long token = NEXT.getAndIncrement();
        VALUES.put(token, value);
        return token;
    }

    static Object get(long token) {
        return token == 0L ? null : VALUES.get(token);
    }
}
