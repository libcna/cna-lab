package org.openeggbert.cna.internal;

import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.ArrayList;
import java.util.Collections;
import java.util.IdentityHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

/** Internal JNI surface for CNA's stable Audio and XACT C ABI. */
public final class NativeAudio {
    private static final Set<AutoCloseable> OWNERS = Collections.newSetFromMap(
            new IdentityHashMap<>());
    private static final Set<Long> REGISTRATIONS = new LinkedHashSet<>();
    private static volatile long audioGeneration;

    private NativeAudio() {
    }

    public static long createSoundEffect(byte[] data, int offset, int count,
            int sampleRate, int channels, int loopStart, int loopLength) {
        long[] output = new long[1];
        check("cna_sound_effect_create_pcm16_range_ext", nativeCreateSoundEffect(
                game("SoundEffect"), data, offset, count, sampleRate, channels,
                loopStart, loopLength, output));
        return handle(output, "cna_sound_effect_create_pcm16_range_ext");
    }

    public static long createSoundEffectEncoded(byte[] data) {
        long[] output = new long[1];
        check("cna_sound_effect_create_from_encoded_ext",
                nativeCreateSoundEffectEncoded(game("SoundEffect.FromStream"), data, output));
        return handle(output, "cna_sound_effect_create_from_encoded_ext");
    }

    public static void destroySoundEffect(long handle) {
        check("cna_sound_effect_destroy", nativeDestroySoundEffect(handle));
    }

    public static long createSoundEffectInstance(long effect) {
        long[] output = new long[1];
        check("cna_sound_effect_create_instance", nativeCreateSoundEffectInstance(effect, output));
        return handle(output, "cna_sound_effect_create_instance");
    }

    public static boolean playSoundEffect(long effect, float volume, float pitch,
            float pan, boolean settings) {
        int result = nativePlaySoundEffect(effect, volume, pitch, pan, settings);
        if (result < 0) throw NativeBindings.failure("cna_sound_effect_play", -result);
        return result != 0;
    }

    public static Duration duration(long effect) {
        long[] ticks = new long[1];
        check("cna_sound_effect_get_duration_ticks", nativeGetSoundEffectDuration(effect, ticks));
        return durationFromTicks(ticks[0]);
    }

    public static String getSoundEffectName(long effect) {
        return nativeString(effect, 0, 0, "cna_sound_effect_copy_name");
    }

    public static void setSoundEffectName(long effect, String value) {
        check("cna_sound_effect_set_name", nativeSetSoundEffectName(effect, utf8(value)));
    }

    public static float getSoundSetting(int kind) {
        float[] output = new float[1];
        check("cna_sound_effect_get_setting", nativeGetSoundSetting(
                game("SoundEffect setting"), kind, output));
        return output[0];
    }

    public static void setSoundSetting(int kind, float value) {
        check("cna_sound_effect_set_setting", nativeSetSoundSetting(
                game("SoundEffect setting"), kind, value));
    }

    public static void instanceTransport(long instance, int operation, boolean immediate) {
        check("cna_sound_effect_instance transport",
                nativeInstanceTransport(instance, operation, immediate));
    }

    public static void setInstanceFloat(long instance, int kind, float value) {
        check("cna_sound_effect_instance_set", nativeSetInstanceFloat(instance, kind, value));
    }

    public static void setInstanceBoolean(long instance, boolean value) {
        check("cna_sound_effect_instance_set_is_looped",
                nativeSetInstanceBoolean(instance, value));
    }

    public static int getInstanceState(long instance) {
        int[] output = new int[1];
        check("cna_sound_effect_instance_get_info", nativeGetInstanceState(instance, output));
        return output[0];
    }

    public static void destroySoundEffectInstance(long instance) {
        check("cna_sound_effect_instance_destroy", nativeDestroySoundEffectInstance(instance));
    }

    public static void apply3D(long instance, float[] listeners, float[] emitter) {
        check("cna_sound_effect_instance_apply_3d",
                nativeApply3D(instance, listeners, emitter));
    }

    public static long createDynamicSoundEffect(int sampleRate, int channels) {
        long[] output = new long[1];
        check("cna_dynamic_sound_effect_instance_create", nativeCreateDynamicSoundEffect(
                game("DynamicSoundEffectInstance"), sampleRate, channels, output));
        return handle(output, "cna_dynamic_sound_effect_instance_create");
    }

    public static int getPendingBufferCount(long instance) {
        int[] output = new int[1];
        check("cna_dynamic_sound_effect_instance_get_pending_buffer_count",
                nativeGetPendingBufferCount(instance, output));
        return output[0];
    }

    public static void submitDynamicBuffer(long instance, byte[] buffer, int offset, int count) {
        check("cna_dynamic_sound_effect_instance_submit_buffer",
                nativeSubmitDynamicBuffer(instance, buffer, offset, count));
    }

