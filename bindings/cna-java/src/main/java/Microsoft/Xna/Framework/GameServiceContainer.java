package Microsoft.Xna.Framework;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;

/** Type-keyed XNA game service container. */
public class GameServiceContainer implements ServiceProvider {

    private final Map<Class<?>, Object> services = new LinkedHashMap<>();

    public GameServiceContainer() {
    }

    public final void AddService(Class<?> type, Object provider) {
        Class<?> serviceType = Objects.requireNonNull(type, "type");
        Object service = Objects.requireNonNull(provider, "provider");
        if (!serviceType.isInstance(service)) {
            throw new IllegalArgumentException(
                    "provider does not implement the registered service type " + serviceType.getName());
        }
        if (services.putIfAbsent(serviceType, service) != null) {
            throw new IllegalArgumentException("A service is already registered for " + serviceType.getName());
        }
    }

    @Override
    public final Object GetService(Class<?> type) {
        return services.get(Objects.requireNonNull(type, "type"));
    }

    public final void RemoveService(Class<?> type) {
        services.remove(Objects.requireNonNull(type, "type"));
    }
}
