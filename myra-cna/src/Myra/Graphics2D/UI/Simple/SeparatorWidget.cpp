// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/SeparatorWidget.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/SeparatorWidget.hpp"

#include <stdexcept>

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;

    int SeparatorWidget::getThicknessProperty() const noexcept
    {
        return thickness_;
    }

    void SeparatorWidget::setThicknessProperty(const int value)
    {
        if (value == thickness_)
        {
            return;
        }
        thickness_ = value;
        InvalidateMeasure();
    }

    Point SeparatorWidget::InternalMeasure(const Point availableSize)
    {
        static_cast<void>(availableSize);
        return getOrientationProperty() == Orientation::Horizontal
            ? Point(0, thickness_)
            : Point(thickness_, 0);
    }

    void SeparatorWidget::InternalRender(Graphics2D::RenderContext& context)
    {
        Image::InternalRender(context);
    }

    void SeparatorWidget::CopyFrom(const Widget& source)
    {
        Image::CopyFrom(source);
        const auto* const separator = dynamic_cast<const SeparatorWidget*>(&source);
        if (separator == nullptr)
        {
            throw std::invalid_argument(
                "SeparatorWidget copy source must be a SeparatorWidget.");
        }
        setThicknessProperty(separator->thickness_);
    }
}
