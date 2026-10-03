// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TextureRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <optional>
#include <string>

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IImage.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace Myra::Graphics2D::TextureAtlases
{
    /** @brief Rectangular, retained region of a CNA Texture2D. */
    class TextureRegion : public IImage
    {
    public:
        TextureRegion(
            std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> texture,
            Microsoft::Xna::Framework::Rectangle bounds);
        TextureRegion(
            const TextureRegion& region,
            Microsoft::Xna::Framework::Rectangle relativeBounds);
        explicit TextureRegion(
            std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> texture);
        ~TextureRegion() override = default;

        [[nodiscard]] const std::optional<std::string>& getNameProperty() const noexcept;
        void setNameProperty(std::optional<std::string> value);

        /** @brief Returns a retained handle to this region's texture. */
        [[nodiscard]] std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D>
            getTextureProperty() const;
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getBoundsProperty() const noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Point getSizeProperty() const override;

        void Draw(
            RenderContext& context,
            Microsoft::Xna::Framework::Rectangle destination,
            Microsoft::Xna::Framework::Color color) const override;

        [[nodiscard]] virtual std::string ToString() const;

    private:
        std::optional<std::string> name_;
        std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> texture_;
        Microsoft::Xna::Framework::Rectangle bounds_;
    };
}
