package Microsoft.Xna.Framework.Graphics;

import java.util.AbstractList;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Objects;

/** Read-only public view of the unique Effects referenced by one ModelMesh. */
public final class ModelEffectCollection extends AbstractList<Effect> {

    private final ArrayList<Effect> effects = new ArrayList<>();

    ModelEffectCollection() {
    }

    @Override
    public Effect get(int index) {
        return effects.get(index);
    }

    @Override
    public int size() {
        return effects.size();
    }

    public Enumerator GetEnumerator() {
        return new Enumerator(effects);
    }

    void addEffect(Effect effect) {
        effects.add(Objects.requireNonNull(effect, "effect"));
    }

    void removeEffect(Effect effect) {
        effects.remove(effect);
    }

    /** XNA cursor plus the reviewed Java Iterator bridge. */
    public static final class Enumerator implements Iterator<Effect> {

        private final java.util.List<Effect> values;
        private int position;

        public Enumerator() {
            values = java.util.List.of();
            position = -1;
        }

        public Enumerator(Enumerator value) {
            Enumerator source = Objects.requireNonNull(value, "value");
            values = java.util.List.copyOf(source.values);
            position = source.position;
        }

        private Enumerator(java.util.List<Effect> effects) {
            values = java.util.List.copyOf(effects);
            position = -1;
        }

        public Effect getCurrent() {
            return values.get(position);
        }

        public boolean MoveNext() {
            position++;
            if (position >= values.size()) {
                position = values.size();
                return false;
            }
            return true;
        }

        public void Dispose() {
        }

        public void close() {
            Dispose();
        }

        @Override
        public boolean hasNext() {
            return position + 1 < values.size();
        }

        @Override
        public Effect next() {
            if (!MoveNext()) {
                throw new NoSuchElementException();
            }
            return getCurrent();
        }
    }
}
