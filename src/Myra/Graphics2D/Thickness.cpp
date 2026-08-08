// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/Thickness.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/Thickness.hpp"

#include <algorithm>
#include <bit>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <vector>

namespace
{
    [[nodiscard]] std::string Trim(std::string value)
    {
        const auto isSpace = [](const unsigned char character) { return std::isspace(character) != 0; };
        const auto first = std::find_if_not(value.begin(), value.end(), isSpace);
        const auto last = std::find_if_not(value.rbegin(), value.rend(), isSpace).base();

        if (first >= last)
        {
            return {};
        }

        return {first, last};
    }

    [[nodiscard]] std::vector<std::string> SplitAndTrim(const std::string& value)
    {
        std::vector<std::string> result;
        std::size_t start = 0;

        while (true)
        {
            const std::size_t separator = value.find(',', start);
            result.push_back(Trim(value.substr(start, separator - start)));
            if (separator == std::string::npos)
            {
                return result;
            }
            start = separator + 1;
        }
    }

    [[nodiscard]] SharpRuntime::intcs ParseInt(const std::string& value, const std::string& original)
    {
        SharpRuntime::intcs result = 0;
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
        if (error != std::errc{} || end != value.data() + value.size())
        {
            throw std::invalid_argument("Could not convert string '" + original + "' to Thickness");
        }
        return result;
    }
}

namespace Myra::Graphics2D
{
    const Thickness Thickness::Zero{};

    std::string Thickness::ToString() const
    {
        if (getSameSizeProperty())
        {
            return std::to_string(Left);
        }

        if (Left == Right && Top == Bottom)
        {
            return std::to_string(Left) + ", " + std::to_string(Top);
        }

        return std::to_string(Left) + ", " + std::to_string(Top) + ", " +
               std::to_string(Right) + ", " + std::to_string(Bottom);
    }

    Thickness Thickness::FromString(const std::string& value)
    {
        if (value.empty())
        {
            return Zero;
        }

        const auto parts = SplitAndTrim(value);
        if (parts.size() != 1 && parts.size() != 2 && parts.size() != 4)
        {
            throw std::invalid_argument("Could not convert string '" + value + "' to Thickness");
        }

        if (parts.size() == 1)
        {
            return Thickness(ParseInt(parts[0], value));
        }

        if (parts.size() == 2)
        {
            return Thickness(ParseInt(parts[0], value), ParseInt(parts[1], value));
        }

        return Thickness(ParseInt(parts[0], value),
                         ParseInt(parts[1], value),
                         ParseInt(parts[2], value),
                         ParseInt(parts[3], value));
    }

    SharpRuntime::intcs Thickness::GetHashCode() const noexcept
    {
        std::uint32_t hashCode = 551583723U;
        constexpr std::uint32_t multiplier = static_cast<std::uint32_t>(-1521134295);
        const auto add = [&hashCode](const SharpRuntime::intcs value) {
            hashCode = hashCode * multiplier + static_cast<std::uint32_t>(value);
        };

        add(Left);
        add(Right);
        add(Top);
        add(Bottom);
        return std::bit_cast<SharpRuntime::intcs>(hashCode);
    }

    Microsoft::Xna::Framework::Rectangle operator-(
        Microsoft::Xna::Framework::Rectangle rectangle,
        const Thickness& thickness) noexcept
    {
        rectangle.X += thickness.Left;
        rectangle.Y += thickness.Top;
        rectangle.Width -= thickness.getWidthProperty();
        rectangle.Height -= thickness.getHeightProperty();

        if (rectangle.Width < 0)
        {
            rectangle.Width = 0;
        }

        if (rectangle.Height < 0)
        {
            rectangle.Height = 0;
        }

        return rectangle;
    }
}
