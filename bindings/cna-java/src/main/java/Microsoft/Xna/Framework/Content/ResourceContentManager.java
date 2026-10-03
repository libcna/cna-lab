package Microsoft.Xna.Framework.Content;

import Microsoft.Xna.Framework.ServiceProvider;
import System.IO.Stream;
import System.Resources.ResourceManager;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.Objects;

/** ContentManager whose XNB byte streams come from a ResourceManager. */
public class ResourceContentManager extends ContentManager {

    private final ResourceManager resourceManager;

    public ResourceContentManager(
            ServiceProvider serviceProvider, ResourceManager resourceManager) {
        super(serviceProvider);
        this.resourceManager = Objects.requireNonNull(resourceManager, "resourceManager");
    }

    @Override
    protected Stream OpenStream(String assetName) {
        Object resource = resourceManager.GetObject(
                Objects.requireNonNull(assetName, "assetName"));
        if (resource instanceof byte[] bytes) {
            InputStream input = new ByteArrayInputStream(bytes.clone());
            return new Stream(input);
        }
        if (resource == null) {
            throw new ContentLoadException("Resource '" + assetName + "' was not found");
        }
        throw new ContentLoadException(
                "Resource '" + assetName + "' is not stored as a byte array");
    }

    @Override
    boolean hasManagedAsset(String assetName) {
        return resourceManager.GetObject(Objects.requireNonNull(assetName, "assetName")) instanceof byte[];
    }
}