    public static long subscribeBufferNeeded(long instance, Object target) {
        long[] output = new long[1];
        check("cna_dynamic_sound_effect_instance_subscribe_buffer_needed",
                nativeSubscribeAudioEvent(instance, -1, target, false, output));
        synchronized (REGISTRATIONS) { REGISTRATIONS.add(output[0]); }
        return output[0];
    }

    public static void unsubscribe(long registration) {
        synchronized (REGISTRATIONS) {
            if (!REGISTRATIONS.contains(registration)) return;
        }
        check("cna_audio_unsubscribe_ext", nativeUnsubscribeAudioEvent(registration));
        synchronized (REGISTRATIONS) { REGISTRATIONS.remove(registration); }
    }

    public static int getMicrophoneCount() {
        long[] output = new long[1];
        check("cna_microphone_get_count", nativeGetMicrophoneCount(
                game("Microphone.All"), output));
        return Math.toIntExact(output[0]);
    }

    public static int getDefaultMicrophoneIndex() {
        long[] output = new long[1];
        int[] present = new int[1];
        check("cna_microphone_get_default_index_ext", nativeGetDefaultMicrophone(
                game("Microphone.Default"), output, present));
        return present[0] == 0 ? -1 : Math.toIntExact(output[0]);
    }

    public static String getMicrophoneName(int index) {
        return nativeString(game("Microphone.Name"), 1, index,
                "cna_microphone_copy_name_at");
    }

    public static int getMicrophoneInt(int index, int kind) {
        int[] output = new int[1];
        check("cna_microphone_get", nativeGetMicrophoneInt(
                game("Microphone"), index, kind, output));
        return output[0];
    }

    public static Duration getMicrophoneDuration(int index) {
        long[] output = new long[1];
        check("cna_microphone_get_buffer_duration_ticks_at", nativeGetMicrophoneDuration(
                game("Microphone.BufferDuration"), index, output));
        return durationFromTicks(output[0]);
    }

    public static void setMicrophoneDuration(int index, Duration duration) {
        check("cna_microphone_set_buffer_duration_ticks_at", nativeSetMicrophoneDuration(
                game("Microphone.BufferDuration"), index, ticks(duration)));
    }

    public static void microphoneTransport(int index, boolean start) {
        check(start ? "cna_microphone_start_at" : "cna_microphone_stop_at",
                nativeMicrophoneTransport(game("Microphone"), index, start));
    }

    public static int getMicrophoneData(int index, byte[] bytes, int offset, int count) {
        long[] output = new long[1];
        check("cna_microphone_get_data_at", nativeGetMicrophoneData(
                game("Microphone.GetData"), index, bytes, offset, count, output));
        return Math.toIntExact(output[0]);
    }

    public static long subscribeMicrophone(int index, Object target) {
        long[] output = new long[1];
        check("cna_microphone_subscribe_buffer_ready_at", nativeSubscribeAudioEvent(
                game("Microphone.BufferReady"), index, target, true, output));
        synchronized (REGISTRATIONS) { REGISTRATIONS.add(output[0]); }
        return output[0];
    }

    public static long createAudioEngine(String settings, Duration lookAhead, String renderer) {
        long[] output = new long[1];
        check("cna_audio_engine_create_with_renderer", nativeCreateAudioEngine(
                game("AudioEngine"), utf8(settings), ticks(lookAhead),
                utf8(renderer == null ? "" : renderer), output));
        return handle(output, "cna_audio_engine_create_with_renderer");
    }

    public static void destroyAudioEngine(long engine) {
        check("cna_audio_engine_destroy", nativeDestroyAudioEngine(engine));
    }

    public static int getRendererCount(long engine) {
        long[] output = new long[1];
        check("cna_audio_engine_get_renderer_count", nativeGetRendererCount(engine, output));
        return Math.toIntExact(output[0]);
    }

    public static String getRendererString(long engine, int index, boolean id) {
        return nativeString(engine, id ? 3 : 2, index,
                id ? "cna_audio_engine_copy_renderer_id"
                        : "cna_audio_engine_copy_renderer_friendly_name");
    }

    public static long getAudioCategory(long engine, String name) {
        long[] output = new long[1];
        check("cna_audio_engine_get_category", nativeGetAudioCategory(engine, utf8(name), output));
        return handle(output, "cna_audio_engine_get_category");
    }

    public static void destroyCategory(long category) {
        check("cna_audio_category_destroy", nativeDestroyCategory(category));
    }

    public static String getCategoryName(long category) {
        return nativeString(category, 4, 0, "cna_audio_category_copy_name");
    }

    public static boolean categoryEquals(long category, long other) {
        int[] output = new int[1];
        check("cna_audio_category_equals", nativeCategoryEquals(category, other, output));
        return output[0] != 0;
    }

