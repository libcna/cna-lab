package Microsoft.Xna.Framework.Input;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

final class MouseStateTests {

    @Test
    void ButtonStateIdentityMatchesXnaMetadata() {
        assertArrayEquals(
                new ButtonState[] {ButtonState.Released, ButtonState.Pressed},
                ButtonState.values());
    }

    @Test
    void DefaultAndExplicitConstructorsExposeImmutableProperties() {
        MouseState empty = new MouseState();
        assertEquals(0, empty.getX());
        assertEquals(0, empty.getY());
        assertEquals(0, empty.getScrollWheelValue());
        assertEquals(ButtonState.Released, empty.getLeftButton());

        MouseState state = sample();
        assertEquals(12, state.getX());
        assertEquals(-3, state.getY());
        assertEquals(120, state.getScrollWheelValue());
        assertEquals(ButtonState.Pressed, state.getLeftButton());
        assertEquals(ButtonState.Released, state.getMiddleButton());
        assertEquals(ButtonState.Pressed, state.getRightButton());
        assertEquals(ButtonState.Pressed, state.getXButton1());
        assertEquals(ButtonState.Released, state.getXButton2());
    }

    @Test
    void EqualityHashAndStringMatchNormalizedXnaBehaviorCorpus() {
        MouseState state = sample();
        MouseState copy = new MouseState(state);
        assertEquals(state, copy);
        assertEquals(-120, state.hashCode());
        assertEquals("{X:12 Y:-3 Buttons:Left Right XButton1 Wheel:120}",
                state.toString());
        assertEquals("{X:0 Y:0 Buttons:None Wheel:0}", new MouseState().toString());
        assertNotEquals(state, new MouseState());
    }

    @Test
    void JavaOnlyNullInputsFailAtTheBoundary() {
        assertThrows(NullPointerException.class, () -> new MouseState((MouseState) null));
        assertThrows(NullPointerException.class, () -> new MouseState(
                0, 0, 0, null, ButtonState.Released, ButtonState.Released,
                ButtonState.Released, ButtonState.Released));
    }

    private static MouseState sample() {
        return new MouseState(
                12, -3, 120,
                ButtonState.Pressed, ButtonState.Released, ButtonState.Pressed,
                ButtonState.Pressed, ButtonState.Released);
    }
}
