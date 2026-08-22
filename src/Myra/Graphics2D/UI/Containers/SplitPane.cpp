// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/SplitPane.cs,
// src/Myra/Graphics2D/UI/Containers/HorizontalSplitPane.cs, and src/Myra/Graphics2D/UI/Containers/VerticalSplitPane.cs
// at 0d79b939310bfe1d00b21803fe15e291caf60aa1. See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/SplitPane.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    SplitPane::SplitPane()
    {
        setChildrenLayoutProperty(&layout_);
    }

    const std::vector<std::shared_ptr<Widget>> &SplitPane::getWidgetsProperty() const noexcept
    {
        return widgets_;
    }

    void SplitPane::AddWidget(std::shared_ptr<Widget> widget)
    {
        InsertWidget(widgets_.size(), std::move(widget));
    }

    void SplitPane::InsertWidget(const std::size_t index, std::shared_ptr<Widget> widget)
    {
        if (!widget)
        {
            throw std::invalid_argument("A SplitPane widget cannot be null.");
        }
        if (index > widgets_.size())
        {
            throw std::out_of_range("SplitPane widget index is outside the collection.");
        }
        if (std::find(widgets_.begin(), widgets_.end(), widget) != widgets_.end())
        {
            throw std::invalid_argument("A SplitPane cannot contain the same widget more than once.");
        }
        widgets_.insert(widgets_.begin() + static_cast<std::ptrdiff_t>(index), std::move(widget));
        Reset();
    }

    bool SplitPane::RemoveWidget(const Widget *const widget)
    {
        const auto iterator =
            std::find_if(widgets_.begin(), widgets_.end(), [widget](const auto &item) { return item.get() == widget; });
        if (iterator == widgets_.end())
        {
            return false;
        }
        widgets_.erase(iterator);
        Reset();
        return true;
    }

    void SplitPane::ClearWidgets()
    {
        widgets_.clear();
        Reset();
    }

    float SplitPane::GetProportion(const int widgetIndex) const noexcept
    {
        if (widgetIndex < 0 || static_cast<std::size_t>(widgetIndex) >= widgets_.size())
        {
            return 0.0F;
        }
        return GetActiveProportions()[widgetIndex * 2]->getValueProperty();
    }

    float SplitPane::GetSplitterPosition(const int leftWidgetIndex) const
    {
        std::shared_ptr<Proportion> left;
        std::shared_ptr<Proportion> right;
        float total = 0.0F;
        GetProportions(leftWidgetIndex, left, right, total);
        if (total == 0.0F)
        {
            return 0.0F;
        }
        return left->getValueProperty() / total;
    }

    void SplitPane::SetSplitterPosition(const int leftWidgetIndex, const float proportion)
    {
        std::shared_ptr<Proportion> left;
        std::shared_ptr<Proportion> right;
        float total = 0.0F;
        GetProportions(leftWidgetIndex, left, right, total);

        const float first = proportion * total;
        const float second = left->getValueProperty() + right->getValueProperty() - first;
        left->setValueProperty(first);
        right->setValueProperty(second);
        InvalidateArrange();
    }

    void SplitPane::Reset()
    {
        ClearChildren();
        handles_.clear();
        layout_.getColumnsProportionsProperty().Clear();
        layout_.getRowsProportionsProperty().Clear();

        for (std::size_t index = 0; index < widgets_.size(); ++index)
        {
            if (index > 0)
            {
                const auto handle = std::make_shared<Button>();
                handle->releaseOnTouchLeft_ = false;
                if (getOrientationProperty() == Orientation::Horizontal)
                {
                    handle->setMouseCursorProperty(MouseCursorType::SizeWE);
                    handle->setVerticalAlignmentProperty(VerticalAlignment::Stretch);
                    Grid::SetColumn(*handle, static_cast<int>(index * 2U - 1U));
                    layout_.getColumnsProportionsProperty().Add(std::make_shared<Proportion>(ProportionType::Auto));
                }
                else
                {
                    handle->setMouseCursorProperty(MouseCursorType::SizeNS);
                    handle->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
                    Grid::SetRow(*handle, static_cast<int>(index * 2U - 1U));
                    layout_.getRowsProportionsProperty().Add(std::make_shared<Proportion>(ProportionType::Auto));
                }
                AddChild(handle);
                handles_.push_back(handle);
            }

            const std::shared_ptr<Proportion> proportion = std::make_shared<Proportion>(
                index + 1U < widgets_.size() ? ProportionType::Part : ProportionType::Fill, 1.0F);
            const std::shared_ptr<Widget> &widget = widgets_[index];
            if (getOrientationProperty() == Orientation::Horizontal)
            {
                Grid::SetColumn(*widget, static_cast<int>(index * 2U));
                layout_.getColumnsProportionsProperty().Add(proportion);
            }
            else
            {
                Grid::SetRow(*widget, static_cast<int>(index * 2U));
                layout_.getRowsProportionsProperty().Add(proportion);
            }
            AddChild(widget);
        }

        FireProportionsChanged();
    }

    void SplitPane::CopyFrom(const Widget &source)
    {
        Widget::CopyFrom(source);
        const auto *const splitPane = dynamic_cast<const SplitPane *>(&source);
        if (splitPane == nullptr)
        {
            throw std::invalid_argument("SplitPane copy source must be a SplitPane.");
        }
        for (const std::shared_ptr<Widget> &widget : splitPane->widgets_)
        {
            AddWidget(widget->Clone());
        }
    }

    const ProportionCollection &SplitPane::GetActiveProportions() const noexcept
    {
        return getOrientationProperty() == Orientation::Horizontal ? layout_.getColumnsProportionsProperty()
                                                                   : layout_.getRowsProportionsProperty();
    }

    ProportionCollection &SplitPane::GetActiveProportions() noexcept
    {
        return getOrientationProperty() == Orientation::Horizontal ? layout_.getColumnsProportionsProperty()
                                                                   : layout_.getRowsProportionsProperty();
    }

    void SplitPane::GetProportions(const int leftWidgetIndex, std::shared_ptr<Proportion> &left,
                                   std::shared_ptr<Proportion> &right, float &total) const
    {
        if (leftWidgetIndex < 0 || static_cast<std::size_t>(leftWidgetIndex) + 1U >= widgets_.size())
        {
            throw std::out_of_range("SplitPane splitter index is outside the collection.");
        }
        const ProportionCollection &proportions = GetActiveProportions();
        total = 0.0F;
        for (int index = 0; index < proportions.getCountProperty(); index += 2)
        {
            total += proportions[index]->getValueProperty();
        }
        const int baseIndex = leftWidgetIndex * 2;
        left = proportions[baseIndex];
        right = proportions[baseIndex + 2];
    }

    void SplitPane::FireProportionsChanged()
    {
        Utility::EventsExtensions::Invoke(ProportionsChanged, this, InputEventType::ProportionChanged);
    }

    Orientation HorizontalSplitPane::getOrientationProperty() const noexcept
    {
        return Orientation::Horizontal;
    }

    std::shared_ptr<Widget> HorizontalSplitPane::CreateCloneInstance() const
    {
        return std::make_shared<HorizontalSplitPane>();
    }

    Orientation VerticalSplitPane::getOrientationProperty() const noexcept
    {
        return Orientation::Vertical;
    }

    std::shared_ptr<Widget> VerticalSplitPane::CreateCloneInstance() const
    {
        return std::make_shared<VerticalSplitPane>();
    }
} // namespace Myra::Graphics2D::UI
