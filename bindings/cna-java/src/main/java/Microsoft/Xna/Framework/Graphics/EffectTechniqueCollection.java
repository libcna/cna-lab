package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;
import System.Collections.Generic.List.Enumerator;

import java.util.ArrayList;
import java.util.Iterator;

/** Read-only, identity-stable collection of Effect techniques. */
public final class EffectTechniqueCollection implements Iterable<EffectTechnique> {

    private final Effect owner;

    EffectTechniqueCollection(Effect owner, long nativeHandle) {
        this.owner = owner;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 2);
    }

    public final int getCount() {
        requireAlive();
        return NativeBindings.getEffectCollectionCount(this, 1);
    }

    public final EffectTechnique get(int index) {
        requireAlive();
        if (index < 0 || index >= getCount()) {
            return null;
        }
        EffectTechnique cached = owner.cachedTechnique(index);
        if (cached != null) {
            return cached;
        }
        long handle = NativeBindings.getEffectCollectionElement(this, 1, index);
        return owner.resolveTechnique(index, handle);
    }

    public final EffectTechnique get(String name) {
        requireAlive();
        for (int index = 0; index < getCount(); index++) {
            EffectTechnique value = get(index);
            if (value.getName().equals(name)) {
                return value;
            }
        }
        return null;
    }

    public final Enumerator<EffectTechnique> GetEnumerator() {
        return new Enumerator<>(snapshot());
    }

    @Override
    public final Iterator<EffectTechnique> iterator() {
        return GetEnumerator();
    }

    private java.util.List<EffectTechnique> snapshot() {
        requireAlive();
        ArrayList<EffectTechnique> values = new ArrayList<>(getCount());
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
