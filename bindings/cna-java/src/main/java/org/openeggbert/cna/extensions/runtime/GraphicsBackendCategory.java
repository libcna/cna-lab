package org.openeggbert.cna.extensions.runtime;

/**
 * How a graphics backend produces its pixels.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. The constant names are CNA's own.
 */
public enum GraphicsBackendCategory {
    Native,
    TranslationLayer,
    Software,
    Web,
    Diagnostic;
}
