// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/ComboView.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Selectors/ComboView.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;

    ComboView::ComboView()
        : layout_(*this), button_(std::make_shared<ToggleButton>()), listView_(std::make_shared<ListView>())
    {
        button_->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        button_->setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        button_->setContentProperty(std::make_shared<Widget>());
        setChildrenLayoutProperty(&layout_);
        layout_.setChildProperty(button_);
        setAcceptsKeyboardFocusProperty(true);
        setHorizontalAlignmentProperty(HorizontalAlignment::Left);
        setVerticalAlignmentProperty(VerticalAlignment::Top);
        setDropdownMaximumHeightProperty(300);

        button_->PressedChanged +=
            [this](void *sender, Events::MyraEventArgs &arguments) { OnButtonPressedChanged(sender, arguments); };
        listView_->SelectedIndexChanged += [this](void *sender, Events::MyraEventArgs &arguments)
        {
            SelectedIndexChanged.Invoke(sender, arguments);
            UpdateSelectedItem();
        };
    }

    std::optional<int> ComboView::getDropdownMaximumHeightProperty() const noexcept
    {
        return listView_->getMaxHeightProperty();
    }

    void ComboView::setDropdownMaximumHeightProperty(std::optional<int> value)
    {
        listView_->setMaxHeightProperty(std::move(value));
    }

    bool ComboView::getIsExpandedProperty() const noexcept
    {
        return button_->getIsPressedProperty();
    }

    std::shared_ptr<ToggleButton> ComboView::getButtonProperty() const
    {
        return button_;
    }

    std::shared_ptr<ListView> ComboView::getListViewProperty() const
    {
        return listView_;
    }

    const std::vector<std::shared_ptr<Widget>> &ComboView::getWidgetsProperty() const noexcept
    {
        return listView_->getWidgetsProperty();
    }

    void ComboView::AddWidget(std::shared_ptr<Widget> widget)
    {
        listView_->AddWidget(std::move(widget));
    }

    bool ComboView::RemoveWidget(const Widget *const widget)
    {
        return listView_->RemoveWidget(widget);
    }

    std::shared_ptr<Widget> ComboView::getSelectedItemProperty() const
    {
        return listView_->getSelectedItemProperty();
    }

    void ComboView::setSelectedItemProperty(std::shared_ptr<Widget> value)
    {
        listView_->setSelectedItemProperty(std::move(value));
    }

    SelectionMode ComboView::getSelectionModeProperty() const noexcept
    {
        return listView_->getSelectionModeProperty();
    }

    void ComboView::setSelectionModeProperty(const SelectionMode value) noexcept
    {
        listView_->setSelectionModeProperty(value);
    }

    std::optional<int> ComboView::getSelectedIndexProperty() const
    {
        return listView_->getSelectedIndexProperty();
    }

    void ComboView::setSelectedIndexProperty(const std::optional<int> value)
    {
        listView_->setSelectedIndexProperty(value);
    }

    Point ComboView::InternalMeasure(const Point availableSize)
    {
        Point result = Widget::InternalMeasure(availableSize);
        const std::optional<int> oldWidth = listView_->getWidthProperty();
        listView_->setWidthProperty(std::nullopt);
        const bool wasVisible = listView_->getVisibleProperty();
        listView_->setVisibleProperty(true);
        const Point listResult = listView_->Measure(Point(10000, 10000));
        result.X = std::max(result.X, listResult.X) + 32;
        listView_->setWidthProperty(oldWidth);
        listView_->setVisibleProperty(wasVisible);
        return result;
    }

    void ComboView::InternalArrange()
    {
        Widget::InternalArrange();
        listView_->setWidthProperty(getActualBoundsProperty().Width);
    }

    std::shared_ptr<Widget> ComboView::CreateCloneInstance() const
    {
        return std::make_shared<ComboView>();
    }

    void ComboView::CopyFrom(const Widget &source)
    {
        Widget::CopyFrom(source);
        const auto *const comboView = dynamic_cast<const ComboView *>(&source);
        if (comboView == nullptr)
        {
            throw std::invalid_argument("ComboView copy source must be a ComboView.");
        }
        setSelectionModeProperty(comboView->getSelectionModeProperty());
        setDropdownMaximumHeightProperty(comboView->getDropdownMaximumHeightProperty());
        for (const std::shared_ptr<Widget> &widget : comboView->getWidgetsProperty())
        {
            AddWidget(widget->Clone());
        }
    }

    void ComboView::OnButtonPressedChanged(void *const sender, Events::MyraEventArgs &arguments)
    {
        static_cast<void>(sender);
        static_cast<void>(arguments);
        if (listView_->getWidgetsProperty().empty())
        {
            return;
        }
        if (button_->getIsPressedProperty() && !listView_->getSelectedIndexProperty())
        {
            listView_->setSelectedIndexProperty(0);
        }
    }

    void ComboView::UpdateSelectedItem()
    {
        const std::shared_ptr<Widget> selectedItem = listView_->getSelectedItemProperty();
        if (selectedItem)
        {
            button_->setContentProperty(selectedItem->Clone());
        }
    }
} // namespace Myra::Graphics2D::UI
