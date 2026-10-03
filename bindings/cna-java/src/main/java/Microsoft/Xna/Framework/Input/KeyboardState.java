package Microsoft.Xna.Framework.Input;

import java.util.Arrays;
import java.util.Objects;

/** Immutable, copy-oriented snapshot of XNA's 256 keyboard-key slots. */
public final class KeyboardState {

    private static final int WORD_COUNT = 4;
    private final long[] words;

    public KeyboardState() {
        words = new long[WORD_COUNT];
    }

    public KeyboardState(KeyboardState value) {
        words = Objects.requireNonNull(value, "value").words.clone();
    }

    public KeyboardState(Keys... keys) {
        words = new long[WORD_COUNT];
        if (keys == null) {
            return;
        }
        for (Keys key : keys) {
            int value = Objects.requireNonNull(key, "keys element").getValue();
            words[value >>> 6] |= 1L << (value & 63);
        }
    }

    KeyboardState(long[] words) {
        if (words.length != WORD_COUNT) {
            throw new IllegalArgumentException("keyboard state must have four words");
        }
        this.words = words.clone();
    }

    static KeyboardState fromValues(int... values) {
        KeyboardState state = new KeyboardState();
        if (values == null) {
            return state;
        }
        for (int value : values) {
            Keys key = keyFromValue(value);
            if (key != null) {
                state.words[value >>> 6] |= 1L << (value & 63);
            }
        }
        return state;
    }

    boolean isKeyDownValue(int value) {
        Keys key = keyFromValue(value);
        return key != null && IsKeyDown(key);
    }

    public KeyState get(Keys key) {
        return IsKeyDown(key) ? KeyState.Down : KeyState.Up;
    }

    public Keys[] GetPressedKeys() {
        Keys[] values = Keys.values();
        Keys[] pressed = new Keys[values.length];
        int count = 0;
        for (Keys key : values) {
            if (IsKeyDown(key)) {
                pressed[count++] = key;
            }
        }
        return Arrays.copyOf(pressed, count);
    }

    public boolean IsKeyDown(Keys key) {
        int value = Objects.requireNonNull(key, "key").getValue();
        return (words[value >>> 6] & (1L << (value & 63))) != 0L;
    }

    public boolean IsKeyUp(Keys key) {
        return !IsKeyDown(key);
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof KeyboardState other
                && Arrays.equals(words, other.words);
    }

    @Override
    public int hashCode() {
        int hash = 0;
        for (long word : words) {
            hash ^= (int) word ^ (int) (word >>> 32);
        }
        return hash;
    }

    private static Keys keyFromValue(int value) {
        for (Keys key : Keys.values()) {
            if (key.getValue() == value) {
                return key;
            }
        }
        return null;
    }
}
