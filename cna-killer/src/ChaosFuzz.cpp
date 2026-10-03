// SPDX-License-Identifier: MIT
// Corrupted input for every decoder a game can hand bytes to: images, WAVE files, XNB assets
// (including LZX/LZ4-compressed ones) and compiled effects. Each starts from a valid file
// generated here, checks that the valid file loads correctly, then corrupts it. XNA refuses bad
// input with an exception; a crash, a hang or a non-System exception is the finding.
#include "ChaosEngine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <sstream>

#include "System/IO/MemoryStream.hpp"
#include "System/IDisposable.hpp"

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"

#include "ChaosSupport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Audio;

namespace CnaKiller
{
    namespace
    {
        using Bytes = std::vector<std::uint8_t>;

        void PutU16(Bytes& out, std::uint16_t value)
        {
            out.push_back(static_cast<std::uint8_t>(value));
            out.push_back(static_cast<std::uint8_t>(value >> 8));
        }

        void PutU32(Bytes& out, std::uint32_t value)
        {
            for (int i = 0; i < 4; ++i)
                out.push_back(static_cast<std::uint8_t>(value >> (8 * i)));
        }

        void Put7BitInt(Bytes& out, std::uint32_t value)
        {
            while (value >= 0x80)
            {
                out.push_back(static_cast<std::uint8_t>(value | 0x80));
                value >>= 7;
            }
            out.push_back(static_cast<std::uint8_t>(value));
        }

        void PutString(Bytes& out, const std::string& text)
        {
            Put7BitInt(out, static_cast<std::uint32_t>(text.size()));
            out.insert(out.end(), text.begin(), text.end());
        }

        constexpr const char* kTexture2DReader =
            "Microsoft.Xna.Framework.Content.Texture2DReader, Microsoft.Xna.Framework.Graphics, "
            "Version=4.0.0.0, Culture=neutral, PublicKeyToken=842cf8be1de50553";
        constexpr const char* kSoundEffectReader =
            "Microsoft.Xna.Framework.Content.SoundEffectReader, Microsoft.Xna.Framework, "
            "Version=4.0.0.0, Culture=neutral, PublicKeyToken=842cf8be1de50553";

        /** @brief An uncompressed XNA 4.0 Windows XNB holding one object read by @p reader. */
        Bytes MakeXnb(const std::string& reader, const Bytes& payload, bool hiDef)
        {
            Bytes body;
            Put7BitInt(body, 1);
            PutString(body, reader);
            PutU32(body, 0);    // reader version
            Put7BitInt(body, 0); // shared resources
            Put7BitInt(body, 1); // the root object's reader, 1-based
            body.insert(body.end(), payload.begin(), payload.end());

            Bytes file{'X', 'N', 'B', 'w', 5, static_cast<std::uint8_t>(hiDef ? 0x01 : 0x00)};
            PutU32(file, static_cast<std::uint32_t>(10 + body.size()));
            file.insert(file.end(), body.begin(), body.end());
            return file;
        }

        Bytes MakeWave(int sampleRate, int channels, int bitsPerSample, const Bytes& samples)
        {
            const int blockAlign = channels * bitsPerSample / 8;
            Bytes file{'R', 'I', 'F', 'F'};
            PutU32(file, static_cast<std::uint32_t>(36 + samples.size()));
            file.insert(file.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
            PutU32(file, 16);
            PutU16(file, 1); // PCM
            PutU16(file, static_cast<std::uint16_t>(channels));
            PutU32(file, static_cast<std::uint32_t>(sampleRate));
            PutU32(file, static_cast<std::uint32_t>(sampleRate * blockAlign));
            PutU16(file, static_cast<std::uint16_t>(blockAlign));
            PutU16(file, static_cast<std::uint16_t>(bitsPerSample));
            file.insert(file.end(), {'d', 'a', 't', 'a'});
            PutU32(file, static_cast<std::uint32_t>(samples.size()));
            file.insert(file.end(), samples.begin(), samples.end());
            return file;
        }

        /** @brief Serves one in-memory asset to ContentManager's documented OpenStream seam. */
        class FuzzContentManager final : public Content::ContentManager
        {
        public:
            FuzzContentManager(System::IServiceProvider* services, GraphicsDevice& device)
                : ContentManager(services)
            {
                setGraphicsDevice(device);
            }

            Bytes asset;
            std::vector<std::shared_ptr<System::IDisposable>> owned;

            template <typename T>
            T Read()
            {
                return ReadAsset<T>("chaos", [this](std::shared_ptr<System::IDisposable> disposable) {
                    owned.push_back(std::move(disposable));
                });
            }

        protected:
            std::unique_ptr<System::IO::Stream> OpenStream(const std::string&) override
            {
                return std::make_unique<System::IO::MemoryStream>(asset.data(), static_cast<SharpRuntime::intcs>(asset.size()),
                                                                  false);
            }
        };
    }