    public static int categoryHashCode(long category) {
        int[] output = new int[1];
        check("cna_audio_category_get_hash_code", nativeCategoryHashCode(category, output));
        return output[0];
    }

    public static void categoryTransport(long category, int operation, float volume) {
        check("cna_audio_category operation", nativeCategoryOperation(category, operation, volume));
    }

    public static float getXactVariable(long engine, long cue, String name,
            boolean set, float value) {
        float[] output = new float[1];
        check("XACT variable", nativeXactVariable(engine, cue, utf8(name), set, value, output));
        return output[0];
    }

    public static void updateAudioEngine(long engine) {
        check("cna_audio_engine_update", nativeUpdateAudioEngine(engine));
    }

    public static long createWaveBank(long engine, String path, int offset,
            short packetSize, boolean streaming) {
        long[] output = new long[1];
        check(streaming ? "cna_wave_bank_create_streaming" : "cna_wave_bank_create",
                nativeCreateWaveBank(engine, utf8(path), offset, packetSize, streaming, output));
        return handle(output, "cna_wave_bank_create");
    }

    public static void destroyWaveBank(long bank) {
        check("cna_wave_bank_destroy", nativeDestroyWaveBank(bank));
    }

    public static boolean getBankBoolean(long bank, int kind, boolean soundBank) {
        int result = nativeGetBankBoolean(bank, kind, soundBank);
        if (result < 0) throw NativeBindings.failure("XACT bank state", -result);
        return result != 0;
    }

    public static long createSoundBank(long engine, String path) {
        long[] output = new long[1];
        check("cna_sound_bank_create", nativeCreateSoundBank(engine, utf8(path), output));
        return handle(output, "cna_sound_bank_create");
    }

    public static void destroySoundBank(long bank) {
        check("cna_sound_bank_destroy", nativeDestroySoundBank(bank));
    }

    public static long getCue(long bank, String name) {
        long[] output = new long[1];
        check("cna_sound_bank_get_cue", nativeGetCue(bank, utf8(name), output));
        return handle(output, "cna_sound_bank_get_cue");
    }

    public static void playCueFromBank(long bank, String name,
            float[] listener, float[] emitter) {
        check("cna_sound_bank_play_cue", nativePlayCueFromBank(
                bank, utf8(name), listener, emitter));
    }

    public static void destroyCue(long cue) { check("cna_cue_destroy", nativeDestroyCue(cue)); }

    public static int[] getCueInfo(long cue) {
        int[] output = new int[8];
        check("cna_cue_get_info", nativeGetCueInfo(cue, output));
        return output;
    }

    public static void applyCue3D(long cue, float[] listener, float[] emitter) {
        check("cna_cue_apply_3d", nativeApplyCue3D(cue, listener, emitter));
    }

    public static void cueTransport(long cue, int operation, int option) {
        check("cna_cue operation", nativeCueTransport(cue, operation, option));
    }

    public static void registerOwner(AutoCloseable owner) {
        synchronized (OWNERS) { OWNERS.add(owner); }
    }

    public static void unregisterOwner(AutoCloseable owner) {
        synchronized (OWNERS) { OWNERS.remove(owner); }
    }

    public static long audioGeneration() { return audioGeneration; }

