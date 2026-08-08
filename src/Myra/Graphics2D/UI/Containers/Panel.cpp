// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Panel.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"

#include <algorithm>

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;

    Point Panel::InternalMeasure(const Point availableSize)
    {
        Point result(0, 0);
        for (const std::shared_ptr<Widget>& widget : getChildrenCopyProperty())
        {
            if (!widget->getVisibleProperty())
            {
                continue;
            }
            const Point measure = widget->Measure(availableSize);
            result.X = std::max(result.X, measure.X);
            result.Y = std::max(result.Y, measure.Y);
        }
        return result;
    }

    void Panel::InternalArrange()
    {
        const auto actualBounds = getActualBoundsProperty();
        for (const std::shared_ptr<Widget>& widget : getChildrenCopyProperty())
        {
            if (widget->getVisibleProperty())
            {
                widget->Arrange(actualBounds);
            }
        }
    }
}
