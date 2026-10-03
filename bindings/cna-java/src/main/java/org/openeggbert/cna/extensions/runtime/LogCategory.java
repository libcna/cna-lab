package org.openeggbert.cna.extensions.runtime;

/**
 * The subsystem a log message belongs to.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. The constant names are CNA's own.
 */
public enum LogCategory {
    Application,
    Error,
    System,
    Audio,
    Video,
    Render,
    Input,
    Test,
    Gpu;
}
