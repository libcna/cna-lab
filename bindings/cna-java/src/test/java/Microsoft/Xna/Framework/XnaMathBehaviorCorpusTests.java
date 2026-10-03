package Microsoft.Xna.Framework;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

final class XnaMathBehaviorCorpusTests {

    @Test
    void Corpus_IsDeterministicAndMatchesIndependentlyDerivedXnaEdges() {
        List<String> first = XnaMathBehaviorCorpus.capture();
        List<String> second = XnaMathBehaviorCorpus.capture();

        assertEquals(first, second);
        assertEquals(94, first.size());
        assertContains(first, "v2.normalize.zero=FFC00000,FFC00000");
        assertContains(first, "vector.divide.scalar=3EDB6DB8,40155556,458099CA,3EAAAAAB");
        assertContains(first, "q.inverse.zero=FFC00000,FFC00000,FFC00000,FFC00000");
        assertContains(first, "q.multiply.grouped=CE47A05E,CF03EDF7,4FC9C4DD,5011D115");
        assertContains(first,
                "matrix.inverse.product=3F800000,00000000,B2000000,00000000,"
                        + "00000000,3F800000,00000000,00000000,33000000,00000000,3F800000,00000000,"
                        + "34000000,00000000,00000000,3F800000");
        assertContains(first, "viewport.project=43D42808,43AC9F3C,3F63AFF4");
        assertContains(first, "viewport.unproject=3E7FFE10,BEFFF906,3FA00111");
        assertContains(first, "viewport.unproject.singular=FFC00000,FFC00000,FFC00000");
        assertContains(first, "color.pack=00FF0080");
        assertContains(first, "color.lerp=7F7F7F7F");
        assertContains(first, "color.nonpremultiplied.extreme=FFFFFFFF");
        assertContains(first, "box.contains.edge=1");
        assertContains(first, "box.nan=0,1");
        assertContains(first, "sphere.contains.edge=0");
        assertContains(first, "sphere.points=3F800000,40800000,40000000,4101FC10");
        assertContains(first, "ray.sphere=40810421");
        assertContains(first, "v2.equals.nan=0,0");
        assertContains(first, "matrix.equals.nan=0,0");
        assertContains(first, "v3.hash=-1077936128");
        assertContains(first, "matrix.identity.hash=-33554432");
        assertContains(first, "integer.hash=3,10");
        assertContains(first, "sphere.negative=IllegalArgumentException");
        assertContains(first, "math.clamp.reversed=40000000");
        assertContains(first, "math.wrap.large=BFC2E06C");
        assertContains(first, "math.splines=C1218313,C1351EBA");
        assertContains(first, "math.hermite.endpoint.nan=1");
        assertContains(first, "sphere.intersects.tangent=0");
        assertContains(first, "box.ray.nearparallel=none");
        assertContains(first, "ray.plane.nearparallel=none");
        assertContains(first, "ray.plane.overloads=00000000,00000000");
        assertContains(first, "v3.transform.negative.length=none");
        assertTrue(first.stream().anyMatch(value -> value.startsWith("v3.transform.negative.index=")));
        assertContains(first, "v3.min.nan=40E00000,FFC00000,FFC00000");
        assertContains(first, "v3.clamp.reversed=40000000,40000000,40000000");
        assertContains(first, "q.slerp=BD9A16EC,3E60D7E7,00000000,3F79023D");
        assertContains(first, "q.axis.large=00000000,3F30464F,00000000,BF39A48F");
        assertContains(first, "q.from.matrix=00000000,3EAF904C,00000000,3F707ABB");
        assertContains(first, "matrix.rotation.large=3D53E807,BF7FA83D");
        assertContains(first, "matrix.perspective.infinity=FFC00000,FFC00000");
        assertContains(first, "matrix.fov.invalid=IllegalArgumentException");
        assertContains(first,
                "matrix.decompose.mirror=1,40000000,40400000,C0800000,"
                        + "00000000,3F7E00AA,00000000,BDFF5579,40A00000,40C00000,40E00000");
        assertContains(first, "matrix.billboard.axis=BF800000,40000000,BF800000");
        assertContains(first, "matrix.shadow.zero.nan=1,1");
        assertContains(first, "matrix.reflection.value=40000000,40800000,BF800000,C0800000");
        assertContains(first,
                "matrix.lookat.degenerate=FFC00000,FFC00000,FFC00000,00000000,"
                        + "FFC00000,FFC00000,FFC00000,00000000,FFC00000,FFC00000,FFC00000,00000000,"
                        + "7FC00000,7FC00000,7FC00000,3F800000");
        assertContains(first, "matrix.transform.infinity=3F800000,7F800000,0");
        assertContains(first, "negate.signedzero=80000000,80000000,80000000");
        assertContains(first, "plane.points.degenerate=FFC00000,FFC00000,FFC00000,7FC00000");
        assertContains(first, "plane.normalize.nearunit=3F19999A,3F4CCCCC,00000000,40000000");
        assertContains(first, "plane.box.coplanar=2");
        assertContains(first, "curve.key.hash=4194305");
        assertContains(first, "curve.key.compare=1,1,NullPointerException");
        assertContains(first, "curve.collection.reposition=40000000,40400000");
        assertContains(first,
                "curve.collection.oob=IndexOutOfBoundsException,IndexOutOfBoundsException");
        assertContains(first, "curve.tangent.epsilon=00000000,00000000");
        assertContains(first, "curve.cycle.preboundary=41A00000");
        assertContains(first, "curve.step.nan=41A00000");
        assertContains(first, "packed.unorm.midpoint=00,0000");
        assertContains(first, "packed.unsigned.rounding=04020200");
        assertContains(first, "packed.snorm.rounding=0000");
        assertContains(first, "packed.snorm.minimum=BF800000,BF800000");
        assertContains(first, "packed.signed.rounding=00020000");
        assertContains(first, "packed.half.saturation=7FFF,7FFF,47800000");
        assertContains(first, "packed.tostring=0A,000A,0000000A");
        assertContains(first, "packed.bgr565.order=F800,07E0,001F");
        assertContains(first, "packed.bgra4444.order=0F00,00F0,000F,F000");
        assertContains(first, "packed.half2.order=C0003C00,3F800000,C0000000");
        assertContains(first, "packed.half4.saturation=7FFF3800C0003C00,47FFE000");
        assertContains(first, "packed.nbyte4.rounding=817F0000");
        assertContains(first, "packed.nshort2.rounding=00000000");
        assertContains(first,
                "packed.nshort4.minimum=BF800000,BF800000,BF800000,BF800000");
        assertContains(first, "packed.rg32.rounding=80000000");
        assertContains(first, "packed.rgba1010102.midpoint=A0080200");
        assertContains(first, "packed.rgba64.rounding=4000FFFF80000000");
        assertContains(first, "packed.short4.rounding=80007FFF00020000");
        assertContains(first, "frustum.near=80000000,80000000,3F800000,C0800000");
        assertContains(first, "frustum.top=00000000,3F6C835F,3EC3EF16,BFF4EADB");
        assertContains(first, "frustum.corner0=BF0D6289,3ED413CB,40800000");
        assertContains(first, "frustum.corner6=40B0BB28,C0848C5D,C09FFFF8");
        assertContains(first, "frustum.contains=1,0,1,1");
        assertContains(first, "frustum.gjk=1,0,1,0,0");
        assertContains(first, "frustum.ray=41800000");
    }

    private static void assertContains(List<String> observations, String expected) {
        assertTrue(observations.contains(expected), () -> "Missing observation: " + expected
                + System.lineSeparator() + String.join(System.lineSeparator(), observations));
    }
}
