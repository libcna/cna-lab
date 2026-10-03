// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/TypeSerializers.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MML/TypeSerializers.hpp"

#include <array>
#include <system_error>

namespace Myra::MML
{
    namespace
    {
        [[nodiscard]] std::string_view Trim(const std::string_view value)
        {
            const size_t first = value.find_first_not_of(" \t\r\n");
            if (first == std::string_view::npos)
            {
                return {};
            }
            return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
        }

        template<size_t Count>
        [[nodiscard]] std::array<std::string_view, Count> SplitNumbers(const std::string& value)
        {
            std::array<std::string_view, Count> result;
            std::string_view remaining(value);
            for (size_t index = 0; index < Count; ++index)
            {
                const size_t separator = remaining.find(',');
                if (index + 1 < Count && separator == std::string_view::npos)
                {
                    throw std::invalid_argument("The MML value has too few comma-separated numbers.");
                }
                if (index + 1 == Count && separator != std::string_view::npos)
                {
                    throw std::invalid_argument("The MML value has too many comma-separated numbers.");
                }
                result[index] = Trim(separator == std::string_view::npos ? remaining : remaining.substr(0, separator));
                if (result[index].empty())
                {
                    throw std::invalid_argument("The MML value contains an empty number.");
                }
                remaining = separator == std::string_view::npos ? std::string_view{} : remaining.substr(separator + 1);
            }
            return result;
        }

        [[nodiscard]] float ParseFloat(const std::string_view value)
        {
            float result = 0.0F;
            const auto [position, error] = std::from_chars(value.data(), value.data() + value.size(), result,
                std::chars_format::general);
            if (error != std::errc{} || position != value.data() + value.size())
            {
                throw std::invalid_argument("The MML value contains an invalid invariant-culture float.");
            }
            return result;
        }

        [[nodiscard]] int ParseInt(const std::string_view value)
        {
            int result = 0;
            const auto [position, error] = std::from_chars(value.data(), value.data() + value.size(), result);
            if (error != std::errc{} || position != value.data() + value.size())
            {
                throw std::invalid_argument("The MML value contains an invalid integer.");
            }
            return result;
        }

        [[nodiscard]] std::string FormatFloat(const float value)
        {
            std::array<char, 64> buffer{};
            const auto [position, error] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value,
                std::chars_format::general);
            if (error != std::errc{})
            {
                throw std::runtime_error("Could not format an MML float value.");
            }
            return {buffer.data(), position};
        }
    }

    Microsoft::Xna::Framework::Vector2 Vector2Serializer::DeserializeT(const std::string& value) const
    {
        const auto parts = SplitNumbers<2>(value);
        return {ParseFloat(parts[0]), ParseFloat(parts[1])};
    }

    std::string Vector2Serializer::SerializeT(const Microsoft::Xna::Framework::Vector2& value) const
    {
        return FormatFloat(value.X) + ", " + FormatFloat(value.Y);
    }

    Graphics2D::Thickness ThicknessSerializer::DeserializeT(const std::string& value) const
    {
        return Graphics2D::Thickness::FromString(value);
    }

    std::string ThicknessSerializer::SerializeT(const Graphics2D::Thickness& value) const { return value.ToString(); }

    Microsoft::Xna::Framework::Rectangle RectangleSerializer::DeserializeT(const std::string& value) const
    {
        const auto parts = SplitNumbers<4>(value);
        return {ParseInt(parts[0]), ParseInt(parts[1]), ParseInt(parts[2]), ParseInt(parts[3])};
    }

    std::string RectangleSerializer::SerializeT(const Microsoft::Xna::Framework::Rectangle& value) const
    {
        return std::to_string(value.X) + ", " + std::to_string(value.Y) + ", " + std::to_string(value.Width) + ", " +
            std::to_string(value.Height);
    }

    const ITypeSerializer* TypeSerializers::Find(const std::type_index type) noexcept
    {
        static const Vector2Serializer vector2;
        static const ThicknessSerializer thickness;
        static const RectangleSerializer rectangle;
        if (type == typeid(Microsoft::Xna::Framework::Vector2))
        {
            return &vector2;
        }
        if (type == typeid(Graphics2D::Thickness))
        {
            return &thickness;
        }
        if (type == typeid(Microsoft::Xna::Framework::Rectangle))
        {
            return &rectangle;
        }
        return nullptr;
    }
}
