package org.openeggbert.cna.extensions.input;

/**
 * What kind of text a field expects, so a host with a screen keyboard can offer the right one.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. The constant names are CNA's own.
 */
public enum TextInputType {
    Text,
    TextName,
    TextEmail,
    TextUsername,
    TextPasswordHidden,
    TextPasswordVisible,
    Number,
    NumberPasswordHidden,
    NumberPasswordVisible
}
