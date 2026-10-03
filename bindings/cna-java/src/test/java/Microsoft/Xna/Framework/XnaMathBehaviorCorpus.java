package Microsoft.Xna.Framework;

import Microsoft.Xna.Framework.Graphics.Viewport;
import Microsoft.Xna.Framework.Graphics.PackedVector.Alpha8;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgr565;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgra4444;
import Microsoft.Xna.Framework.Graphics.PackedVector.Bgra5551;
import Microsoft.Xna.Framework.Graphics.PackedVector.Byte4;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfSingle;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfVector2;
import Microsoft.Xna.Framework.Graphics.PackedVector.HalfVector4;
import Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedByte2;
import Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedByte4;
import Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedShort2;
import Microsoft.Xna.Framework.Graphics.PackedVector.NormalizedShort4;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rg32;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rgba1010102;
import Microsoft.Xna.Framework.Graphics.PackedVector.Rgba64;
import Microsoft.Xna.Framework.Graphics.PackedVector.Short2;
import Microsoft.Xna.Framework.Graphics.PackedVector.Short4;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/** Engine-neutral inputs mirrored from the shared XNA behavior corpus. */
final class XnaMathBehaviorCorpus {

    private XnaMathBehaviorCorpus() {
    }

    static List<String> capture() {
        List<String> observations = new ArrayList<>();

        add(observations, "v2.normalize.zero", Vector2.Normalize(Vector2.getZero()));
        add(observations, "v3.normalize.zero", Vector3.Normalize(Vector3.getZero()));
        add(observations, "v4.normalize.zero", Vector4.Normalize(Vector4.getZero()));
        add(observations, "vector.divide.scalar", new Vector4(
                Vector2.Divide(new Vector2(3.0f), 7.0f).X,
                Vector3.Divide(new Vector3(7.0f), 3.0f).X,
                Vector4.Divide(new Vector4(12345.67f), 3.0f).X,
                Matrix.Divide(Matrix.getIdentity(), 3.0f).M11));
        add(observations, "q.normalize.zero", Quaternion.Normalize(new Quaternion()));
        add(observations, "q.inverse.zero", Quaternion.Inverse(new Quaternion()));

        Quaternion yaw = Quaternion.CreateFromAxisAngle(Vector3.getUp(), 0.7f);
        Quaternion pitch = Quaternion.CreateFromAxisAngle(Vector3.getRight(), -0.4f);
        add(observations, "q.multiply", Quaternion.Multiply(yaw, pitch));
        add(observations, "q.multiply.grouped", Quaternion.Multiply(
                new Quaternion(45889.05859375f, -42412.4453125f, 96034.96875f, -76386.84375f),
                new Quaternion(-16375.435546875f, 51428.1875f, -69603.09375f, -2207.3798828125f)));
        add(observations, "q.concatenate", Quaternion.Concatenate(yaw, pitch));
        add(observations, "v3.qtransform", Vector3.Transform(
                new Vector3(1.25f, -2.5f, 3.75f), Quaternion.Multiply(yaw, pitch)));

        Matrix matrix = Matrix.Multiply(
                Matrix.Multiply(Matrix.CreateScale(2.0f, 3.0f, 4.0f), Matrix.CreateRotationY(0.25f)),
                Matrix.CreateTranslation(5.0f, 6.0f, 7.0f));
        add(observations, "v2.transform", Vector2.Transform(new Vector2(1.5f, -2.0f), matrix));
        add(observations, "v3.transform", Vector3.Transform(new Vector3(1.5f, -2.0f, 0.25f), matrix));
        add(observations, "v4.transform", Vector4.Transform(new Vector4(1.5f, -2.0f, 0.25f, 1.0f), matrix));
        add(observations, "matrix.inverse.product", Matrix.Multiply(matrix, Matrix.Invert(matrix)));
        add(observations, "matrix.inverse.singular", Matrix.Invert(new Matrix()));

        Viewport viewport = new Viewport(11, 13, 640, 360);
        viewport.setMinDepth(0.2f);
        viewport.setMaxDepth(0.9f);
        Matrix viewportWorld = Matrix.Multiply(
                Matrix.Multiply(Matrix.CreateScale(1.5f, 0.75f, 2.0f), Matrix.CreateRotationY(0.31f)),
                Matrix.CreateTranslation(2.0f, -1.0f, 0.5f));
        Matrix viewportView = Matrix.CreateLookAt(
                new Vector3(4.0f, 3.0f, 8.0f), Vector3.getZero(), Vector3.getUp());
        Matrix viewportProjection = Matrix.CreatePerspectiveFieldOfView(
                0.9f, 16.0f / 9.0f, 0.1f, 100.0f);
        Vector3 viewportProjected = viewport.Project(
                new Vector3(0.25f, -0.5f, 1.25f),
                viewportProjection, viewportView, viewportWorld);
        add(observations, "viewport.project", viewportProjected);
        add(observations, "viewport.unproject", viewport.Unproject(
                viewportProjected, viewportProjection, viewportView, viewportWorld));
        add(observations, "viewport.unproject.singular", viewport.Unproject(
                new Vector3(100.0f, 50.0f, 0.5f),
                Matrix.getIdentity(), Matrix.getIdentity(), new Matrix()));

        Color packed = new Color(0.5f, Float.NaN, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY);
        observations.add("color.pack=" + hex(packed.getPackedValue()));
        observations.add("color.lerp=" + hex(Color.Lerp(
                new Color(0, 0, 0, 0), new Color(255, 255, 255, 255), 0.5f).getPackedValue()));
        observations.add("color.nonpremultiplied.extreme=" + hex(Color.FromNonPremultiplied(
                Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE).getPackedValue()));

        Plane transformedPlane = Plane.Transform(
                new Plane(Vector3.getUp(), -2.0f), Matrix.CreateTranslation(0.0f, 5.0f, 0.0f));
        add(observations, "plane.transform", new Vector4(transformedPlane.Normal, transformedPlane.D));

        BoundingBox box = new BoundingBox(new Vector3(-1.0f), new Vector3(1.0f));
        observations.add("box.contains.edge=" + box.Contains(new Vector3(1.0f, 0.0f, 0.0f)).ordinal());
        BoundingBox nanBox = new BoundingBox(
                new Vector3(Float.NaN, -1.0f, -1.0f),
                new Vector3(Float.NaN, 1.0f, 1.0f));
        observations.add("box.nan=" + box.Contains(new Vector3(Float.NaN, 0.0f, 0.0f)).ordinal()
                + ',' + flag(box.Intersects(nanBox)));
        BoundingSphere sphere = new BoundingSphere(Vector3.getZero(), 1.0f);
        observations.add("sphere.contains.edge=" + sphere.Contains(Vector3.getUnitX()).ordinal());
        BoundingSphere pointsSphere = BoundingSphere.CreateFromPoints(List.of(
                new Vector3(-4.0f, 1.0f, 0.0f),
                new Vector3(6.0f, -2.0f, 3.0f),
                new Vector3(0.0f, 8.0f, -5.0f),
                new Vector3(2.0f, 0.0f, 9.0f)));
        add(observations, "sphere.points", new Vector4(pointsSphere.Center, pointsSphere.Radius));
        observations.add("ray.sphere=" + nullableBits(
                new Ray(new Vector3(-5.0f, 0.25f, 0.0f), Vector3.getUnitX()).Intersects(sphere)));

        Vector2 nanVector = new Vector2(Float.NaN, 0.0f);
        Vector2 nanVectorCopy = new Vector2(nanVector);
        observations.add("v2.equals.nan=" + flag(nanVector.equals(nanVectorCopy))
                + ',' + flag(nanVector.equals(nanVectorCopy)));
        Matrix nanMatrix = Matrix.getIdentity();
        nanMatrix.M11 = Float.NaN;
        Matrix nanMatrixCopy = new Matrix(nanMatrix);
        observations.add("matrix.equals.nan=" + flag(nanMatrix.equals(nanMatrixCopy))
                + ',' + flag(nanMatrix.equals(nanMatrixCopy)));
        observations.add("v3.hash=" + new Vector3(1.0f, 2.0f, 3.0f).hashCode());
        observations.add("matrix.identity.hash=" + Matrix.getIdentity().hashCode());
        observations.add("integer.hash=" + new Point(1, 2).hashCode()
                + ',' + new Rectangle(1, 2, 3, 4).hashCode());
        observations.add("sphere.negative=" + exceptionName(
                () -> new BoundingSphere(Vector3.getZero(), -1.0f)));
        observations.add("math.clamp.reversed=" + bits(MathHelper.Clamp(0.0f, 2.0f, 1.0f)));
        observations.add("math.wrap.large=" + bits(MathHelper.WrapAngle(123456.789f)));
        observations.add("math.splines=" + bits(MathHelper.CatmullRom(-10.0f, -10.0f, -10.0f, -7.0f, 0.3f))
                + ',' + bits(MathHelper.Hermite(-10.0f, -10.0f, -10.0f, -10.0f, 1.1f)));
        observations.add("math.hermite.endpoint.nan=" + flag(Float.isNaN(
                MathHelper.Hermite(1.0f, Float.POSITIVE_INFINITY, 2.0f, 0.0f, 0.0f))));

        BoundingSphere tangentSphere = new BoundingSphere(new Vector3(2.0f, 0.0f, 0.0f), 1.0f);
        observations.add("sphere.intersects.tangent=" + flag(sphere.Intersects(tangentSphere)));
        BoundingBox rayBox = new BoundingBox(new Vector3(-1.0f), new Vector3(1.0f));
        Ray nearParallelRay = new Ray(new Vector3(2.0f, 0.0f, 0.0f), new Vector3(-5.0E-7f, 0.0f, 0.0f));
        observations.add("box.ray.nearparallel=" + nullableBits(nearParallelRay.Intersects(rayBox)));
        Ray nearParallelPlaneRay = new Ray(Vector3.getZero(), new Vector3(5.0E-6f, 1.0f, 0.0f));
        observations.add("ray.plane.nearparallel=" + nullableBits(
                nearParallelPlaneRay.Intersects(new Plane(Vector3.getUnitX(), -1.0f))));
        Ray justBehindPlaneRay = new Ray(new Vector3(5.0E-6f, 0.0f, 0.0f), Vector3.getUnitX());
        Plane originPlane = new Plane(Vector3.getUnitX(), 0.0f);
        observations.add("ray.plane.overloads=" + nullableBits(justBehindPlaneRay.Intersects(originPlane))
                + ',' + nullableBits(justBehindPlaneRay.Intersects(originPlane)));
        observations.add("v3.transform.negative.length=" + exceptionName(() -> Vector3.Transform(
                new Vector3[] {Vector3.getZero()}, 0, Matrix.getIdentity(),
                new Vector3[] {Vector3.getZero()}, 0, -1)));
        observations.add("v3.transform.negative.index=" + exceptionName(() -> Vector3.Transform(
                new Vector3[] {Vector3.getZero()}, -1, Matrix.getIdentity(),
                new Vector3[] {Vector3.getZero()}, 0, 1)));
        float xnaNaN = Float.intBitsToFloat(0xFFC00000);
        add(observations, "v3.min.nan", Vector3.Min(
                new Vector3(xnaNaN, 1.0f, xnaNaN),
                new Vector3(7.0f, xnaNaN, xnaNaN)));
        add(observations, "v3.clamp.reversed", Vector3.Clamp(
                Vector3.getZero(), new Vector3(2.0f), new Vector3(1.0f)));
        add(observations, "q.slerp", Quaternion.Slerp(yaw, pitch, 0.37f));
        add(observations, "q.axis.large", Quaternion.CreateFromAxisAngle(Vector3.getUp(), 123456.789f));
        add(observations, "q.from.matrix", Quaternion.CreateFromRotationMatrix(Matrix.CreateRotationY(0.7f)));

        Matrix largeRotation = Matrix.CreateRotationY(123456.789f);
        add(observations, "matrix.rotation.large", new Vector2(largeRotation.M11, largeRotation.M31));
        Matrix infinitePerspective = Matrix.CreatePerspective(4.0f, 3.0f, 0.1f, Float.POSITIVE_INFINITY);
        observations.add("matrix.perspective.infinity=" + bits(infinitePerspective.M33)
                + ',' + bits(infinitePerspective.M43));
        observations.add("matrix.fov.invalid=" + exceptionName(() ->
                Matrix.CreatePerspectiveFieldOfView(0.0f, 1.0f, 0.1f, 100.0f)));
        Matrix mirroredMatrix = Matrix.Multiply(
                Matrix.Multiply(Matrix.CreateScale(-2.0f, 3.0f, 4.0f), Matrix.CreateRotationY(0.25f)),
                Matrix.CreateTranslation(5.0f, 6.0f, 7.0f));
        Matrix.Decomposition decomposition = mirroredMatrix.Decompose();
        Vector3 mirroredScale = decomposition.getScale();
        Quaternion mirroredRotation = decomposition.getRotation();
        Vector3 mirroredTranslation = decomposition.getTranslation();
        observations.add("matrix.decompose.mirror=" + flag(decomposition.getSucceeded())
                + ',' + bits(mirroredScale.X) + ',' + bits(mirroredScale.Y) + ',' + bits(mirroredScale.Z)
                + ',' + bits(mirroredRotation.X) + ',' + bits(mirroredRotation.Y)
                + ',' + bits(mirroredRotation.Z) + ',' + bits(mirroredRotation.W)
                + ',' + bits(mirroredTranslation.X) + ',' + bits(mirroredTranslation.Y)
                + ',' + bits(mirroredTranslation.Z));
        Matrix constrainedBillboard = Matrix.CreateConstrainedBillboard(
                new Vector3(0.0f, 10.0f, 0.0f), Vector3.getZero(),
                new Vector3(0.0f, 2.0f, 0.0f), null, null);
        observations.add("matrix.billboard.axis=" + bits(constrainedBillboard.M11)
                + ',' + bits(constrainedBillboard.M22) + ',' + bits(constrainedBillboard.M33));
        Matrix zeroPlaneShadow = Matrix.CreateShadow(
                Vector3.getForward(), new Plane(Vector3.getZero(), 0.0f));
        observations.add("matrix.shadow.zero.nan=" + flag(Float.isNaN(zeroPlaneShadow.M11))
                + ',' + flag(Float.isNaN(zeroPlaneShadow.M44)));
        Plane reflectionPlane = new Plane(new Vector3(2.0f, 0.0f, 0.0f), 4.0f);
        Matrix reflectionMatrix = Matrix.CreateReflection(reflectionPlane);
        observations.add("matrix.reflection.value=" + bits(reflectionPlane.Normal.X)
                + ',' + bits(reflectionPlane.D) + ',' + bits(reflectionMatrix.M11)
                + ',' + bits(reflectionMatrix.M41));
        add(observations, "matrix.lookat.degenerate", Matrix.CreateLookAt(
                Vector3.getZero(), Vector3.getZero(), Vector3.getUp()));
        Matrix infiniteTransform = Matrix.getIdentity();
        infiniteTransform.M14 = Float.POSITIVE_INFINITY;
        Matrix transformed = Matrix.Transform(infiniteTransform, Quaternion.getIdentity());
        observations.add("matrix.transform.infinity=" + bits(transformed.M11)
                + ',' + bits(transformed.M14) + ',' + flag(Float.isNaN(transformed.M11)));
        observations.add("negate.signedzero=" + bits(Vector4.Negate(Vector4.getZero()).X)
                + ',' + bits(Quaternion.Negate(new Quaternion()).X)
                + ',' + bits(Matrix.Negate(new Matrix()).M11));
        observations.add("matrix.tostring=" + Matrix.getIdentity());
        Plane degeneratePlane = new Plane(Vector3.getZero(), Vector3.getZero(), Vector3.getZero());
        add(observations, "plane.points.degenerate", new Vector4(degeneratePlane.Normal, degeneratePlane.D));
        Plane nearUnitPlane = Plane.Normalize(new Plane(new Vector3(0.6f, 0.79999995f, 0.0f), 2.0f));
        add(observations, "plane.normalize.nearunit", new Vector4(nearUnitPlane.Normal, nearUnitPlane.D));
        observations.add("plane.box.coplanar="
                + new Plane(Vector3.getZero(), 0.0f).Intersects(box).ordinal());

        CurveKey curveKey = new CurveKey(1.0f, 2.0f, 3.0f, 4.0f, CurveContinuity.Step);
        observations.add("curve.key.hash=" + curveKey.hashCode());
        CurveKey nanCurveKey = new CurveKey(xnaNaN, 0.0f);
        CurveKey finiteCurveKey = new CurveKey(0.0f, 0.0f);
        observations.add("curve.key.compare=" + nanCurveKey.CompareTo(finiteCurveKey)
                + ',' + finiteCurveKey.CompareTo(nanCurveKey)
                + ',' + exceptionName(() -> finiteCurveKey.CompareTo(null)));

        CurveKeyCollection curveKeys = new CurveKeyCollection();
        curveKeys.Add(new CurveKey(0.0f, 1.0f));
        curveKeys.Add(new CurveKey(5.0E-8f, 2.0f));
        CurveKey replacementKey = new CurveKey(1.0E-7f, 3.0f);
        curveKeys.set(0, replacementKey);
        observations.add("curve.collection.reposition=" + bits(curveKeys.get(0).getValue())
                + ',' + bits(curveKeys.get(1).getValue()));
        observations.add("curve.collection.oob="
                + exceptionName(() -> curveKeys.set(-1, replacementKey))
                + ',' + exceptionName(() -> curveKeys.set(curveKeys.getCount(), replacementKey)));

        Curve tangentCurve = new Curve();
        tangentCurve.getKeys().Add(new CurveKey(0.0f, 0.0f));
        tangentCurve.getKeys().Add(new CurveKey(1.0f, 5.0E-9f));
        tangentCurve.getKeys().Add(new CurveKey(2.0f, 1.0E-8f));
        tangentCurve.ComputeTangent(1, CurveTangent.Smooth);
        observations.add("curve.tangent.epsilon="
                + bits(tangentCurve.getKeys().get(1).getTangentIn())
                + ',' + bits(tangentCurve.getKeys().get(1).getTangentOut()));

        Curve loopCurve = new Curve();
        loopCurve.setPreLoop(CurveLoopType.Cycle);
        loopCurve.getKeys().Add(new CurveKey(0.0f, 10.0f, 0.0f, 0.0f, CurveContinuity.Step));
        loopCurve.getKeys().Add(new CurveKey(1.0f, 20.0f, 0.0f, 0.0f, CurveContinuity.Step));
        observations.add("curve.cycle.preboundary=" + bits(loopCurve.Evaluate(-1.0f)));
        observations.add("curve.step.nan=" + bits(loopCurve.Evaluate(xnaNaN)));

        Alpha8 alphaMidpoint = new Alpha8(0.5f / 255.0f);
        Bgra5551 oneBitAlphaMidpoint = new Bgra5551(0.0f, 0.0f, 0.0f, 0.5f);
        observations.add("packed.unorm.midpoint="
                + hex(alphaMidpoint.getPackedValue(), 2)
                + ',' + hex(oneBitAlphaMidpoint.getPackedValue(), 4));
        observations.add("packed.unsigned.rounding="
                + hex(new Byte4(0.5f, 1.5f, 2.5f, 3.5f).getPackedValue(), 8));
        observations.add("packed.snorm.rounding="
                + hex(new NormalizedByte2(0.5f / 127.0f, -0.5f / 127.0f).getPackedValue(), 4));
        NormalizedByte2 minimumSNorm = new NormalizedByte2();
        minimumSNorm.setPackedValue(0x8080);
        add(observations, "packed.snorm.minimum", minimumSNorm.ToVector2());
        observations.add("packed.signed.rounding="
                + hex(new Short2(0.5f, 1.5f).getPackedValue(), 8));
        HalfSingle exponent31Half = new HalfSingle();
        exponent31Half.setPackedValue(0x7C00);
        observations.add("packed.half.saturation="
                + hex(new HalfSingle(Float.POSITIVE_INFINITY).getPackedValue(), 4)
                + ',' + hex(new HalfSingle(Float.intBitsToFloat(0x7FC00000)).getPackedValue(), 4)
                + ',' + bits(exponent31Half.ToSingle()));
        Alpha8 alphaString = new Alpha8();
        alphaString.setPackedValue(0x0A);
        Bgra5551 bgraString = new Bgra5551();
        bgraString.setPackedValue(0x000A);
        Byte4 byteString = new Byte4();
        byteString.setPackedValue(0x0000000AL);
        observations.add("packed.tostring=" + alphaString + ',' + bgraString + ',' + byteString);

        observations.add("packed.bgr565.order="
                + hex(new Bgr565(1.0f, 0.0f, 0.0f).getPackedValue(), 4)
                + ',' + hex(new Bgr565(0.0f, 1.0f, 0.0f).getPackedValue(), 4)
                + ',' + hex(new Bgr565(0.0f, 0.0f, 1.0f).getPackedValue(), 4));
        observations.add("packed.bgra4444.order="
                + hex(new Bgra4444(1.0f, 0.0f, 0.0f, 0.0f).getPackedValue(), 4)
                + ',' + hex(new Bgra4444(0.0f, 1.0f, 0.0f, 0.0f).getPackedValue(), 4)
                + ',' + hex(new Bgra4444(0.0f, 0.0f, 1.0f, 0.0f).getPackedValue(), 4)
                + ',' + hex(new Bgra4444(0.0f, 0.0f, 0.0f, 1.0f).getPackedValue(), 4));
        HalfVector2 halfPair = new HalfVector2(1.0f, -2.0f);
        observations.add("packed.half2.order=" + hex(halfPair.getPackedValue(), 8)
                + ',' + bits(halfPair.ToVector2().X) + ',' + bits(halfPair.ToVector2().Y));
        HalfVector4 halfFour = new HalfVector4(
                1.0f, -2.0f, 0.5f, Float.POSITIVE_INFINITY);
        observations.add("packed.half4.saturation=" + hex(halfFour.getPackedValue(), 16)
                + ',' + bits(halfFour.ToVector4().W));
        NormalizedByte4 normalizedBytes = new NormalizedByte4(
                0.5f / 127.0f, -0.5f / 127.0f, 1.0f, -1.0f);
        observations.add("packed.nbyte4.rounding="
                + hex(normalizedBytes.getPackedValue(), 8));
        NormalizedShort2 normalizedShorts = new NormalizedShort2(
                0.5f / 32_767.0f, -0.5f / 32_767.0f);
        observations.add("packed.nshort2.rounding="
                + hex(normalizedShorts.getPackedValue(), 8));
        NormalizedShort4 normalizedMinimum = new NormalizedShort4();
        normalizedMinimum.setPackedValue(0x8000_8000_8000_8000L);
        add(observations, "packed.nshort4.minimum", normalizedMinimum.ToVector4());
        observations.add("packed.rg32.rounding="
                + hex(new Rg32(0.5f / 65_535.0f, 0.5f).getPackedValue(), 8));
        observations.add("packed.rgba1010102.midpoint="
                + hex(new Rgba1010102(0.5f, 0.5f, 0.5f, 0.5f).getPackedValue(), 8));
        observations.add("packed.rgba64.rounding="
                + hex(new Rgba64(0.5f / 65_535.0f, 0.5f, 1.0f, 0.25f)
                        .getPackedValue(), 16));
        observations.add("packed.short4.rounding="
                + hex(new Short4(0.5f, 1.5f, 40_000.0f, -40_000.0f)
                        .getPackedValue(), 16));

        Matrix frustumProjection = Matrix.CreatePerspectiveFieldOfView(
                MathHelper.PiOver4, 4.0f / 3.0f, 1.0f, 10.0f);
        Matrix frustumMatrix = Matrix.Multiply(
                Matrix.CreateLookAt(new Vector3(0.0f, 0.0f, 5.0f), Vector3.getZero(), Vector3.getUp()),
                frustumProjection);
        BoundingFrustum frustum = new BoundingFrustum(frustumMatrix);
        Plane near = frustum.getNear();
        Plane top = frustum.getTop();
        add(observations, "frustum.near", new Vector4(near.Normal, near.D));
        add(observations, "frustum.top", new Vector4(top.Normal, top.D));
        Vector3[] frustumCorners = frustum.GetCorners();
        add(observations, "frustum.corner0", frustumCorners[0]);
        add(observations, "frustum.corner6", frustumCorners[6]);
        observations.add("frustum.contains=" + frustum.Contains(Vector3.getZero()).ordinal()
                + ',' + frustum.Contains(new Vector3(0.0f, 0.0f, 6.0f)).ordinal()
                + ',' + frustum.Contains(new BoundingBox(new Vector3(-0.5f), new Vector3(0.5f))).ordinal()
                + ',' + frustum.Contains(new BoundingSphere(Vector3.getZero(), 0.5f)).ordinal());
        BoundingFrustum distantFrustum = new BoundingFrustum(Matrix.Multiply(
                Matrix.CreateLookAt(
                        new Vector3(100.0f, 0.0f, 5.0f),
                        new Vector3(100.0f, 0.0f, 0.0f), Vector3.getUp()),
                frustumProjection));
        observations.add("frustum.gjk="
                + flag(frustum.Intersects(new BoundingBox(new Vector3(-0.5f), new Vector3(0.5f))))
                + ',' + flag(frustum.Intersects(new BoundingBox(new Vector3(100.0f), new Vector3(101.0f))))
                + ',' + flag(frustum.Intersects(new BoundingSphere(Vector3.getZero(), 0.5f)))
                + ',' + flag(frustum.Intersects(new BoundingSphere(new Vector3(100.0f), 0.5f)))
                + ',' + flag(frustum.Intersects(distantFrustum)));
        observations.add("frustum.ray=" + nullableBits(
                frustum.Intersects(new Ray(new Vector3(0.0f, 0.0f, 20.0f), Vector3.getForward()))));

        return List.copyOf(observations);
    }

