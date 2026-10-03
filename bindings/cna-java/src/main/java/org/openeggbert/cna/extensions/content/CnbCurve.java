package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Curve;
import Microsoft.Xna.Framework.CurveContinuity;
import Microsoft.Xna.Framework.CurveKey;
import Microsoft.Xna.Framework.CurveLoopType;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.Objects;

/**
 * The {@code .cnb} curve family, which crosses straight to XNA's own {@link Curve}.
 *
 * <p><strong>No compiled-curve type, on purpose.</strong> A curve is small enough that the
 * compiled form is the runtime form laid out flat, so there is nothing to hold between reading a
 * file and having the object -- which is why this is a pair of functions rather than a class with
 * a handle. The native curve exists only for the length of one call, and is released before
 * either method returns.
 *
 * <p>XNA's {@code Curve} is implemented in Java rather than over a native handle, so the crossing
 * is a copy in both directions. The loop types and the per-key continuity carry the same numbers
 * on both sides; the tests assert that rather than assuming it, because the numbers are wire
 * format in the file and a mismatch would silently change how a curve evaluates.
 */
public final class CnbCurve {

    private CnbCurve() {
    }

    /**
     * Encodes a curve as a complete {@code .cnb} file.
     *
     * @param curve the curve to encode, keys and loop behaviour together
     * @param contentName the source content name to record
     * @return the whole file
     * @throws CnbFormatException when the format cannot represent the curve
     */
    public static byte[] encode(Curve curve, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(curve, "curve");
        Objects.requireNonNull(contentName, "contentName");
        byte[] name = CnbExtension.utf8(contentName);
        long[] handle = new long[1];
        CnbExtension.check("CnbCurve.encode", NativeCnbRoutes.curveCreate(handle));
        try {
            CnbExtension.check("CnbCurve.encode", NativeCnbRoutes
                    .curveSetPreLoop(handle[0], curve.getPreLoop().ordinal()));
            CnbExtension.check("CnbCurve.encode", NativeCnbRoutes
                    .curveSetPostLoop(handle[0], curve.getPostLoop().ordinal()));
            writeKeys(curve, handle[0]);
            long[] size = new long[1];
            int probe = NativeCnbRoutes.cnbEncodeCurve(handle[0], name, new byte[0], size);
            if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
                CnbExtension.check("CnbCurve.encode", probe);
            }
            byte[] destination = new byte[Math.toIntExact(size[0])];
            long[] written = new long[1];
            CnbExtension.check("CnbCurve.encode", NativeCnbRoutes
                    .cnbEncodeCurve(handle[0], name, destination, written));
            return CnbExtension.trim(destination, written[0]);
        } finally {
            CnbExtension.check("CnbCurve.encode", NativeCnbRoutes.curveDestroy(handle[0]));
        }
    }

    /** Decodes the document as an ordinary XNA curve. */
    static Curve decode(long document) {
        long[] handle = new long[1];
        CnbExtension.check("CnbDocument.decodeCurve",
                NativeCnbRoutes.cnbDecodeCurve(document, handle));
        try {
            Curve curve = new Curve();
            int[] loop = new int[1];
            CnbExtension.check("CnbDocument.decodeCurve",
                    NativeCnbRoutes.curveGetPreLoop(handle[0], loop));
            curve.setPreLoop(loopType(loop[0]));
            CnbExtension.check("CnbDocument.decodeCurve",
                    NativeCnbRoutes.curveGetPostLoop(handle[0], loop));
            curve.setPostLoop(loopType(loop[0]));
            readKeys(curve, handle[0]);
            return curve;
        } finally {
            CnbExtension.check("CnbDocument.decodeCurve",
                    NativeCnbRoutes.curveDestroy(handle[0]));
        }
    }

    private static void writeKeys(Curve curve, long handle) {
        long[] keys = new long[1];
        // The view retains the curve and is a handle of its own, so it is released here even
        // though the curve it mutates outlives this block.
        CnbExtension.check("CnbCurve.encode", NativeCnbRoutes.curveGetKeys(handle, keys));
        try {
            for (int index = 0; index < curve.getKeys().getCount(); index++) {
                CurveKey key = curve.getKeys().get(index);
                long[] integral = new long[1];
                float[] floating = new float[4];
                CnbExtension.check("CnbCurve.encode", NativeCnbRoutes.curveKeyInitFull(
                        key.getPosition(), key.getValue(), key.getTangentIn(),
                        key.getTangentOut(), key.getContinuity().ordinal(), integral, floating));
                CnbExtension.check("CnbCurve.encode", NativeCnbRoutes
                        .curveKeyCollectionAdd(keys[0], integral, floating));
            }
        } finally {
            CnbExtension.check("CnbCurve.encode",
                    NativeCnbRoutes.curveKeyCollectionDestroy(keys[0]));
        }
    }

    private static void readKeys(Curve curve, long handle) {
        long[] keys = new long[1];
        CnbExtension.check("CnbDocument.decodeCurve",
                NativeCnbRoutes.curveGetKeys(handle, keys));
        try {
            long[] count = new long[1];
            CnbExtension.check("CnbDocument.decodeCurve",
                    NativeCnbRoutes.curveKeyCollectionGetCount(keys[0], count));
            for (int index = 0; index < count[0]; index++) {
                long[] integral = new long[1];
                float[] floating = new float[4];
                CnbExtension.check("CnbDocument.decodeCurve", NativeCnbRoutes
                        .curveKeyCollectionGet(keys[0], index, integral, floating));
                curve.getKeys().Add(new CurveKey(floating[0], floating[1], floating[2],
                        floating[3], continuity(integral[0])));
            }
        } finally {
            CnbExtension.check("CnbDocument.decodeCurve",
                    NativeCnbRoutes.curveKeyCollectionDestroy(keys[0]));
        }
    }

    private static CurveLoopType loopType(int value) {
        CurveLoopType[] values = CurveLoopType.values();
        if (value < 0 || value >= values.length) {
            throw new CnbFormatException("the file names curve loop type " + value
                    + ", which XNA has no constant for");
        }
        return values[value];
    }

    private static CurveContinuity continuity(long value) {
        CurveContinuity[] values = CurveContinuity.values();
        if (value < 0 || value >= values.length) {
            throw new CnbFormatException("the file names curve continuity " + value
                    + ", which XNA has no constant for");
        }
        return values[(int) value];
    }
}
