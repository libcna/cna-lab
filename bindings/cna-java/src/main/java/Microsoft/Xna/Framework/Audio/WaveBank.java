package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeAudio;

import java.nio.file.Path;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Owned XACT wave-bank dependency. */
@SuppressWarnings("this-escape")
public class WaveBank implements AutoCloseable {
    private final Object lock = new Object();
    private final AudioEngine engine;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposingListeners =
            new CopyOnWriteArrayList<>();
    private long handle;
    private boolean disposingRaised;

    public WaveBank(AudioEngine audioEngine, String nonStreamingWaveBankFilename) {
        engine = Objects.requireNonNull(audioEngine, "audioEngine");
        if (nonStreamingWaveBankFilename == null || nonStreamingWaveBankFilename.isEmpty()) {
            throw new NullPointerException("nonStreamingWaveBankFilename");
        }
        handle = NativeAudio.createWaveBank(engine.requireHandle(),
                Path.of(nonStreamingWaveBankFilename).toAbsolutePath().toString(),
                0, (short) 0, false);
        engine.register(this);
    }

    public WaveBank(AudioEngine audioEngine, String streamingWaveBankFilename,
            int offset, short packetsize) {
        engine = Objects.requireNonNull(audioEngine, "audioEngine");
        if (streamingWaveBankFilename == null || streamingWaveBankFilename.isEmpty()) {
            throw new NullPointerException("streamingWaveBankFilename");
        }
        if (offset < 0) throw new IllegalArgumentException("offset must not be negative");
        handle = NativeAudio.createWaveBank(engine.requireHandle(),
                Path.of(streamingWaveBankFilename).toAbsolutePath().toString(),
                offset, packetsize, true);
        engine.register(this);
    }

    public final void addDisposingListener(EventHandler<EventArgs> listener) {
        requireHandle(); disposingListeners.add(Objects.requireNonNull(listener, "listener"));
    }
    public final void removeDisposingListener(EventHandler<EventArgs> listener) {
        disposingListeners.remove(listener);
    }
    public final boolean getIsDisposed() { synchronized (lock) { return handle == 0L; } }
    public final boolean getIsInUse() { return NativeAudio.getBankBoolean(requireHandle(), 2, false); }
    public final boolean getIsPrepared() { return NativeAudio.getBankBoolean(requireHandle(), 1, false); }

    protected void Dispose(boolean disposing) { closeInternal(disposing); }
    public final void Dispose() { Dispose(true); }
    @Override public final void close() { Dispose(); }

    private void closeInternal(boolean disposing) {
        long closing;
        boolean raiseDisposing;
        Throwable failure = null;
        synchronized (lock) {
            if (handle == 0L) return;
            closing = handle;
        }
        try {
            NativeAudio.destroyWaveBank(closing);
            synchronized (lock) {
                handle = 0L;
                raiseDisposing = disposing && !disposingRaised;
                if (raiseDisposing) disposingRaised = true;
            }
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
            if (handle == 0L) throw new IllegalStateException("WaveBank is disposed");
            return handle;
        }
    }
}
