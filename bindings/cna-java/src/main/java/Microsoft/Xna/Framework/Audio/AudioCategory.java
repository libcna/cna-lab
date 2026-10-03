package Microsoft.Xna.Framework.Audio;

import org.openeggbert.cna.internal.NativeAudio;

import java.util.Objects;

/** XACT category value retaining its owning engine. */
public final class AudioCategory {
    private final AudioEngine engine;
    private final long handle;
    private final String name;

    public AudioCategory() {
        engine = null;
        handle = 0L;
        name = null;
    }

    public AudioCategory(AudioCategory value) {
        AudioCategory selected = Objects.requireNonNull(value, "value");
        engine = selected.engine;
        handle = selected.handle;
        name = selected.name;
    }

    AudioCategory(AudioEngine engine, long handle, String name) {
        this.engine = engine;
        this.handle = handle;
        this.name = name;
    }

    public String getName() {
        return engine == null ? null : NativeAudio.getCategoryName(requireHandle());
    }
    public void Pause() { NativeAudio.categoryTransport(requireHandle(), 0, 0.0f); }
    public void Resume() { NativeAudio.categoryTransport(requireHandle(), 1, 0.0f); }
    public void SetVolume(float volume) {
        if (volume < 0.0f) throw new IllegalArgumentException("volume must not be negative");
        NativeAudio.categoryTransport(requireHandle(), 2, volume);
    }
    public void Stop(AudioStopOptions options) {
        NativeAudio.categoryTransport(requireHandle(),
                3 + Objects.requireNonNull(options, "options").ordinal(), 0.0f);
    }

    public boolean equals(AudioCategory other) { return equals((Object) other); }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof AudioCategory value)) return false;
        if (engine == null || value.engine == null) {
            return engine == null && value.engine == null;
        }
        return NativeAudio.categoryEquals(requireHandle(), value.requireHandle());
    }

    @Override
    public int hashCode() {
        return engine == null ? 0 : NativeAudio.categoryHashCode(requireHandle());
    }

    @Override
    public String toString() { return engine == null ? "" : getName(); }

    private long requireHandle() {
        if (engine == null || handle == 0L) {
            throw new IllegalStateException("AudioCategory is uninitialized");
        }
        engine.requireHandle();
        return handle;
    }
}
