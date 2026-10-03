package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Per-material native clone of an existing XNA Effect. */
public class EffectMaterial extends Effect {

    public EffectMaterial(Effect cloneSource) {
        super(Objects.requireNonNull(cloneSource, "cloneSource"), true);
    }
}
