package Microsoft.Xna.Framework.Graphics.PackedVector;

import Microsoft.Xna.Framework.Vector4;

/** Non-generic XNA packed-vector conversion contract. */
public interface IPackedVector {

    void PackFromVector4(Vector4 vector);

    Vector4 ToVector4();
}
