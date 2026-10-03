package Microsoft.Xna.Framework;

/** Java compatibility projection of {@code System.IServiceProvider}. */
@FunctionalInterface
public interface ServiceProvider {

    Object GetService(Class<?> type);
}
