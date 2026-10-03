// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/ColorHSV.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace Myra::Utility
{
    /** @brief HSV color with hue in 0..360 and saturation/value in 0..100. */
    struct ColorHSV
    {
        SharpRuntime::intcs H = 0;
        SharpRuntime::intcs S = 0;
        SharpRuntime::intcs V = 0;

        [[nodiscard]] static ColorHSV FromRGB(const Microsoft::Xna::Framework::Color& color)
        {
            const float red = static_cast<float>(color.getRProperty()) / 255.0F;
            const float green = static_cast<float>(color.getGProperty()) / 255.0F;
            const float blue = static_cast<float>(color.getBProperty()) / 255.0F;

            const float minimum = std::min(std::min(red, green), blue);
            const float maximum = std::max(std::max(red, green), blue);
            float hue = 0.0F;
            float saturation = 0.0F;
            float value = maximum;
            const float delta = maximum - minimum;

            if (maximum != 0.0F)
            {
                saturation = delta / maximum;
            }
            else
            {
                return {RoundToEven(hue), RoundToEven(saturation), RoundToEven(value)};
            }

            if (delta == 0.0F)
            {
                hue = 0.0F;
            }
            else if (red == maximum)
            {
                hue = (green - blue) / delta;
            }
            else if (green == maximum)
            {
                hue = 2.0F + (blue - red) / delta;
            }
            else
            {
                hue = 4.0F + (red - green) / delta;
            }

            hue *= 60.0F;
            if (hue < 0.0F)
            {
                hue += 360.0F;
            }

            saturation *= 100.0F;
            value *= 100.0F;
            return {RoundToEven(hue), RoundToEven(saturation), RoundToEven(value)};
        }

        [[nodiscard]] Microsoft::Xna::Framework::Color ToRGB() const
        {
            float hue = static_cast<float>(H);
            if (hue == 360.0F)
            {
                hue = 359.0F;
            }

            float saturation = static_cast<float>(S);
            float value = static_cast<float>(V);
            hue = std::max(0.0F, std::min(360.0F, hue));
            saturation = std::max(0.0F, std::min(100.0F, saturation));
            value = std::max(0.0F, std::min(100.0F, value));
            saturation /= 100.0F;
            value /= 100.0F;
            hue /= 60.0F;

            const auto sector = static_cast<SharpRuntime::intcs>(std::floor(hue));
            const float fraction = hue - static_cast<float>(sector);
            const float p = value * (1.0F - saturation);
            const float q = value * (1.0F - saturation * fraction);
            const float t = value * (1.0F - saturation * (1.0F - fraction));

            SharpRuntime::intcs red = 0;
            SharpRuntime::intcs green = 0;
            SharpRuntime::intcs blue = 0;
            switch (sector)
            {
            case 0:
                red = RoundToEven(255.0F * value);
                green = RoundToEven(255.0F * t);
                blue = RoundToEven(255.0F * p);
                break;
            case 1:
                red = RoundToEven(255.0F * q);
                green = RoundToEven(255.0F * value);
                blue = RoundToEven(255.0F * p);
                break;
            case 2:
                red = RoundToEven(255.0F * p);
                green = RoundToEven(255.0F * value);
                blue = RoundToEven(255.0F * t);
                break;
            case 3:
                red = RoundToEven(255.0F * p);
                green = RoundToEven(255.0F * q);
                blue = RoundToEven(255.0F * value);
                break;
            case 4:
                red = RoundToEven(255.0F * t);
                green = RoundToEven(255.0F * p);
                blue = RoundToEven(255.0F * value);
                break;
            default:
                red = RoundToEven(255.0F * value);
                green = RoundToEven(255.0F * p);
                blue = RoundToEven(255.0F * q);
                break;
            }

            return {red, green, blue, 255};
        }

        [[nodiscard]] constexpr bool Equals(const ColorHSV& other) const noexcept
        {
            // This intentionally preserves the observable comparison in the
            // selected Myra source: a.S == b.V, not a.S == b.S.
            return H == other.H && V == other.V && S == other.V;
        }

        [[nodiscard]] SharpRuntime::intcs GetHashCode() const noexcept
        {
            std::uint32_t hashCode = static_cast<std::uint32_t>(H);
            hashCode = (hashCode * 397U) ^ static_cast<std::uint32_t>(S);
            hashCode = (hashCode * 397U) ^ static_cast<std::uint32_t>(V);
            return std::bit_cast<SharpRuntime::intcs>(hashCode);
        }

    private:
        [[nodiscard]] static SharpRuntime::intcs RoundToEven(const float value)
        {
            const float lower = std::floor(value);
            const float fraction = value - lower;
            if (fraction < 0.5F)
            {
                return static_cast<SharpRuntime::intcs>(lower);
            }

            if (fraction > 0.5F)
            {
                return static_cast<SharpRuntime::intcs>(lower + 1.0F);
            }

            const auto lowerInteger = static_cast<SharpRuntime::intcs>(lower);
            return (lowerInteger % 2 == 0) ? lowerInteger : lowerInteger + 1;
        }
    };

    [[nodiscard]] constexpr bool operator==(const ColorHSV& left, const ColorHSV& right) noexcept
    {
        return left.Equals(right);
    }

    [[nodiscard]] constexpr bool operator!=(const ColorHSV& left, const ColorHSV& right) noexcept
    {
        return !(left == right);
    }
}
