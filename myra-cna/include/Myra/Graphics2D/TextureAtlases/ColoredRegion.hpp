// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/ColoredRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Graphics2D/IImage.hpp"

namespace Myra::Graphics2D::TextureAtlases
{
    class TextureRegion;

    /** @brief Retained texture region drawn with a configurable tint. */
    class ColoredRegion final : public IImage
    {
    public:
        ColoredRegion(
            std::shared_ptr<TextureRegion> textureRegion,
            Microsoft::Xna::Framework::Color color);
        ~ColoredRegion() override = default;

        [[nodiscard]] std::shared_ptr<TextureRegion> getTextureRegionProperty() const;
        void setTextureRegionProperty(std::shared_ptr<TextureRegion> value);

        [[nodiscard]] Microsoft::Xna::Framework::Point getSizeProperty() const override;

        [[nodiscard]] Microsoft::Xna::Framework::Color getColorProperty() const noexcept;
        void setColorProperty(Microsoft::Xna::Framework::Color value) noexcept;

        void Draw(
            RenderContext& context,
            Microsoft::Xna::Framework::Rectangle destination,
            Microsoft::Xna::Framework::Color color) const override;

    private:
        std::shared_ptr<TextureRegion> textureRegion_;
        Microsoft::Xna::Framework::Color color_;
    };
}
