// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/Brushes/SolidBrush.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/MML/IHasColor.hpp"

namespace Myra::Graphics2D::Brushes
{
    /** @brief Brush that fills a rectangle with a tintable solid color. */
    class SolidBrush : public IBrush, public MML::IHasColor
    {
    public:
        explicit SolidBrush(Microsoft::Xna::Framework::Color color);
        ~SolidBrush() override = default;

        [[nodiscard]] Microsoft::Xna::Framework::Color getColorProperty() const override;
        void setColorProperty(Microsoft::Xna::Framework::Color value) noexcept;

        void Draw(
            RenderContext& context,
            Microsoft::Xna::Framework::Rectangle destination,
            Microsoft::Xna::Framework::Color color) const override;

    private:
        Microsoft::Xna::Framework::Color color_ =
            Microsoft::Xna::Framework::Color::White;
    };
}
