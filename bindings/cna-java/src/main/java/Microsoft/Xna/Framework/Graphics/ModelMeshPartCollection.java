package Microsoft.Xna.Framework.Graphics;

import java.util.AbstractList;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Objects;

/** Read-only, identity-stable XNA Model mesh-part collection. */
public final class ModelMeshPartCollection extends AbstractList<ModelMeshPart> {

    private final ModelMeshPart[] parts;

    ModelMeshPartCollection(ModelMeshPart[] parts) {
        this.parts = Objects.requireNonNull(parts, "parts").clone();
        for (int index = 0; index < this.parts.length; index++) {
            Objects.requireNonNull(this.parts[index], "parts[" + index + "]");
        }
    }

    @Override
    public ModelMeshPart get(int index) {
        return parts[index];
    }

    @Override
    public int size() {
        return parts.length;
    }

    public Enumerator GetEnumerator() {
        return new Enumerator(this);
    }

    /** XNA cursor plus the reviewed Java Iterator bridge. */
    public static final class Enumerator implements Iterator<ModelMeshPart> {

        private final ModelMeshPart[] values;
        private int position;

        public Enumerator() {
            values = new ModelMeshPart[0];
            position = -1;
        }

        public Enumerator(Enumerator value) {
            Enumerator source = Objects.requireNonNull(value, "value");
            values = source.values.clone();
            position = source.position;
        }

        private Enumerator(ModelMeshPartCollection collection) {
            values = collection.parts.clone();
            position = -1;
        }

        public ModelMeshPart getCurrent() {
            return values[position];
        }

        public boolean MoveNext() {
            position++;
            if (position >= values.length) {
                position = values.length;
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
            return position + 1 < values.length;
        }

        @Override
        public ModelMeshPart next() {
            if (!MoveNext()) {
                throw new NoSuchElementException();
            }
            return getCurrent();
        }
    }
}
