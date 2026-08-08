// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/Mathematics.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <cmath>

#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace Myra::Utility
{
    /** @brief Shared math helpers used by Myra layout and rendering code. */
    class Mathematics final
    {
    public:
        Mathematics() = delete;

        static constexpr float ZeroTolerance = 1.0e-6F;

        [[nodiscard]] static Microsoft::Xna::Framework::Point getPointZeroProperty()
        {
            return {0, 0};
        }

        [[nodiscard]] static constexpr bool EpsilonEquals(const float left,
                                                           const float right,
                                                           const float epsilon = ZeroTolerance) noexcept
        {
            return std::fabs(left - right) <= epsilon;
        }

        [[nodiscard]] static constexpr bool EpsilonEquals(const Microsoft::Xna::Framework::Vector2& left,
                                                           const Microsoft::Xna::Framework::Vector2& right,
                                                           const float epsilon = ZeroTolerance) noexcept
        {
            return EpsilonEquals(left.X, right.X, epsilon) && EpsilonEquals(left.Y, right.Y, epsilon);
        }

        [[nodiscard]] static constexpr bool IsZero(const float value) noexcept
        {
            return EpsilonEquals(value, 0.0F);
        }

        [[nodiscard]] static Microsoft::Xna::Framework::Point ToPoint(
            const Microsoft::Xna::Framework::Vector2& value)
        {
            return {RoundToEven(value.X), RoundToEven(value.Y)};
        }

        [[nodiscard]] static Microsoft::Xna::Framework::Vector2 ToVector2(
            const Microsoft::Xna::Framework::Point& value)
        {
            return {static_cast<float>(value.X), static_cast<float>(value.Y)};
        }

        [[nodiscard]] static Microsoft::Xna::Framework::Vector2 Transform(
            const Microsoft::Xna::Framework::Vector2& value,
            const Microsoft::Xna::Framework::Matrix& matrix)
        {
            return Microsoft::Xna::Framework::Vector2::Transform(value, matrix);
        }

        [[nodiscard]] static Microsoft::Xna::Framework::Rectangle Transform(
            const Microsoft::Xna::Framework::Rectangle& rectangle,
            const Microsoft::Xna::Framework::Matrix& matrix)
        {
            const auto position = Transform(
                Microsoft::Xna::Framework::Vector2(static_cast<float>(rectangle.X), static_cast<float>(rectangle.Y)),
                matrix);
            const Microsoft::Xna::Framework::Vector2 transformScale(matrix.M11, matrix.M22);
            const Microsoft::Xna::Framework::Vector2 scale(
                static_cast<float>(rectangle.Width) * transformScale.X,
                static_cast<float>(rectangle.Height) * transformScale.Y);

            return {static_cast<SharpRuntime::intcs>(position.X),
                    static_cast<SharpRuntime::intcs>(position.Y),
                    static_cast<SharpRuntime::intcs>(scale.X),
                    static_cast<SharpRuntime::intcs>(scale.Y)};
        }

        static void CalculateInverse(const Microsoft::Xna::Framework::Matrix& source,
                                     Microsoft::Xna::Framework::Matrix& result)
        {
            Microsoft::Xna::Framework::Matrix::Invert(source, result);
        }

        [[nodiscard]] static constexpr SharpRuntime::intcs Clamp(const SharpRuntime::intcs value,
                                                                   const SharpRuntime::intcs minimum,
                                                                   const SharpRuntime::intcs maximum) noexcept
        {
            if (value < minimum)
            {
                return minimum;
            }

            if (value > maximum)
            {
                return maximum;
            }

            return value;
        }

        [[nodiscard]] static constexpr float Clamp(const float value,
                                                   const float minimum,
                                                   const float maximum) noexcept
        {
            if (value < minimum)
            {
                return minimum;
            }

            if (value > maximum)
            {
                return maximum;
            }

            return value;
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
}
