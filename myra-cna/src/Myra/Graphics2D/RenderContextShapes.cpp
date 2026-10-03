// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra and MonoGame.Extended, MIT Licenses,
// Copyright (c) 2017-2020 The Myra Team and Copyright (c) 2015 Dylan Wilson.
// Ported from: src/Myra/Graphics2D/RenderContext.Shapes.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md, THIRD_PARTY_NOTICES.md, and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/RenderContext.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <vector>

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/DefaultAssets.hpp"
#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"
#include "Myra/Utility/CrossEngineStuff.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D
{
    namespace
    {
        using Microsoft::Xna::Framework::Color;
        using Microsoft::Xna::Framework::Rectangle;
        using Microsoft::Xna::Framework::Vector2;
        using TextureAtlases::TextureRegion;

        [[nodiscard]] int CheckedAdd(const int left, const int right)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) + right;
            if (result < std::numeric_limits<int>::min()
                || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("RenderContext shape geometry overflows int.");
            }
            return static_cast<int>(result);
        }

        [[nodiscard]] int CheckedSubtract(const int left, const int right)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) - right;
            if (result < std::numeric_limits<int>::min()
                || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("RenderContext shape geometry overflows int.");
            }
            return static_cast<int>(result);
        }

        void ValidateSpriteScalar(const float value)
        {
            if (!std::isfinite(value))
            {
                throw std::invalid_argument(
                    "RenderContext shape geometry must be finite.");
            }

            const double widened = value;
            if (widened < static_cast<double>(std::numeric_limits<int>::min())
                || widened > static_cast<double>(std::numeric_limits<int>::max()))
            {
                throw std::overflow_error(
                    "RenderContext shape geometry exceeds the SpriteBatch integer range.");
            }
        }

        void ValidateSpriteVector(const Vector2 value)
        {
            ValidateSpriteScalar(value.X);
            ValidateSpriteScalar(value.Y);
        }

        [[nodiscard]] Rectangle RectangleFromVectors(
            const Vector2 location,
            const Vector2 size)
        {
            return {
                Utility::Mathematics::TruncateToInt(location.X),
                Utility::Mathematics::TruncateToInt(location.Y),
                Utility::Mathematics::TruncateToInt(size.X),
                Utility::Mathematics::TruncateToInt(size.Y)};
        }

        void DrawPolygonEdge(
            RenderContext& context,
            const TextureRegion& region,
            const Vector2 point1,
            const Vector2 point2,
            const Color color,
            const float thickness)
        {
            ValidateSpriteVector(point1);
            ValidateSpriteVector(point2);
            ValidateSpriteScalar(thickness);

            const float length = Vector2::Distance(point1, point2);
            ValidateSpriteScalar(length);
            const float angle = std::atan2(point2.Y - point1.Y, point2.X - point1.X);
            ValidateSpriteScalar(angle);

            context.Draw(
                *region.getTextureProperty(),
                point1,
                region.getBoundsProperty(),
                color,
                angle,
                Vector2(length, thickness),
                0.0F);
        }

        [[nodiscard]] std::vector<Vector2> CreateArcHelper(
            const double radius,
            const int sides,
            const double step,
            double theta = 0.0)
        {
            std::vector<Vector2> points(static_cast<std::size_t>(sides));
            for (int i = 0; i < sides; ++i)
            {
                const Vector2 point(
                    static_cast<float>(radius * std::cos(theta)),
                    static_cast<float>(radius * std::sin(theta)));
                ValidateSpriteVector(point);
                points[static_cast<std::size_t>(i)] = point;
                theta += step;
            }
            return points;
        }

        [[nodiscard]] std::vector<Vector2> CreateCircle(
            const float radius,
            const int sides)
        {
            if (sides < 0)
            {
                throw std::invalid_argument("RenderContext circle side count cannot be negative.");
            }
            if (sides == 0)
            {
                return {};
            }
            ValidateSpriteScalar(radius);

            constexpr double fullCircle = 2.0 * std::numbers::pi_v<double>;
            return CreateArcHelper(
                radius, sides, fullCircle / static_cast<double>(sides));
        }

        [[nodiscard]] std::vector<Vector2> CreateArc(
            const float radius,
            const int sides,
            const float startAngle,
            const float endAngle)
        {
            if (sides < 0)
            {
                throw std::invalid_argument("RenderContext arc side count cannot be negative.");
            }
            if (sides == 0)
            {
                return {};
            }
            ValidateSpriteScalar(radius);
            ValidateSpriteScalar(startAngle);
            ValidateSpriteScalar(endAngle);

            const double maximum = std::max(
                static_cast<double>(endAngle) - static_cast<double>(startAngle), 0.0);
            return CreateArcHelper(
                radius,
                sides,
                maximum / static_cast<double>(sides),
                startAngle);
        }
    }

    void RenderContext::FillRectangle(const Rectangle rectangle, const Color color)
    {
        FillRectangle(
            Vector2(static_cast<float>(rectangle.X), static_cast<float>(rectangle.Y)),
            Vector2(static_cast<float>(rectangle.Width), static_cast<float>(rectangle.Height)),
            color);
    }

    void RenderContext::FillRectangle(
        const Vector2 location,
        const Vector2 size,
        const Color color)
    {
        DefaultAssets::getWhiteRegionProperty()->Draw(
            *this, RectangleFromVectors(location, size), color);
    }

    void RenderContext::FillRectangle(
        const float x,
        const float y,
        const float width,
        const float height,
        const Color color)
    {
        FillRectangle(Vector2(x, y), Vector2(width, height), color);
    }

    void RenderContext::DrawRectangle(
        const Rectangle rectangle,
        const Color color,
        const float thickness)
    {
        const auto region = DefaultAssets::getWhiteRegionProperty();
        const int truncatedThickness = Utility::Mathematics::TruncateToInt(thickness);
        const Color multipliedColor =
            Utility::CrossEngineStuff::MultiplyColor(color, getOpacityProperty());
        const int right = CheckedAdd(rectangle.X, rectangle.Width);
        const int bottom = CheckedAdd(rectangle.Y, rectangle.Height);

        region->Draw(
            *this,
            Rectangle(rectangle.X, rectangle.Y, rectangle.Width, truncatedThickness),
            multipliedColor);
        region->Draw(
            *this,
            Rectangle(
                rectangle.X,
                CheckedSubtract(bottom, truncatedThickness),
                rectangle.Width,
                truncatedThickness),
            multipliedColor);
        region->Draw(
            *this,
            Rectangle(rectangle.X, rectangle.Y, truncatedThickness, rectangle.Height),
            multipliedColor);
        region->Draw(
            *this,
            Rectangle(
                CheckedSubtract(right, truncatedThickness),
                rectangle.Y,
                truncatedThickness,
                rectangle.Height),
            multipliedColor);
    }

    void RenderContext::DrawRectangle(
        const Vector2 location,
        const Vector2 size,
        const Color color,
        const float thickness)
    {
        DrawRectangle(RectangleFromVectors(location, size), color, thickness);
    }

    void RenderContext::DrawPolygon(
        const Vector2 offset,
        const std::span<const Vector2> points,
        const Color color,
        const float thickness)
    {
        if (points.empty())
        {
            return;
        }
        if (points.size() == 1U)
        {
            DrawPoint(points.front(), color,
                static_cast<float>(Utility::Mathematics::TruncateToInt(thickness)));
            return;
        }

        const auto region = DefaultAssets::getWhiteRegionProperty();
        for (std::size_t i = 0; i + 1U < points.size(); ++i)
        {
            DrawPolygonEdge(
                *this, *region, points[i] + offset, points[i + 1U] + offset,
                color, thickness);
        }
        DrawPolygonEdge(
            *this, *region, points.back() + offset, points.front() + offset,
            color, thickness);
    }

    void RenderContext::DrawLine(
        const float x1,
        const float y1,
        const float x2,
        const float y2,
        const Color color,
        const float thickness)
    {
        DrawLine(Vector2(x1, y1), Vector2(x2, y2), color, thickness);
    }

    void RenderContext::DrawLine(
        const Vector2 point1,
        const Vector2 point2,
        const Color color,
        const float thickness)
    {
        ValidateSpriteVector(point1);
        ValidateSpriteVector(point2);
        const float distance = Vector2::Distance(point1, point2);
        ValidateSpriteScalar(distance);
        const float angle = std::atan2(point2.Y - point1.Y, point2.X - point1.X);
        ValidateSpriteScalar(angle);
        DrawLine(point1, distance, angle, color, thickness);
    }

    void RenderContext::DrawLine(
        Vector2 point,
        const float length,
        const float angle,
        const Color color,
        const float thickness)
    {
        ValidateSpriteVector(point);
        ValidateSpriteScalar(length);
        ValidateSpriteScalar(angle);
        ValidateSpriteScalar(thickness);

        const double yOffset = static_cast<double>(thickness) * std::cos(angle) / 2.0;
        if (!std::isfinite(yOffset))
        {
            throw std::invalid_argument("RenderContext line offset must be finite.");
        }
        const int truncatedOffset = Utility::Mathematics::TruncateToInt(
            static_cast<float>(yOffset));
        point.Y = static_cast<float>(
            static_cast<double>(point.Y) - static_cast<double>(truncatedOffset));
        ValidateSpriteVector(point);

        const auto region = DefaultAssets::getWhiteRegionProperty();
        Draw(
            *region->getTextureProperty(),
            point,
            region->getBoundsProperty(),
            color,
            angle,
            Vector2(length, thickness),
            0.0F);
    }

    void RenderContext::DrawPoint(
        const float x,
        const float y,
        const Color color,
        const float size)
    {
        DrawPoint(Vector2(x, y), color, size);
    }

    void RenderContext::DrawPoint(
        const Vector2 position,
        const Color color,
        const float size)
    {
        ValidateSpriteVector(position);
        ValidateSpriteScalar(size);
        const Vector2 scale = Vector2::One * size;
        const Vector2 offset = Vector2(0.5F) - Vector2(size * 0.5F);
        const Vector2 destination = position + offset;
        ValidateSpriteVector(destination);

        const auto region = DefaultAssets::getWhiteRegionProperty();
        Draw(
            *region->getTextureProperty(),
            destination,
            region->getBoundsProperty(),
            color,
            0.0F,
            scale,
            0.0F);
    }

    void RenderContext::DrawCircle(
        const Vector2 center,
        const float radius,
        const int sides,
        const Color color,
        const float thickness)
    {
        const auto points = CreateCircle(radius, sides);
        DrawPolygon(center, points, color, thickness);
    }

    void RenderContext::DrawCircle(
        const float x,
        const float y,
        const float radius,
        const int sides,
        const Color color,
        const float thickness)
    {
        DrawCircle(Vector2(x, y), radius, sides, color, thickness);
    }

    void RenderContext::DrawArc(
        const Vector2 center,
        const float radius,
        const int sides,
        const Color color,
        const float startAngle,
        const float endAngle,
        const float thickness)
    {
        const auto points = CreateArc(radius, sides, startAngle, endAngle);
        DrawPolygon(center, points, color, thickness);
    }

    void RenderContext::DrawArc(
        const float x,
        const float y,
        const float radius,
        const int sides,
        const Color color,
        const float startAngle,
        const float endAngle,
        const float thickness)
    {
        DrawArc(
            Vector2(x, y), radius, sides, color, startAngle, endAngle, thickness);
    }
}
