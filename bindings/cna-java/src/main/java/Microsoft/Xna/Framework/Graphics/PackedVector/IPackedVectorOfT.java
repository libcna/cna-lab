package Microsoft.Xna.Framework.Graphics.PackedVector;

/** Generic XNA packed-value contract; primitive CLR values use their Java boxed type here. */
public interface IPackedVectorOfT<T> extends IPackedVector {

    T getPackedValue();

    void setPackedValue(T value);
}
