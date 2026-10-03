package Microsoft.Xna.Framework;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.UUID;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertInstanceOf;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

final class CoreRuntimeNativeIntegrationTests {

    @Test
    void titleContainerUsesTitleRelativePortableReadOnlyStreams() throws Exception {
        Path titleDirectory = Path.of(TitleContainer.class.getProtectionDomain()
                .getCodeSource().getLocation().toURI()).toAbsolutePath().normalize();
        if (!Files.isDirectory(titleDirectory)) titleDirectory = titleDirectory.getParent();
        Path fixtureDirectory = titleDirectory.resolve("title-fixture-" + UUID.randomUUID());
        Path fixture = fixtureDirectory.resolve("payload.bin");
        byte[] expected = new byte[] {0, 1, 2, 3, (byte)255};
        Files.createDirectories(fixtureDirectory);
        Files.write(fixture, expected);
        try {
            String portableName = fixtureDirectory.getFileName() + "\\payload.bin";
            try (InputStream stream = TitleContainer.OpenStream(portableName)) {
                assertArrayEquals(expected, stream.readAllBytes());
            }
            assertThrows(NullPointerException.class, () -> TitleContainer.OpenStream(null));
            assertThrows(IllegalArgumentException.class, () -> TitleContainer.OpenStream(""));
            assertThrows(IllegalArgumentException.class,
                    () -> TitleContainer.OpenStream(fixture.toAbsolutePath().toString()));
            UncheckedIOException missing = assertThrows(
                    UncheckedIOException.class,
                    () -> TitleContainer.OpenStream(
                            fixtureDirectory.getFileName() + "/missing.bin"));
            assertInstanceOf(FileNotFoundException.class, missing.getCause());
            assertTrue(missing.getMessage().contains("missing.bin"));
        } finally {
            Files.deleteIfExists(fixture);
            Files.deleteIfExists(fixtureDirectory);
        }
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void frameworkDispatcherRequiresAndPumpsTheCurrentNativeGame() {
        assertThrows(IllegalStateException.class, FrameworkDispatcher::Update);
        PumpGame game = new PumpGame();
        try {
            game.RunOneFrame();
            assertTrue(game.pumpedInsideUpdate);
            assertDoesNotThrow(FrameworkDispatcher::Update);
        } finally {
            game.close();
        }
        assertThrows(IllegalStateException.class, FrameworkDispatcher::Update);
    }

    private static final class PumpGame extends Game {
        private boolean pumpedInsideUpdate;

        @Override
        protected void Update(GameTime gameTime) {
            FrameworkDispatcher.Update();
            pumpedInsideUpdate = true;
        }
    }
}
