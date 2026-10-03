// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/NinePatchRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"
#include "Myra/Graphics2D/Thickness.hpp"

namespace Myra::Graphics2D::TextureAtlases
{
    /** @brief Nine-slice texture region with fixed corners and stretchable edges/center. */
    class NinePatchRegion final : public TextureRegion
    {
    public:
        NinePatchRegion(
            std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> texture,
            Microsoft::Xna::Framework::Rectangle bounds,
            Thickness info);
        ~NinePatchRegion() override = default;

        [[nodiscard]] Thickness getInfoProperty() const noexcept;

        void Draw(
            RenderContext& context,
            Microsoft::Xna::Framework::Rectangle destination,
            Microsoft::Xna::Framework::Color color) const override;

    private:
        Thickness info_;
        std::unique_ptr<TextureRegion> topLeft_;
        std::unique_ptr<TextureRegion> topCenter_;
        std::unique_ptr<TextureRegion> topRight_;
        std::unique_ptr<TextureRegion> centerLeft_;
        std::unique_ptr<TextureRegion> center_;
        std::unique_ptr<TextureRegion> centerRight_;
        std::unique_ptr<TextureRegion> bottomLeft_;
        std::unique_ptr<TextureRegion> bottomCenter_;
        std::unique_ptr<TextureRegion> bottomRight_;
    };
}
