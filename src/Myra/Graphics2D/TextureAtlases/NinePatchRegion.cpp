// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/NinePatchRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/NinePatchRegion.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Myra::Graphics2D::TextureAtlases
{
    namespace
    {
        [[nodiscard]] int Checked(const std::int64_t value)
        {
            if (value < std::numeric_limits<int>::min()
                || value > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("NinePatchRegion geometry overflows int.");
            }
            return static_cast<int>(value);
        }

        [[nodiscard]] Microsoft::Xna::Framework::Rectangle MakeRectangle(
            const std::int64_t x,
            const std::int64_t y,
            const std::int64_t width,
            const std::int64_t height)
        {
            return {Checked(x), Checked(y), Checked(width), Checked(height)};
        }
    }

    NinePatchRegion::NinePatchRegion(
        std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> texture,
        const Microsoft::Xna::Framework::Rectangle bounds,
        const Thickness info)
        : TextureRegion(std::move(texture), bounds), info_(info)
    {
        const int centerWidth = Checked(
            static_cast<std::int64_t>(bounds.Width) - info.Left - info.Right);
        const int centerHeight = Checked(
            static_cast<std::int64_t>(bounds.Height) - info.Top - info.Bottom);
        const auto retainedTexture = getTextureProperty();

        int y = bounds.Y;
        if (info.Top > 0)
        {
            if (info.Left > 0)
            {
                topLeft_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(bounds.X, y, info.Left, info.Top));
            }
            if (centerWidth > 0)
            {
                topCenter_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(
                        static_cast<std::int64_t>(bounds.X) + info.Left,
                        y,
                        centerWidth,
                        info.Top));
            }
            if (info.Right > 0)
            {
                topRight_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(
                        static_cast<std::int64_t>(bounds.X) + info.Left + centerWidth,
                        y,
                        info.Right,
                        info.Top));
            }
        }

        y = Checked(static_cast<std::int64_t>(y) + info.Top);
        if (centerHeight > 0)
        {
            if (info.Left > 0)
            {
                centerLeft_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(bounds.X, y, info.Left, centerHeight));
            }
            if (centerWidth > 0)
            {
                center_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(
                        static_cast<std::int64_t>(bounds.X) + info.Left,
                        y,
                        centerWidth,
                        centerHeight));
            }
            if (info.Right > 0)
            {
                centerRight_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(
                        static_cast<std::int64_t>(bounds.X) + info.Left + centerWidth,
                        y,
                        info.Right,
                        centerHeight));
            }
        }

        y = Checked(static_cast<std::int64_t>(y) + centerHeight);
        if (info.Bottom > 0)
        {
            if (info.Left > 0)
            {
                bottomLeft_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(bounds.X, y, info.Left, info.Bottom));
            }
            if (centerWidth > 0)
            {
                bottomCenter_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(
                        static_cast<std::int64_t>(bounds.X) + info.Left,
                        y,
                        centerWidth,
                        info.Bottom));
            }
            if (info.Right > 0)
            {
                bottomRight_ = std::make_unique<TextureRegion>(retainedTexture,
                    MakeRectangle(
                        static_cast<std::int64_t>(bounds.X) + info.Left + centerWidth,
                        y,
                        info.Right,
                        info.Bottom));
            }
        }
    }

    Thickness NinePatchRegion::getInfoProperty() const noexcept
    {
        return info_;
    }

    void NinePatchRegion::Draw(
        RenderContext& context,
        const Microsoft::Xna::Framework::Rectangle destination,
        const Microsoft::Xna::Framework::Color color) const
    {
        int y = destination.Y;
        const int left = std::min(info_.Left, destination.Width);
        const int top = std::min(info_.Top, destination.Height);
        const int right = std::min(info_.Right, destination.Width);
        const int bottom = std::min(info_.Bottom, destination.Height);
        const int centerWidth = std::max(
            Checked(static_cast<std::int64_t>(destination.Width) - left - right), 0);
        const int centerHeight = std::max(
            Checked(static_cast<std::int64_t>(destination.Height) - top - bottom), 0);
        const int centerX = Checked(static_cast<std::int64_t>(destination.X) + left);
        // Preserve selected-upstream positioning: right patches use the source
        // left thickness rather than its destination-clamped counterpart.
        const int rightX = Checked(
            static_cast<std::int64_t>(destination.X) + info_.Left + centerWidth);

        if (topLeft_ != nullptr)
        {
            topLeft_->Draw(context,
                MakeRectangle(destination.X, y, left, top), color);
        }
        if (topCenter_ != nullptr && centerWidth > 0)
        {
            topCenter_->Draw(context,
                MakeRectangle(centerX, y, centerWidth, top), color);
        }
        if (topRight_ != nullptr)
        {
            topRight_->Draw(context,
                MakeRectangle(rightX, y, right, top), color);
        }

        y = Checked(static_cast<std::int64_t>(y) + top);
        if (centerLeft_ != nullptr && centerHeight > 0)
        {
            centerLeft_->Draw(context,
                MakeRectangle(destination.X, y, left, centerHeight), color);
        }
        if (center_ != nullptr && centerWidth > 0 && centerHeight > 0)
        {
            center_->Draw(context,
                MakeRectangle(centerX, y, centerWidth, centerHeight), color);
        }
        if (centerRight_ != nullptr && centerHeight > 0)
        {
            centerRight_->Draw(context,
                MakeRectangle(rightX, y, right, centerHeight), color);
        }

        y = Checked(static_cast<std::int64_t>(y) + centerHeight);
        if (bottomLeft_ != nullptr)
        {
            bottomLeft_->Draw(context,
                MakeRectangle(destination.X, y, left, bottom), color);
        }
        if (bottomCenter_ != nullptr && centerWidth > 0)
        {
            bottomCenter_->Draw(context,
                MakeRectangle(centerX, y, centerWidth, bottom), color);
        }
        if (bottomRight_ != nullptr)
        {
            bottomRight_->Draw(context,
                MakeRectangle(rightX, y, right, bottom), color);
        }
    }
}
