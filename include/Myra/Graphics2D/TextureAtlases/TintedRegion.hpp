// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TintedRegion.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <cstddef>
#include <memory>

#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/MML/IHasColor.hpp"

namespace Myra::Graphics2D::TextureAtlases
{
    class TextureRegion;

    /** @brief Immutable retained texture region with a color tint. */
    class TintedRegion final : public IImage, public MML::IHasColor
    {
    public:
        static constexpr char Separator = '|';

        TintedRegion(
            std::shared_ptr<TextureRegion> region,
            Microsoft::Xna::Framework::Color color);
        ~TintedRegion() override = default;

        [[nodiscard]] std::shared_ptr<TextureRegion> getRegionProperty() const;
        [[nodiscard]] Microsoft::Xna::Framework::Color getColorProperty() const override;
        [[nodiscard]] Microsoft::Xna::Framework::Point getSizeProperty() const override;

        void Draw(
            RenderContext& context,
            Microsoft::Xna::Framework::Rectangle destination,
            Microsoft::Xna::Framework::Color color) const override;

        [[nodiscard]] bool Equals(const TintedRegion& other) const noexcept;
        [[nodiscard]] std::size_t GetHashCode() const noexcept;

    private:
        std::shared_ptr<TextureRegion> region_;
        Microsoft::Xna::Framework::Color color_;
    };
}
