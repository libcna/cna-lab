// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/VerticalSlider.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Range/VerticalSlider.hpp"

namespace Myra::Graphics2D::UI
{
    VerticalSlider::VerticalSlider()
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Left);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);
    }

    Orientation VerticalSlider::getOrientationProperty() const noexcept
    {
        return Orientation::Vertical;
    }

    HorizontalAlignment VerticalSlider::getHorizontalAlignmentProperty() const noexcept
    {
        return Slider::getHorizontalAlignmentProperty();
    }

    void VerticalSlider::setHorizontalAlignmentProperty(const HorizontalAlignment value)
    {
        Slider::setHorizontalAlignmentProperty(value);
    }

    VerticalAlignment VerticalSlider::getVerticalAlignmentProperty() const noexcept
    {
        return Slider::getVerticalAlignmentProperty();
    }

    void VerticalSlider::setVerticalAlignmentProperty(const VerticalAlignment value)
    {
        Slider::setVerticalAlignmentProperty(value);
    }

    std::shared_ptr<Widget> VerticalSlider::CreateCloneInstance() const
    {
        return std::make_shared<VerticalSlider>();
    }
} // namespace Myra::Graphics2D::UI
