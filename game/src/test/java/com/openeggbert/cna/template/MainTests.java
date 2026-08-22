package com.openeggbert.cna.template;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

final class MainTests {

    @Test
    void ParsesDeterministicFrameModes() {
        assertEquals(0, Main.parseFrameLimit(new String[] {}));
        assertEquals(60, Main.parseFrameLimit(new String[] {"--smoke-test"}));
        assertEquals(600, Main.parseFrameLimit(new String[] {"--stability-test"}));
        assertEquals(17, Main.parseFrameLimit(new String[] {"--frames", "17"}));
        assertEquals(23, Main.parseFrameLimit(new String[] {"--frames=23"}));
    }

    @Test
    void RejectsInvalidArguments() {
        assertThrows(IllegalArgumentException.class,
                () -> Main.parseFrameLimit(new String[] {"--frames", "0"}));
        assertThrows(IllegalArgumentException.class,
                () -> Main.parseFrameLimit(new String[] {"--frames"}));
        assertThrows(IllegalArgumentException.class,
                () -> Main.parseFrameLimit(new String[] {"--pretend-web-works"}));
    }

    @Test
    void ConfiguresMappedGameWindowBeforeNativeStartup() {
        try (HelloGame game = new HelloGame(1)) {
            assertEquals("CNA-Java: HelloGame", game.getWindow().getTitle());
        }
    }
}
