package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeAudio;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Root owner for XACT settings, banks, categories, and cues. */
@SuppressWarnings("this-escape")
public class AudioEngine implements AutoCloseable {
    public static final int ContentVersion = 39;

    private final Object lock = new Object();
    private final List<Object> dependents = new ArrayList<>();
    private final List<Long> categoryHandles = new ArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposingListeners =
            new CopyOnWriteArrayList<>();
    private long handle;
    private boolean disposingRaised;

    public AudioEngine(String settingsFile) {
        this(settingsFile, Duration.ofMillis(250), "");
    }

    public AudioEngine(String settingsFile, Duration lookAheadTime, String rendererId) {
        if (settingsFile == null || settingsFile.isEmpty()) throw new NullPointerException("settingsFile");
        Objects.requireNonNull(lookAheadTime, "lookAheadTime");
        validateSettingsFile(settingsFile);
        handle = NativeAudio.createAudioEngine(
                Path.of(settingsFile).toAbsolutePath().toString(), lookAheadTime, rendererId);
        NativeAudio.registerOwner(this);
    }

    public final void addDisposingListener(EventHandler<EventArgs> listener) {
        requireHandle();
        disposingListeners.add(Objects.requireNonNull(listener, "listener"));
    }
    public final void removeDisposingListener(EventHandler<EventArgs> listener) {
        disposingListeners.remove(listener);
    }

    public final AudioCategory GetCategory(String name) {
        if (name == null || name.isEmpty()) throw new NullPointerException("name");
        synchronized (lock) {
            long category = NativeAudio.getAudioCategory(requireHandle(), name);
            categoryHandles.add(category);
            return new AudioCategory(this, category, name);
        }
    }

    public final float GetGlobalVariable(String name) {
        if (name == null || name.isEmpty()) throw new NullPointerException("name");
        return NativeAudio.getXactVariable(requireHandle(), 0L, name, false, 0.0f);
    }

    public final void SetGlobalVariable(String name, float value) {
        if (name == null || name.isEmpty()) throw new NullPointerException("name");
        NativeAudio.getXactVariable(requireHandle(), 0L, name, true, value);
    }

    public final void Update() { NativeAudio.updateAudioEngine(requireHandle()); }

    public final boolean getIsDisposed() { synchronized (lock) { return handle == 0L; } }

    public final List<RendererDetail> getRendererDetails() {
        synchronized (lock) {
            long selected = requireHandle();
            int count = NativeAudio.getRendererCount(selected);
            if (count == 0) return List.of();
            List<RendererDetail> details = new ArrayList<>(count);
            for (int i = 0; i < count; i++) {
                details.add(new RendererDetail(
                        NativeAudio.getRendererString(selected, i, false),
                        NativeAudio.getRendererString(selected, i, true)));
            }
            return List.copyOf(details);
        }
    }

    protected void Dispose(boolean disposing) {
        List<Object> closingDependents;
        List<Long> closingCategories;
        long closingHandle;
        boolean raiseDisposing = false;
        Throwable failure = null;
        synchronized (lock) {
            if (handle == 0L) return;
            closingDependents = new ArrayList<>(dependents);
            closingCategories = new ArrayList<>(categoryHandles);
            closingHandle = handle;
        }
        for (int i = closingDependents.size() - 1; i >= 0; i--) {
            try { closeDependent(closingDependents.get(i)); }
            catch (Throwable exception) { failure = append(failure, exception); }
        }
        for (int i = closingCategories.size() - 1; i >= 0; i--) {
            long category = closingCategories.get(i);
            try {
                NativeAudio.destroyCategory(category);
                synchronized (lock) { categoryHandles.remove(category); }
            }
            catch (Throwable exception) { failure = append(failure, exception); }
        }
        if (failure == null) {
            try {
                NativeAudio.destroyAudioEngine(closingHandle);
                synchronized (lock) {
                    handle = 0L;
                    dependents.clear();
                    categoryHandles.clear();
                    raiseDisposing = disposing && !disposingRaised;
                    if (raiseDisposing) disposingRaised = true;
                }
                NativeAudio.unregisterOwner(this);
            } catch (Throwable exception) { failure = append(failure, exception); }
        }
        if (raiseDisposing) {
            for (EventHandler<EventArgs> listener : disposingListeners) {
                try { listener.invoke(this, EventArgs.Empty); }
                catch (Throwable exception) { failure = append(failure, exception); }
            }
        }
        if (getIsDisposed()) disposingListeners.clear();
        rethrow(failure);
    }

    public final void Dispose() { Dispose(true); }
    @Override public final void close() { Dispose(); }

    final long requireHandle() {
        synchronized (lock) {
            if (handle == 0L) throw new IllegalStateException("AudioEngine is disposed");
            return handle;
        }
    }

    final void register(Object dependent) {
        synchronized (lock) { requireHandle(); dependents.add(dependent); }
    }

    final void unregister(Object dependent) {
        synchronized (lock) { dependents.remove(dependent); }
    }

    private static void validateSettingsFile(String value) {
        Path path = Path.of(value).toAbsolutePath();
        byte[] prefix;
        boolean hasPayload;
        try {
            byte[] bytes = Files.readAllBytes(path);
            prefix = bytes.length < 4 ? bytes : java.util.Arrays.copyOf(bytes, 4);
            hasPayload = bytes.length > 4;
        } catch (IOException exception) {
            throw new IllegalArgumentException("Could not read XACT settings file", exception);
        }
        if (prefix.length != 4 || !hasPayload
                || prefix[0] != 'X' || prefix[1] != 'G'
                || prefix[2] != 'S' || prefix[3] != 'F') {
            throw new IllegalArgumentException("settingsFile is not an XACT settings bank");
        }
    }

    private static void closeDependent(Object dependent) {
        if (dependent instanceof Cue cue) cue.close();
        else if (dependent instanceof SoundBank soundBank) soundBank.close();
        else if (dependent instanceof WaveBank waveBank) waveBank.close();
        else throw new IllegalStateException("Unknown AudioEngine dependency");
    }

    static Throwable append(Throwable failure, Throwable next) {
        if (failure == null) return next;
        failure.addSuppressed(next);
        return failure;
    }

    static void rethrow(Throwable failure) {
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        if (failure != null) throw new RuntimeException(failure);
    }
}
