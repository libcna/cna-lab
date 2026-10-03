package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;
import System.Collections.Generic.List.Enumerator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/** Read-only, identity-stable collection of Effect annotations. */
public final class EffectAnnotationCollection implements Iterable<EffectAnnotation> {

    private final Effect owner;
    private final Map<Integer, EffectAnnotation> byIndex = new HashMap<>();

    EffectAnnotationCollection(Effect owner, long nativeHandle) {
        this.owner = owner;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 4);
    }

    public final int getCount() {
        requireAlive();
        return NativeBindings.getEffectCollectionCount(this, 3);
    }

    public final EffectAnnotation get(int index) {
        requireAlive();
        if (index < 0 || index >= getCount()) {
            return null;
        }
        EffectAnnotation existing = byIndex.get(index);
        if (existing != null) {
            return existing;
        }
        EffectAnnotation created = new EffectAnnotation(
                owner, NativeBindings.getEffectCollectionElement(this, 3, index));
        byIndex.put(index, created);
        return created;
    }

    public final EffectAnnotation get(String name) {
        requireAlive();
        for (int index = 0; index < getCount(); index++) {
            EffectAnnotation value = get(index);
            if (value.getName().equals(name)) {
                return value;
            }
        }
        return null;
    }

    public final Enumerator<EffectAnnotation> GetEnumerator() {
        return new Enumerator<>(snapshot());
    }

    @Override
    public final Iterator<EffectAnnotation> iterator() {
        return GetEnumerator();
    }

    private java.util.List<EffectAnnotation> snapshot() {
        requireAlive();
        ArrayList<EffectAnnotation> values = new ArrayList<>(getCount());
        for (int index = 0; index < getCount(); index++) {
            values.add(get(index));
        }
        return values;
    }

    private void requireAlive() {
        owner.requireEffectAlive();
        NativeBindings.requireEffectMember(this);
    }
}
