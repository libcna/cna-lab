// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Container.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Container.hpp"

#include <stdexcept>
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

    bool Container::InputFallsThrough(const Microsoft::Xna::Framework::Point)
    {
        return getBackgroundProperty() == nullptr;
    }

    std::shared_ptr<Widget> Container::CreateCloneInstance() const
    {
        return std::make_shared<Container>();
    }

    void Container::CopyFrom(const Widget& source)
    {
        Widget::CopyFrom(source);
        const auto* const container = dynamic_cast<const Container*>(&source);
        if (container == nullptr)
        {
            throw std::invalid_argument("Container copy source must be a Container.");
        }

        const std::vector<std::shared_ptr<Widget>> snapshot = container->getWidgetsProperty();
        for (const std::shared_ptr<Widget>& child : snapshot)
        {
            AddWidget(child->Clone());
        }
    }
}
