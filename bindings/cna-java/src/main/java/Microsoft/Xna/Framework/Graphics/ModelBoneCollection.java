package Microsoft.Xna.Framework.Graphics;

import java.util.AbstractList;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Objects;

/** Read-only, identity-stable XNA Model bone collection. */
public final class ModelBoneCollection extends AbstractList<ModelBone> {

    private final ModelBone[] bones;

    ModelBoneCollection(ModelBone[] bones) {
        this.bones = Objects.requireNonNull(bones, "bones").clone();
        for (int index = 0; index < this.bones.length; index++) {
            Objects.requireNonNull(this.bones[index], "bones[" + index + "]");
        }
    }

    @Override
    public ModelBone get(int index) {
        return bones[index];
    }

    public ModelBone get(String boneName) {
        ModelBone result = TryGetValue(boneName);
        if (result == null) {
            throw new NoSuchElementException("No model bone named " + boneName);
        }
        return result;
    }

    @Override
    public int size() {
        return bones.length;
    }

    public ModelBone TryGetValue(String boneName) {
        requireName(boneName, "boneName");
        for (ModelBone bone : bones) {
            if (bone.getName() != null && bone.getName().equalsIgnoreCase(boneName)) {
                return bone;
            }
        }
        return null;
    }

    public Enumerator GetEnumerator() {
        return new Enumerator(this);
    }

    private static void requireName(String value, String parameter) {
        Objects.requireNonNull(value, parameter);
        if (value.isEmpty()) {
            throw new IllegalArgumentException(parameter + " must not be empty");
        }
    }

    /** XNA cursor plus the reviewed Java Iterator bridge. */
    public static final class Enumerator implements Iterator<ModelBone> {

        private final ModelBone[] values;
        private int position;

        public Enumerator() {
            values = new ModelBone[0];
            position = -1;
        }

        public Enumerator(Enumerator value) {
            Enumerator source = Objects.requireNonNull(value, "value");
            values = source.values.clone();
            position = source.position;
        }

        private Enumerator(ModelBoneCollection collection) {
            values = collection.bones.clone();
            position = -1;
        }

        public ModelBone getCurrent() {
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
        public ModelBone next() {
            if (!MoveNext()) {
                throw new NoSuchElementException();
            }
            return getCurrent();
        }
    }
}
