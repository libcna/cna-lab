// SPDX-License-Identifier: MIT
// Format tables, half-float conversion and pixel comparison shared by the checking actions. The
// decoders follow XNA's own PackedVector bit layouts rather than calling CNA's, so a wrong
// conversion inside CNA cannot hide behind the same wrong conversion here.
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"

namespace CnaKiller
{
    /** @brief splitmix64 step: a job's own deterministic stream, seeded from the main one. */
    inline std::uint64_t SplitMixNext(std::uint64_t& state)
    {
        std::uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
}

namespace CnaKiller::Support
{
    using Microsoft::Xna::Framework::Graphics::SurfaceFormat;

    struct FormatInfo
    {
        SurfaceFormat format;
        const char* name;
        /** Bytes per texel, or per 4x4 block for a compressed format. */
        int bytes;
        bool compressed;
    };

    inline const std::vector<FormatInfo>& TextureFormats()
    {
        static const std::vector<FormatInfo> formats{
            {SurfaceFormat::Color, "Color", 4, false},
            {SurfaceFormat::Bgr565, "Bgr565", 2, false},
            {SurfaceFormat::Bgra5551, "Bgra5551", 2, false},
            {SurfaceFormat::Bgra4444, "Bgra4444", 2, false},
            {SurfaceFormat::Dxt1, "Dxt1", 8, true},
            {SurfaceFormat::Dxt3, "Dxt3", 16, true},
            {SurfaceFormat::Dxt5, "Dxt5", 16, true},
            {SurfaceFormat::NormalizedByte2, "NormalizedByte2", 2, false},
            {SurfaceFormat::NormalizedByte4, "NormalizedByte4", 4, false},
            {SurfaceFormat::Rgba1010102, "Rgba1010102", 4, false},
            {SurfaceFormat::Rg32, "Rg32", 4, false},
            {SurfaceFormat::Rgba64, "Rgba64", 8, false},
            {SurfaceFormat::Alpha8, "Alpha8", 1, false},
            {SurfaceFormat::Single, "Single", 4, false},
            {SurfaceFormat::Vector2, "Vector2", 8, false},
            {SurfaceFormat::Vector4, "Vector4", 16, false},
            {SurfaceFormat::HalfSingle, "HalfSingle", 2, false},
            {SurfaceFormat::HalfVector2, "HalfVector2", 4, false},
            {SurfaceFormat::HalfVector4, "HalfVector4", 8, false},
            {SurfaceFormat::HdrBlendable, "HdrBlendable", 8, false},
        };
        return formats;
    }

    /** @brief The formats XNA's HiDef profile accepts for a render target. */
    inline const std::vector<SurfaceFormat>& RenderTargetFormats()
    {
        static const std::vector<SurfaceFormat> formats{
            SurfaceFormat::Color, SurfaceFormat::Bgr565, SurfaceFormat::Bgra5551, SurfaceFormat::Bgra4444,
            SurfaceFormat::Rgba1010102, SurfaceFormat::Rg32, SurfaceFormat::Rgba64, SurfaceFormat::Alpha8,
            SurfaceFormat::Single, SurfaceFormat::Vector2, SurfaceFormat::Vector4, SurfaceFormat::HalfSingle,
            SurfaceFormat::HalfVector2, SurfaceFormat::HalfVector4, SurfaceFormat::HdrBlendable};
        return formats;
    }

    inline const FormatInfo& Info(SurfaceFormat format)
    {
        for (const FormatInfo& info : TextureFormats())
        {
            if (info.format == format)
                return info;
        }
        static const FormatInfo unknown{format, "<unknown>", 0, false};
        return unknown;
    }

    inline int LevelSize(int size, int level)
    {
        return std::max(1, size >> level);
    }

    /** @brief Bytes of a w x h region in @p format (blocks for a compressed format). */
    inline std::size_t RegionBytes(const FormatInfo& info, int width, int height)
    {
        if (info.compressed)
            return static_cast<std::size_t>((width + 3) / 4) * static_cast<std::size_t>((height + 3) / 4) *
                   static_cast<std::size_t>(info.bytes);
        return static_cast<std::size_t>(width) * static_cast<std::size_t>(height) *
               static_cast<std::size_t>(info.bytes);
    }