    static void closeAllForGameShutdown() {
        List<AutoCloseable> owners;
        synchronized (OWNERS) {
            owners = new ArrayList<>(OWNERS);
            OWNERS.clear();
        }
        Collections.reverse(owners);
        RuntimeException failure = null;
        for (AutoCloseable owner : owners) {
            try { owner.close(); }
            catch (RuntimeException exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            } catch (Exception exception) {
                RuntimeException wrapped = new RuntimeException("Audio shutdown failed", exception);
                if (failure == null) failure = wrapped; else failure.addSuppressed(wrapped);
            }
        }
        List<Long> registrations;
        synchronized (REGISTRATIONS) {
            registrations = new ArrayList<>(REGISTRATIONS);
        }
        for (long registration : registrations) {
            try { unsubscribe(registration); }
            catch (RuntimeException exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        audioGeneration++;
        if (failure != null) throw failure;
    }

    private static String nativeString(long handle, int kind, int index, String operation) {
        long[] size = new long[1];
        check(operation, nativeGetStringSize(handle, kind, index, size));
        byte[] output = new byte[Math.toIntExact(size[0])];
        if (output.length != 0) check(operation, nativeCopyString(handle, kind, index, output));
        return new String(output, StandardCharsets.UTF_8);
    }

    private static byte[] utf8(String value) { return value.getBytes(StandardCharsets.UTF_8); }

    private static long game(String owner) {
        NativeBindings.requireAvailable();
        return NativeBindings.currentGameValue(owner);
    }

    private static long handle(long[] output, String operation) {
        if (output[0] == 0L) throw NativeBindings.failure(operation, -1);
        return output[0];
    }

    private static void check(String operation, int result) {
        if (result != 0) throw NativeBindings.failure(operation, result);
    }

    private static long ticks(Duration duration) {
        try {
            return Math.addExact(Math.multiplyExact(duration.getSeconds(), 10_000_000L),
                    duration.getNano() / 100L);
        } catch (ArithmeticException overflow) {
            throw new IllegalArgumentException("Duration is outside XNA TimeSpan range", overflow);
        }
    }

    private static Duration durationFromTicks(long ticks) {
        return Duration.ofSeconds(Math.floorDiv(ticks, 10_000_000L),
                Math.floorMod(ticks, 10_000_000L) * 100L);
    }

    private static native int nativeCreateSoundEffect(long game, byte[] data,
            int offset, int count, int sampleRate, int channels,
            int loopStart, int loopLength, long[] output);
    private static native int nativeCreateSoundEffectEncoded(long game, byte[] data, long[] output);
    private static native int nativeDestroySoundEffect(long effect);
    private static native int nativeCreateSoundEffectInstance(long effect, long[] output);
    private static native int nativePlaySoundEffect(long effect, float volume,
            float pitch, float pan, boolean settings);
    private static native int nativeGetSoundEffectDuration(long effect, long[] output);
    private static native int nativeSetSoundEffectName(long effect, byte[] name);
    private static native int nativeGetSoundSetting(long game, int kind, float[] output);
    private static native int nativeSetSoundSetting(long game, int kind, float value);
    private static native int nativeInstanceTransport(long instance, int operation, boolean immediate);
    private static native int nativeSetInstanceFloat(long instance, int kind, float value);
    private static native int nativeSetInstanceBoolean(long instance, boolean value);
    private static native int nativeGetInstanceState(long instance, int[] output);
    private static native int nativeDestroySoundEffectInstance(long instance);
    private static native int nativeApply3D(long instance, float[] listeners, float[] emitter);
    private static native int nativeCreateDynamicSoundEffect(long game, int sampleRate,
            int channels, long[] output);
    private static native int nativeGetPendingBufferCount(long instance, int[] output);
    private static native int nativeSubmitDynamicBuffer(long instance, byte[] buffer,
            int offset, int count);
    private static native int nativeSubscribeAudioEvent(long handle, int index,
            Object target, boolean microphone, long[] output);
    private static native int nativeUnsubscribeAudioEvent(long registration);
    private static native int nativeGetMicrophoneCount(long game, long[] output);
    private static native int nativeGetDefaultMicrophone(long game, long[] index, int[] present);
    private static native int nativeGetMicrophoneInt(long game, int index, int kind, int[] output);
    private static native int nativeGetMicrophoneDuration(long game, int index, long[] output);
    private static native int nativeSetMicrophoneDuration(long game, int index, long ticks);
    private static native int nativeMicrophoneTransport(long game, int index, boolean start);
    private static native int nativeGetMicrophoneData(long game, int index, byte[] bytes,
            int offset, int count, long[] output);
    private static native int nativeCreateAudioEngine(long game, byte[] settings,
            long lookAheadTicks, byte[] renderer, long[] output);
    private static native int nativeDestroyAudioEngine(long engine);
    private static native int nativeGetRendererCount(long engine, long[] output);
    private static native int nativeGetAudioCategory(long engine, byte[] name, long[] output);
    private static native int nativeDestroyCategory(long category);
    private static native int nativeCategoryOperation(long category, int operation, float volume);
    private static native int nativeCategoryEquals(long category, long other, int[] output);
    private static native int nativeCategoryHashCode(long category, int[] output);
    private static native int nativeXactVariable(long engine, long cue, byte[] name,
            boolean set, float value, float[] output);
    private static native int nativeUpdateAudioEngine(long engine);
    private static native int nativeCreateWaveBank(long engine, byte[] path, int offset,
            short packetSize, boolean streaming, long[] output);
    private static native int nativeDestroyWaveBank(long bank);
    private static native int nativeGetBankBoolean(long bank, int kind, boolean soundBank);
    private static native int nativeCreateSoundBank(long engine, byte[] path, long[] output);
    private static native int nativeDestroySoundBank(long bank);
    private static native int nativeGetCue(long bank, byte[] name, long[] output);
    private static native int nativePlayCueFromBank(long bank, byte[] name,
            float[] listener, float[] emitter);
    private static native int nativeDestroyCue(long cue);
    private static native int nativeGetCueInfo(long cue, int[] output);
    private static native int nativeApplyCue3D(long cue, float[] listener, float[] emitter);
    private static native int nativeCueTransport(long cue, int operation, int option);
    private static native int nativeGetStringSize(long handle, int kind, int index, long[] output);
    private static native int nativeCopyString(long handle, int kind, int index, byte[] output);
}
