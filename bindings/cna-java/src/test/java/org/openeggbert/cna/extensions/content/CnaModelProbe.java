package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Content.ContentManager;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Graphics.Model;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.function.Consumer;

import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Runs one body against CNA's own view of an authored model, inside a real frame.
 *
 * <p>The asset is upstream's -- an authored XNA 4.0 {@code .xnb} from the CNA checkout this build
 * already qualifies against. Without it there is nothing honest to measure, so a test stops rather
 * than inventing a model whose graph would only agree with the assumptions that built it.
 */
final class CnaModelProbe {

    private static final String CUBE =
            "tests/assets/xnb/monogame/windows/uncompressed/BlenderDefaultCube.xnb";

    private CnaModelProbe() {
    }

    /** Runs one body over a loaded {@link CnaModel}, or does nothing when the asset is absent. */
    static void run(Consumer<CnaModel> body) {
        String root = System.getProperty("cna.root");
        if (root == null) {
            return;
        }
        Path asset = Path.of(root).resolve(CUBE);
        if (!Files.isRegularFile(asset)) {
            return;
        }
        try (Probe probe = new Probe(asset, body)) {
            probe.RunOneFrame();
            if (probe.failure instanceof RuntimeException runtime) {
                throw runtime;
            }
            if (probe.failure instanceof Error error) {
                throw error;
            }
            if (probe.failure != null) {
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    private static final class Probe extends Game {

        private final Path asset;
        private final Consumer<CnaModel> body;
        private boolean ran;
        private Throwable failure;

        private Probe(Path asset, Consumer<CnaModel> body) {
            this.asset = asset;
            this.body = body;
            new GraphicsDeviceManager(this);
        }

        @Override
        protected void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                // The game's own content manager, because its service provider carries the
                // graphics device the buffers and effects need.
                ContentManager content = getContent();
                content.setRootDirectory(stage().toString());
                Model managed = content.Load(Model.class, "cube");
                try (CnaModel loaded = CnaModel.From(managed)) {
                    body.accept(loaded);
                }
            } catch (Throwable exception) {
                failure = exception;
            }
        }

        private Path stage() {
            try {
                Path directory = Files.createTempDirectory("cna-java-animations");
                directory.toFile().deleteOnExit();
                Path staged = directory.resolve("cube.xnb");
                Files.copy(asset, staged);
                staged.toFile().deleteOnExit();
                return directory;
            } catch (IOException exception) {
                throw new UncheckedIOException(exception);
            }
        }
    }
}
