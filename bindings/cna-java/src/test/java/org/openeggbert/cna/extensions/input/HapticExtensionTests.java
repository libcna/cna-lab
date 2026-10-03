package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.time.Duration;
import java.util.List;
import java.util.Set;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Force feedback, against the live runtime.
 *
 * <p>This host has no haptic hardware, which is exactly the branch CNA documents: opening
 * succeeds and hands back a closed device on which every operation is a safe no-op reporting
 * that nothing was applied. Nothing here fakes a device to turn a test green.
 *
 * <p>The effect encoding is tested independently of hardware, because it is a pure value
 * transformation and getting a field into the wrong slot would be silent: a periodic magnitude
 * landing in the ramp start would simply produce the wrong force.
 */
final class HapticExtensionTests {

    @Test
    void everyFamilyEncodesOnlyItsOwnFields() {
        long[] constant = ConstantHapticEffect.of(
                HapticDirection.polar(9000), Duration.ofMillis(250), 20000).encode();
        assertEquals(44, constant.length);
        assertEquals(0, constant[HapticEffectLayout.TYPE]);
        assertEquals(HapticDirectionType.Polar.ordinal(),
                constant[HapticEffectLayout.DIRECTION_TYPE]);
        assertEquals(9000, constant[HapticEffectLayout.DIRECTION_FIRST]);
        assertEquals(250, constant[HapticEffectLayout.LENGTH]);
        assertEquals(20000, constant[HapticEffectLayout.LEVEL]);
        assertEquals(0, constant[HapticEffectLayout.MAGNITUDE],
                "a constant effect writes no periodic magnitude");
        assertEquals(0, constant[HapticEffectLayout.RAMP_START]);

        long[] periodic = PeriodicHapticEffect.of(
                PeriodicWave.Triangle, Duration.ofSeconds(1), Duration.ofMillis(40), -100)
                .encode();
        assertEquals(3, periodic[HapticEffectLayout.TYPE], "Triangle is CNA's identity 3");
        assertEquals(40, periodic[HapticEffectLayout.PERIOD]);
        assertEquals(-100, periodic[HapticEffectLayout.MAGNITUDE]);
        assertEquals(0, periodic[HapticEffectLayout.LEVEL],
                "a periodic effect writes no constant level");

        long[] ramp = RampHapticEffect.of(
                HapticDirection.cartesian(1, 2, 3), Duration.ofMillis(500), -50, 50).encode();
        assertEquals(6, ramp[HapticEffectLayout.TYPE]);
        assertEquals(-50, ramp[HapticEffectLayout.RAMP_START]);
        assertEquals(50, ramp[HapticEffectLayout.RAMP_END]);
        assertEquals(3, ramp[HapticEffectLayout.DIRECTION_FIRST + 2]);

        // Every per-axis value has to land in its own slot; a condition with three axes is
        // where an off-by-one in the layout would show up.
        ConditionHapticEffect condition = new ConditionHapticEffect(ConditionKind.Spring,
                Duration.ofMillis(100), Duration.ZERO,
                List.of(new HapticAxisCondition(1, 2, 3, 4, 5, 6),
                        new HapticAxisCondition(11, 12, 13, 14, 15, 16),
                        new HapticAxisCondition(21, 22, 23, 24, 25, 26)),
                HapticTrigger.NONE);
        long[] encoded = condition.encode();
        assertEquals(7, encoded[HapticEffectLayout.TYPE]);
        for (int axis = 0; axis < 3; axis++) {
            assertEquals(1 + axis * 10, encoded[HapticEffectLayout.RIGHT_SATURATION + axis]);
            assertEquals(2 + axis * 10, encoded[HapticEffectLayout.LEFT_SATURATION + axis]);
            assertEquals(3 + axis * 10, encoded[HapticEffectLayout.RIGHT_COEFFICIENT + axis]);
            assertEquals(4 + axis * 10, encoded[HapticEffectLayout.LEFT_COEFFICIENT + axis]);
            assertEquals(5 + axis * 10, encoded[HapticEffectLayout.DEADBAND + axis]);
            assertEquals(6 + axis * 10, encoded[HapticEffectLayout.CENTER + axis]);
        }
        assertEquals(0, encoded[HapticEffectLayout.DIRECTION_TYPE],
                "a condition carries direction per axis, not on the effect");

        long[] leftRight = new LeftRightHapticEffect(Duration.ofMillis(80), 60000, 30000).encode();
        assertEquals(11, leftRight[HapticEffectLayout.TYPE]);
        assertEquals(60000, leftRight[HapticEffectLayout.LARGE_MAGNITUDE]);
        assertEquals(30000, leftRight[HapticEffectLayout.SMALL_MAGNITUDE]);

        CustomHapticEffect custom = new CustomHapticEffect(HapticDirection.NORTH,
                Duration.ofMillis(60), Duration.ZERO, 2, Duration.ofMillis(10),
                List.of(0, 32768, 65535), HapticTrigger.NONE,
                new HapticEnvelope(Duration.ofMillis(5), 100, Duration.ofMillis(7), 200));
        long[] customEncoded = custom.encode();
        assertEquals(12, customEncoded[HapticEffectLayout.TYPE]);
        assertEquals(2, customEncoded[HapticEffectLayout.CUSTOM_CHANNELS]);
        assertEquals(10, customEncoded[HapticEffectLayout.CUSTOM_PERIOD]);
        assertEquals(5, customEncoded[HapticEffectLayout.ATTACK_LENGTH]);
        assertEquals(200, customEncoded[HapticEffectLayout.FADE_LEVEL]);
        assertEquals(List.of(0, 32768, 65535), custom.samples(),
                "an unsigned sample above 32767 survives, which a short could not carry");
    }

