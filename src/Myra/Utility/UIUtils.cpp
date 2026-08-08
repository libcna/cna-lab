// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/UIUtils.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Utility/UIUtils.hpp"

#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::Utility
{
    bool UIUtils::ProcessWidgets(Graphics2D::UI::Widget& root, const WidgetOperation& operation)
    {
        if (!operation)
        {
            throw std::invalid_argument("A widget traversal operation cannot be empty.");
        }
        if (!root.getVisibleProperty())
        {
            return true;
        }
        if (!operation(root))
        {
            return false;
        }
        const std::vector<std::shared_ptr<Graphics2D::UI::Widget>> snapshot =
            root.getChildrenCopyProperty();
        for (const std::shared_ptr<Graphics2D::UI::Widget>& widget : snapshot)
        {
            if (!widget)
            {
                throw std::logic_error("A widget hierarchy cannot contain a null child.");
            }
            if (!ProcessWidgets(*widget, operation))
            {
                return false;
            }
        }
        return true;
    }

    void UIUtils::SortWidgetsByZIndex(
        std::vector<std::shared_ptr<Graphics2D::UI::Widget>>& widgets)
    {
        size_t count = widgets.size();
        do
        {
            size_t newCount = 0;
            for (size_t index = 1; index < count; ++index)
            {
                const std::shared_ptr<Graphics2D::UI::Widget>& previous = widgets[index - 1];
                const std::shared_ptr<Graphics2D::UI::Widget>& current = widgets[index];
                if (!previous || !current)
                {
                    throw std::invalid_argument("A widget Z-order list cannot contain null entries.");
                }
                if (previous->getZIndexProperty() > current->getZIndexProperty())
                {
                    std::swap(widgets[index - 1], widgets[index]);
                    newCount = index;
                }
            }
            count = newCount;
        }
        while (count > 1);
    }
}
