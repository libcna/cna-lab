// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/Thickness.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <string>

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace Myra::Graphics2D
{
    /**
     * @brief Represents the thicknesses of the four sides of a rectangle.
     *
     * The value order is left, top, right, bottom, matching upstream Myra.
     */
    struct Thickness
    {
        /** A thickness with every side set to zero. */
        static const Thickness Zero;

        /** Gets or sets the left thickness. */
        SharpRuntime::intcs Left = 0;

        /** Gets or sets the right thickness. */
        SharpRuntime::intcs Right = 0;

        /** Gets or sets the top thickness. */
        SharpRuntime::intcs Top = 0;

        /** Gets or sets the bottom thickness. */
        SharpRuntime::intcs Bottom = 0;

        /** Creates a zero thickness. */
        constexpr Thickness() = default;

        /** Creates a thickness with individual left, top, right, and bottom values. */
        constexpr Thickness(SharpRuntime::intcs left,
                            SharpRuntime::intcs top,
                            SharpRuntime::intcs right,
                            SharpRuntime::intcs bottom) noexcept
            : Left(left), Right(right), Top(top), Bottom(bottom)
        {
        }

        /** Creates a thickness with horizontal and vertical values. */
        constexpr Thickness(SharpRuntime::intcs horizontalValue,
                            SharpRuntime::intcs verticalValue) noexcept
            : Thickness(horizontalValue, verticalValue, horizontalValue, verticalValue)
        {
        }

        /** Creates a thickness with the same value on every side. */
        constexpr explicit Thickness(SharpRuntime::intcs value) noexcept
            : Thickness(value, value, value, value)
        {
        }

        [[nodiscard]] constexpr SharpRuntime::intcs getWidthProperty() const noexcept
        {
            return Left + Right;
        }

        [[nodiscard]] constexpr SharpRuntime::intcs getHeightProperty() const noexcept
        {
            return Top + Bottom;
        }

        [[nodiscard]] constexpr bool getSameSizeProperty() const noexcept
        {
            return Left == Top && Top == Right && Right == Bottom;
        }

        /** Returns the upstream compact string form: value, h/v, or l/t/r/b. */
        [[nodiscard]] std::string ToString() const;

        /** Parses one-, two-, or four-value thickness text. */
        [[nodiscard]] static Thickness FromString(const std::string& value);

        [[nodiscard]] constexpr bool Equals(const Thickness& other) const noexcept
        {
            return Left == other.Left && Right == other.Right && Top == other.Top && Bottom == other.Bottom;
        }

        /** Returns the deterministic 32-bit hash algorithm used by upstream Myra. */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const noexcept;
    };

    [[nodiscard]] constexpr bool operator==(const Thickness& left, const Thickness& right) noexcept
    {
        return left.Equals(right);
    }

    [[nodiscard]] constexpr bool operator!=(const Thickness& left, const Thickness& right) noexcept
    {
        return !(left == right);
    }

    /** Returns a rectangle reduced by the given thickness, with dimensions clamped to zero. */
    [[nodiscard]] Microsoft::Xna::Framework::Rectangle operator-(
        Microsoft::Xna::Framework::Rectangle rectangle,
        const Thickness& thickness) noexcept;
}
