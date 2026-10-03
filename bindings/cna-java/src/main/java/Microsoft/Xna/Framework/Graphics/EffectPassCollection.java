package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;
import System.Collections.Generic.List.Enumerator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/** Read-only, identity-stable collection of Effect passes. */
public final class EffectPassCollection implements Iterable<EffectPass> {

    private final Effect owner;
    private final EffectTechnique technique;
    private final Map<Integer, EffectPass> byIndex = new HashMap<>();

    EffectPassCollection(Effect owner, EffectTechnique technique, long nativeHandle) {
        this.owner = owner;
        this.technique = technique;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 3);
    }

    public final int getCount() {
        requireAlive();
        return NativeBindings.getEffectCollectionCount(this, 2);
    }

    public final EffectPass get(int index) {
        requireAlive();
        if (index < 0 || index >= getCount()) {
            return null;
        }
        EffectPass existing = byIndex.get(index);
        if (existing != null) {
            return existing;
        }
        EffectPass created = new EffectPass(
                owner, NativeBindings.getEffectCollectionElement(this, 2, index));
        byIndex.put(index, created);
        return created;
    }

    public final EffectPass get(String name) {
        requireAlive();
        for (int index = 0; index < getCount(); index++) {
            EffectPass value = get(index);
            if (value.getName().equals(name)) {
                return value;
            }
        }
        return null;
    }

    public final Enumerator<EffectPass> GetEnumerator() {
        return new Enumerator<>(snapshot());
    }

    @Override
    public final Iterator<EffectPass> iterator() {
        return GetEnumerator();
    }

    private java.util.List<EffectPass> snapshot() {
        requireAlive();
        ArrayList<EffectPass> values = new ArrayList<>(getCount());
        for (int index = 0; index < getCount(); index++) {
            values.add(get(index));
        }
        return values;
    }

    private void requireAlive() {
        technique.requireAlive();
        NativeBindings.requireEffectMember(this);
    }
}
