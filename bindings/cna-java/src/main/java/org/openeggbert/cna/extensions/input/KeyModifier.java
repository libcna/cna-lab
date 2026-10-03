package org.openeggbert.cna.extensions.input;

import java.util.Collections;
import java.util.EnumSet;
import java.util.Set;

/**
 * A modifier key or lock that is currently active.
 *
 * <p>A CNA extension. XNA's {@code KeyboardState} reports {@code LeftShift} and {@code RightShift}
 * as separate physical keys and has no notion of a lock being on at all, so a game cannot ask
 * whether Caps Lock is engaged. This is the host's own modifier state.
 */
public enum KeyModifier {

    /** Either shift key is held. */
    Shift(0x01),

    /** Either control key is held. */
    Ctrl(0x02),

    /** Either alt key is held. */
    Alt(0x04),

    /** Either platform key -- Windows, Command or Super -- is held. */
    Gui(0x08),

    /** Caps Lock is engaged. */
    Caps(0x10),

    /** Num Lock is engaged. */
    Num(0x20),

    /** Scroll Lock is engaged. */
    Scroll(0x40),

    /** The layout's AltGr-style mode key is engaged. */
    Mode(0x80);

    private final int mask;

    KeyModifier(int mask) {
        this.mask = mask;
    }

    /** Returns CNA's own bit for this modifier. */
    public int getMask() {
        return mask;
    }

    /** Decodes CNA's bit set. A bit CNA adds later is ignored rather than guessed at. */
    static Set<KeyModifier> decode(long modifiers) {
        EnumSet<KeyModifier> decoded = EnumSet.noneOf(KeyModifier.class);
        for (KeyModifier modifier : values()) {
            if ((modifiers & modifier.mask) != 0) {
                decoded.add(modifier);
            }
        }
        return Collections.unmodifiableSet(decoded);
    }
}
