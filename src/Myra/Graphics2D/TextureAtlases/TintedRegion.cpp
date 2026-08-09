// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TintedRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/TintedRegion.hpp"

#include <functional>
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

    TintedRegion::TintedRegion(
        std::shared_ptr<TextureRegion> region,
        const Microsoft::Xna::Framework::Color color)
        : region_(std::move(region)), color_(color)
    {
        if (region_ == nullptr)
        {
            throw std::invalid_argument("TintedRegion requires a non-null TextureRegion.");
        }
    }

    std::shared_ptr<TextureRegion> TintedRegion::getRegionProperty() const
    {
        return region_;
    }

    Microsoft::Xna::Framework::Color TintedRegion::getColorProperty() const
    {
        return color_;
    }

    Microsoft::Xna::Framework::Point TintedRegion::getSizeProperty() const
    {
        return region_->getSizeProperty();
    }

    void TintedRegion::Draw(
        RenderContext& context,
        const Microsoft::Xna::Framework::Rectangle destination,
        Microsoft::Xna::Framework::Color color) const
    {
        if (color_ != Microsoft::Xna::Framework::Color::White)
        {
            color = MultiplyColors(color_, color);
        }
        region_->Draw(context, destination, color);
    }

    bool TintedRegion::Equals(const TintedRegion& other) const noexcept
    {
        return region_ == other.region_ && color_ == other.color_;
    }

    std::size_t TintedRegion::GetHashCode() const noexcept
    {
        return std::hash<const TextureRegion*>{}(region_.get()) ^ color_.GetHashCode();
    }
}