    // -----------------------------------------------------------------------------------------
    // Images: SaveAsPng/SaveAsJpeg, FromStream, then corrupted copies
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionFuzzImage()
    {
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        const int width = RandomInt(1, 65);
        const int height = RandomInt(1, 65);
        const bool png = RandomInt(0, 3) != 0;
        std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
        const Bytes noise = RandomBytes(pixels.size() * 3);
        for (std::size_t i = 0; i < pixels.size(); ++i)
            pixels[i] = Color(noise[i * 3], noise[i * 3 + 1], noise[i * 3 + 2], std::uint8_t{255});

        Texture2D source(device, width, height);
        source.SetData(pixels.data(), static_cast<int>(pixels.size()));
        System::IO::MemoryStream encoded;
        if (png)
            source.SaveAsPng(&encoded, width, height);
        else
            source.SaveAsJpeg(&encoded, width, height);
        const std::vector<SharpRuntime::bytecs> encodedBytes = encoded.ToArray();
        Bytes bytes(encodedBytes.begin(), encodedBytes.end());

        // The valid file first: same size, and for PNG the same pixels (opaque, so premultiplying
        // on load cannot change them).
        {
            System::IO::MemoryStream input(bytes.data(), static_cast<SharpRuntime::intcs>(bytes.size()), false);
            Texture2D decoded = Texture2D::FromStream(device, input);
            findings_.CountCheck();
            if (decoded.getWidthProperty() != width || decoded.getHeightProperty() != height)
            {
                Report(FindingKind::Mismatch, std::string(png ? "SaveAsPng" : "SaveAsJpeg") + " then FromStream changes the size",
                       std::to_string(width) + "x" + std::to_string(height) + " came back " +
                           std::to_string(decoded.getWidthProperty()) + "x" + std::to_string(decoded.getHeightProperty()));
            }
            else if (png)
            {
                std::vector<Color> readBack(pixels.size());
                decoded.GetData(readBack.data(), static_cast<int>(readBack.size()));
                for (std::size_t i = 0; i < pixels.size(); ++i)
                {
                    if (readBack[i].getPackedValueProperty() != pixels[i].getPackedValueProperty())
                    {
                        Report(FindingKind::Mismatch, "SaveAsPng then FromStream changes opaque pixels",
                               std::to_string(width) + "x" + std::to_string(height) + ", texel " + std::to_string(i) +
                                   " " + Support::Describe(pixels[i]) + " came back " + Support::Describe(readBack[i]));
                        break;
                    }
                }
            }
        }

        // Then corrupted copies, or bytes that only start like an image.
        const int kind = RandomInt(0, 6);
        if (kind < 4)
        {
            Mutate(bytes);
            if (Chance(2))
                Mutate(bytes);
        }
        else if (kind == 4)
        {
            static constexpr std::array<const char*, 4> kMagic{"\x89PNG\r\n\x1a\n", "\xFF\xD8\xFF\xE0", "GIF89a", "BM"};
            const std::string magic = kMagic[static_cast<std::size_t>(RandomInt(0, 4))];
            bytes.assign(magic.begin(), magic.end());
            const Bytes tail = RandomBytes(static_cast<std::size_t>(RandomInt(0, 512)));
            bytes.insert(bytes.end(), tail.begin(), tail.end());
        }
        else
        {
            bytes = RandomBytes(static_cast<std::size_t>(RandomInt(0, 256)));
        }
        const bool resized = Chance(4);
        const int boxWidth = RandomInt(-2, 300);
        const int boxHeight = RandomInt(-2, 300);
        const bool zoom = Chance(2);
        (void)Tolerate(std::string("Texture2D.FromStream on a corrupted ") + (png ? "PNG" : "JPEG"), [&] {
            System::IO::MemoryStream input(bytes.data(), static_cast<SharpRuntime::intcs>(bytes.size()), false);
            if (resized)
                (void)Texture2D::FromStream(device, input, boxWidth, boxHeight, zoom);
            else
                (void)Texture2D::FromStream(device, input);
        });
    }

