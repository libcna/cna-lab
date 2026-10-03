// SPDX-License-Identifier: MIT
// Audio chaos: procedural sounds, instances driven through every state transition and 3D
// positioning, streaming voices fed from their BufferNeeded event, and the global settings.
#include "ChaosEngine.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "System/InvalidOperationException.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEmitter.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioListener.hpp"
#include "Microsoft/Xna/Framework/Audio/InstancePlayLimitException.hpp"
#include "Microsoft/Xna/Framework/Audio/NoAudioHardwareException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;

namespace CnaKiller
{
    void ChaosEngine::WithAudio(const std::function<void()>& body)
    {
        if (!audioAvailable_)
            return;
        try
        {
            body();
        }
        catch (const NoAudioHardwareException& exception)
        {
            // XNA's own answer on a machine without audio: every later audio call would say the
            // same, so the audio actions stop here rather than filling the findings with it.
            audioAvailable_ = false;
            log_.Note(std::string("no audio hardware (") + exception.what() +
                      "); audio actions are skipped from now on -- run with SDL_AUDIO_DRIVER=dummy to keep them");
        }
    }

    void ChaosEngine::ActionCreateSound()
    {
        EvictIfFull(sounds_);

        const int sampleRate = random_.Next(8000, 48001);
        const AudioChannels channels = random_.Next(0, 2) == 0 ? AudioChannels::Mono : AudioChannels::Stereo;
        const double durationSeconds = 0.05 + random_.NextDouble() * 0.4;
        const int frames = static_cast<int>(durationSeconds * sampleRate);
        const int sampleCount = frames * static_cast<int>(channels);

        // A short, loud burst of tone-ish noise: cheap to synthesize and unpleasant on purpose --
        // this "game" is not trying to be pleasant, it is trying to break the audio backend.
        const double frequency = 80.0 + random_.NextDouble() * 4000.0;
        std::vector<SharpRuntime::bytecs> pcm(static_cast<std::size_t>(sampleCount) * 2);
        for (int i = 0; i < sampleCount; ++i)
        {
            const double t = static_cast<double>(i) / sampleRate;
            const double sample = std::sin(2.0 * MathHelper::Pi * frequency * t);
            const auto amplitude = static_cast<std::int16_t>(sample * 20000.0);
            pcm[static_cast<std::size_t>(i) * 2 + 0] = static_cast<SharpRuntime::bytecs>(amplitude & 0xFF);
            pcm[static_cast<std::size_t>(i) * 2 + 1] = static_cast<SharpRuntime::bytecs>((amplitude >> 8) & 0xFF);
        }

        // Sometimes a window of the buffer with a loop region inside it.
        const bool windowed = Chance(3) && frames > 8;
        const int blockAlign = 2 * static_cast<int>(channels);
        const int firstFrame = windowed ? RandomInt(0, frames / 2) : 0;
        const int frameCount = windowed ? RandomInt(1, frames - firstFrame + 1) : frames;
        const int loopStart = windowed ? RandomInt(0, frameCount) : 0;
        const int loopLength = windowed ? RandomInt(0, frameCount - loopStart + 1) : 0;

        WithAudio([&] {
            auto sound = std::make_unique<ManagedSound>();
            if (windowed)
                sound->soundEffect = std::make_shared<SoundEffect>(pcm, firstFrame * blockAlign, frameCount * blockAlign,
                                                                   sampleRate, channels, loopStart, loopLength);
            else
                sound->soundEffect = std::make_shared<SoundEffect>(pcm, sampleRate, channels);

            // XNA's duration is the sample count over the rate.
            const double expected = static_cast<double>(windowed ? frameCount : frames) / sampleRate;
            const double actual = sound->soundEffect->getDurationProperty().getTotalSecondsProperty();
            findings_.CountCheck();
            if (std::abs(actual - expected) > 0.002)
            {
                Report(FindingKind::Mismatch, "SoundEffect.Duration is not the sample count over the sample rate",
                       std::to_string(windowed ? frameCount : frames) + " frames at " + std::to_string(sampleRate) +
                           " Hz: expected " + std::to_string(expected) + " s, got " + std::to_string(actual) + " s");
            }
            sounds_.Add(std::move(sound));
        });
    }

    void ChaosEngine::ActionPlaySound()
    {
        if (sounds_.Empty())
        {
            ActionCreateSound();
            if (sounds_.Empty())
                return;
        }
        SoundEffect& sound = *sounds_.RandomItem(random_).soundEffect;
        const bool withParameters = Chance(2);
        const float volume = RandomFloat(0, 1);
        const float pitch = RandomFloat(-1, 1);
        const float pan = RandomFloat(-1, 1);
        WithAudio([&] {
            try
            {
                if (withParameters)
                    (void)sound.Play(volume, pitch, pan);
                else
                    (void)sound.Play();
            }
            catch (const InstancePlayLimitException&)
            {
                findings_.CountRefusal(); // XNA's own limit on concurrently playing voices
            }
        });
    }

