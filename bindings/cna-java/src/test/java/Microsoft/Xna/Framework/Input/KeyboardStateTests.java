package Microsoft.Xna.Framework.Input;

import Microsoft.Xna.Framework.PlayerIndex;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

final class KeyboardStateTests {

    @Test
    void EnumIdentityAndNumericValuesMatchXnaMetadata() {
        assertArrayEquals(
                new PlayerIndex[] {PlayerIndex.One, PlayerIndex.Two,
                        PlayerIndex.Three, PlayerIndex.Four},
                PlayerIndex.values());
        assertArrayEquals(new KeyState[] {KeyState.Up, KeyState.Down}, KeyState.values());
        assertEquals(160, Keys.values().length);
        assertEquals(27, Keys.Escape.getValue());
        assertEquals(242, Keys.OemCopy.getValue());
        assertEquals(254, Keys.OemClear.getValue());
    }

    @Test
    void ConstructorCreatesDeduplicatedImmutableSnapshot() {
        Keys[] source = {Keys.Z, Keys.A, Keys.A};
        KeyboardState state = new KeyboardState(source);
        source[0] = Keys.Escape;

        assertArrayEquals(new Keys[] {Keys.A, Keys.Z}, state.GetPressedKeys());
        assertTrue(state.IsKeyDown(Keys.A));
        assertTrue(state.IsKeyUp(Keys.Escape));
        assertEquals(KeyState.Down, state.get(Keys.Z));
        assertEquals(KeyState.Up, state.get(Keys.Space));
    }

    @Test
    void NullVarargsMatchesXnaEmptyStateAndInvalidJavaNullElementsFail() {
        assertArrayEquals(new Keys[0], new KeyboardState((Keys[]) null).GetPressedKeys());
        assertThrows(NullPointerException.class, () -> new KeyboardState(Keys.A, null));
        assertThrows(NullPointerException.class, () -> new KeyboardState().IsKeyDown(null));
    }

    @Test
    void CopyEqualityHashAndReturnedArraysAreValueOriented() {
        KeyboardState state = new KeyboardState(Keys.A, Keys.Z);
        KeyboardState copy = new KeyboardState(state);
        Keys[] first = copy.GetPressedKeys();
        first[0] = Keys.Escape;

        assertEquals(state, copy);
        assertEquals(state.hashCode(), copy.hashCode());
        assertEquals((1 << 1) ^ (1 << 26), state.hashCode());
        assertArrayEquals(new Keys[] {Keys.A, Keys.Z}, copy.GetPressedKeys());
        assertThrows(NullPointerException.class, () -> new KeyboardState((KeyboardState) null));
    }
}
