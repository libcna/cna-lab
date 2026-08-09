// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/VerticalSeparator.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Graphics2D/UI/Simple/SeparatorWidget.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Image-backed vertical separator, stretched across its available height. */
    class VerticalSeparator : public SeparatorWidget
    {
    public:
        VerticalSeparator();
        ~VerticalSeparator() override = default;

        [[nodiscard]] HorizontalAlignment getHorizontalAlignmentProperty() const noexcept override;
        void setHorizontalAlignmentProperty(HorizontalAlignment value) override;
        [[nodiscard]] VerticalAlignment getVerticalAlignmentProperty() const noexcept override;
        void setVerticalAlignmentProperty(VerticalAlignment value) override;
        [[nodiscard]] Orientation getOrientationProperty() const noexcept override;

    protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
    };
}
