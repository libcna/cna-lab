package Microsoft.Xna.Framework.Design;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

final class XnaDesignBehaviorCorpusTests {

    @Test
    void MatchesReferenceBackedNormalizedDesignObservations() {
        assertEquals(List.of(
                "design.math.support=1,1,1,1,1",
                "design.point.properties=X:int,Y:int",
                "design.rectangle.properties=X:int,Y:int,Width:int,Height:int",
                "design.vector2.properties=X:float,Y:float",
                "design.vector3.properties=X:float,Y:float,Z:float",
                "design.vector4.properties=X:float,Y:float,Z:float,W:float",
                "design.quaternion.properties=X:float,Y:float,Z:float,W:float",
                "design.color.properties=R:int,G:int,B:int,A:int",
                "design.matrix.properties=Translation:Vector3,M11:float,M12:float,M13:float,M14:float,"
                        + "M21:float,M22:float,M23:float,M24:float,M31:float,M32:float,M33:float,"
                        + "M34:float,M41:float,M42:float,M43:float,M44:float",
                "design.box.properties=Min:Vector3,Max:Vector3",
                "design.sphere.properties=Center:Vector3,Radius:float",
                "design.plane.properties=Normal:Vector3,D:float",
                "design.ray.properties=Position:Vector3,Direction:Vector3",
                "design.point.support=1,0,1,1,1,1",
                "design.rectangle.support=0,0,1,1,1,1",
                "design.vector3.support=1,0,1,1,1,1",
                "design.box.support=0,0,1,1,1,1",
                "design.point.format.root=1, -2",
                "design.point.format.de=1; -2",
                "design.vector3.format.root=1.25, -2.5, 3.75",
                "design.vector3.format.de=1,25; -2,5; 3,75",
                "design.vector4.format.root=NaN, Infinity, -Infinity, 0",
                "design.vector4.format.de=NaN; +unendlich; -unendlich; 0",
                "design.color.format=0, 255, 10, 40",
                "design.fallback.format=1,1,1",
                "design.point.parse.bounds=2147483647,-2147483648",
                "design.vector3.parse.bits=80000000,0DA24260,7F7FFFFF",
                "design.vector3.parse.de=3FC00000,C0100000,40700000",
                "design.vector3.parse.special=7FC00000,7F800000,FF800000",
                "design.color.parse=0,255,10,40",
                "design.invalid.count=IllegalArgumentException,IllegalArgumentException,"
                        + "IllegalArgumentException,IllegalArgumentException",
                "design.invalid.culture=IllegalArgumentException",
                "design.invalid.range=IllegalArgumentException,IllegalArgumentException,"
                        + "IllegalArgumentException",
                "design.point.create={X:1 Y:2}",
                "design.vector3.create.extra={X:1.0 Y:2.0 Z:3.0}",
                "design.matrix.create=1.0,8.0,13.0,16.0",
                "design.sphere.snapshot=1.0,99.0",
                "design.expressions=Point,Rectangle,Vector2,Vector3,Vector4,Quaternion,Color,"
                        + "Matrix,BoundingBox,BoundingSphere,Plane,Ray",
                "design.map.failures=NullPointerException,IllegalArgumentException,"
                        + "IllegalArgumentException,IllegalArgumentException",
                "design.base.failures={X:0 Y:0},UnsupportedOperationException,"
                        + "UnsupportedOperationException"
        ), XnaDesignBehaviorCorpus.capture());
    }
}
