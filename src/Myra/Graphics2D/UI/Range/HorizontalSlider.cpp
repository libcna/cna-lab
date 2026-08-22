// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/HorizontalSlider.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Range/HorizontalSlider.hpp"

namespace Myra::Graphics2D::UI
{
    HorizontalSlider::HorizontalSlider()
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Top);
    }

    Orientation HorizontalSlider::getOrientationProperty() const noexcept
    {
        return Orientation::Horizontal;
    }

    HorizontalAlignment HorizontalSlider::getHorizontalAlignmentProperty() const noexcept
    {
        return Slider::getHorizontalAlignmentProperty();
    }

    void HorizontalSlider::setHorizontalAlignmentProperty(const HorizontalAlignment value)
    {
        Slider::setHorizontalAlignmentProperty(value);
    }

    std::shared_ptr<Widget> HorizontalSlider::CreateCloneInstance() const
    {
        return std::make_shared<HorizontalSlider>();
    }
} // namespace Myra::Graphics2D::UI
