package Microsoft.Xna.Framework.Media;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.junit.jupiter.api.io.TempDir;
import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.CnaNativeException;
import org.openeggbert.cna.internal.NativeMedia;

import java.time.Duration;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Path;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.*;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class MediaVideoNativeIntegrationTests {

    @Test
    void HeadlessMediaLibraryExposesStableReadOnlyCollectionsAndDisposesChildren() {
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            List<MediaSource> sources = MediaSource.GetAvailableMediaSources();
            assertNotNull(sources);
            assertThrows(UnsupportedOperationException.class,
                    () -> sources.add(sources.isEmpty() ? null : sources.get(0)));

            MediaLibrary library = new MediaLibrary();
            SongCollection songs = library.getSongs();
            assertSame(songs, library.getSongs());
            assertSame(library.getAlbums(), library.getAlbums());
            assertSame(library.getArtists(), library.getArtists());
            assertSame(library.getGenres(), library.getGenres());
            assertSame(library.getPlaylists(), library.getPlaylists());
            assertSame(library.getPictures(), library.getPictures());
            assertSame(library.getSavedPictures(), library.getSavedPictures());
            assertSame(library.getMediaSource(), library.getMediaSource());
            assertEquals(MediaSourceType.LocalDevice,
                    library.getMediaSource().getMediaSourceType());

            assertCollection(songs.getCount(), songs);
            assertCollection(library.getAlbums().getCount(), library.getAlbums());
            assertCollection(library.getArtists().getCount(), library.getArtists());
            assertCollection(library.getGenres().getCount(), library.getGenres());
            assertCollection(library.getPlaylists().getCount(), library.getPlaylists());
            assertCollection(library.getPictures().getCount(), library.getPictures());
            assertCollection(library.getSavedPictures().getCount(), library.getSavedPictures());
            assertThrows(IndexOutOfBoundsException.class, () -> songs.get(-1));
            assertThrows(IndexOutOfBoundsException.class, () -> songs.get(songs.getCount()));

            PictureAlbum root = library.getRootPictureAlbum();
            assertSame(root, library.getRootPictureAlbum());
            library.close();
            assertTrue(library.getIsDisposed());
            assertTrue(songs.getIsDisposed());
            assertThrows(IllegalStateException.class, songs::getCount);
            library.close();
        }
    }

    @Test
    void MediaPlayerGlobalStateQueueVisualizationAndGameRecreationAreNativeBacked() {
        MediaQueue firstQueue;
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            assertEquals(MediaState.Stopped, MediaPlayer.getState());
            assertEquals(Duration.ZERO, MediaPlayer.getPlayPosition());
            firstQueue = MediaPlayer.getQueue();
            assertSame(firstQueue, MediaPlayer.getQueue());
            assertTrue(firstQueue.getCount() >= 0);
            if (firstQueue.getCount() == 0) {
                assertEquals(-1, firstQueue.getActiveSongIndex());
                assertNull(firstQueue.getActiveSong());
                assertThrows(IndexOutOfBoundsException.class, () -> firstQueue.get(0));
            }

            MediaPlayer.setVolume(-1f);
            assertEquals(0f, MediaPlayer.getVolume());
            MediaPlayer.setVolume(2f);
            assertEquals(1f, MediaPlayer.getVolume());
            MediaPlayer.setVolume(Float.NaN);
            assertTrue(Float.isNaN(MediaPlayer.getVolume()));
            MediaPlayer.setVolume(1f);

            MediaPlayer.setIsMuted(true);
            MediaPlayer.setIsRepeating(true);
            MediaPlayer.setIsShuffled(true);
            assertTrue(MediaPlayer.getIsMuted());
            assertTrue(MediaPlayer.getIsRepeating());
            assertTrue(MediaPlayer.getIsShuffled());
            MediaPlayer.setIsMuted(false);
            MediaPlayer.setIsRepeating(false);
            MediaPlayer.setIsShuffled(false);

            VisualizationData data = new VisualizationData();
            MediaPlayer.GetVisualizationData(data);
            assertEquals(256, data.getFrequencies().size());
            assertEquals(256, data.getSamples().size());

            MediaPlayer.Pause();
            MediaPlayer.Resume();
            MediaPlayer.Stop();
            MediaPlayer.MoveNext();
            MediaPlayer.MovePrevious();
        }

        assertThrows(IllegalStateException.class, firstQueue::getCount);
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            assertNotSame(firstQueue, MediaPlayer.getQueue());
            assertEquals(MediaState.Stopped, MediaPlayer.getState());
        }
    }

    @Test
    void SongFromUriDrivesPropertiesPlaybackQueueIdentityAndRelease(@TempDir Path directory)
            throws IOException {
        Path wave = directory.resolve("tone.wav");
        Files.write(wave, wavPcm16Silence(8_000, 160));
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            Song song = Song.FromUri("tone", wave.toUri());
            assertEquals("tone", song.getName());
            assertFalse(song.getIsDisposed());
            assertNotNull(song.getDuration());
            assertTrue(song.getPlayCount() >= 0);
            assertTrue(song.getTrackNumber() >= 0);
            assertSame(song.getAlbum(), song.getAlbum());
            assertSame(song.getArtist(), song.getArtist());
            assertSame(song.getGenre(), song.getGenre());

            MediaQueue queue = MediaPlayer.getQueue();
            MediaPlayer.Play(song);
            assertSame(queue, MediaPlayer.getQueue());
            assertEquals(1, queue.getCount());
            assertEquals(0, queue.getActiveSongIndex());
            assertSame(queue.get(0), queue.get(0));
            assertSame(queue.get(0), queue.getActiveSong());
            MediaPlayer.MoveNext();
            MediaPlayer.MovePrevious();
            MediaPlayer.Stop();

            song.close();
            song.close();
            assertTrue(song.getIsDisposed());
        }
    }

    @Test
    void NativeMediaEventsAreDeferredOrderedMutableAndExceptionContained() {
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            AtomicInteger activeCalls = new AtomicInteger();
            AtomicInteger stateCalls = new AtomicInteger();
            EventHandler<EventArgs> duplicate = (sender, args) -> {
                assertNull(sender);
                assertSame(EventArgs.Empty, args);
                activeCalls.incrementAndGet();
            };
            AtomicReference<EventHandler<EventArgs>> selfRemoving = new AtomicReference<>();
            selfRemoving.set((sender, args) -> {
                stateCalls.incrementAndGet();
                MediaPlayer.removeMediaStateChangedListener(selfRemoving.get());
            });
            MediaPlayer.addActiveSongChangedListener(duplicate);
            MediaPlayer.addActiveSongChangedListener(duplicate);
            MediaPlayer.addMediaStateChangedListener(selfRemoving.get());

            NativeMedia.raiseEventForQualification(0);
            NativeMedia.raiseEventForQualification(1);
            assertEquals(0, activeCalls.get(), "native callbacks must only enqueue");
            assertEquals(0, stateCalls.get(), "native callbacks must only enqueue");
            game.RunOneFrame();
            assertEquals(2, activeCalls.get());
            assertEquals(1, stateCalls.get());

            NativeMedia.raiseEventForQualification(1);
            game.RunOneFrame();
            assertEquals(1, stateCalls.get(), "self-removal affects the next dispatch");

            MediaPlayer.removeActiveSongChangedListener(duplicate);
            NativeMedia.raiseEventForQualification(0);
            game.RunOneFrame();
            assertEquals(3, activeCalls.get(), "one duplicate registration remains");
            MediaPlayer.removeActiveSongChangedListener(duplicate);

            AtomicInteger callbackCycles = new AtomicInteger();
            EventHandler<EventArgs> cycleListener =
                    (sender, args) -> callbackCycles.incrementAndGet();
            MediaPlayer.addActiveSongChangedListener(cycleListener);
            for (int cycle = 0; cycle < 100; cycle++) {
                NativeMedia.raiseEventForQualification(0);
            }
            assertEquals(0, callbackCycles.get());
            game.RunOneFrame();
            assertEquals(100, callbackCycles.get());
            MediaPlayer.removeActiveSongChangedListener(cycleListener);

            VideoPlayer closeDuringCallback = new VideoPlayer();
            EventHandler<EventArgs> closing = (sender, args) -> closeDuringCallback.close();
            MediaPlayer.addMediaStateChangedListener(closing);
            NativeMedia.raiseEventForQualification(1);
            game.RunOneFrame();
            assertTrue(closeDuringCallback.getIsDisposed());
            MediaPlayer.removeMediaStateChangedListener(closing);

            AtomicInteger afterThrow = new AtomicInteger();
            EventHandler<EventArgs> throwing = (sender, args) -> {
                throw new IllegalStateException("media-event-failure");
            };
            EventHandler<EventArgs> later = (sender, args) -> afterThrow.incrementAndGet();
            MediaPlayer.addMediaStateChangedListener(throwing);
            MediaPlayer.addMediaStateChangedListener(later);
            NativeMedia.raiseEventForQualification(1);
            CnaNativeException failure = assertThrows(
                    CnaNativeException.class, game::RunOneFrame);
            assertEquals(9, failure.getResult());
            assertTrue(failure.getMessage().contains("media-event-failure"));
            assertEquals(1, afterThrow.get(), "one handler must not suppress later handlers");
            MediaPlayer.removeMediaStateChangedListener(throwing);
            MediaPlayer.removeMediaStateChangedListener(later);
        }
    }

    @Test
    void ThrowingUpdateSkipsQueuedMediaDeliveryAndShutdownDiscardsTheEvent() {
        AtomicInteger calls = new AtomicInteger();
        EventHandler<EventArgs> listener = (sender, args) -> calls.incrementAndGet();
        FailingUpdateGame game = new FailingUpdateGame();
        game.RunOneFrame();
        MediaPlayer.addActiveSongChangedListener(listener);
        NativeMedia.raiseEventForQualification(0);
        game.failNextUpdate();
        CnaNativeException failure = assertThrows(CnaNativeException.class, game::RunOneFrame);
        assertEquals(9, failure.getResult());
        assertEquals(0, calls.get());
        game.close();

        try (Game nextGame = preparedGame()) {
            assertNotNull(nextGame.getWindow());
            NativeMedia.dispatchPendingEvents();
            assertEquals(0, calls.get());
            NativeMedia.raiseEventForQualification(0);
            assertEquals(0, calls.get(), "the recreated Game still defers native callbacks");
            nextGame.RunOneFrame();
            assertEquals(1, calls.get(), "the process-global subscription survives Game recreation");
        } finally {
            MediaPlayer.removeActiveSongChangedListener(listener);
        }
    }

    @Test
    void VideoPlayerCachesXnaSettingsAfterDisposalAndOwnsNoFrameTexture() {
        try (ActionGame game = new ActionGame()) {
            game.RunOneFrame();
            VideoPlayer player = new VideoPlayer();
            assertEquals(MediaState.Stopped, player.getState());
            assertNull(player.getVideo());
            assertEquals(Duration.ZERO, player.getPlayPosition());
            assertFalse(player.getIsLooped());
            assertFalse(player.getIsMuted());
            assertEquals(1f, player.getVolume());
            assertThrows(IllegalStateException.class, player::GetTexture);
            assertThrows(NullPointerException.class, () -> player.Play(null));
            assertThrows(IllegalArgumentException.class, () -> player.setVolume(-0.01f));
            assertThrows(IllegalArgumentException.class, () -> player.setVolume(1.01f));

            player.setVolume(Float.NaN);
            assertTrue(Float.isNaN(player.getVolume()));
            player.setIsLooped(true);
            player.setIsMuted(true);
            player.setVolume(0.25f);

            AtomicReference<Video> createdVideo = new AtomicReference<>();
            game.onNextUpdate(() -> createdVideo.set(FacadeFactory.createVideo(
                    game.getGraphicsDevice(), "__cna_java_missing_video__.mp4",
                    1234, 320, 180, 29.97f,
                    VideoSoundtrackType.MusicAndDialog.ordinal())));
            game.RunOneFrame();
            Video video = createdVideo.get();
            assertNotNull(video);
            assertEquals(Duration.ofMillis(1234), video.getDuration());
            assertEquals(320, video.getWidth());
            assertEquals(180, video.getHeight());
            assertEquals(29.97f, video.getFramesPerSecond());
            assertEquals(VideoSoundtrackType.MusicAndDialog,
                    video.getVideoSoundtrackType());

            // Explicit metadata creation is native-verified. Depending on backend configuration,
            // the missing file is either refused by Play or accepted with no available frame.
            boolean played;
            try {
                player.Play(video);
                played = true;
            } catch (CnaNativeException backendFailure) {
                played = false;
                assertNull(player.getVideo(), "a failed Play must not replace cached Video");
            }
            if (played) {
                assertSame(video, player.getVideo());
                try {
                    assertNull(player.GetTexture(), "HEADLESS must not fabricate a decoded frame");
                } catch (CnaNativeException backendFailure) {
                    // A backend may surface frame unavailability as a native failure.
                }
                player.Stop();
            }
            NativeMedia.closeVideo(video);

            player.close();
            player.close();
            assertTrue(player.getIsDisposed());
            assertTrue(player.getIsLooped());
            assertTrue(player.getIsMuted());
            assertEquals(0.25f, player.getVolume());
            assertThrows(IllegalStateException.class, player::getState);
            assertThrows(IllegalStateException.class, player::Pause);
        }
    }

    @Test
    void MediaAndVideoOwnershipStressSurvivesRepeatedGameLifetimes() {
        final int cycles = 40;
        for (int iteration = 0; iteration < cycles; iteration++) {
            try (Game game = preparedGame()) {
                assertNotNull(game.getWindow());
                MediaLibrary library = new MediaLibrary();
                SongCollection songs = library.getSongs();
                songs.getCount();
                library.close();
                library.close();
                assertTrue(songs.getIsDisposed());

                VideoPlayer player = new VideoPlayer();
                player.setIsLooped((iteration & 1) == 0);
                player.setIsMuted((iteration & 1) != 0);
                player.setVolume(iteration / (float)cycles);
                player.close();
                player.close();
                assertTrue(player.getIsDisposed());
            }
        }

        // A final fresh game proves no process-global queue/player handle retained a dead parent.
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            assertEquals(MediaState.Stopped, MediaPlayer.getState());
            assertNotNull(MediaPlayer.getQueue());
            try (VideoPlayer player = new VideoPlayer()) {
                assertEquals(MediaState.Stopped, player.getState());
            }
        }
    }

    @Test
    void RefusedWrongThreadMediaAndVideoDestructionRemainRetryable() {
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            MediaLibrary library = new MediaLibrary();
            VideoPlayer player = new VideoPlayer();
            AtomicReference<Throwable> libraryFailure = new AtomicReference<>();
            AtomicReference<Throwable> playerFailure = new AtomicReference<>();

            runOnThread(() -> {
                try {
                    library.close();
                } catch (Throwable failure) {
                    libraryFailure.set(failure);
                }
            }, "cna-java-media-wrong-thread-release");
            runOnThread(() -> {
                try {
                    player.close();
                } catch (Throwable failure) {
                    playerFailure.set(failure);
                }
            }, "cna-java-video-wrong-thread-release");

            CnaNativeException refusedLibrary = assertInstanceOf(
                    CnaNativeException.class, libraryFailure.get());
            CnaNativeException refusedPlayer = assertInstanceOf(
                    CnaNativeException.class, playerFailure.get());
            assertEquals(8, refusedLibrary.getResult());
            assertEquals(8, refusedPlayer.getResult());
            assertFalse(library.getIsDisposed());
            assertFalse(player.getIsDisposed());

            library.close();
            player.close();
            assertTrue(library.getIsDisposed());
            assertTrue(player.getIsDisposed());
        }
    }

    private static Game preparedGame() {
        Game game = new Game();
        new GraphicsDeviceManager(game);
        game.RunOneFrame();
        return game;
    }

    private static final class ActionGame extends Game {
        private Runnable nextUpdate;

        private ActionGame() {
            new GraphicsDeviceManager(this);
        }

        private void onNextUpdate(Runnable action) {
            nextUpdate = action;
        }

        @Override protected void Update(Microsoft.Xna.Framework.GameTime gameTime) {
            Runnable action = nextUpdate;
            nextUpdate = null;
            if (action != null) action.run();
            super.Update(gameTime);
        }
    }

    private static final class FailingUpdateGame extends Game {
        private boolean fail;

        private FailingUpdateGame() {
            new GraphicsDeviceManager(this);
        }

        private void failNextUpdate() {
            fail = true;
        }

        @Override protected void Update(Microsoft.Xna.Framework.GameTime gameTime) {
            if (fail) throw new IllegalStateException("throwing-update");
            super.Update(gameTime);
        }
    }

    private static byte[] wavPcm16Silence(int sampleRate, int sampleCount) {
        int dataBytes = sampleCount * 2;
        ByteBuffer buffer = ByteBuffer.allocate(44 + dataBytes).order(ByteOrder.LITTLE_ENDIAN);
        buffer.put(new byte[] {'R', 'I', 'F', 'F'}).putInt(36 + dataBytes);
        buffer.put(new byte[] {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
        buffer.putInt(16).putShort((short)1).putShort((short)1);
        buffer.putInt(sampleRate).putInt(sampleRate * 2).putShort((short)2).putShort((short)16);
        buffer.put(new byte[] {'d', 'a', 't', 'a'}).putInt(dataBytes);
        return buffer.array();
    }

    private static void runOnThread(Runnable operation, String name) {
        Thread thread = new Thread(operation, name);
        thread.start();
        try {
            thread.join();
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            throw new IllegalStateException("Interrupted while waiting for release probe", exception);
        }
    }

    private static void assertCollection(int count, Iterable<?> collection) {
        assertTrue(count >= 0);
        int iterated = 0;
        for (Object value : collection) {
            assertNotNull(value);
            iterated++;
        }
        assertEquals(count, iterated);
    }
}
