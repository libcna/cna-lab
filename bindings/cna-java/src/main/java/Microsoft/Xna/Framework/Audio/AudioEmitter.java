package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.Vector3;

import java.util.Objects;

/** Emitter transform and Doppler scaling used for positional sound. */
public class AudioEmitter {
    private float dopplerScale = 1.0f;
    private Vector3 forward = Vector3.getForward();
    private Vector3 position = Vector3.getZero();
    private Vector3 up = Vector3.getUp();
    private Vector3 velocity = Vector3.getZero();

    public AudioEmitter() {
    }

    public final float getDopplerScale() { return dopplerScale; }
    public final void setDopplerScale(float value) {
        if (value < 0.0f) throw new IllegalArgumentException("DopplerScale must not be negative");
        dopplerScale = value;
    }
    public final Vector3 getForward() { return new Vector3(forward); }
    public final void setForward(Vector3 value) { forward = copy(value); }
    public final Vector3 getPosition() { return new Vector3(position); }
    public final void setPosition(Vector3 value) { position = copy(value); }
    public final Vector3 getUp() { return new Vector3(up); }
    public final void setUp(Vector3 value) { up = copy(value); }
    public final Vector3 getVelocity() { return new Vector3(velocity); }
    public final void setVelocity(Vector3 value) { velocity = copy(value); }

    final float[] nativeValues() {
        return new float[] {dopplerScale, forward.X, forward.Y, forward.Z,
                position.X, position.Y, position.Z, up.X, up.Y, up.Z,
                velocity.X, velocity.Y, velocity.Z};
    }

    private static Vector3 copy(Vector3 value) {
        return new Vector3(Objects.requireNonNull(value, "value"));
    }
}
