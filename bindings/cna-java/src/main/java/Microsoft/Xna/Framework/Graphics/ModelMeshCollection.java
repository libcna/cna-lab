package Microsoft.Xna.Framework.Graphics;

import java.util.AbstractList;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Objects;

/** Read-only, identity-stable XNA Model mesh collection. */
public final class ModelMeshCollection extends AbstractList<ModelMesh> {

    private final ModelMesh[] meshes;

    ModelMeshCollection(ModelMesh[] meshes) {
        this.meshes = Objects.requireNonNull(meshes, "meshes").clone();
        for (int index = 0; index < this.meshes.length; index++) {
            Objects.requireNonNull(this.meshes[index], "meshes[" + index + "]");
        }
    }

    @Override
    public ModelMesh get(int index) {
        return meshes[index];
    }

    public ModelMesh get(String meshName) {
        ModelMesh result = TryGetValue(meshName);
        if (result == null) {
            throw new NoSuchElementException("No model mesh named " + meshName);
        }
        return result;
    }

    @Override
    public int size() {
        return meshes.length;
    }

    public ModelMesh TryGetValue(String meshName) {
        requireName(meshName, "meshName");
        for (ModelMesh mesh : meshes) {
            if (mesh.getName() != null && mesh.getName().equalsIgnoreCase(meshName)) {
                return mesh;
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
    public static final class Enumerator implements Iterator<ModelMesh> {

        private final ModelMesh[] values;
        private int position;

        public Enumerator() {
            values = new ModelMesh[0];
            position = -1;
        }

        public Enumerator(Enumerator value) {
            Enumerator source = Objects.requireNonNull(value, "value");
            values = source.values.clone();
            position = source.position;
        }

        private Enumerator(ModelMeshCollection collection) {
            values = collection.meshes.clone();
            position = -1;
        }

        public ModelMesh getCurrent() {
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
        public ModelMesh next() {
            if (!MoveNext()) {
                throw new NoSuchElementException();
            }
            return getCurrent();
        }
    }
}
