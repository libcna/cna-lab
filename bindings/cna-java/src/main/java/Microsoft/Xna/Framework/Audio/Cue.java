package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeAudio;

import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Owned prepared XACT cue. */
public final class Cue implements AutoCloseable {
    private final Object lock = new Object();
    private final AudioEngine engine;
    private final SoundBank bank;
    private final String name;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposingListeners =
            new CopyOnWriteArrayList<>();
    private long handle;
    private boolean disposingRaised;

    Cue(AudioEngine engine, SoundBank bank, long handle, String name) {
        this.engine = engine;
        this.bank = bank;
        this.handle = handle;
        this.name = name;
    }

    public void addDisposingListener(EventHandler<EventArgs> listener) {
        requireHandle(); disposingListeners.add(Objects.requireNonNull(listener, "listener"));
    }
    public void removeDisposingListener(EventHandler<EventArgs> listener) {
        disposingListeners.remove(listener);
    }

    public void Apply3D(AudioListener listener, AudioEmitter emitter) {
        NativeAudio.applyCue3D(requireHandle(),
                Objects.requireNonNull(listener, "listener").nativeValues(),
                Objects.requireNonNull(emitter, "emitter").nativeValues());
    }
    public float GetVariable(String name) {
        if (name == null || name.isEmpty()) throw new NullPointerException("name");
        return NativeAudio.getXactVariable(engine.requireHandle(), requireHandle(),
                name, false, 0.0f);
    }
    public void SetVariable(String name, float value) {
        if (name == null || name.isEmpty()) throw new NullPointerException("name");
        NativeAudio.getXactVariable(engine.requireHandle(), requireHandle(),
                name, true, value);
    }
    public void Pause() { NativeAudio.cueTransport(requireHandle(), 1, 0); }
    public void Play() { NativeAudio.cueTransport(requireHandle(), 0, 0); }
    public void Resume() { NativeAudio.cueTransport(requireHandle(), 2, 0); }
    public void Stop(AudioStopOptions options) {
        NativeAudio.cueTransport(requireHandle(), 3,
                Objects.requireNonNull(options, "options").ordinal());
    }

    public boolean getIsCreated() { return info(0); }
    public boolean getIsDisposed() { synchronized (lock) { return handle == 0L; } }
    public boolean getIsPaused() { return info(2); }
    public boolean getIsPlaying() { return info(3); }
    public boolean getIsPrepared() { return info(4); }
    public boolean getIsPreparing() { return info(5); }
    public boolean getIsStopped() { return info(6); }
    public boolean getIsStopping() { return info(7); }
    public String getName() { requireHandle(); return name; }

    public void Dispose() { closeInternal(true); }
    @Override public void close() { Dispose(); }

    private boolean info(int index) {
        return NativeAudio.getCueInfo(requireHandle())[index] != 0;
    }

    private void closeInternal(boolean disposing) {
        long closing;
        boolean raiseDisposing;
        Throwable failure = null;
        synchronized (lock) {
            if (handle == 0L) return;
            closing = handle;
        }
        try {
            NativeAudio.destroyCue(closing);
            synchronized (lock) {
                handle = 0L;
                raiseDisposing = disposing && !disposingRaised;
                if (raiseDisposing) disposingRaised = true;
            }
            bank.cueClosed(this);
            engine.unregister(this);
        } catch (Throwable exception) {
            AudioEngine.rethrow(exception);
            return;
        }
        if (raiseDisposing) {
            for (EventHandler<EventArgs> listener : disposingListeners) {
                try { listener.invoke(this, EventArgs.Empty); }
                catch (Throwable exception) {
                    failure = AudioEngine.append(failure, exception);
                }
            }
        }
        disposingListeners.clear();
        AudioEngine.rethrow(failure);
    }

    private long requireHandle() {
        synchronized (lock) {
            engine.requireHandle();
            if (handle == 0L) throw new IllegalStateException("Cue is disposed");
            return handle;
        }
    }
}