    @Test
    void anEffectThatCannotBeCarriedIsRefusedRatherThanTruncated() {
        assertThrows(IllegalArgumentException.class, () -> new CustomHapticEffect(
                HapticDirection.NORTH, Duration.ZERO, Duration.ZERO, 1, Duration.ofMillis(1),
                List.of(65536), HapticTrigger.NONE, HapticEnvelope.NONE));
        assertThrows(IllegalArgumentException.class, () -> new CustomHapticEffect(
                HapticDirection.NORTH, Duration.ZERO, Duration.ZERO, 0, Duration.ofMillis(1),
                List.of(1), HapticTrigger.NONE, HapticEnvelope.NONE));
        assertThrows(IllegalArgumentException.class, () -> new ConditionHapticEffect(
                ConditionKind.Damper, Duration.ZERO, Duration.ZERO,
                List.of(HapticAxisCondition.symmetric(1, 1),
                        HapticAxisCondition.symmetric(1, 1),
                        HapticAxisCondition.symmetric(1, 1),
                        HapticAxisCondition.symmetric(1, 1)),
                HapticTrigger.NONE));
        assertThrows(IllegalArgumentException.class, () -> ConstantHapticEffect.of(
                HapticDirection.NORTH, Duration.ofMillis(-1), 1).encode());
    }

    @Test
    void anInfiniteEffectIsTheAbsenceOfALength() {
        long[] forever = new LeftRightHapticEffect(null, 1, 1).encode();
        assertEquals(HapticEffectLayout.INFINITE_LENGTH, forever[HapticEffectLayout.LENGTH]);
    }

