package Microsoft.Xna.Framework;

import Microsoft.Xna.Framework.Content.ContentLoadException;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.DepthFormat;
import Microsoft.Xna.Framework.Graphics.GraphicsProfile;
import Microsoft.Xna.Framework.Graphics.PresentationParameters;
import Microsoft.Xna.Framework.Graphics.SurfaceFormat;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.*;

final class LifecycleAndContentTests {

    @Test
    void Game_PropertiesFollowNormativeMappingAndCloseIsIdempotent() {
        TestGame game = new TestGame();
        GraphicsDeviceManager manager = new GraphicsDeviceManager(game);
        assertSame(game.getGraphicsDevice(), manager.getGraphicsDevice());
        assertSame(manager, game.getServices().GetService(IGraphicsDeviceManager.class));
        assertSame(manager, game.getServices().GetService(
                Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService.class));
        assertNotNull(game.getContent());
        game.setIsMouseVisible(true);
        assertTrue(game.getIsMouseVisible());
        game.close();
        game.close();
        assertThrows(IllegalStateException.class, game::Run);
        assertThrows(IllegalStateException.class, manager::getGraphicsDevice);
    }

    @Test
    void GraphicsDeviceManager_ManagedPreferencesValidateAndSnapshotBeforeNativeCreation() {
        try (TestGame game = new TestGame()) {
            GraphicsDeviceManager manager = new GraphicsDeviceManager(game);
            assertEquals(800, GraphicsDeviceManager.DefaultBackBufferWidth);
            assertEquals(480, GraphicsDeviceManager.DefaultBackBufferHeight);
            assertEquals(800, manager.getPreferredBackBufferWidth());
            assertEquals(480, manager.getPreferredBackBufferHeight());
            assertEquals(GraphicsProfile.Reach, manager.getGraphicsProfile());
            assertEquals(SurfaceFormat.Color, manager.getPreferredBackBufferFormat());
            assertEquals(DepthFormat.Depth24, manager.getPreferredDepthStencilFormat());
            assertTrue(manager.getSynchronizeWithVerticalRetrace());
            assertFalse(manager.getIsFullScreen());
            assertFalse(manager.getPreferMultiSampling());

            manager.setPreferredBackBufferWidth(1024);
            manager.setPreferredBackBufferHeight(576);
            manager.setGraphicsProfile(GraphicsProfile.HiDef);
            manager.setPreferredBackBufferFormat(SurfaceFormat.Bgr565);
            manager.setPreferredDepthStencilFormat(DepthFormat.Depth16);
            manager.setSynchronizeWithVerticalRetrace(false);
            manager.setPreferMultiSampling(true);
            manager.setSupportedOrientations(
                    DisplayOrientation.LandscapeLeft.Or(DisplayOrientation.LandscapeRight));
            assertEquals(1024, manager.getPreferredBackBufferWidth());
            assertEquals(576, manager.getPreferredBackBufferHeight());
            assertThrows(IllegalArgumentException.class,
                    () -> manager.setPreferredBackBufferWidth(0));
            assertThrows(IllegalArgumentException.class,
                    () -> manager.setPreferredBackBufferHeight(-1));
            assertThrows(NullPointerException.class, () -> manager.setGraphicsProfile(null));
        }
    }

    @Test
    void GraphicsDeviceInformation_CloneCopiesMutablePresentationState() {
        GraphicsDeviceInformation information = new GraphicsDeviceInformation();
        PresentationParameters parameters = information.getPresentationParameters();
        assertTrue(parameters.getIsFullScreen());
        assertEquals(new Rectangle(), parameters.getBounds());
        parameters.setBackBufferWidth(640);
        parameters.setBackBufferHeight(360);
        parameters.setIsFullScreen(false);
        parameters.setDisplayOrientation(DisplayOrientation.Portrait);

        GraphicsDeviceInformation clone = information.Clone();
        assertEquals(information, clone);
        assertEquals(information.hashCode(), clone.hashCode());
        assertNotSame(parameters, clone.getPresentationParameters());
        assertEquals(new Rectangle(0, 0, 640, 360),
                clone.getPresentationParameters().getBounds());

        parameters.setBackBufferWidth(800);
        assertEquals(640, clone.getPresentationParameters().getBackBufferWidth());
        assertNotEquals(information, clone);
    }

    @Test
    void ContentManager_ValidatesNamesAndDisposedState() {
        GameServiceContainer services = new GameServiceContainer();
        var content = new Microsoft.Xna.Framework.Content.ContentManager(services);
        assertSame(services, content.getServiceProvider());
        content.setRootDirectory("Content");
        assertEquals("Content", content.getRootDirectory());
        assertThrows(IllegalArgumentException.class, () -> content.Load(String.class, ""));
        assertThrows(ContentLoadException.class, () -> content.Load(String.class, "missing"));
        content.close();
        assertThrows(IllegalStateException.class, content::getRootDirectory);
    }

    @Test
    void ContentManager_ExactConstructorsValidateAndSetRootDirectory() {
        GameServiceContainer services = new GameServiceContainer();
        var content = new Microsoft.Xna.Framework.Content.ContentManager(services, "Assets");
        assertEquals("Assets", content.getRootDirectory());
        assertThrows(NullPointerException.class,
                () -> new Microsoft.Xna.Framework.Content.ContentManager(null));
        assertThrows(NullPointerException.class,
                () -> new Microsoft.Xna.Framework.Content.ContentManager(services, null));
        content.close();
    }

    @Test
    void ContentManager_OpenStreamCleansPathsAddsXnbAndWrapsMissingFiles(
            @TempDir Path contentRoot) throws IOException {
        Files.write(contentRoot.resolve("fixture.xnb"), new byte[] {1, 2, 3, 4});
        try (ExposedContentManager content = new ExposedContentManager(
                new GameServiceContainer(), contentRoot.toString());
             InputStream stream = content.open("folder/../fixture")) {
            assertArrayEquals(new byte[] {1, 2, 3, 4}, stream.readAllBytes());
            assertThrows(ContentLoadException.class, () -> content.open("missing"));
        }
    }

    @Test
    void GraphicsDevice_RejectsUseBeforeNativeRun() {
        try (TestGame game = new TestGame()) {
            GraphicsDevice device = game.getGraphicsDevice();
            assertThrows(IllegalStateException.class, () -> device.Clear(Color.CornflowerBlue));
            assertThrows(NullPointerException.class, () -> device.Clear(null));
        }
    }

    private static final class TestGame extends Game {
    }

    private static final class ExposedContentManager
            extends Microsoft.Xna.Framework.Content.ContentManager {

        private ExposedContentManager(ServiceProvider services, String rootDirectory) {
            super(services, rootDirectory);
        }

        private InputStream open(String assetName) {
            return OpenStream(assetName);
        }
    }
}
