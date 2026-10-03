package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Parent-owned mutable Effect parameter view with strongly typed value families. */
public final class EffectParameter {

    private final Effect owner;
    private EffectParameterCollection elements;
    private EffectParameterCollection structureMembers;
    private EffectAnnotationCollection annotations;

    EffectParameter(Effect owner, long nativeHandle) {
        this.owner = owner;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 5);
    }

    public final String getName() { requireAlive(); return NativeBindings.getEffectString(this, 2); }
    public final String getSemantic() { requireAlive(); return NativeBindings.getEffectString(this, 3); }
    public final int getRowCount() { return info()[0]; }
    public final int getColumnCount() { return info()[1]; }
    public final EffectParameterClass getParameterClass() { return EffectAnnotation.parameterClass(info()[2]); }
    public final EffectParameterType getParameterType() { return EffectAnnotation.parameterType(info()[3]); }

    public final EffectParameterCollection getElements() {
        requireAlive();
        if (elements == null) {
            elements = new EffectParameterCollection(
                    owner, NativeBindings.getEffectMemberCollection(this, 6));
        }
        return elements;
    }

    public final EffectParameterCollection getStructureMembers() {
        requireAlive();
        if (structureMembers == null) {
            structureMembers = new EffectParameterCollection(
                    owner, NativeBindings.getEffectMemberCollection(this, 7));
        }
        return structureMembers;
    }

    public final EffectAnnotationCollection getAnnotations() {
        requireAlive();
        if (annotations == null) {
            annotations = new EffectAnnotationCollection(
                    owner, NativeBindings.getEffectMemberCollection(this, 8));
        }
        return annotations;
    }

    public final boolean GetValueBoolean() {
        requireAlive();
        return NativeBindings.getEffectIntValue(this, 0) != 0;
    }
    public final int GetValueInt32() {
        requireAlive();
        return NativeBindings.getEffectIntValue(this, 1);
    }
    public final float GetValueSingle() { return scalarFloats(2, 1)[0]; }
    public final String GetValueString() { requireAlive(); return NativeBindings.getEffectString(this, 6); }
    public final Matrix GetValueMatrix() { return matrix(scalarFloats(3, 16), 0); }
    public final Matrix GetValueMatrixTranspose() { return matrix(scalarFloats(4, 16), 0); }
    public final Quaternion GetValueQuaternion() {
        float[] values = scalarFloats(5, 4);
        return new Quaternion(values[0], values[1], values[2], values[3]);
    }
    public final Vector2 GetValueVector2() { return vector2(scalarFloats(6, 2), 0); }
    public final Vector3 GetValueVector3() { return vector3(scalarFloats(7, 3), 0); }
    public final Vector4 GetValueVector4() { return vector4(scalarFloats(8, 4), 0); }

    public final boolean[] GetValueBooleanArray(int count) {
        int[] values = NativeBindings.getEffectInts(this, 0, requireCount(count));
        boolean[] result = new boolean[values.length];
        for (int index = 0; index < values.length; index++) result[index] = values[index] != 0;
        return result;
    }

    public final int[] GetValueInt32Array(int count) {
        requireAlive();
        return NativeBindings.getEffectInts(this, 1, requireCount(count));
    }

    public final float[] GetValueSingleArray(int count) {
        requireAlive();
        return NativeBindings.getEffectFloats(this, 2, requireCount(count), 1);
    }

    public final Matrix[] GetValueMatrixArray(int count) { return matrices(3, count); }
    public final Matrix[] GetValueMatrixTransposeArray(int count) { return matrices(4, count); }

    public final Quaternion[] GetValueQuaternionArray(int count) {
        requireAlive();
        float[] values = NativeBindings.getEffectFloats(this, 5, requireCount(count), 4);
        Quaternion[] result = new Quaternion[values.length / 4];
        for (int index = 0; index < result.length; index++) {
            int offset = index * 4;
            result[index] = new Quaternion(
                    values[offset], values[offset + 1], values[offset + 2], values[offset + 3]);
        }
        return result;
    }

    public final Vector2[] GetValueVector2Array(int count) {
        requireAlive();
        float[] values = NativeBindings.getEffectFloats(this, 6, requireCount(count), 2);
        Vector2[] result = new Vector2[values.length / 2];
        for (int index = 0; index < result.length; index++) result[index] = vector2(values, index * 2);
        return result;
    }

    public final Vector3[] GetValueVector3Array(int count) {
        requireAlive();
        float[] values = NativeBindings.getEffectFloats(this, 7, requireCount(count), 3);
        Vector3[] result = new Vector3[values.length / 3];
        for (int index = 0; index < result.length; index++) result[index] = vector3(values, index * 3);
        return result;
    }

    public final Vector4[] GetValueVector4Array(int count) {
        requireAlive();
        float[] values = NativeBindings.getEffectFloats(this, 8, requireCount(count), 4);
        Vector4[] result = new Vector4[values.length / 4];
        for (int index = 0; index < result.length; index++) result[index] = vector4(values, index * 4);
        return result;
    }

    public final Texture2D GetValueTexture2D() {
        requireAlive();
        return NativeBindings.getEffectTexture2D(owner, this);
    }

    public final Texture3D GetValueTexture3D() {
        requireAlive();
        return NativeBindings.getEffectTexture3D(owner, this);
    }

    public final TextureCube GetValueTextureCube() {
        requireAlive();
        return NativeBindings.getEffectTextureCube(owner, this);
    }

    public final void SetValue(boolean value) { setIntValue(0, value ? 1 : 0); }
    public final void SetValue(int value) { setIntValue(1, value); }
    public final void SetValue(float value) { setFloatValue(2, new float[] {value}); }

    public final void SetValue(String value) {
        requireAlive();
        NativeBindings.setEffectString(this, Objects.requireNonNull(value, "value"));
    }

    public final void SetValue(Matrix value) { setFloatValue(3, matrixValues(value)); }
    public final void SetValueTranspose(Matrix value) { setFloatValue(4, matrixValues(value)); }
    public final void SetValue(Quaternion value) { setFloatValue(5, quaternionValues(value)); }
    public final void SetValue(Vector2 value) { setFloatValue(6, vector2Values(value)); }
    public final void SetValue(Vector3 value) { setFloatValue(7, vector3Values(value)); }
    public final void SetValue(Vector4 value) { setFloatValue(8, vector4Values(value)); }

    public final void SetValue(boolean[] value) {
        Objects.requireNonNull(value, "value");
        int[] values = new int[value.length];
        for (int index = 0; index < value.length; index++) values[index] = value[index] ? 1 : 0;
        setInts(0, values);
    }

    public final void SetValue(int[] value) {
        setInts(1, Objects.requireNonNull(value, "value").clone());
    }

    public final void SetValue(float[] value) {
        float[] values = Objects.requireNonNull(value, "value").clone();
        setFloats(2, values, values.length);
    }

    public final void SetValue(Matrix[] value) { setMatrices(3, value); }
    public final void SetValueTranspose(Matrix[] value) { setMatrices(4, value); }
    public final void SetValue(Quaternion[] value) { setQuaternionArray(value); }
    public final void SetValue(Vector2[] value) { setVector2Array(value); }
    public final void SetValue(Vector3[] value) { setVector3Array(value); }
    public final void SetValue(Vector4[] value) { setVector4Array(value); }

    public final void SetValue(Texture value) {
        requireAlive();
        NativeBindings.setEffectTexture(owner, this, value);
    }

    static Vector2 vector2(float[] values, int offset) {
        return new Vector2(values[offset], values[offset + 1]);
    }

    static Vector3 vector3(float[] values, int offset) {
        return new Vector3(values[offset], values[offset + 1], values[offset + 2]);
    }

    static Vector4 vector4(float[] values, int offset) {
        return new Vector4(values[offset], values[offset + 1], values[offset + 2], values[offset + 3]);
    }

    static Matrix matrix(float[] values, int offset) {
        return new Matrix(
                values[offset], values[offset + 1], values[offset + 2], values[offset + 3],
                values[offset + 4], values[offset + 5], values[offset + 6], values[offset + 7],
                values[offset + 8], values[offset + 9], values[offset + 10], values[offset + 11],
                values[offset + 12], values[offset + 13], values[offset + 14], values[offset + 15]);
    }

    private int[] info() {
        requireAlive();
        return NativeBindings.getEffectInfo(this, 0);
    }

    private float[] scalarFloats(int valueType, int width) {
        requireAlive();
        return NativeBindings.getEffectFloatValue(this, valueType, width);
    }

    private void setIntValue(int valueType, int value) {
        requireAlive();
        NativeBindings.setEffectIntValue(this, valueType, value);
    }

    private void setFloatValue(int valueType, float[] value) {
        requireAlive();
        NativeBindings.setEffectFloatValue(this, valueType, value);
    }

    private Matrix[] matrices(int valueType, int count) {
        requireAlive();
        float[] values = NativeBindings.getEffectFloats(this, valueType, requireCount(count), 16);
        Matrix[] result = new Matrix[values.length / 16];
        for (int index = 0; index < result.length; index++) result[index] = matrix(values, index * 16);
        return result;
    }

    private void setInts(int valueType, int[] values) {
        requireAlive();
        requireNonEmpty(values.length);
        NativeBindings.setEffectInts(this, valueType, values);
    }

    private void setFloats(int valueType, float[] values, int count) {
        requireAlive();
        requireNonEmpty(count);
        NativeBindings.setEffectFloats(this, valueType, values, count);
    }

    private void setMatrices(int valueType, Matrix[] source) {
        Objects.requireNonNull(source, "value");
        float[] values = new float[Math.multiplyExact(source.length, 16)];
        for (int index = 0; index < source.length; index++) {
            System.arraycopy(matrixValues(source[index]), 0, values, index * 16, 16);
        }
        setFloats(valueType, values, source.length);
    }

    private void setQuaternionArray(Quaternion[] source) {
        Objects.requireNonNull(source, "value");
        float[] values = new float[Math.multiplyExact(source.length, 4)];
        for (int index = 0; index < source.length; index++) {
            System.arraycopy(quaternionValues(source[index]), 0, values, index * 4, 4);
        }
        setFloats(5, values, source.length);
    }

    private void setVector2Array(Vector2[] source) {
        Objects.requireNonNull(source, "value");
        float[] values = new float[Math.multiplyExact(source.length, 2)];
        for (int index = 0; index < source.length; index++) {
            System.arraycopy(vector2Values(source[index]), 0, values, index * 2, 2);
        }
        setFloats(6, values, source.length);
    }

    private void setVector3Array(Vector3[] source) {
        Objects.requireNonNull(source, "value");
        float[] values = new float[Math.multiplyExact(source.length, 3)];
        for (int index = 0; index < source.length; index++) {
            System.arraycopy(vector3Values(source[index]), 0, values, index * 3, 3);
        }
        setFloats(7, values, source.length);
    }

    private void setVector4Array(Vector4[] source) {
        Objects.requireNonNull(source, "value");
        float[] values = new float[Math.multiplyExact(source.length, 4)];
        for (int index = 0; index < source.length; index++) {
            System.arraycopy(vector4Values(source[index]), 0, values, index * 4, 4);
        }
        setFloats(8, values, source.length);
    }

    private static float[] matrixValues(Matrix value) {
        Matrix v = new Matrix(Objects.requireNonNull(value, "value"));
        return new float[] {
                v.M11, v.M12, v.M13, v.M14, v.M21, v.M22, v.M23, v.M24,
                v.M31, v.M32, v.M33, v.M34, v.M41, v.M42, v.M43, v.M44};
    }

    private static float[] quaternionValues(Quaternion value) {
        Quaternion v = new Quaternion(Objects.requireNonNull(value, "value"));
        return new float[] {v.X, v.Y, v.Z, v.W};
    }

    private static float[] vector2Values(Vector2 value) {
        Vector2 v = new Vector2(Objects.requireNonNull(value, "value"));
        return new float[] {v.X, v.Y};
    }

    private static float[] vector3Values(Vector3 value) {
        Vector3 v = new Vector3(Objects.requireNonNull(value, "value"));
        return new float[] {v.X, v.Y, v.Z};
    }

    private static float[] vector4Values(Vector4 value) {
        Vector4 v = new Vector4(Objects.requireNonNull(value, "value"));
        return new float[] {v.X, v.Y, v.Z, v.W};
    }

    private static int requireCount(int count) {
        if (count < 0) throw new IllegalArgumentException("Effect value count must not be negative");
        return count;
    }

    private static void requireNonEmpty(int count) {
        if (count == 0) throw new IllegalArgumentException("Effect value arrays must not be empty");
    }

    private void requireAlive() {
        owner.requireEffectAlive();
        NativeBindings.requireEffectMember(this);
    }
}
