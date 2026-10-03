// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TextureRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"

namespace Myra::Graphics2D::TextureAtlases
{
    namespace
    {
        [[nodiscard]] int CheckedOffset(const int coordinate, const int offset)
        {
            const std::int64_t result = static_cast<std::int64_t>(coordinate) + offset;
            if (result < std::numeric_limits<int>::min()
                || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("TextureRegion relative bounds overflow int.");
            }
            return static_cast<int>(result);
        }
    }

    TextureRegion::TextureRegion(
        std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> texture,
        const Microsoft::Xna::Framework::Rectangle bounds)
        : texture_(std::move(texture)), bounds_(bounds)
    {
        if (texture_ == nullptr)
        {
            throw std::invalid_argument("TextureRegion requires a non-null Texture2D handle.");
        }
    }

    TextureRegion::TextureRegion(
        const TextureRegion& region,
        Microsoft::Xna::Framework::Rectangle relativeBounds)
        : texture_(region.texture_), bounds_(relativeBounds)
    {
        bounds_.X = CheckedOffset(bounds_.X, region.bounds_.X);
        bounds_.Y = CheckedOffset(bounds_.Y, region.bounds_.Y);
    }

    TextureRegion::TextureRegion(
        std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> texture)
        : TextureRegion(
            texture,
            texture == nullptr
                ? Microsoft::Xna::Framework::Rectangle()
                : Microsoft::Xna::Framework::Rectangle(
                    0, 0, texture->getWidthProperty(), texture->getHeightProperty()))
    {
    }

    const std::optional<std::string>& TextureRegion::getNameProperty() const noexcept
    {
        return name_;
    }

    void TextureRegion::setNameProperty(std::optional<std::string> value)
    {
        name_ = std::move(value);
    }

    std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D>
        TextureRegion::getTextureProperty() const
    {
        return texture_;
    }

    Microsoft::Xna::Framework::Rectangle TextureRegion::getBoundsProperty() const noexcept
    {
        return bounds_;
    }

    Microsoft::Xna::Framework::Point TextureRegion::getSizeProperty() const
    {
        return {bounds_.Width, bounds_.Height};
    }

    void TextureRegion::Draw(
        RenderContext& context,
        const Microsoft::Xna::Framework::Rectangle destination,
        const Microsoft::Xna::Framework::Color color) const
    {
        context.Draw(*texture_, destination, bounds_, color);
    }

    std::string TextureRegion::ToString() const
    {
        return name_.value_or(std::string());
    }
}