    void ChaosEngine::ActionDestroySound()
    {
        sounds_.DestroyRandom(random_);
    }

    void ChaosEngine::ActionSoundInstances()
    {
        if (sounds_.Empty())
        {
            ActionCreateSound();
            if (sounds_.Empty())
                return;
        }

        const int operation = instances_.Empty() ? 0 : RandomInt(0, 12);
        const std::size_t soundIndex = static_cast<std::size_t>(RandomInt(0, static_cast<int>(sounds_.Size())));
        const std::size_t instanceIndex =
            instances_.Empty() ? 0 : static_cast<std::size_t>(RandomInt(0, static_cast<int>(instances_.Size())));
        const float a = RandomFloat(-1, 1);
        const float b = RandomFloat(-1, 1);
        const bool flag = Chance(2);
        const Vector3 position{RandomFloat(-100, 100), RandomFloat(-100, 100), RandomFloat(-100, 100)};
        const Vector3 velocity{RandomFloat(-400, 400), RandomFloat(-400, 400), RandomFloat(-400, 400)};

        WithAudio([&] {
            try
            {
                if (operation == 0)
                {
                    EvictIfFull(instances_);
                    auto managed = std::make_unique<ManagedInstance>();
                    managed->parent = sounds_.At(soundIndex).soundEffect;
                    managed->instance = std::make_unique<SoundEffectInstance>(managed->parent->CreateInstance());
                    managed->instance->setIsLoopedProperty(flag);
                    managed->instance->setVolumeProperty((a + 1) / 2);
                    managed->instance->Play();
                    instances_.Add(std::move(managed));
                    return;
                }

                ManagedInstance& managed = instances_.At(instanceIndex);
                SoundEffectInstance& instance = *managed.instance;
                switch (operation)
                {
                    case 1: instance.Play(); break;
                    case 2: instance.Pause(); break;
                    case 3: instance.Resume(); break;
                    case 4: instance.Stop(flag); break;
                    case 5: instance.setVolumeProperty((a + 1) / 2); break;
                    case 6: instance.setPitchProperty(a); break;
                    case 7:
                        // XNA refuses Pan on an instance playing in 3D; whether this one is depends
                        // on its history, so a refusal is as acceptable as success.
                        (void)Tolerate("SoundEffectInstance.Pan on an instance in any state",
                                       [&] { instance.setPanProperty(b); });
                        break;
                    case 8:
                    {
                        // XNA refuses Apply3D on an instance already playing without 3D.
                        AudioListener listener;
                        AudioEmitter emitter;
                        emitter.setPositionProperty(position);
                        emitter.setVelocityProperty(velocity);
                        emitter.setDopplerScaleProperty((a + 1) * 2);
                        (void)Tolerate("SoundEffectInstance.Apply3D on an instance in any state",
                                       [&] { instance.Apply3D(listener, emitter); });
                        break;
                    }
                    case 9:
                        instance.Dispose();
                        instances_.DestroyAt(instanceIndex);
                        break;
                    case 10:
                    {
                        // XNA disposes every instance of a SoundEffect when the SoundEffect is disposed.
                        const std::shared_ptr<SoundEffect> parent = managed.parent;
                        parent->Dispose();
                        findings_.CountCheck();
                        for (std::size_t i = 0; i < instances_.Size(); ++i)
                        {
                            if (instances_.At(i).parent == parent && !instances_.At(i).instance->getIsDisposedProperty())
                            {
                                Report(FindingKind::Mismatch, "SoundEffect.Dispose leaves an instance created from it undisposed",
                                       "instance state " + std::to_string(static_cast<int>(instances_.At(i).instance->getStateProperty())));
                                break;
                            }
                        }
                        for (std::size_t i = instances_.Size(); i-- > 0;)
                        {
                            if (instances_.At(i).parent == parent)
                                instances_.DestroyAt(i);
                        }
                        for (std::size_t i = sounds_.Size(); i-- > 0;)
                        {
                            if (sounds_.At(i).soundEffect == parent)
                                sounds_.DestroyAt(i);
                        }
                        break;
                    }
                    default:
                        (void)instance.getStateProperty();
                        break;
                }
            }
            catch (const InstancePlayLimitException&)
            {
                findings_.CountRefusal();
            }
        });
    }

