// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/ColoredRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/ColoredRegion.hpp"

#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"

namespace Myra::Graphics2D::TextureAtlases
{
    namespace
    {
        [[nodiscard]] Microsoft::Xna::Framework::Color MultiplyColors(
            const Microsoft::Xna::Framework::Color& left,
            const Microsoft::Xna::Framework::Color& right)
        {
            return {
                static_cast<int>(left.getRProperty() * right.getRProperty() / 255.0F),
                static_cast<int>(left.getGProperty() * right.getGProperty() / 255.0F),
                static_cast<int>(left.getBProperty() * right.getBProperty() / 255.0F),
                static_cast<int>(left.getAProperty() * right.getAProperty() / 255.0F),
            };
        }
    }

    ColoredRegion::ColoredRegion(
        std::shared_ptr<TextureRegion> textureRegion,
        const Microsoft::Xna::Framework::Color color)
        : textureRegion_(std::move(textureRegion)), color_(color)
    {
        if (textureRegion_ == nullptr)
        {
            throw std::invalid_argument("ColoredRegion requires a non-null TextureRegion.");
        }
    }

    std::shared_ptr<TextureRegion> ColoredRegion::getTextureRegionProperty() const
    {
        return textureRegion_;
    }

    void ColoredRegion::setTextureRegionProperty(std::shared_ptr<TextureRegion> value)
    {
        if (value == nullptr)
        {
            throw std::invalid_argument("ColoredRegion.TextureRegion cannot be null.");
        }
        textureRegion_ = std::move(value);
    }

    Microsoft::Xna::Framework::Point ColoredRegion::getSizeProperty() const
    {
        return textureRegion_->getSizeProperty();
    }

    Microsoft::Xna::Framework::Color ColoredRegion::getColorProperty() const noexcept
    {
        return color_;
    }

    void ColoredRegion::setColorProperty(const Microsoft::Xna::Framework::Color value) noexcept
    {
        color_ = value;
    }

    void ColoredRegion::Draw(
        RenderContext& context,
        const Microsoft::Xna::Framework::Rectangle destination,
        const Microsoft::Xna::Framework::Color color) const
    {
        textureRegion_->Draw(
            context,
            destination,
            color == Microsoft::Xna::Framework::Color::White
                ? color_
                : MultiplyColors(color_, color));
    }
}