    private static void add(List<String> output, String name, Vector2 value) {
        output.add(name + '=' + bits(value.X) + ',' + bits(value.Y));
    }

    private static void add(List<String> output, String name, Vector3 value) {
        output.add(name + '=' + bits(value.X) + ',' + bits(value.Y) + ',' + bits(value.Z));
    }

    private static void add(List<String> output, String name, Vector4 value) {
        output.add(name + '=' + bits(value.X) + ',' + bits(value.Y) + ',' + bits(value.Z) + ',' + bits(value.W));
    }

    private static void add(List<String> output, String name, Quaternion value) {
        output.add(name + '=' + bits(value.X) + ',' + bits(value.Y) + ',' + bits(value.Z) + ',' + bits(value.W));
    }

    private static void add(List<String> output, String name, Matrix value) {
        output.add(name + '=' + bits(value.M11) + ',' + bits(value.M12) + ',' + bits(value.M13) + ',' + bits(value.M14)
                + ',' + bits(value.M21) + ',' + bits(value.M22) + ',' + bits(value.M23) + ',' + bits(value.M24)
                + ',' + bits(value.M31) + ',' + bits(value.M32) + ',' + bits(value.M33) + ',' + bits(value.M34)
                + ',' + bits(value.M41) + ',' + bits(value.M42) + ',' + bits(value.M43) + ',' + bits(value.M44));
    }

    private static String bits(float value) {
        return String.format(Locale.ROOT, "%08X", Float.floatToRawIntBits(value));
    }

    private static String hex(long value) {
        return String.format(Locale.ROOT, "%08X", value & 0xFFFF_FFFFL);
    }

    private static String hex(long value, int digits) {
        long mask = digits == 2 ? 0xFFL
                : digits == 4 ? 0xFFFFL
                : digits == 8 ? 0xFFFF_FFFFL
                : -1L;
        return String.format(Locale.ROOT, "%0" + digits + "X", value & mask);
    }

    private static String nullableBits(Float value) {
        return value == null ? "none" : bits(value);
    }

    private static int flag(boolean value) {
        return value ? 1 : 0;
    }

    private static String exceptionName(Runnable action) {
        try {
            action.run();
            return "none";
        } catch (RuntimeException exception) {
            return exception.getClass().getSimpleName();
        }
    }
}
