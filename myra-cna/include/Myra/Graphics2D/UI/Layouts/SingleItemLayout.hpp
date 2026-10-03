// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Layouts/SingleItemLayout.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <stdexcept>

#include "Myra/Graphics2D/UI/ILayout.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Layout implementation for a container whose first child has type @p T. */
    template<typename T>
    class SingleItemLayout final : public ILayout
    {
    public:
        explicit SingleItemLayout(Widget& container) noexcept : container_(container) {}

        [[nodiscard]] std::shared_ptr<T> getChildProperty() const
        {
            const auto& children = container_.getChildrenProperty();
            if (children.empty())
            {
                return nullptr;
            }

            const std::shared_ptr<T> child = std::dynamic_pointer_cast<T>(children.front());
            if (!child)
            {
                throw std::bad_cast();
            }
            return child;
        }

        void setChildProperty(std::shared_ptr<T> value)
        {
            container_.ClearChildren();
            if (value)
            {
                container_.AddChild(std::move(value));
            }
        }

        [[nodiscard]] Microsoft::Xna::Framework::Point Measure(
            const std::vector<std::shared_ptr<Widget>>& /*widgets*/,
            const Microsoft::Xna::Framework::Point availableSize) override
        {
            const std::shared_ptr<T> child = getChildProperty();
            return child ? child->Measure(availableSize) : Microsoft::Xna::Framework::Point(0, 0);
        }

        void Arrange(const std::vector<std::shared_ptr<Widget>>& /*widgets*/,
            const Microsoft::Xna::Framework::Rectangle bounds) override
        {
            const std::shared_ptr<T> child = getChildProperty();
            if (child && child->getVisibleProperty())
            {
                child->Arrange(bounds);
            }
        }

    private:
        Widget& container_;
    };
}
