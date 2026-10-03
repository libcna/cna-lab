package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;
import System.Collections.Generic.List.Enumerator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Locale;
import java.util.Map;

/** Read-only, identity-stable collection of Effect parameters. */
public final class EffectParameterCollection implements Iterable<EffectParameter> {

    private final Effect owner;
    private final Map<Integer, EffectParameter> byIndex = new HashMap<>();

    EffectParameterCollection(Effect owner, long nativeHandle) {
        this.owner = owner;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 1);
    }

    public final int getCount() {
        requireAlive();
        return NativeBindings.getEffectCollectionCount(this, 0);
    }

    public final EffectParameter get(int index) {
        requireAlive();
        if (index < 0 || index >= getCount()) {
            return null;
        }
        EffectParameter existing = byIndex.get(index);
        if (existing != null) {
            return existing;
        }
        EffectParameter created = new EffectParameter(
                owner, NativeBindings.getEffectCollectionElement(this, 0, index));
        byIndex.put(index, created);
        return created;
    }

    public final EffectParameter get(String name) {
        requireAlive();
        for (int index = 0; index < getCount(); index++) {
            EffectParameter value = get(index);
            if (value.getName().equals(name)) {
                return value;
            }
        }
        return null;
    }

    public final EffectParameter GetParameterBySemantic(String semantic) {
        requireAlive();
        if (semantic == null) {
            return null;
        }
        String expected = semantic.toLowerCase(Locale.ROOT);
        for (int index = 0; index < getCount(); index++) {
            EffectParameter value = get(index);
            String actual = value.getSemantic();
            if (actual != null && actual.toLowerCase(Locale.ROOT).equals(expected)) {
                return value;
            }
        }
        return null;
    }

    public final Enumerator<EffectParameter> GetEnumerator() {
        return new Enumerator<>(snapshot());
    }

    @Override
    public final Iterator<EffectParameter> iterator() {
        return GetEnumerator();
    }

    private java.util.List<EffectParameter> snapshot() {
        requireAlive();
        ArrayList<EffectParameter> values = new ArrayList<>(getCount());
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
