// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/VerticalSlider.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Graphics2D/UI/Range/Slider.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Slider whose knob moves top to bottom. */
    class VerticalSlider final : public Slider
    {
      public:
        VerticalSlider();
        ~VerticalSlider() override = default;

        [[nodiscard]] Orientation getOrientationProperty() const noexcept override;
        [[nodiscard]] HorizontalAlignment getHorizontalAlignmentProperty() const noexcept override;
        void setHorizontalAlignmentProperty(HorizontalAlignment value) override;
        [[nodiscard]] VerticalAlignment getVerticalAlignmentProperty() const noexcept override;
        void setVerticalAlignmentProperty(VerticalAlignment value) override;

      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
    };
} // namespace Myra::Graphics2D::UI
