package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Input.Keys;
import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

/**
 * What the host knows about the keyboard that XNA's {@code Keyboard} cannot express.
 *
 * <p>A CNA extension. XNA reports which keys are down and nothing else: it cannot name a key in
 * the player's own layout, cannot separate the physical key from the character it currently
 * produces, and has no notion of Caps Lock being engaged. A rebindable-controls screen needs all
 * three.
 *
 * <p><strong>A key and a scancode are different things.</strong> A scancode names the physical
 * key -- the one next to left shift -- and a key names what that physical key produces under the
 * active layout. They are the same value type here because CNA uses one identity for both, and
 * the method names say which is meant.
 */
public final class KeyboardExtensions {

    /**
     * CNA's key identity is XNA's virtual-key value, so a value read back has to become a
     * {@link Keys} constant again. XNA's enum has no lookup of its own and must not gain one --
     * it is Microsoft's contract -- so the table lives here, where it is this package's business.
     */
    private static final Map<Integer, Keys> BY_VALUE = byValue();

    private KeyboardExtensions() {
    }

    private static Map<Integer, Keys> byValue() {
        Map<Integer, Keys> table = new HashMap<>();
        for (Keys key : Keys.values()) {
            table.put(key.getValue(), key);
        }
        return Map.copyOf(table);
    }

    /**
     * Returns the key one value names.
     *
     * <p>A value XNA has no constant for answers {@link Keys#None}: CNA's key space is the
     * larger one, and inventing a constant for a key XNA never had would put a non-XNA member
     * into Microsoft's enum.
     */
    private static Keys fromValue(int value) {
        return BY_VALUE.getOrDefault(value, Keys.None);
    }

    /**
     * Returns the host's name for what a key produces under the active layout.
     *
     * @param key the key to name
     * @return the layout-dependent name, empty when the host has none
     */
    public static String getKeyName(Keys key) {
        return text(key, true);
    }

    /**
     * Returns the host's name for a physical key, independent of layout.
     *
     * @param scancode the physical key to name
     * @return the layout-independent name, empty when the host has none
     */
    public static String getScancodeName(Keys scancode) {
        return text(scancode, false);
    }

    /**
     * Looks a key up by the name the active layout gives it.
     *
     * @param name the layout-dependent name
     * @return the key, or {@link Keys#None} when the host does not recognise the name
     */
    public static Keys getKeyFromName(String name) {
        return lookup(name, true);
    }

    /**
     * Looks a physical key up by its layout-independent name.
     *
     * @param name the layout-independent name
     * @return the physical key, or {@link Keys#None} when the host does not recognise the name
     */
    public static Keys getScancodeFromName(String name) {
        return lookup(name, false);
    }

    /**
     * Translates a physical key into what it currently produces.
     *
     * <p>This is what turns "the key next to left shift" into "Z" on a QWERTY layout and "W" on
     * an AZERTY one, which is the difference a controls screen has to get right.
     *
     * @param scancode the physical key
     * @return the key the active layout maps it to
     */
    public static Keys getKeyFromScancode(Keys scancode) {
        Objects.requireNonNull(scancode, "scancode");
        int[] key = new int[1];
        check("getKeyFromScancode", NativeInputExtensionRoutes
                .keyboardGetKeyFromScancodeExt(game(), scancode.getValue(), key));
        return fromValue(key[0]);
    }

    /**
     * Returns the modifier keys and locks the host reports as active.
     *
     * <p>An empty set means no modifier is active, which is different from the host not knowing:
     * CNA answers with its own state and this reports it unchanged.
     *
     * @return the active modifiers, never null
     */
    public static Set<KeyModifier> getModifiers() {
        int[] modifiers = new int[1];
        check("getModifiers",
                NativeInputExtensionRoutes.keyboardGetModStateExt(game(), modifiers));
        return KeyModifier.decode(modifiers[0]);
    }

    private static String text(Keys key, boolean layoutDependent) {
        Objects.requireNonNull(key, "key");
        long game = game();
        long[] bytes = new long[1];
        check("nameSize", layoutDependent
                ? NativeInputExtensionRoutes.keyboardGetKeyNameSizeExt(game, key.getValue(), bytes)
                : NativeInputExtensionRoutes
                        .keyboardGetScancodeNameSizeExt(game, key.getValue(), bytes));
        byte[] destination = new byte[(int) bytes[0]];
        check("name", layoutDependent
                ? NativeInputExtensionRoutes
                        .keyboardCopyKeyNameExt(game, key.getValue(), destination, bytes)
                : NativeInputExtensionRoutes
                        .keyboardCopyScancodeNameExt(game, key.getValue(), destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    private static Keys lookup(String name, boolean layoutDependent) {
        Objects.requireNonNull(name, "name");
        byte[] encoded = name.getBytes(StandardCharsets.UTF_8);
        int[] key = new int[1];
        check("fromName", layoutDependent
                ? NativeInputExtensionRoutes.keyboardGetKeyFromNameExt(game(), encoded, key)
                : NativeInputExtensionRoutes.keyboardGetScancodeFromNameExt(game(), encoded, key));
        return fromValue(key[0]);
    }

    private static long game() {
        return InputExtension.game("KeyboardExtensions");
    }

    private static void check(String operation, int result) {
        InputExtension.check("KeyboardExtensions." + operation, result);
    }
}