    void ChaosEngine::ActionDynamicSound()
    {
        const int operation = dynamicSounds_.Empty() ? 0 : RandomInt(0, 9);
        const std::size_t index =
            dynamicSounds_.Empty() ? 0 : static_cast<std::size_t>(RandomInt(0, static_cast<int>(dynamicSounds_.Size())));
        const int sampleRate = RandomInt(8000, 48001);
        const bool stereo = Chance(2);
        const int frames = RandomInt(1, 4000);
        const int burst = RandomInt(1, 80);

        WithAudio([&] {
            if (operation == 0)
            {
                EvictIfFull(dynamicSounds_);
                auto managed = std::make_unique<ManagedDynamicSound>();
                managed->instance = std::make_unique<DynamicSoundEffectInstance>(
                    sampleRate, stereo ? AudioChannels::Stereo : AudioChannels::Mono);
                managed->blockAlign = stereo ? 4 : 2;
                managed->sampleRate = sampleRate;
                // Feed it from its own BufferNeeded event, wherever CNA raises that from.
                DynamicSoundEffectInstance* voice = managed->instance.get();
                const auto refill = std::make_shared<std::vector<SharpRuntime::bytecs>>(
                    static_cast<std::size_t>(frames) * static_cast<std::size_t>(managed->blockAlign), 0);
                voice->BufferNeeded += [voice, refill](System::Object*, const System::EventArgs&) {
                    if (voice->getPendingBufferCountProperty() < 3)
                        voice->SubmitBuffer(*refill);
                };
                voice->setVolumeProperty(0.2f);
                voice->Play();
                dynamicSounds_.Add(std::move(managed));
                return;
            }

            ManagedDynamicSound& managed = dynamicSounds_.At(index);
            DynamicSoundEffectInstance& voice = *managed.instance;
            const std::vector<SharpRuntime::bytecs> buffer(
                static_cast<std::size_t>(frames) * static_cast<std::size_t>(managed.blockAlign), 0);
            switch (operation)
            {
                case 1:
                    voice.SubmitBuffer(buffer);
                    break;
                case 2:
                {
                    // A window of whole frames inside the buffer.
                    const int offsetFrames = frames / 3;
                    voice.SubmitBuffer(buffer, offsetFrames * managed.blockAlign,
                                       (frames - offsetFrames) * managed.blockAlign);
                    break;
                }
                case 3:
                    // XNA refuses more than 64 pending buffers; past that is an InvalidOperationException.
                    for (int i = 0; i < burst; ++i)
                    {
                        if (!Tolerate("SubmitBuffer beyond the pending-buffer limit", [&] { voice.SubmitBuffer(buffer); }))
                            break;
                    }
                    break;
                case 4: voice.Play(); break;
                case 5: voice.Pause(); break;
                case 6: voice.Stop(); break;
                case 7:
                {
                    // XNA's AudioFormat: duration = FromMilliseconds(frames * 1000f / rate), which
                    // .NET Framework rounds to a whole millisecond; size = whole frames of that
                    // duration, (int)(ms * (rate / 1000f)), plus frames % channels, times BlockAlign.
                    const int channels = managed.blockAlign / 2;
                    const int bytes = frames * managed.blockAlign;
                    const float exactMs = static_cast<float>(bytes / managed.blockAlign) * 1000.0f /
                                          static_cast<float>(managed.sampleRate);
                    const auto xnaMs = static_cast<long long>(static_cast<double>(exactMs) + 0.5);
                    const int xnaFrames = static_cast<int>(static_cast<double>(xnaMs) *
                                                           static_cast<double>(static_cast<float>(managed.sampleRate) / 1000.0f));
                    const int xnaBytes = (xnaFrames + xnaFrames % channels) * managed.blockAlign;

                    const System::TimeSpan duration = voice.GetSampleDuration(bytes);
                    const int back = voice.GetSampleSizeInBytes(System::TimeSpan::FromMilliseconds(static_cast<double>(xnaMs)));
                    findings_.CountCheck();
                    if (std::abs(duration.getTotalMillisecondsProperty() - static_cast<double>(xnaMs)) > 1e-6)
                    {
                        Report(FindingKind::Mismatch, "DynamicSoundEffectInstance.GetSampleDuration differs from XNA",
                               std::to_string(bytes) + " bytes at " + std::to_string(managed.sampleRate) + " Hz: XNA " +
                                   std::to_string(xnaMs) + " ms, CNA " +
                                   std::to_string(duration.getTotalMillisecondsProperty()) + " ms");
                    }
                    if (back != xnaBytes)
                    {
                        Report(FindingKind::Mismatch, "DynamicSoundEffectInstance.GetSampleSizeInBytes differs from XNA",
                               std::to_string(xnaMs) + " ms at " + std::to_string(managed.sampleRate) + " Hz, " +
                                   std::to_string(channels) + " channel(s): XNA " + std::to_string(xnaBytes) +
                                   " bytes, CNA " + std::to_string(back) + " bytes");
                    }
                    break;
                }
                default:
                    dynamicSounds_.DestroyAt(index);
                    break;
            }
        });
    }

    void ChaosEngine::ActionAudioGlobals()
    {
        const int pick = RandomInt(0, 4);
        const float value = RandomFloat(0, 1);
        WithAudio([&] {
            switch (pick)
            {
                case 0: SoundEffect::setMasterVolumeProperty(value); break;
                case 1: SoundEffect::setDistanceScaleProperty(0.01f + value * 10.0f); break;
                case 2: SoundEffect::setDopplerScaleProperty(value * 5.0f); break;
                default: SoundEffect::setSpeedOfSoundProperty(1.0f + value * 1000.0f); break;
            }
        });
    }
}
