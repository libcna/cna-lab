package org.openeggbert.cna.extensions.devices;

import java.util.Objects;

/**
 * One file-type choice a file dialog offers.
 *
 * <p>A CNA extension: XNA 4.0 has no file dialog. The pattern is the host's own syntax and CNA
 * passes it through unchanged, so what it accepts is the platform's business rather than this
 * projection's.
 *
 * @param name the human-readable name, such as {@code "Saved games"}
 * @param pattern the platform's filter pattern, such as {@code "sav"}
 */
public record FileDialogFilter(String name, String pattern) {

    /** Rejects a null half at construction, where the caller can still see which one. */
    public FileDialogFilter {
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(pattern, "pattern");
    }
}
