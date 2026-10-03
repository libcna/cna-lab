package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import org.openeggbert.cna.internal.NativeBindings;

/** Parent-owned immutable Effect annotation view. */
public final class EffectAnnotation {

    private final Effect owner;

    EffectAnnotation(Effect owner, long nativeHandle) {
        this.owner = owner;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 8);
    }

    public final String getName() {
        requireAlive();
        return NativeBindings.getEffectString(this, 4);
    }

    public final String getSemantic() {
        requireAlive();
        return NativeBindings.getEffectString(this, 5);
    }

    public final int getRowCount() {
        return info()[0];
    }

    public final int getColumnCount() {
        return info()[1];
    }

    public final EffectParameterClass getParameterClass() {
        return parameterClass(info()[2]);
    }

    public final EffectParameterType getParameterType() {
        return parameterType(info()[3]);
    }

    public final boolean GetValueBoolean() {
        requireAlive();
        return NativeBindings.getEffectInts(this, 0, 1)[0] != 0;
    }

    public final int GetValueInt32() {
        requireAlive();
        return NativeBindings.getEffectInts(this, 1, 1)[0];
    }

    public final float GetValueSingle() {
        requireAlive();
        return NativeBindings.getEffectFloats(this, 2, 1, 1)[0];
    }

    public final String GetValueString() {
        requireAlive();
        return NativeBindings.getEffectString(this, 7);
    }

    public final Vector2 GetValueVector2() {
        requireAlive();
        return EffectParameter.vector2(NativeBindings.getEffectFloats(this, 6, 1, 2), 0);
    }

    public final Vector3 GetValueVector3() {
        requireAlive();
        return EffectParameter.vector3(NativeBindings.getEffectFloats(this, 7, 1, 3), 0);
    }

    public final Vector4 GetValueVector4() {
        requireAlive();
        return EffectParameter.vector4(NativeBindings.getEffectFloats(this, 8, 1, 4), 0);
    }

    public final Matrix GetValueMatrix() {
        requireAlive();
        return EffectParameter.matrix(NativeBindings.getEffectFloats(this, 3, 1, 16), 0);
    }

    private int[] info() {
        requireAlive();
        return NativeBindings.getEffectInfo(this, 1);
    }

    private void requireAlive() {
        owner.requireEffectAlive();
        NativeBindings.requireEffectMember(this);
    }

    static EffectParameterClass parameterClass(int value) {
        EffectParameterClass[] values = EffectParameterClass.values();
        if (value < 0 || value >= values.length) {
            throw new IllegalStateException("Unknown EffectParameterClass " + value);
        }
        return values[value];
    }

    static EffectParameterType parameterType(int value) {
        EffectParameterType[] values = EffectParameterType.values();
        if (value < 0 || value >= values.length) {
            throw new IllegalStateException("Unknown EffectParameterType " + value);
        }
        return values[value];
    }
}
