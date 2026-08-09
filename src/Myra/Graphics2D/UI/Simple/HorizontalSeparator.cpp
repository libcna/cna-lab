// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/HorizontalSeparator.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/HorizontalSeparator.hpp"

namespace Myra::Graphics2D::UI
{
    HorizontalSeparator::HorizontalSeparator()
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Center);
    }

    HorizontalAlignment HorizontalSeparator::getHorizontalAlignmentProperty() const noexcept
    {
        return SeparatorWidget::getHorizontalAlignmentProperty();
    }

    void HorizontalSeparator::setHorizontalAlignmentProperty(const HorizontalAlignment value)
    {
        SeparatorWidget::setHorizontalAlignmentProperty(value);
    }

    VerticalAlignment HorizontalSeparator::getVerticalAlignmentProperty() const noexcept
    {
        return SeparatorWidget::getVerticalAlignmentProperty();
    }

    void HorizontalSeparator::setVerticalAlignmentProperty(const VerticalAlignment value)
    {
        SeparatorWidget::setVerticalAlignmentProperty(value);
    }

    Orientation HorizontalSeparator::getOrientationProperty() const noexcept
    {
        return Orientation::Horizontal;
    }

    std::shared_ptr<Widget> HorizontalSeparator::CreateCloneInstance() const
    {
        return std::make_shared<HorizontalSeparator>();
    }
}