    inline float HalfToFloat(std::uint16_t half)
    {
        const int sign = (half >> 15) & 1;
        const int exponent = (half >> 10) & 31;
        const int mantissa = half & 1023;
        float value;
        if (exponent == 0)
            value = std::ldexp(static_cast<float>(mantissa), -24);
        else if (exponent == 31)
            value = mantissa == 0 ? INFINITY : NAN;
        else
            value = std::ldexp(static_cast<float>(mantissa + 1024), exponent - 25);
        return sign ? -value : value;
    }

    /**
     * @brief Rewrites random bytes into values every format stores bit-exactly.
     *
     * Float formats get finite normal numbers (a NaN payload or a denormal may legitimately be
     * canonicalised); signed-normalized bytes avoid -128, which is the same value as -127.
     */
    inline void MakeExactlyStorable(const FormatInfo& info, std::vector<std::uint8_t>& bytes)
    {
        switch (info.format)
        {
            case SurfaceFormat::Single:
            case SurfaceFormat::Vector2:
            case SurfaceFormat::Vector4:
                for (std::size_t i = 0; i + 3 < bytes.size(); i += 4)
                {
                    std::uint32_t bits;
                    std::memcpy(&bits, &bytes[i], 4);
                    const std::uint32_t exponent = 117u + (bits % 20u);
                    bits = (bits & 0x807FFFFFu) | (exponent << 23);
                    std::memcpy(&bytes[i], &bits, 4);
                }
                break;
            case SurfaceFormat::HalfSingle:
            case SurfaceFormat::HalfVector2:
            case SurfaceFormat::HalfVector4:
            case SurfaceFormat::HdrBlendable:
                for (std::size_t i = 0; i + 1 < bytes.size(); i += 2)
                {
                    std::uint16_t bits;
                    std::memcpy(&bits, &bytes[i], 2);
                    const int exponent = (bits >> 10) & 31;
                    if (exponent == 0 || exponent == 31)
                        bits = static_cast<std::uint16_t>((bits & 0x83FFu) | (15u << 10));
                    std::memcpy(&bytes[i], &bits, 2);
                }
                break;
            case SurfaceFormat::NormalizedByte2:
            case SurfaceFormat::NormalizedByte4:
                for (std::uint8_t& byte : bytes)
                {
                    if (byte == 0x80)
                        byte = 0x81;
                }
                break;
            default:
                break;
        }
    }

    /** @brief One decoded texel: channel values, which channels the format has, and their precision. */
    struct Decoded
    {
        std::array<float, 4> value{};
        std::array<bool, 4> present{};
        std::array<float, 4> tolerance{};
    };

