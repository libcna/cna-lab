// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/ListView.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Selectors/ListView.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/Simple/SeparatorWidget.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        [[nodiscard]] int CheckedListIndex(const std::size_t value)
        {
            if (value > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            {
                throw std::overflow_error("ListView index is outside the supported integer range.");
            }
            return static_cast<int>(value);
        }

        [[nodiscard]] int CheckedScrollCoordinate(const std::int64_t value)
        {
            if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("ListView scroll coordinate is outside the supported integer range.");
            }
            return static_cast<int>(value);
        }
    } // namespace

    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Input::Keys;

    ListView::ListView()
        : callbackState_(std::make_shared<CallbackState>()), layout_(*this),
          scrollViewer_(std::make_shared<ScrollViewer>()), box_(std::make_shared<VerticalStackPanel>())
    {
        callbackState_->owner = this;
        setChildrenLayoutProperty(&layout_);
        layout_.setChildProperty(scrollViewer_);
        scrollViewer_->setContentProperty(box_);
        setAcceptsKeyboardFocusProperty(true);
    }

    ListView::~ListView()
    {
        callbackState_->owner = nullptr;
        ClearButtonSubscriptions();
    }

    const std::vector<std::shared_ptr<Widget>> &ListView::getWidgetsProperty() const noexcept
    {
        return widgets_;
    }

    void ListView::AddWidget(std::shared_ptr<Widget> widget)
    {
        InsertWidget(widgets_.size(), std::move(widget));
    }

    void ListView::InsertWidget(const std::size_t index, std::shared_ptr<Widget> widget)
    {
        if (!widget)
        {
            throw std::invalid_argument("A ListView widget cannot be null.");
        }
        if (index > widgets_.size())
        {
            throw std::out_of_range("ListView widget index is outside the collection.");
        }
        widgets_.insert(widgets_.begin() + static_cast<std::ptrdiff_t>(index), std::move(widget));
        RebuildDisplay();
    }

    bool ListView::RemoveWidget(const Widget *const widget)
    {
        const auto iterator =
            std::find_if(widgets_.begin(), widgets_.end(), [widget](const auto &item) { return item.get() == widget; });
        if (iterator == widgets_.end())
        {
            return false;
        }
        if (*iterator == selectedItem_)
        {
            setSelectedItemProperty(nullptr);
        }
        widgets_.erase(iterator);
        RebuildDisplay();
        return true;
    }

    void ListView::ClearWidgets()
    {
        widgets_.clear();
        RebuildDisplay();
    }

    std::shared_ptr<ScrollViewer> ListView::getScrollViewerProperty() const
    {
        return scrollViewer_;
    }

    SelectionMode ListView::getSelectionModeProperty() const noexcept
    {
        return selectionMode_;
    }

    void ListView::setSelectionModeProperty(const SelectionMode value) noexcept
    {
        selectionMode_ = value;
    }

    std::optional<int> ListView::getSelectedIndexProperty() const
    {
        if (!selectedItem_)
        {
            return std::nullopt;
        }
        const auto iterator = std::find(widgets_.begin(), widgets_.end(), selectedItem_);
        if (iterator == widgets_.end())
        {
            return std::nullopt;
        }
        return static_cast<int>(std::distance(widgets_.begin(), iterator));
    }

    void ListView::setSelectedIndexProperty(const std::optional<int> value)
    {
        if (!value || *value < 0 || static_cast<std::size_t>(*value) >= widgets_.size())
        {
            setSelectedItemProperty(nullptr);
            return;
        }
        setSelectedItemProperty(widgets_[static_cast<std::size_t>(*value)]);
    }

    std::shared_ptr<Widget> ListView::getSelectedItemProperty() const
    {
        return selectedItem_;
    }

    void ListView::setSelectedItemProperty(std::shared_ptr<Widget> value)
    {
        if (value == selectedItem_)
        {
            return;
        }
        selectedItem_ = std::move(value);
        if (selectedItem_)
        {
            if (auto *const button = dynamic_cast<ListViewButton *>(selectedItem_->getParentProperty()))
            {
                button->setIsPressedProperty(true);
            }
        }
        Utility::EventsExtensions::Invoke(SelectedIndexChanged, this, InputEventType::SelectedIndexChanged);
    }

    void ListView::OnMouseWheel(const float delta)
    {
        Widget::OnMouseWheel(delta);
        scrollViewer_->OnMouseWheel(delta);
    }

    void ListView::OnKeyDown(const Keys key)
    {
        const std::shared_ptr<CallbackState> callbackState = callbackState_;
        Widget::OnKeyDown(key);
        if (callbackState->owner != this)
        {
            return;
        }

        switch (key)
        {
        case Keys::Up:
        {
            if (widgets_.empty())
            {
                return;
            }
            std::size_t index;
            if (const std::optional<int> selected = getSelectedIndexProperty())
            {
                if (*selected <= 0)
                {
                    return;
                }
                index = static_cast<std::size_t>(*selected - 1);
            }
            else
            {
                index = widgets_.size() - 1;
            }
            while (index > 0 && dynamic_cast<SeparatorWidget *>(widgets_[index].get()) != nullptr)
            {
                --index;
            }
            setSelectedIndexProperty(CheckedListIndex(index));
            if (callbackState->owner == this)
            {
                UpdateScrolling();
            }
            break;
        }
        case Keys::Down:
        {
            std::size_t index = 0;
            if (const std::optional<int> selected = getSelectedIndexProperty())
            {
                index = static_cast<std::size_t>(*selected) + 1;
            }
            while (index < widgets_.size() && dynamic_cast<SeparatorWidget *>(widgets_[index].get()) != nullptr)
            {
                ++index;
            }
            if (index < widgets_.size())
            {
                setSelectedIndexProperty(CheckedListIndex(index));
                if (callbackState->owner == this)
                {
                    UpdateScrolling();
                }
            }
            break;
        }
        case Keys::Enter:
            HideComboDropdown();
            break;
        default:
            break;
        }
    }

    std::shared_ptr<Widget> ListView::CreateCloneInstance() const
    {
        return std::make_shared<ListView>();
    }

    void ListView::CopyFrom(const Widget &source)
    {
        Widget::CopyFrom(source);
        const auto *const listView = dynamic_cast<const ListView *>(&source);
        if (listView == nullptr)
        {
            throw std::invalid_argument("ListView copy source must be a ListView.");
        }
        setSelectionModeProperty(listView->selectionMode_);
        for (const std::shared_ptr<Widget> &widget : listView->widgets_)
        {
            AddWidget(widget->Clone());
        }
    }

    std::shared_ptr<Widget> ListView::Wrap(std::shared_ptr<Widget> widget)
    {
        if (std::dynamic_pointer_cast<SeparatorWidget>(widget))
        {
            return widget;
        }

        const auto button = std::make_shared<ListViewButton>();
        button->setContentProperty(std::move(widget));
        button->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        const Events::MyraEventHandler::Token token = button->Click.Add(
            [callbackState = callbackState_](void *sender, Events::MyraEventArgs &arguments)
            {
                if (ListView *const owner = callbackState->owner)
                {
                    owner->ButtonOnClick(sender, arguments);
                }
            });
        buttonSubscriptions_.push_back({button, token});
        return button;
    }

    void ListView::RebuildDisplay()
    {
        ClearButtonSubscriptions();
        box_->ClearChildren();
        try
        {
            for (const std::shared_ptr<Widget> &widget : widgets_)
            {
                box_->AddWidget(Wrap(widget));
            }
        }
        catch (...)
        {
            ClearButtonSubscriptions();
            box_->ClearChildren();
            throw;
        }
    }

    void ListView::ClearButtonSubscriptions() noexcept
    {
        for (const ButtonSubscription &subscription : buttonSubscriptions_)
        {
            try
            {
                static_cast<void>(subscription.button->Click.Remove(subscription.token));
            }
            catch (...)
            {
            }
        }
        buttonSubscriptions_.clear();
    }

    void ListView::ButtonOnClick(void *const sender, Events::MyraEventArgs &arguments)
    {
        static_cast<void>(arguments);
        const std::shared_ptr<CallbackState> callbackState = callbackState_;
        const auto *const button = static_cast<ListViewButton *>(sender);
        if (!button->getIsPressedProperty())
        {
            return;
        }
        if (selectionMode_ == SelectionMode::Single)
        {
            setSelectedItemProperty(button->getContentProperty());
        }
        if (callbackState->owner == this)
        {
            HideComboDropdown();
        }
    }

    void ListView::HideComboDropdown()
    {
        Desktop *const desktop = getDesktopProperty();
        if (desktop != nullptr && desktop->getContextMenuProperty().get() == this)
        {
            desktop->HideContextMenu();
        }
    }

    void ListView::UpdateScrolling()
    {
        const std::shared_ptr<Widget> selectedItem = selectedItem_;
        if (!selectedItem)
        {
            return;
        }

        scrollViewer_->UpdateArrange();
        const Point position =
            box_->ToLocal(selectedItem->ToGlobal(selectedItem->getBoundsProperty().getLocationProperty()));
        const int lineHeight = selectedItem->getActualBoundsProperty().Height;
        Point scrollPosition = scrollViewer_->getScrollPositionProperty();
        const auto scrollBounds = scrollViewer_->getBoundsProperty();
        const std::int64_t lineBottom = static_cast<std::int64_t>(position.Y) + lineHeight;
        const std::int64_t viewportBottom = static_cast<std::int64_t>(scrollPosition.Y) + scrollBounds.Height;
        if (position.Y < scrollPosition.Y)
        {
            scrollPosition.Y = position.Y;
        }
        else if (lineBottom > viewportBottom)
        {
            scrollPosition.Y = CheckedScrollCoordinate(lineBottom - static_cast<std::int64_t>(scrollBounds.Height));
        }
        scrollViewer_->setScrollPositionProperty(scrollPosition);
    }
} // namespace Myra::Graphics2D::UI