    // -----------------------------------------------------------------------------------------
    // WAVE files through SoundEffect.FromStream
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionFuzzWave()
    {
        const int sampleRate = RandomInt(8000, 48001);
        const int channels = RandomInt(1, 3);
        const int bits = Chance(4) ? 8 : 16;
        const int frames = RandomInt(1, 4000);
        const Bytes samples = RandomBytes(static_cast<std::size_t>(frames * channels * bits / 8));
        Bytes file = MakeWave(sampleRate, channels, bits, samples);
        const bool corrupt = !Chance(3);
        if (corrupt)
        {
            Mutate(file);
            if (Chance(2))
                Mutate(file);
        }

        WithAudio([&] {
            if (!corrupt)
            {
                std::istringstream input(std::string(file.begin(), file.end()), std::ios::binary);
                const std::unique_ptr<SoundEffect> sound(SoundEffect::FromStream(input));
                const double expected = static_cast<double>(frames) / sampleRate;
                const double actual = sound->getDurationProperty().getTotalSecondsProperty();
                findings_.CountCheck();
                if (std::abs(actual - expected) > 0.002)
                {
                    Report(FindingKind::Mismatch, "SoundEffect.FromStream reports the wrong Duration for a PCM WAVE file",
                           std::to_string(frames) + " frames, " + std::to_string(channels) + " channels, " +
                               std::to_string(bits) + "-bit at " + std::to_string(sampleRate) + " Hz: expected " +
                               std::to_string(expected) + " s, got " + std::to_string(actual) + " s");
                }
                return;
            }
            (void)Tolerate("SoundEffect.FromStream on a corrupted WAVE file", [&] {
                std::istringstream input(std::string(file.begin(), file.end()), std::ios::binary);
                const std::unique_ptr<SoundEffect> sound(SoundEffect::FromStream(input));
            });
        });
    }

