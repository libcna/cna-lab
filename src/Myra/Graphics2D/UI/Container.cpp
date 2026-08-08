// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Container.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Container.hpp"

#include <utility>

namespace Myra::Graphics2D::UI
{
    Container::Container()
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        setVerticalAlignmentProperty(VerticalAlignment::Stretch);
    }

    const std::vector<std::shared_ptr<Widget>>& Container::getWidgetsProperty() const noexcept
    {
        return getChildrenProperty();
    }

    void Container::AddWidget(std::shared_ptr<Widget> widget)
    {
        AddChild(std::move(widget));
    }

    bool Container::RemoveWidget(const Widget* const widget)
    {
        return RemoveChild(widget);
    }
}
