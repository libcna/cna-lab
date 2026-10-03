package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeAudio;

import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Owned XACT sound bank and cue factory. */
@SuppressWarnings("this-escape")
public class SoundBank implements AutoCloseable {
    private final Object lock = new Object();
    private final AudioEngine engine;
    private final List<Cue> cues = new ArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposingListeners =
            new CopyOnWriteArrayList<>();
    private long handle;
    private boolean disposingRaised;

    public SoundBank(AudioEngine audioEngine, String filename) {
        engine = Objects.requireNonNull(audioEngine, "audioEngine");
        if (filename == null || filename.isEmpty()) throw new NullPointerException("filename");
        handle = NativeAudio.createSoundBank(engine.requireHandle(),
                Path.of(filename).toAbsolutePath().toString());
        engine.register(this);
    }

    public final void addDisposingListener(EventHandler<EventArgs> listener) {
        requireHandle(); disposingListeners.add(Objects.requireNonNull(listener, "listener"));
    }
    public final void removeDisposingListener(EventHandler<EventArgs> listener) {
        disposingListeners.remove(listener);
    }

    public final Cue GetCue(String name) {
        if (name == null || name.isEmpty()) throw new NullPointerException("name");
        synchronized (lock) {
            Cue cue = new Cue(engine, this,
                    NativeAudio.getCue(requireHandle(), name), name);
            cues.add(cue);
            engine.register(cue);
            return cue;
        }
    }

    public final void PlayCue(String name) {
        if (name == null || name.isEmpty()) throw new NullPointerException("name");
        NativeAudio.playCueFromBank(requireHandle(), name, null, null);
    }

    public final void PlayCue(String name, AudioListener listener, AudioEmitter emitter) {
        if (name == null || name.isEmpty()) throw new NullPointerException("name");
        NativeAudio.playCueFromBank(requireHandle(), name,
                Objects.requireNonNull(listener, "listener").nativeValues(),
                Objects.requireNonNull(emitter, "emitter").nativeValues());
    }

    public final boolean getIsDisposed() { synchronized (lock) { return handle == 0L; } }
    public final boolean getIsInUse() { return NativeAudio.getBankBoolean(requireHandle(), 0, true); }

    protected void Dispose(boolean disposing) { closeInternal(disposing); }
    public final void Dispose() { Dispose(true); }
    @Override public final void close() { Dispose(); }

    final void cueClosed(Cue cue) { synchronized (lock) { cues.remove(cue); } }

    private void closeInternal(boolean disposing) {
        long closing;
        List<Cue> closingCues;
        boolean raiseDisposing = false;
        Throwable failure = null;
        synchronized (lock) {
            if (handle == 0L) return;
            closingCues = new ArrayList<>(cues);
            closing = handle;
        }
        for (int i = closingCues.size() - 1; i >= 0; i--) {
            try { closingCues.get(i).close(); }
            catch (Throwable exception) { failure = AudioEngine.append(failure, exception); }
        }
        if (failure == null) {
            try {
                NativeAudio.destroySoundBank(closing);
                synchronized (lock) {
                    handle = 0L;
                    cues.clear();
                    raiseDisposing = disposing && !disposingRaised;
                    if (raiseDisposing) disposingRaised = true;
                }
                engine.unregister(this);
            } catch (Throwable exception) {
                failure = AudioEngine.append(failure, exception);
            }
        }
        if (raiseDisposing) {
            for (EventHandler<EventArgs> listener : disposingListeners) {
                try { listener.invoke(this, EventArgs.Empty); }
                catch (Throwable exception) {
                    failure = AudioEngine.append(failure, exception);
                }
            }
        }
        if (getIsDisposed()) disposingListeners.clear();
        AudioEngine.rethrow(failure);
    }

    private long requireHandle() {
        synchronized (lock) {
            engine.requireHandle();
            if (handle == 0L) throw new IllegalStateException("SoundBank is disposed");
            return handle;
        }
    }
}
