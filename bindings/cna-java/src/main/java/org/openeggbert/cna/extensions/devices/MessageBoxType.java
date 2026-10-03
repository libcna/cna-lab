package org.openeggbert.cna.extensions.devices;

/**
 * How severe a message box's contents are, which is what decides the host's icon and sound.
 *
 * <p>A CNA extension: XNA 4.0 has no message box at all. The ordinals are CNA's own
 * {@code CNA_MESSAGE_BOX_TYPE_*} values, so the mapping is the identity and nothing translates.
 */
public enum MessageBoxType {

    /** An error the reader must know about. */
    Error,

    /** A warning. */
    Warning,

    /** Information. */
    Information;

    static MessageBoxType of(int value) {
        MessageBoxType[] all = values();
        if (value < 0 || value >= all.length) {
            throw new IllegalStateException("CNA reported message box type " + value
                    + ", which this ABI does not name");
        }
        return all[value];
    }
}
