package Microsoft.Xna.Framework;

import java.util.Objects;

/** XNA curve evaluation and tangent behavior implemented entirely in Java. */
public class Curve {

    private CurveLoopType preLoop = CurveLoopType.Constant;
    private CurveLoopType postLoop = CurveLoopType.Constant;
    private CurveKeyCollection keys = new CurveKeyCollection();

    public Curve() {
    }

    public final CurveLoopType getPreLoop() {
        return preLoop;
    }

    public final void setPreLoop(CurveLoopType value) {
        preLoop = Objects.requireNonNull(value, "value");
    }

    public final CurveLoopType getPostLoop() {
        return postLoop;
    }

    public final void setPostLoop(CurveLoopType value) {
        postLoop = Objects.requireNonNull(value, "value");
    }

    public final CurveKeyCollection getKeys() {
        return keys;
    }

    public final boolean getIsConstant() {
        return keys.getCount() <= 1;
    }

    public final Curve Clone() {
        Curve result = new Curve();
        result.preLoop = preLoop;
        result.postLoop = postLoop;
        result.keys = keys.Clone();
        return result;
    }

    public final void ComputeTangent(int keyIndex, CurveTangent tangentType) {
        ComputeTangent(keyIndex, tangentType, tangentType);
    }

    public final void ComputeTangent(
            int keyIndex,
            CurveTangent tangentInType,
            CurveTangent tangentOutType) {
        if (keys.getCount() <= keyIndex || keyIndex < 0) {
            throw new IllegalArgumentException("keyIndex");
        }
        Objects.requireNonNull(tangentInType, "tangentInType");
        Objects.requireNonNull(tangentOutType, "tangentOutType");
        CurveKey key = keys.get(keyIndex);
        float previousPosition = key.position;
        float currentPosition = key.position;
        float nextPosition = key.position;
        float previousValue = key.internalValue;
        float currentValue = key.internalValue;
        float nextValue = key.internalValue;
        if (keyIndex > 0) {
            previousPosition = keys.get(keyIndex - 1).position;
            previousValue = keys.get(keyIndex - 1).internalValue;
        }
        if (keyIndex + 1 < keys.getCount()) {
            nextPosition = keys.get(keyIndex + 1).position;
            nextValue = keys.get(keyIndex + 1).internalValue;
        }

        switch (tangentInType) {
            case Smooth -> {
                float positionSpan = nextPosition - previousPosition;
                float valueSpan = nextValue - previousValue;
                key.tangentIn = Math.abs(valueSpan) < 1.1920929E-7f
                        ? 0.0f
                        : valueSpan * Math.abs(previousPosition - currentPosition) / positionSpan;
            }
            case Linear -> key.tangentIn = currentValue - previousValue;
            default -> key.tangentIn = 0.0f;
        }

        switch (tangentOutType) {
            case Smooth -> {
                float positionSpan = nextPosition - previousPosition;
                float valueSpan = nextValue - previousValue;
                key.tangentOut = Math.abs(valueSpan) < 1.1920929E-7f
                        ? 0.0f
                        : valueSpan * Math.abs(nextPosition - currentPosition) / positionSpan;
            }
            case Linear -> key.tangentOut = nextValue - currentValue;
            default -> key.tangentOut = 0.0f;
        }
    }

    public final void ComputeTangents(CurveTangent tangentType) {
        ComputeTangents(tangentType, tangentType);
    }

    public final void ComputeTangents(CurveTangent tangentInType, CurveTangent tangentOutType) {
        for (int index = 0; index < keys.getCount(); index++) {
            ComputeTangent(index, tangentInType, tangentOutType);
        }
    }

    public final float Evaluate(float position) {
        if (keys.getCount() == 0) {
            return 0.0f;
        }
        if (keys.getCount() == 1) {
            return keys.get(0).internalValue;
        }
        CurveKey first = keys.get(0);
        CurveKey last = keys.get(keys.getCount() - 1);
        float valuePosition = position;
        float offset = 0.0f;
        if (valuePosition < first.position) {
            if (preLoop == CurveLoopType.Constant) {
                return first.internalValue;
            }
            if (preLoop == CurveLoopType.Linear) {
                return first.internalValue - first.tangentIn * (first.position - valuePosition);
            }
            ensureCache();
            float cycle = calculateCycle(valuePosition);
            float positionOffset = valuePosition - (first.position + cycle * keys.timeRange);
            if (preLoop == CurveLoopType.Cycle) {
                valuePosition = first.position + positionOffset;
            } else if (preLoop == CurveLoopType.CycleOffset) {
                valuePosition = first.position + positionOffset;
                offset = (last.internalValue - first.internalValue) * cycle;
            } else {
                valuePosition = (((int)cycle & 1) != 0)
                        ? last.position - positionOffset
                        : first.position + positionOffset;
            }
        } else if (last.position < valuePosition) {
            if (postLoop == CurveLoopType.Constant) {
                return last.internalValue;
            }
            if (postLoop == CurveLoopType.Linear) {
                return last.internalValue - last.tangentOut * (last.position - valuePosition);
            }
            ensureCache();
            float cycle = calculateCycle(valuePosition);
            float positionOffset = valuePosition - (first.position + cycle * keys.timeRange);
            if (postLoop == CurveLoopType.Cycle) {
                valuePosition = first.position + positionOffset;
            } else if (postLoop == CurveLoopType.CycleOffset) {
                valuePosition = first.position + positionOffset;
                offset = (last.internalValue - first.internalValue) * cycle;
            } else {
                valuePosition = (((int)cycle & 1) != 0)
                        ? last.position - positionOffset
                        : first.position + positionOffset;
            }
        }
        Segment segment = findSegment(valuePosition);
        return offset + hermite(segment.first, segment.second, segment.amount);
    }

    private void ensureCache() {
        if (!keys.cacheAvailable) {
            keys.computeCacheValues();
        }
    }

    private float calculateCycle(float position) {
        float cycle = (position - keys.get(0).position) * keys.invTimeRange;
        if (cycle < 0.0f) {
            cycle -= 1.0f;
        }
        return (int)cycle;
    }

    private Segment findSegment(float position) {
        float amount = position;
        CurveKey first = keys.get(0);
        CurveKey second = null;
        for (int index = 1; index < keys.getCount(); index++) {
            second = keys.get(index);
            if (second.position >= position) {
                double firstPosition = first.position;
                double secondPosition = second.position;
                double requestedPosition = position;
                double span = secondPosition - firstPosition;
                amount = 0.0f;
                if (span > 1.0E-10) {
                    amount = (float)((requestedPosition - firstPosition) / span);
                }
                break;
            }
            first = second;
        }
        return new Segment(first, second, amount);
    }

    private static float hermite(CurveKey first, CurveKey second, float amount) {
        if (first.continuity == CurveContinuity.Step) {
            return !(amount < 1.0f) ? second.internalValue : first.internalValue;
        }
        float squared = amount * amount;
        float cubed = squared * amount;
        return first.internalValue * (2.0f * cubed - 3.0f * squared + 1.0f)
                + second.internalValue * (-2.0f * cubed + 3.0f * squared)
                + first.tangentOut * (cubed - 2.0f * squared + amount)
                + second.tangentIn * (cubed - squared);
    }

    private record Segment(CurveKey first, CurveKey second, float amount) {
    }
}
