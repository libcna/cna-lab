// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Range/HorizontalProgressBar.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Range/HorizontalProgressBar.hpp"

namespace Myra::Graphics2D::UI
{
    HorizontalProgressBar::HorizontalProgressBar()
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Top);
    }

    Orientation HorizontalProgressBar::getOrientationProperty() const noexcept
    {
        return Orientation::Horizontal;
    }

    HorizontalAlignment HorizontalProgressBar::getHorizontalAlignmentProperty() const noexcept
    {
        return ProgressBar::getHorizontalAlignmentProperty();
    }

    void HorizontalProgressBar::setHorizontalAlignmentProperty(
        const HorizontalAlignment value)
    {
        ProgressBar::setHorizontalAlignmentProperty(value);
    }

    VerticalAlignment HorizontalProgressBar::getVerticalAlignmentProperty() const noexcept
    {
        return ProgressBar::getVerticalAlignmentProperty();
    }

    void HorizontalProgressBar::setVerticalAlignmentProperty(const VerticalAlignment value)
    {
        ProgressBar::setVerticalAlignmentProperty(value);
    }

    std::shared_ptr<Widget> HorizontalProgressBar::CreateCloneInstance() const
    {
        return std::make_shared<HorizontalProgressBar>();
    }
}
