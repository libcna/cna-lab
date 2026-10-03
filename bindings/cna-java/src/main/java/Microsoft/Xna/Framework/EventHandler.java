package Microsoft.Xna.Framework;

/** Java projection of the standard CLR {@code EventHandler<TEventArgs>} delegate. */
@FunctionalInterface
public interface EventHandler<T> {

    void invoke(Object sender, T args);
}
