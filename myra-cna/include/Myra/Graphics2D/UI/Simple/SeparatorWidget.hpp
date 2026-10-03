// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/SeparatorWidget.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Graphics2D/UI/Simple/Image.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Abstract image-backed separator with a fixed cross-axis thickness. */
    class SeparatorWidget : public Image
    {
    public:
        ~SeparatorWidget() override = default;

        [[nodiscard]] int getThicknessProperty() const noexcept;
        void setThicknessProperty(int value);
        [[nodiscard]] virtual Orientation getOrientationProperty() const noexcept = 0;

        void InternalRender(Graphics2D::RenderContext& context) override;

    protected:
        SeparatorWidget() = default;

        [[nodiscard]] Microsoft::Xna::Framework::Point InternalMeasure(
            Microsoft::Xna::Framework::Point availableSize) override;
        void CopyFrom(const Widget& source) override;

    private:
        int thickness_ = 0;
    };
}
