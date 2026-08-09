// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/Brushes/SolidBrush.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/Brushes/SolidBrush.hpp"

#include "Myra/DefaultAssets.hpp"
#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"

namespace Myra::Graphics2D::Brushes
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

    SolidBrush::SolidBrush(const Microsoft::Xna::Framework::Color color)
        : color_(color)
    {
    }

    Microsoft::Xna::Framework::Color SolidBrush::getColorProperty() const
    {
        return color_;
    }

    void SolidBrush::setColorProperty(const Microsoft::Xna::Framework::Color value) noexcept
    {
        color_ = value;
    }

    void SolidBrush::Draw(
        RenderContext& context,
        const Microsoft::Xna::Framework::Rectangle destination,
        const Microsoft::Xna::Framework::Color color) const
    {
        const auto white = DefaultAssets::getWhiteRegionProperty();
        white->Draw(
            context,
            destination,
            color == Microsoft::Xna::Framework::Color::White
                ? color_
                : MultiplyColors(color_, color));
    }
}