    // -----------------------------------------------------------------------------------------
    // XNB assets through ContentManager.ReadAsset
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionFuzzXnb()
    {
        GraphicsDevice& device = Device();
        Support::UnbindAll(device);
        FuzzContentManager content(&game_->getServicesProperty(), device);
        const bool texture = !Chance(3);
        const int corruption = RandomInt(0, 5); // 0: valid, 1-2: mutated, 3: LZX garbage, 4: LZ4 garbage

        int width = 0;
        int height = 0;
        Bytes texels;
        int frames = 0;
        int sampleRate = 0;
        if (texture)
        {
            width = RandomInt(1, 65);
            height = RandomInt(1, 65);
            const bool mipMap = Chance(2);
            texels = RandomBytes(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);
            Bytes payload;
            PutU32(payload, 0); // SurfaceFormat.Color
            PutU32(payload, static_cast<std::uint32_t>(width));
            PutU32(payload, static_cast<std::uint32_t>(height));
            int levels = 1;
            if (mipMap)
            {
                for (int w = width, h = height; w > 1 || h > 1; w = std::max(1, w / 2), h = std::max(1, h / 2))
                    ++levels;
            }
            PutU32(payload, static_cast<std::uint32_t>(levels));
            for (int level = 0; level < levels; ++level)
            {
                const int w = Support::LevelSize(width, level);
                const int h = Support::LevelSize(height, level);
                PutU32(payload, static_cast<std::uint32_t>(w * h * 4));
                if (level == 0)
                    payload.insert(payload.end(), texels.begin(), texels.end());
                else
                    payload.resize(payload.size() + static_cast<std::size_t>(w * h * 4), 0x7F);
            }
            content.asset = MakeXnb(kTexture2DReader, payload, Chance(2));
        }
        else
        {
            sampleRate = RandomInt(8000, 48001);
            const int channels = RandomInt(1, 3);
            frames = RandomInt(1, 4000);
            const Bytes samples = RandomBytes(static_cast<std::size_t>(frames * channels * 2));
            Bytes payload;
            PutU32(payload, 18); // WAVEFORMATEX with cbSize
            PutU16(payload, 1);
            PutU16(payload, static_cast<std::uint16_t>(channels));
            PutU32(payload, static_cast<std::uint32_t>(sampleRate));
            PutU32(payload, static_cast<std::uint32_t>(sampleRate * channels * 2));
            PutU16(payload, static_cast<std::uint16_t>(channels * 2));
            PutU16(payload, 16);
            PutU16(payload, 0);
            PutU32(payload, static_cast<std::uint32_t>(samples.size()));
            payload.insert(payload.end(), samples.begin(), samples.end());
            PutU32(payload, 0);                                              // loop start
            PutU32(payload, static_cast<std::uint32_t>(frames));             // loop length
            PutU32(payload, static_cast<std::uint32_t>(frames * 1000LL / sampleRate)); // duration in ms
            content.asset = MakeXnb(kSoundEffectReader, payload, false);
        }

        if (corruption == 0)
        {
            if (texture)
            {
                Texture2D loaded = content.Read<Texture2D>();
                findings_.CountCheck();
                if (loaded.getWidthProperty() != width || loaded.getHeightProperty() != height ||
                    loaded.getFormatProperty() != SurfaceFormat::Color)
                {
                    Report(FindingKind::Mismatch, "a Texture2D XNB loads with the wrong size or format",
                           std::to_string(width) + "x" + std::to_string(height) + " Color came back " +
                               std::to_string(loaded.getWidthProperty()) + "x" + std::to_string(loaded.getHeightProperty()));
                    return;
                }
                Bytes readBack(texels.size());
                loaded.GetData(0, nullptr, readBack.data(), 0, static_cast<int>(readBack.size()));
                if (readBack != texels)
                {
                    const auto at = std::mismatch(texels.begin(), texels.end(), readBack.begin()).first - texels.begin();
                    Report(FindingKind::Mismatch, "a Texture2D XNB loads different texels than the file holds",
                           std::to_string(width) + "x" + std::to_string(height) + ", first difference at byte " +
                               std::to_string(at));
                }
                return;
            }
            WithAudio([&] {
                SoundEffect sound = content.Read<SoundEffect>();
                const double expected = static_cast<double>(frames) / sampleRate;
                const double actual = sound.getDurationProperty().getTotalSecondsProperty();
                findings_.CountCheck();
                if (std::abs(actual - expected) > 0.002)
                {
                    Report(FindingKind::Mismatch, "a SoundEffect XNB loads with the wrong Duration",
                           std::to_string(frames) + " frames at " + std::to_string(sampleRate) + " Hz: expected " +
                               std::to_string(expected) + " s, got " + std::to_string(actual) + " s");
                }
            });
            return;
        }

        Bytes& file = content.asset;
        if (corruption <= 2)
        {
            Mutate(file);
            if (Chance(2))
                Mutate(file);
        }
        else
        {
            // The compressed forms: a real header, a decompressed size, then garbage for LZX or LZ4.
            file[5] = static_cast<std::uint8_t>(file[5] | (corruption == 3 ? 0x80 : 0x40));
            const std::uint32_t decompressedSize = static_cast<std::uint32_t>(file.size() - 10);
            Bytes compressed(file.begin(), file.begin() + 10);
            PutU32(compressed, Chance(4) ? static_cast<std::uint32_t>(RandomInt(0, 1 << 30)) : decompressedSize);
            const Bytes garbage = RandomBytes(static_cast<std::size_t>(RandomInt(0, 2048)));
            compressed.insert(compressed.end(), garbage.begin(), garbage.end());
            const auto total = static_cast<std::uint32_t>(compressed.size());
            std::memcpy(compressed.data() + 6, &total, 4);
            file = compressed;
        }

        const bool asTexture = Chance(4) ? !texture : texture; // sometimes the wrong asset type
        const std::string what = std::string("ContentManager.ReadAsset<") + (asTexture ? "Texture2D" : "SoundEffect") +
                                 "> on a " + (corruption <= 2 ? "corrupted" : corruption == 3 ? "garbage-LZX" : "garbage-LZ4") +
                                 " XNB";
        if (asTexture)
        {
            (void)Tolerate(what, [&] { (void)content.Read<Texture2D>(); });
        }
        else
        {
            WithAudio([&] { (void)Tolerate(what, [&] { (void)content.Read<SoundEffect>(); }); });
        }
    }

    // -----------------------------------------------------------------------------------------
    // Compiled effects
    // -----------------------------------------------------------------------------------------

    void ChaosEngine::ActionFuzzEffect()
    {
        std::vector<SharpRuntime::bytecs> code;
        switch (RandomInt(0, 3))
        {
            case 0:
                break; // empty
            case 1:
            {
                // XNA 4.0's effect header, then an fx_2_0 token, then garbage.
                PutU32(code, 0xBCF00BCFu);
                PutU32(code, 4);
                PutU32(code, 0xFEFF0901u);
                const Bytes tail = RandomBytes(static_cast<std::size_t>(RandomInt(0, 4096)));
                code.insert(code.end(), tail.begin(), tail.end());
                break;
            }
            default:
                code = RandomBytes(static_cast<std::size_t>(RandomInt(1, 4096)));
                break;
        }
        (void)Tolerate("Effect(GraphicsDevice, byte[]) on corrupted bytecode", [&] { Effect effect(Device(), code); });
    }
}
