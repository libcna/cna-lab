package Microsoft.Xna.Framework;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.UncheckedIOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.NotDirectoryException;
import java.nio.file.Path;
import java.util.Objects;

/** Portable title-relative file access matching XNA's read-only stream contract. */
public final class TitleContainer {

    private TitleContainer() {
    }

    public static InputStream OpenStream(String name) {
        String requested = Objects.requireNonNull(name, "name");
        if (requested.isEmpty()) {
            throw new IllegalArgumentException("name must not be empty");
        }
        String portable = requested.replace('\\', '/');
        Path relative;
        try {
            relative = Path.of(portable);
        } catch (RuntimeException exception) {
            throw new IllegalArgumentException("Title file name is not a valid path", exception);
        }
        if (relative.isAbsolute()) {
            throw new IllegalArgumentException(
                    "TitleContainer.OpenStream requires a title-relative path");
        }
        Path resolved = titleDirectory().resolve(relative).normalize();
        try {
            return Files.newInputStream(resolved);
        } catch (NoSuchFileException | NotDirectoryException exception) {
            FileNotFoundException missing = new FileNotFoundException(
                    "Could not find title file '" + requested + "'");
            missing.initCause(exception);
            throw new UncheckedIOException(missing);
        } catch (IOException exception) {
            throw new UncheckedIOException(
                    "Could not open title file '" + requested + "'", exception);
        }
    }

    private static Path titleDirectory() {
        try {
            URI location = TitleContainer.class.getProtectionDomain()
                    .getCodeSource().getLocation().toURI();
            Path path = Path.of(location).toAbsolutePath().normalize();
            return Files.isDirectory(path) ? path : path.getParent();
        } catch (URISyntaxException | RuntimeException exception) {
            return Path.of("").toAbsolutePath().normalize();
        }
    }
}
