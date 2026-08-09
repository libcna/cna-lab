// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/VerticalSeparator.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/VerticalSeparator.hpp"

namespace Myra::Graphics2D::UI
{
    VerticalSeparator::VerticalSeparator()
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Center);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);
    }

    HorizontalAlignment VerticalSeparator::getHorizontalAlignmentProperty() const noexcept
    {
        return SeparatorWidget::getHorizontalAlignmentProperty();
    }

    void VerticalSeparator::setHorizontalAlignmentProperty(const HorizontalAlignment value)
    {
        SeparatorWidget::setHorizontalAlignmentProperty(value);
    }

    VerticalAlignment VerticalSeparator::getVerticalAlignmentProperty() const noexcept
    {
        return SeparatorWidget::getVerticalAlignmentProperty();
    }

    void VerticalSeparator::setVerticalAlignmentProperty(const VerticalAlignment value)
    {
        SeparatorWidget::setVerticalAlignmentProperty(value);
    }

    Orientation VerticalSeparator::getOrientationProperty() const noexcept
    {
        return Orientation::Vertical;
    }

    std::shared_ptr<Widget> VerticalSeparator::CreateCloneInstance() const
    {
        return std::make_shared<VerticalSeparator>();
    }
}
