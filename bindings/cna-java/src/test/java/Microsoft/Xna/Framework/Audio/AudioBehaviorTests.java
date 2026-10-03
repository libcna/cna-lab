package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.Vector3;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Files;
import java.nio.file.Path;
import java.time.Duration;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

final class AudioBehaviorTests {
    @Test
    void enumIdentitiesAndValueTypesMatchXnaDefaults() {
        assertEquals(1, AudioChannels.Mono.getValue());
        assertEquals(2, AudioChannels.Stereo.getValue());
        assertEquals(0, SoundState.Playing.ordinal());
        assertEquals(1, SoundState.Paused.ordinal());
        assertEquals(2, SoundState.Stopped.ordinal());
        assertEquals(0, MicrophoneState.Started.ordinal());
        assertEquals(1, MicrophoneState.Stopped.ordinal());

        RendererDetail empty = new RendererDetail();
        assertNull(empty.getFriendlyName());
        assertNull(empty.getRendererId());
        assertEquals(empty, new RendererDetail(empty));
        assertEquals("Microsoft.Xna.Framework.Audio.RendererDetail", empty.toString());

        AudioCategory category = new AudioCategory();
        assertNull(category.getName());
        assertEquals("", category.toString());
        assertEquals(category, new AudioCategory(category));
        assertNotEquals(category, null);
        assertThrows(IllegalStateException.class, category::Pause);
    }

    @Test
    void listenerAndEmitterUseXnaDefaultsAndDefensiveVectorCopies() {
        AudioListener listener = new AudioListener();
        assertEquals(Vector3.getForward(), listener.getForward());
        assertEquals(Vector3.getUp(), listener.getUp());
        assertEquals(Vector3.getZero(), listener.getPosition());
        assertEquals(Vector3.getZero(), listener.getVelocity());

        Vector3 position = new Vector3(1.0f, 2.0f, 3.0f);
        listener.setPosition(position);
        position.X = 91.0f;
        assertEquals(new Vector3(1.0f, 2.0f, 3.0f), listener.getPosition());
        Vector3 returned = listener.getPosition();
        assertNotSame(returned, listener.getPosition());
        returned.Y = 77.0f;
        assertEquals(2.0f, listener.getPosition().Y);

        AudioEmitter emitter = new AudioEmitter();
        assertEquals(1.0f, emitter.getDopplerScale());
        assertEquals(Vector3.getForward(), emitter.getForward());
        emitter.setDopplerScale(Float.NaN);
        assertTrue(Float.isNaN(emitter.getDopplerScale()));
        assertThrows(IllegalArgumentException.class, () -> emitter.setDopplerScale(-0.01f));
        assertThrows(NullPointerException.class, () -> emitter.setForward(null));
    }

    @Test
    void sampleArithmeticPreservesObservedXnaFloatBehavior() {
        assertEquals(88_198, SoundEffect.GetSampleSizeInBytes(
                Duration.ofSeconds(1), 44_100, AudioChannels.Mono));
        assertEquals(176_400, SoundEffect.GetSampleSizeInBytes(
                Duration.ofSeconds(1), 44_100, AudioChannels.Stereo));
        assertEquals(Duration.ofSeconds(1), SoundEffect.GetSampleDuration(
                88_200, 44_100, AudioChannels.Mono));
        assertEquals(Duration.ZERO, SoundEffect.GetSampleDuration(
                0, 8_000, AudioChannels.Mono));
        assertEquals(0, SoundEffect.GetSampleSizeInBytes(
                Duration.ZERO, 8_000, AudioChannels.Stereo));

        assertThrows(IllegalArgumentException.class, () -> SoundEffect.GetSampleDuration(
                -1, 44_100, AudioChannels.Mono));
        assertThrows(IllegalArgumentException.class, () -> SoundEffect.GetSampleDuration(
                2, 7_999, AudioChannels.Mono));
        assertThrows(NullPointerException.class, () -> SoundEffect.GetSampleDuration(
                2, 44_100, null));
        assertThrows(IllegalArgumentException.class, () -> SoundEffect.GetSampleSizeInBytes(
                Duration.ofNanos(-1), 44_100, AudioChannels.Mono));
        assertThrows(IllegalArgumentException.class, () -> SoundEffect.setMasterVolume(Float.NaN));
        assertThrows(IllegalArgumentException.class, () -> SoundEffect.setDistanceScale(Float.NaN));
        assertThrows(IllegalArgumentException.class, () -> SoundEffect.setDopplerScale(Float.NaN));
        assertThrows(IllegalArgumentException.class, () -> SoundEffect.setSpeedOfSound(Float.NaN));
    }

    @Test
    void strictConstructorValidationPrecedesNativeRuntimeAccess(@TempDir Path root)
            throws Exception {
        assertThrows(IllegalArgumentException.class,
                () -> new SoundEffect(null, 44_100, AudioChannels.Mono));
        assertThrows(IllegalArgumentException.class,
                () -> new SoundEffect(new int[0], 44_100, AudioChannels.Mono));
        assertThrows(IllegalArgumentException.class,
                () -> new SoundEffect(new int[] {0, 0}, 7_999, AudioChannels.Mono));
        assertThrows(IllegalArgumentException.class,
                () -> new SoundEffect(new int[] {0, 0, 0}, 44_100, AudioChannels.Mono));
        assertThrows(IllegalArgumentException.class,
                () -> new SoundEffect(new int[] {0, 256}, 44_100, AudioChannels.Mono));
        assertThrows(IllegalArgumentException.class,
                () -> new DynamicSoundEffectInstance(48_001, AudioChannels.Mono));

        Path invalid = root.resolve("invalid.xgs");
        Files.write(invalid, new byte[] {'N', 'O', 'P', 'E'});
        assertThrows(IllegalArgumentException.class,
                () -> new AudioEngine(invalid.toString()));
        Path signatureOnly = root.resolve("signature-only.xgs");
        Files.write(signatureOnly, new byte[] {'X', 'G', 'S', 'F'});
        assertThrows(IllegalArgumentException.class,
                () -> new AudioEngine(signatureOnly.toString()));
        assertThrows(NullPointerException.class, () -> new AudioEngine(null));
        assertThrows(NullPointerException.class, () -> new SoundBank(null, "missing.xsb"));
        assertTrue(new NoAudioHardwareException() instanceof RuntimeException);
    }
}
