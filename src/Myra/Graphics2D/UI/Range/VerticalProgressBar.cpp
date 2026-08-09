// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/VerticalProgressBar.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Range/VerticalProgressBar.hpp"

namespace Myra::Graphics2D::UI
{
    VerticalProgressBar::VerticalProgressBar()
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Left);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);
    }

    Orientation VerticalProgressBar::getOrientationProperty() const noexcept
    {
        return Orientation::Vertical;
    }

    HorizontalAlignment VerticalProgressBar::getHorizontalAlignmentProperty() const noexcept
    {
        return ProgressBar::getHorizontalAlignmentProperty();
    }

    void VerticalProgressBar::setHorizontalAlignmentProperty(
        const HorizontalAlignment value)
    {
        ProgressBar::setHorizontalAlignmentProperty(value);
    }

    VerticalAlignment VerticalProgressBar::getVerticalAlignmentProperty() const noexcept
    {
        return ProgressBar::getVerticalAlignmentProperty();
    }

    void VerticalProgressBar::setVerticalAlignmentProperty(const VerticalAlignment value)
    {
        ProgressBar::setVerticalAlignmentProperty(value);
    }

    std::shared_ptr<Widget> VerticalProgressBar::CreateCloneInstance() const
    {
        return std::make_shared<VerticalProgressBar>();
    }
}
