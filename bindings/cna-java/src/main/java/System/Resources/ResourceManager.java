package System.Resources;

import java.util.Map;
import java.util.Objects;

/** Portable resource lookup carrier for ResourceContentManager. */
public class ResourceManager {
    private final Map<String, ?> resources;

    public ResourceManager(Map<String, ?> resources) {
        this.resources = Map.copyOf(Objects.requireNonNull(resources, "resources"));
    }

    public Object GetObject(String name) {
        return resources.get(name);
    }
}
