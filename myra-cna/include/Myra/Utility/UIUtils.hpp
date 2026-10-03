// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/UIUtils.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace Myra::Graphics2D::UI
{
    class Widget;
}

namespace Myra::Utility
{
    /** @brief Widget-tree traversal and stable Z-index ordering helpers. */
    class UIUtils final
    {
    public:
        using WidgetOperation = std::function<bool(Graphics2D::UI::Widget&)>;

        /** @brief Visits visible widgets depth-first and stops when the operation returns false. */
        [[nodiscard]] static bool ProcessWidgets(
            Graphics2D::UI::Widget& root, const WidgetOperation& operation);

        /** @brief Stably orders widgets by ascending ZIndex using the upstream bubble sort. */
        static void SortWidgetsByZIndex(
            std::vector<std::shared_ptr<Graphics2D::UI::Widget>>& widgets);
    };
}