    @Test
    void featureBitsAreCnasOwnAndAnUnknownBitIsIgnored() {
        assertEquals(0x00008000, HapticFeature.Custom.getMask());
        assertEquals(Set.of(), HapticFeature.decode(0));
        assertEquals(Set.of(HapticFeature.Constant, HapticFeature.Sine),
                HapticFeature.decode(0x00000003));
        // A bit CNA adds later must not become a wrong constant.
        assertEquals(Set.of(), HapticFeature.decode(0x40000000));
        assertThrows(UnsupportedOperationException.class,
                () -> HapticFeature.decode(1).add(HapticFeature.Gain));
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void aHostWithNoHapticsHandsBackClosedDevices() {
        try (Game game = new Game()) {
            HapticProbe probe = new HapticProbe(game);
            game.getComponents().add(probe);
            game.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    private static final class HapticProbe extends GameComponent {

        private boolean ran;
        private Throwable failure;

        private HapticProbe(Game game) {
            super(game);
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                probe();
            } catch (Throwable exception) {
                failure = exception;
            }
        }

        private void probe() {
            List<HapticDeviceInfo> devices = HapticDevices.enumerate();
            assertNotNull(devices);
            assertThrows(UnsupportedOperationException.class,
                    () -> devices.add(new HapticDeviceInfo(1, "x")));

            // Opening never fails for absent hardware: CNA hands back a real handle that
            // reports itself closed, and every operation on it is a safe no-op. That is what
            // lets a game write the force-feedback path once and run it anywhere.
            try (HapticDevice mouse = HapticDevices.openFromMouse()) {
                assertEquals(HapticDevices.isMouseHaptic(), mouse.getIsOpen());
                HapticCapabilities capabilities = mouse.getCapabilities();
                assertNotNull(capabilities.Features());
                assertEquals(mouse.getIsOpen(), capabilities.IsOpen());
                if (!capabilities.IsOpen()) {
                    // A closed device reports its maxima as absent, not as zero: zero would
                    // mean "stores no effects", which is a different claim.
                    assertEquals(null, capabilities.MaxEffects());
                    assertEquals(null, capabilities.MaxEffectsPlaying());
                    assertFalse(mouse.InitializeRumble());
                    assertFalse(mouse.PlayRumble(0.5f, Duration.ofMillis(50)));
                    assertFalse(mouse.StopRumble());
                    assertFalse(mouse.setGain(50));
                    assertFalse(mouse.setAutocenter(50));
                    assertFalse(mouse.Pause());
                    assertFalse(mouse.Resume());
                    assertFalse(mouse.StopAllEffects());
                }

                HapticEffect effect = new LeftRightHapticEffect(
                        Duration.ofMillis(100), 40000, 20000);
                assertEquals(capabilities.Features().contains(HapticFeature.LeftRight)
                                && capabilities.IsOpen(),
                        mouse.isEffectSupported(effect));

                try (HapticEffectPlayback playback = mouse.createEffect(effect)) {
                    if (!capabilities.IsOpen()) {
                        assertFalse(playback.getIsStored(),
                                "a closed device cannot have stored the effect");
                    }
                    if (!playback.getIsStored()) {
                        // A device that could not store the effect answers through the value
                        // rather than failing, which is CNA's own behaviour.
                        assertFalse(playback.Run());
                        assertFalse(playback.Stop());
                        assertFalse(playback.getIsPlaying());
                        assertFalse(playback.Update(effect));
                    }
                    assertThrows(IllegalArgumentException.class, () -> playback.Run(0));
                }
            }

            // An effect outlives nothing: closing the device frees every effect it owns, so
            // closing the playback afterwards must not free the same identifier twice.
            HapticDevice device = HapticDevices.openFromJoystick(4242);
            HapticEffectPlayback orphan = device.createEffect(
                    new LeftRightHapticEffect(Duration.ofMillis(10), 1, 1));
            device.close();
            orphan.close();
            orphan.close();
            assertThrows(IllegalStateException.class, orphan::Run);
            assertThrows(IllegalStateException.class, device::getIsOpen);
            device.close();

            try (HapticDevice another = HapticDevices.open(0)) {
                assertThrows(NullPointerException.class,
                        () -> another.isEffectSupported(null));
                assertThrows(NullPointerException.class, () -> another.createEffect(null));
                assertThrows(NullPointerException.class,
                        () -> another.PlayRumble(1.0f, null));
            }
        }
    }
}
