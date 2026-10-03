package Microsoft.Xna.Framework;

import java.util.Objects;

/** Mutable control point retained by reference in a {@link CurveKeyCollection}. */
public class CurveKey {

    final float position;
    float internalValue;
    float tangentOut;
    float tangentIn;
    CurveContinuity continuity;

    public CurveKey(float position, float value) {
        this(position, value, 0.0f, 0.0f, CurveContinuity.Smooth);
    }

    public CurveKey(float position, float value, float tangentIn, float tangentOut) {
        this(position, value, tangentIn, tangentOut, CurveContinuity.Smooth);
    }

    public CurveKey(
            float position,
            float value,
            float tangentIn,
            float tangentOut,
            CurveContinuity continuity) {
        this.position = position;
        internalValue = value;
        this.tangentIn = tangentIn;
        this.tangentOut = tangentOut;
        this.continuity = Objects.requireNonNull(continuity, "continuity");
    }

    public final float getPosition() {
        return position;
    }

    public final float getValue() {
        return internalValue;
    }

    public final void setValue(float value) {
        internalValue = value;
    }

    public final float getTangentIn() {
        return tangentIn;
    }

    public final void setTangentIn(float value) {
        tangentIn = value;
    }

    public final float getTangentOut() {
        return tangentOut;
    }

    public final void setTangentOut(float value) {
        tangentOut = value;
    }

    public final CurveContinuity getContinuity() {
        return continuity;
    }

    public final void setContinuity(CurveContinuity value) {
        continuity = Objects.requireNonNull(value, "value");
    }

    public final CurveKey Clone() {
        return new CurveKey(position, internalValue, tangentIn, tangentOut, continuity);
    }

    public final boolean equals(CurveKey other) {
        return other != null
                && other.position == position
                && other.internalValue == internalValue
                && other.tangentIn == tangentIn
                && other.tangentOut == tangentOut
                && other.continuity == continuity;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof CurveKey other && equals(other);
    }

    @Override
    public int hashCode() {
        return FloatSemantics.hash(position)
                + FloatSemantics.hash(internalValue)
                + FloatSemantics.hash(tangentIn)
                + FloatSemantics.hash(tangentOut)
                + continuity.ordinal();
    }

    public final int CompareTo(CurveKey other) {
        float otherPosition = Objects.requireNonNull(other, "other").position;
        if (position != otherPosition) {
            return position < otherPosition ? -1 : 1;
        }
        return 0;
    }
}
