// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/Selector.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Layouts/SingleItemLayout.hpp"
#include "Myra/Graphics2D/UI/Selectors/ISelector.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Abstract selector scaffold that owns one display widget. */
    template <std::derived_from<Widget> WidgetT, std::derived_from<ISelectorItem> ItemT>
    class SelectorBase : public Widget, public ISelectorT<ItemT>
    {
      public:
        ~SelectorBase() override = default;

      protected:
        explicit SelectorBase(std::shared_ptr<WidgetT> widget) : layout_(*this)
        {
            if (!widget)
            {
                throw std::invalid_argument("A selector requires a non-null display widget.");
            }
            setChildrenLayoutProperty(&layout_);
            layout_.setChildProperty(std::move(widget));
        }

        [[nodiscard]] std::shared_ptr<WidgetT> getInternalChildProperty() const { return layout_.getChildProperty(); }

      private:
        SingleItemLayout<WidgetT> layout_;
    };

    /** @brief Generic observable-collection selection implementation. */
    template <std::derived_from<Widget> WidgetT, std::derived_from<ISelectorItem> ItemT>
    class Selector : public SelectorBase<WidgetT, ItemT>
    {
      public:
        using ItemCollection = typename ISelectorT<ItemT>::ItemCollection;

        Events::MyraEventHandler SelectedIndexChanged;
        Events::MyraEventHandler ItemsCollectionChanged;

        ~Selector() override = default;

        [[nodiscard]] SelectionMode getSelectionModeProperty() const noexcept override { return selectionMode_; }

        void setSelectionModeProperty(const SelectionMode value) noexcept override { selectionMode_ = value; }

        [[nodiscard]] const ItemCollection &getItemsProperty() const noexcept override { return items_; }

        [[nodiscard]] ItemCollection &getItemsProperty() noexcept override { return items_; }

        [[nodiscard]] std::optional<int> getSelectedIndexProperty() const override
        {
            if (!selectedItem_)
            {
                return std::nullopt;
            }
            return static_cast<int>(items_.IndexOf(selectedItem_));
        }

        void setSelectedIndexProperty(const std::optional<int> value) override
        {
            if (!value || *value < 0 || *value >= items_.getCountProperty())
            {
                setSelectedItemProperty(nullptr);
                return;
            }
            setSelectedItemProperty(items_.getItem(*value));
        }

        [[nodiscard]] std::shared_ptr<ItemT> getSelectedItemProperty() const override { return selectedItem_; }

        void setSelectedItemProperty(std::shared_ptr<ItemT> value) override
        {
            if (value == selectedItem_)
            {
                return;
            }
            if (selectionMode_ == SelectionMode::Single && selectedItem_)
            {
                selectedItem_->setIsSelectedProperty(false);
            }

            selectedItem_ = std::move(value);
            if (selectedItem_)
            {
                selectedItem_->setIsSelectedProperty(true);
            }
            Utility::EventsExtensions::Invoke(SelectedIndexChanged, this, InputEventType::ValueChanged);
            OnSelectedItemChanged();
        }

        Events::MyraEventHandler &getSelectedIndexChangedEvent() noexcept override { return SelectedIndexChanged; }

      protected:
        explicit Selector(std::shared_ptr<WidgetT> widget) : SelectorBase<WidgetT, ItemT>(std::move(widget))
        {
            const std::shared_ptr<WidgetT> child = this->getInternalChildProperty();
            child->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
            child->setVerticalAlignmentProperty(VerticalAlignment::Stretch);
            this->setHorizontalAlignmentProperty(HorizontalAlignment::Left);
            this->setVerticalAlignmentProperty(VerticalAlignment::Top);
            items_.CollectionChanged.emplace_back([this](void *, const auto &arguments)
                                                  { OnItemsCollectionChanged(arguments); });
        }

        virtual void Reset() = 0;
        virtual void InsertItem(std::shared_ptr<ItemT> item, int index) = 0;
        virtual void RemoveItem(const std::shared_ptr<ItemT> &item) = 0;

        virtual void OnItemCollectionChanged()
        {
            Utility::EventsExtensions::Invoke(ItemsCollectionChanged, this, InputEventType::ValueChanged);
        }

        virtual void OnSelectedItemChanged() {}

      private:
        void OnItemsCollectionChanged(
            const System::Collections::Specialized::NotifyCollectionChangedEventArgs<std::shared_ptr<ItemT>> &arguments)
        {
            using System::Collections::Specialized::NotifyCollectionChangedAction;

            switch (arguments.Action)
            {
            case NotifyCollectionChangedAction::Add:
            {
                int index = static_cast<int>(arguments.NewStartingIndex);
                for (const std::shared_ptr<ItemT> &item : arguments.NewItems)
                {
                    InsertItem(item, index);
                    ++index;
                }
                break;
            }
            case NotifyCollectionChangedAction::Remove:
                for (const std::shared_ptr<ItemT> &item : arguments.OldItems)
                {
                    RemoveItem(item);
                }
                break;
            case NotifyCollectionChangedAction::Reset:
                Reset();
                break;
            default:
                break;
            }

            OnItemCollectionChanged();
            this->InvalidateMeasure();
        }

        SelectionMode selectionMode_ = SelectionMode::Single;
        ItemCollection items_;
        std::shared_ptr<ItemT> selectedItem_;
    };
} // namespace Myra::Graphics2D::UI