    inline Decoded DecodeRenderTargetTexel(SurfaceFormat format, const std::uint8_t* p)
    {
        Decoded d;
        const auto unorm = [&](int channel, std::uint32_t raw, std::uint32_t max) {
            d.value[static_cast<std::size_t>(channel)] = static_cast<float>(raw) / static_cast<float>(max);
            d.present[static_cast<std::size_t>(channel)] = true;
            d.tolerance[static_cast<std::size_t>(channel)] = 1.0f / static_cast<float>(max) + 1e-4f;
        };
        const auto real = [&](int channel, float value, float tolerance) {
            d.value[static_cast<std::size_t>(channel)] = value;
            d.present[static_cast<std::size_t>(channel)] = true;
            d.tolerance[static_cast<std::size_t>(channel)] = tolerance;
        };
        std::uint16_t u16 = 0;
        std::uint32_t u32 = 0;
        std::uint64_t u64 = 0;
        float f[4]{};
        std::uint16_t h[4]{};
        switch (format)
        {
            case SurfaceFormat::Color:
                for (int c = 0; c < 4; ++c)
                    unorm(c, p[c], 255);
                break;
            case SurfaceFormat::Bgr565:
                std::memcpy(&u16, p, 2);
                unorm(0, (u16 >> 11) & 31u, 31);
                unorm(1, (u16 >> 5) & 63u, 63);
                unorm(2, u16 & 31u, 31);
                break;
            case SurfaceFormat::Bgra5551:
                std::memcpy(&u16, p, 2);
                unorm(0, (u16 >> 10) & 31u, 31);
                unorm(1, (u16 >> 5) & 31u, 31);
                unorm(2, u16 & 31u, 31);
                unorm(3, (u16 >> 15) & 1u, 1);
                break;
            case SurfaceFormat::Bgra4444:
                std::memcpy(&u16, p, 2);
                unorm(0, (u16 >> 8) & 15u, 15);
                unorm(1, (u16 >> 4) & 15u, 15);
                unorm(2, u16 & 15u, 15);
                unorm(3, (u16 >> 12) & 15u, 15);
                break;
            case SurfaceFormat::Rgba1010102:
                std::memcpy(&u32, p, 4);
                unorm(0, u32 & 1023u, 1023);
                unorm(1, (u32 >> 10) & 1023u, 1023);
                unorm(2, (u32 >> 20) & 1023u, 1023);
                unorm(3, (u32 >> 30) & 3u, 3);
                break;
            case SurfaceFormat::Rg32:
                std::memcpy(&u32, p, 4);
                unorm(0, u32 & 0xFFFFu, 65535);
                unorm(1, u32 >> 16, 65535);
                break;
            case SurfaceFormat::Rgba64:
                std::memcpy(&u64, p, 8);
                for (int c = 0; c < 4; ++c)
                    unorm(c, static_cast<std::uint32_t>((u64 >> (16 * c)) & 0xFFFFu), 65535);
                break;
            case SurfaceFormat::Alpha8:
                unorm(3, p[0], 255);
                break;
            case SurfaceFormat::Single:
                std::memcpy(f, p, 4);
                real(0, f[0], 1e-5f);
                break;
            case SurfaceFormat::Vector2:
                std::memcpy(f, p, 8);
                real(0, f[0], 1e-5f);
                real(1, f[1], 1e-5f);
                break;
            case SurfaceFormat::Vector4:
                std::memcpy(f, p, 16);
                for (int c = 0; c < 4; ++c)
                    real(c, f[c], 1e-5f);
                break;
            case SurfaceFormat::HalfSingle:
                std::memcpy(h, p, 2);
                real(0, HalfToFloat(h[0]), 1e-3f);
                break;
            case SurfaceFormat::HalfVector2:
                std::memcpy(h, p, 4);
                real(0, HalfToFloat(h[0]), 1e-3f);
                real(1, HalfToFloat(h[1]), 1e-3f);
                break;
            case SurfaceFormat::HalfVector4:
            case SurfaceFormat::HdrBlendable:
                std::memcpy(h, p, 8);
                for (int c = 0; c < 4; ++c)
                    real(c, HalfToFloat(h[c]), 1e-3f);
                break;
            default:
                break;
        }
        return d;
    }

    /** @brief Byte-distance comparison of two RGBA8 pixels, per channel. */
    inline bool Near(const Microsoft::Xna::Framework::Color& a, const Microsoft::Xna::Framework::Color& b,
                     int tolerance)
    {
        return std::abs(a.getRProperty() - b.getRProperty()) <= tolerance &&
               std::abs(a.getGProperty() - b.getGProperty()) <= tolerance &&
               std::abs(a.getBProperty() - b.getBProperty()) <= tolerance &&
               std::abs(a.getAProperty() - b.getAProperty()) <= tolerance;
    }

    inline std::string Describe(const Microsoft::Xna::Framework::Color& c)
    {
        return "(" + std::to_string(c.getRProperty()) + "," + std::to_string(c.getGProperty()) + "," +
               std::to_string(c.getBProperty()) + "," + std::to_string(c.getAProperty()) + ")";
    }

    /**
     * @brief Takes every texture and vertex/index buffer off the device.
     *
     * XNA refuses SetData on a texture still set in Textures[] or VertexTextures[], and on a
     * vertex buffer still bound, with InvalidOperationException. A checking action that is not
     * testing that rule unbinds first, as a correct XNA game has to.
     */
    inline void UnbindAll(Microsoft::Xna::Framework::Graphics::GraphicsDevice& device)
    {
        for (int slot = 0; slot < 16; ++slot)
            device.getTexturesProperty()(slot, nullptr);
        for (int slot = 0; slot < 4; ++slot)
            device.getVertexTexturesProperty()(slot, nullptr);
        device.SetVertexBuffer(nullptr);
        device.SetIndexBuffer(nullptr);
    }
}
