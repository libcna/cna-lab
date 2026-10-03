package org.openeggbert.cna.extensions.runtime;

/**
 * How far a graphics backend has been taken.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. The constant names are CNA's own.
 */
public enum GraphicsBackendMaturity {
    Production,
    Supported,
    Experimental,
    Historical,
    Deprecated;
}
