package Microsoft.Xna.Framework.Input;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

final class XnaInputBehaviorCorpusTests {

    @Test
    void Corpus_IsDeterministicAndMatchesIndependentlyDerivedXnaEdges() {
        List<String> first = XnaInputBehaviorCorpus.capture();
        List<String> second = XnaInputBehaviorCorpus.capture();

        assertEquals(first, second);
        assertEquals(23, first.size());
        assertContains(first, "keyboard.null.count=0");
        assertContains(first, "keyboard.pressed=65,90");
        assertContains(first, "keyboard.invalid=0,0");
        assertContains(first, "keyboard.hash=67108866");
        assertContains(first, "mouse.string={X:12 Y:-3 Buttons:Left Right XButton1 Wheel:120}");
        assertContains(first, "mouse.hash=-120");
        assertContains(first, "thumbs.clamp=3F800000,BF800000,3E800000,BF000000");
        assertContains(first, "triggers.clamp=00000000,3F800000");
        assertContains(first, "gamepad.null=none");
        assertContains(first, "gamepad.virtual=0,1,1,1,0,1");
        assertContains(first, "gamepad.filtered=1,0,0,0");
        assertContains(first, "gamepad.string={IsConnected:True}");
        assertContains(first, "buttons.string={Buttons:A Y Back}");
        assertContains(first, "buttons.hash=1");
        assertContains(first, "dpad.string={DPad:Up Right}");
        assertContains(first, "dpad.hash=2147483647");
        assertContains(first, "touch.previous.none=0,-1,0");
        assertContains(first, "touch.equals=1,0");
        assertContains(first, "touch.hash=2139095045");
        assertContains(first, "touch.string={Position:{X:1 Y:2}}");
        assertContains(first, "touch.collection.clone=5");
        assertContains(first, "touch.collection.contains=0");
        assertContains(first, "touch.collection.oob=IndexOutOfBoundsException");
    }

    private static void assertContains(List<String> observations, String expected) {
        assertTrue(observations.contains(expected), () -> "Missing observation: " + expected
                + System.lineSeparator() + String.join(System.lineSeparator(), observations));
    }
}
