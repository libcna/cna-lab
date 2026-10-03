// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Myra identifies its transform algorithm as derived from MonoGame
// SpriteBatch.DrawString (Ms-PL); see THIRD_PARTY_NOTICES.md.
// Ported from: src/Myra/Graphics2D/Transform.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <cmath>

#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D
{
    /**
     * @brief Stores a 2D transform matrix together with its scale and rotation.
     *
     * Scale and rotation are retained because upstream Myra passes them to
     * SpriteBatch separately from the compound matrix.
     */
    class Transform
    {
    public:
        Transform(const Microsoft::Xna::Framework::Vector2& offset,
                  const Microsoft::Xna::Framework::Vector2& origin,
                  const Microsoft::Xna::Framework::Vector2& scale,
                  const float rotation)
            : matrix_(BuildTransform(offset, origin, scale, rotation)), scale_(scale), rotation_(rotation)
        {
            Utility::Mathematics::CalculateInverse(matrix_, inverseMatrix_);
        }

        [[nodiscard]] const Microsoft::Xna::Framework::Vector2& getScaleProperty() const noexcept
        {
            return scale_;
        }

        [[nodiscard]] float getRotationProperty() const noexcept
        {
            return rotation_;
        }

        [[nodiscard]] const Microsoft::Xna::Framework::Matrix& getMatrixProperty() const noexcept
        {
            return matrix_;
        }

        void AddTransform(const Transform& newTransform)
        {
            matrix_ = newTransform.matrix_ * matrix_;
            Utility::Mathematics::CalculateInverse(matrix_, inverseMatrix_);
            scale_.X *= newTransform.scale_.X;
            scale_.Y *= newTransform.scale_.Y;
            rotation_ += newTransform.rotation_;
        }

        [[nodiscard]] Microsoft::Xna::Framework::Vector2 Apply(
            const Microsoft::Xna::Framework::Vector2& source) const
        {
            return Utility::Mathematics::Transform(source, matrix_);
        }

        [[nodiscard]] Microsoft::Xna::Framework::Point Apply(const Microsoft::Xna::Framework::Point& source) const
        {
            return Utility::Mathematics::ToPoint(
                Apply(Microsoft::Xna::Framework::Vector2(static_cast<float>(source.X), static_cast<float>(source.Y))));
        }

        [[nodiscard]] Microsoft::Xna::Framework::Vector2 InverseApply(
            const Microsoft::Xna::Framework::Vector2& source) const
        {
            return Utility::Mathematics::Transform(source, inverseMatrix_);
        }

        [[nodiscard]] Microsoft::Xna::Framework::Point InverseApply(
            const Microsoft::Xna::Framework::Point& source) const
        {
            return Utility::Mathematics::ToPoint(InverseApply(
                Microsoft::Xna::Framework::Vector2(static_cast<float>(source.X), static_cast<float>(source.Y))));
        }

        [[nodiscard]] Microsoft::Xna::Framework::Rectangle Apply(
            const Microsoft::Xna::Framework::Rectangle& source) const
        {
            return Utility::Mathematics::Transform(source, matrix_);
        }

    private:
        [[nodiscard]] static Microsoft::Xna::Framework::Matrix BuildTransform(
            const Microsoft::Xna::Framework::Vector2& position,
            const Microsoft::Xna::Framework::Vector2& origin,
            const Microsoft::Xna::Framework::Vector2& scale,
            const float rotation)
        {
            auto result = Microsoft::Xna::Framework::Matrix::getIdentityProperty();
            float offsetX = 0.0F;
            float offsetY = 0.0F;

            if (rotation == 0.0F)
            {
                result.M11 = scale.X;
                result.M22 = scale.Y;
                offsetX = position.X - (origin.X * result.M11);
                offsetY = position.Y - (origin.Y * result.M22);
            }
            else
            {
                const float cosine = std::cos(rotation);
                const float sine = std::sin(rotation);
                result.M11 = scale.X * cosine;
                result.M12 = scale.X * sine;
                result.M21 = scale.Y * -sine;
                result.M22 = scale.Y * cosine;
                offsetX = position.X - (origin.X * result.M11) - (origin.Y * result.M21);
                offsetY = position.Y - (origin.X * result.M12) - (origin.Y * result.M22);
            }

            offsetX += origin.X;
            offsetY += origin.Y;
            result.M41 = offsetX;
            result.M42 = offsetY;
            return result;
        }

        Microsoft::Xna::Framework::Matrix matrix_;
        Microsoft::Xna::Framework::Matrix inverseMatrix_;
        Microsoft::Xna::Framework::Vector2 scale_;
        float rotation_;
    };
}
