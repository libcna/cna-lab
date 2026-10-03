package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;

import java.util.Arrays;
import java.util.Objects;

/** Fixed-size device texture collection for one shader stage. */
public final class TextureCollection {

    private final GraphicsDevice graphicsDevice;
    private final int shaderStage;
    private final Texture[] textures;

    TextureCollection(GraphicsDevice graphicsDevice, int shaderStage, int slotCount) {
        this.graphicsDevice = Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        this.shaderStage = shaderStage;
        this.textures = new Texture[slotCount];
    }

    public Texture get(int index) {
        validateIndex(index);
        graphicsDevice.ensureOpen();
        Texture result = NativeBindings.getGraphicsDeviceTexture(
                graphicsDevice, shaderStage, index, textures[index]);
        textures[index] = result;
        return result;
    }

    public void set(int index, Texture value) {
        validateIndex(index);
        graphicsDevice.ensureOpen();
        if (value != null) {
            value.ensureNotDisposed();
            if (value.getGraphicsDevice() != graphicsDevice) {
                throw new IllegalArgumentException(
                        "Texture was created for a different GraphicsDevice");
            }
        }
        if (textures[index] == value) {
            return;
        }
        NativeBindings.setGraphicsDeviceTexture(
                graphicsDevice, shaderStage, index, value);
        textures[index] = value;
    }

    final void invalidate() {
        Arrays.fill(textures, null);
    }

    private void validateIndex(int index) {
        if (index < 0 || index >= textures.length) {
            throw new IndexOutOfBoundsException(
                    "Texture index " + index + " is outside 0.." + (textures.length - 1));
        }
    }
}
