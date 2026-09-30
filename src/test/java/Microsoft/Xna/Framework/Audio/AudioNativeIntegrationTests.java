package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.FrameworkDispatcher;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import System.IO.Stream;
import org.openeggbert.cna.internal.CnaNativeException;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.junit.jupiter.api.io.TempDir;

import java.time.Duration;
import java.lang.reflect.Method;
import java.io.ByteArrayInputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class AudioNativeIntegrationTests {
    @Test
    void nullAudioRuntimeExercisesSoundDynamicMicrophoneAndShutdownOwnership(@TempDir Path root)
            throws Exception {
        Path truncatedSettings = root.resolve("truncated.xgs");
        Files.write(truncatedSettings, new byte[] {'X', 'G', 'S', 'F', 0});
        ShutdownGame game = new ShutdownGame(truncatedSettings.toString());
        try {
            game.RunOneFrame();
            assertTrue(game.completed);
            assertFalse(game.liveAtShutdown.getIsDisposed());
        } finally {
            game.close();
        }
        assertTrue(game.liveAtShutdown.getIsDisposed());
    }

    private static final class ShutdownGame extends Game {
        private boolean completed;
        private SoundEffect liveAtShutdown;
        private final String truncatedSettings;

        private ShutdownGame(String truncatedSettings) {
            this.truncatedSettings = truncatedSettings;
            new GraphicsDeviceManager(this);
        }

        @Override
        protected void Update(GameTime gameTime) {
            assertThrows(CnaNativeException.class,
                    () -> new AudioEngine(truncatedSettings));

            float master = SoundEffect.getMasterVolume();
            float distance = SoundEffect.getDistanceScale();
            float doppler = SoundEffect.getDopplerScale();
            float speed = SoundEffect.getSpeedOfSound();
            try {
                SoundEffect.setMasterVolume(0.5f);
                SoundEffect.setDistanceScale(2.0f);
                SoundEffect.setDopplerScale(0.25f);
                SoundEffect.setSpeedOfSound(300.0f);
                assertEquals(0.5f, SoundEffect.getMasterVolume());
                assertEquals(2.0f, SoundEffect.getDistanceScale());
                assertEquals(0.25f, SoundEffect.getDopplerScale());
                assertEquals(300.0f, SoundEffect.getSpeedOfSound());
            } finally {
                SoundEffect.setMasterVolume(master);
                SoundEffect.setDistanceScale(distance);
                SoundEffect.setDopplerScale(doppler);
                SoundEffect.setSpeedOfSound(speed);
            }

            int[] pcm = new int[1_764];
            SoundEffectInstance disposedWithParent;
            try (SoundEffect effect = new SoundEffect(pcm, 44_100, AudioChannels.Mono)) {
                assertEquals(Duration.ofMillis(20), effect.getDuration());
                effect.setName("native-null-audio");
                assertEquals("native-null-audio", effect.getName());
                effect.setName("");
                assertEquals("", effect.getName());
                // Play answers whether a voice was started: false with no mixer (NULL audio), true
                // with one (SDL3). Either way the two overloads agree.
                boolean played = effect.Play();
                assertEquals(played, effect.Play(0.5f, -0.25f, 0.75f));

                SoundEffectInstance first = effect.CreateInstance();
                disposedWithParent = first;
                SoundEffectInstance second = effect.CreateInstance();
                assertNotSame(first, second);
                first.setVolume(0.5f);
                first.setPitch(-0.25f);
                first.setPan(0.75f);
                first.setIsLooped(true);
                assertEquals(0.5f, first.getVolume());
                assertEquals(-0.25f, first.getPitch());
                assertEquals(0.75f, first.getPan());
                assertTrue(first.getIsLooped());
                assertThrows(IllegalArgumentException.class, () -> first.setVolume(Float.NaN));
                assertThrows(IllegalArgumentException.class, () -> first.setPitch(Float.NaN));
                assertThrows(IllegalArgumentException.class, () -> first.setPan(Float.NaN));
                first.Apply3D(new AudioListener(), new AudioEmitter());
                first.Apply3D(new AudioListener[] {new AudioListener()}, new AudioEmitter());
                // CNA ABI 0.9.0 accepts any positive listener count. That is the
                // reference behaviour -- XNA's Apply3D(AudioListener[], AudioEmitter)
                // imposes no count restriction -- so Java must not reintroduce one.
                // CNA's mixer has a single stereo gain pair, so the listener nearest the
                // emitter decides the applied attenuation, pan and Doppler.
                first.Apply3D(new AudioListener[] {new AudioListener(), new AudioListener()},
                        new AudioEmitter());
                assertThrows(CnaNativeException.class,
                        () -> first.Apply3D(new AudioListener[0], new AudioEmitter()));
                first.Play();
                first.Pause();
                first.Resume();
                first.Stop(false);
                first.Stop();
                assertEquals(SoundState.Stopped, first.getState());
                second.close();
                assertTrue(second.getIsDisposed());
                second.close();
            }
            assertTrue(disposedWithParent.getIsDisposed());
            assertThrows(IllegalStateException.class, disposedWithParent::Play);

            try (SoundEffect encoded = SoundEffect.FromStream(
                    new Stream(new ByteArrayInputStream(wavPcm16Silence(8_000, 160))))) {
                assertFalse(encoded.getIsDisposed());
                try (SoundEffectInstance instance = encoded.CreateInstance()) {
                    instance.Play();
                    instance.Stop();
                }
            }

            AtomicInteger callbacks = new AtomicInteger();
            try (DynamicSoundEffectInstance dynamic =
                         new DynamicSoundEffectInstance(8_000, AudioChannels.Mono)) {
                Microsoft.Xna.Framework.EventHandler<EventArgs> listener =
                        (sender, args) -> callbacks.incrementAndGet();
                dynamic.addBufferNeededListener(listener);
                dynamic.SubmitBuffer(new int[320]);
                assertEquals(1, dynamic.getPendingBufferCount());
                dynamic.Play();
                FrameworkDispatcher.Update();
                dynamic.removeBufferNeededListener(listener);

                Microsoft.Xna.Framework.EventHandler<EventArgs> throwing =
                        (sender, args) -> { throw new IllegalStateException("buffer-needed"); };
                dynamic.addBufferNeededListener(throwing);
                invokeBufferNeeded(dynamic);
                IllegalStateException callbackFailure = assertThrows(
                        IllegalStateException.class, dynamic::getPendingBufferCount);
                assertEquals("buffer-needed", callbackFailure.getMessage());
                dynamic.removeBufferNeededListener(throwing);
                assertFalse(dynamic.getIsLooped());
                assertThrows(IllegalArgumentException.class, () -> dynamic.setIsLooped(true));
            }

            DynamicSoundEffectInstance closeDuringCallback =
                    new DynamicSoundEffectInstance(8_000, AudioChannels.Mono);
            closeDuringCallback.addBufferNeededListener(
                    (sender, args) -> closeDuringCallback.close());
            invokeBufferNeeded(closeDuringCallback);
            assertTrue(closeDuringCallback.getIsDisposed());
            invokeBufferNeeded(closeDuringCallback);

            List<Microphone> microphones = Microphone.getAll();
            assertSame(microphones, Microphone.getAll());
            Microphone defaultMicrophone = Microphone.getDefault();
            if (microphones.isEmpty()) assertNull(defaultMicrophone);
            else assertTrue(microphones.stream().anyMatch(value -> value == defaultMicrophone));

            for (int i = 0; i < 25; i++) {
                try (SoundEffect effect = new SoundEffect(pcm, 44_100, AudioChannels.Mono);
                     SoundEffectInstance instance = effect.CreateInstance()) {
                    instance.Stop();
                }
            }

            SoundEffect threadAffine = new SoundEffect(pcm, 44_100, AudioChannels.Mono);
            AtomicReference<Throwable> wrongThreadFailure = new AtomicReference<>();
            Thread wrongThread = new Thread(() -> {
                try { threadAffine.close(); }
                catch (Throwable failure) { wrongThreadFailure.set(failure); }
            }, "cna-java-audio-wrong-thread-release");
            startAndJoin(wrongThread);
            CnaNativeException refused = org.junit.jupiter.api.Assertions.assertInstanceOf(
                    CnaNativeException.class, wrongThreadFailure.get());
            assertEquals(8, refused.getResult());
            assertFalse(threadAffine.getIsDisposed());
            assertEquals(Duration.ofMillis(20), threadAffine.getDuration());
            threadAffine.close();
            assertTrue(threadAffine.getIsDisposed());

            DynamicSoundEffectInstance registered =
                    new DynamicSoundEffectInstance(8_000, AudioChannels.Mono);
            registered.addBufferNeededListener((sender, args) -> { });
            wrongThreadFailure.set(null);
            Thread wrongCallbackThread = new Thread(() -> {
                try { registered.close(); }
                catch (Throwable failure) { wrongThreadFailure.set(failure); }
            }, "cna-java-audio-wrong-thread-callback-release");
            startAndJoin(wrongCallbackThread);
            refused = org.junit.jupiter.api.Assertions.assertInstanceOf(
                    CnaNativeException.class, wrongThreadFailure.get());
            assertEquals(8, refused.getResult());
            assertFalse(registered.getIsDisposed());
            registered.close();
            assertTrue(registered.getIsDisposed());

            liveAtShutdown = new SoundEffect(pcm, 44_100, AudioChannels.Mono);
            completed = true;
            Exit();
        }

        private static void invokeBufferNeeded(DynamicSoundEffectInstance instance) {
            try {
                Method callback = DynamicSoundEffectInstance.class
                        .getDeclaredMethod("nativeBufferNeeded");
                callback.setAccessible(true);
                callback.invoke(instance);
            } catch (ReflectiveOperationException exception) {
                throw new AssertionError("Could not invoke the managed callback boundary", exception);
            }
        }

        private static void startAndJoin(Thread thread) {
            thread.start();
            try { thread.join(5_000L); }
            catch (InterruptedException exception) {
                Thread.currentThread().interrupt();
                throw new AssertionError("Interrupted while waiting for release probe", exception);
            }
            assertFalse(thread.isAlive());
        }

        private static byte[] wavPcm16Silence(int sampleRate, int sampleCount) {
            int dataLength = sampleCount * 2;
            ByteBuffer wav = ByteBuffer.allocate(44 + dataLength).order(ByteOrder.LITTLE_ENDIAN);
            wav.put(new byte[] {'R', 'I', 'F', 'F'}).putInt(36 + dataLength);
            wav.put(new byte[] {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '}).putInt(16);
            wav.putShort((short) 1).putShort((short) 1).putInt(sampleRate);
            wav.putInt(sampleRate * 2).putShort((short) 2).putShort((short) 16);
            wav.put(new byte[] {'d', 'a', 't', 'a'}).putInt(dataLength);
            return wav.array();
        }
    }
}
