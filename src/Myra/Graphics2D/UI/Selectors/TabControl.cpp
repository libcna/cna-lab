// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/TabControl.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Selectors/TabControl.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/UI/Simple/Image.hpp"

namespace Myra::Graphics2D::UI
{
    TabControl::TabControl()
        : Selector<Grid, TabItem>(std::make_shared<Grid>()), buttonsGrid_(std::make_shared<Grid>()),
          contentPanel_(std::make_shared<Panel>())
    {
        setHorizontalAlignmentProperty(HorizontalAlignment::Left);
        setVerticalAlignmentProperty(VerticalAlignment::Top);
        setClipToBoundsProperty(true);

        const std::shared_ptr<Grid> layout = getInternalChildProperty();
        layout->setDefaultColumnProportionProperty(Proportion::Fill);
        layout->setDefaultRowProportionProperty(Proportion::Fill);
        buttonsGrid_->setDefaultColumnProportionProperty(Proportion::Auto);
        buttonsGrid_->setDefaultRowProportionProperty(Proportion::Auto);
        layout->AddWidget(buttonsGrid_);
        layout->AddWidget(contentPanel_);
        UpdateSelectorPosition();
    }

    TabControl::~TabControl()
    {
        ClearItemSubscriptions();
    }

    TabSelectorPosition TabControl::getTabSelectorPositionProperty() const noexcept
    {
        return tabSelectorPosition_;
    }

    void TabControl::setTabSelectorPositionProperty(const TabSelectorPosition value)
    {
        if (value == tabSelectorPosition_)
        {
            return;
        }
        tabSelectorPosition_ = value;
        UpdateSelectorPosition();
    }

    std::shared_ptr<Grid> TabControl::getButtonsGridProperty() const
    {
        return buttonsGrid_;
    }

    std::shared_ptr<Panel> TabControl::getContentPanelProperty() const
    {
        return contentPanel_;
    }

    void TabControl::Reset()
    {
        ClearItemSubscriptions();
        buttons_.clear();
        buttonsGrid_->ClearChildren();
        contentPanel_->ClearChildren();
        setSelectedItemProperty(nullptr);
    }

    void TabControl::InsertItem(std::shared_ptr<TabItem> item, const int /*index*/)
    {
        if (!item)
        {
            throw std::invalid_argument("A TabControl item cannot be null.");
        }
        AddItemSubscription(item);
        RebuildButtons();
        if (getItemsProperty().getCountProperty() == 1)
        {
            setSelectedItemProperty(item);
        }
    }

    void TabControl::RemoveItem(const std::shared_ptr<TabItem> &item)
    {
        RemoveItemSubscription(item.get());
        if (item == getSelectedItemProperty())
        {
            setSelectedItemProperty(nullptr);
        }
        RebuildButtons();
    }

    void TabControl::OnSelectedItemChanged()
    {
        UpdateContent();
        for (int index = 0; index < getItemsProperty().getCountProperty(); ++index)
        {
            if (static_cast<std::size_t>(index) < buttons_.size())
            {
                buttons_[static_cast<std::size_t>(index)]->setIsPressedProperty(getItemsProperty().getItem(index) ==
                                                                                getSelectedItemProperty());
            }
        }
    }

    std::shared_ptr<Widget> TabControl::CreateCloneInstance() const
    {
        return std::make_shared<TabControl>();
    }

    void TabControl::CopyFrom(const Widget &source)
    {
        Widget::CopyFrom(source);
        const auto *const tabControl = dynamic_cast<const TabControl *>(&source);
        if (tabControl == nullptr)
        {
            throw std::invalid_argument("TabControl copy source must be a TabControl.");
        }
        setTabSelectorPositionProperty(tabControl->tabSelectorPosition_);
        for (const std::shared_ptr<TabItem> &item : tabControl->getItemsProperty())
        {
            getItemsProperty().Add(item->Clone());
        }
    }

    void TabControl::UpdateSelectorPosition()
    {
        const std::shared_ptr<Grid> layout = getInternalChildProperty();
        layout->getColumnsProportionsProperty().Clear();
        layout->getRowsProportionsProperty().Clear();
        switch (tabSelectorPosition_)
        {
        case TabSelectorPosition::Top:
            Grid::SetColumn(*buttonsGrid_, 0);
            Grid::SetRow(*buttonsGrid_, 0);
            Grid::SetColumn(*contentPanel_, 0);
            Grid::SetRow(*contentPanel_, 1);
            layout->getRowsProportionsProperty().Add(Proportion::Auto);
            layout->getRowsProportionsProperty().Add(Proportion::Fill);
            break;
        case TabSelectorPosition::Right:
            Grid::SetColumn(*buttonsGrid_, 1);
            Grid::SetRow(*buttonsGrid_, 0);
            Grid::SetColumn(*contentPanel_, 0);
            Grid::SetRow(*contentPanel_, 0);
            layout->getColumnsProportionsProperty().Add(Proportion::Fill);
            layout->getColumnsProportionsProperty().Add(Proportion::Auto);
            break;
        case TabSelectorPosition::Bottom:
            Grid::SetColumn(*buttonsGrid_, 0);
            Grid::SetRow(*buttonsGrid_, 1);
            Grid::SetColumn(*contentPanel_, 0);
            Grid::SetRow(*contentPanel_, 0);
            layout->getRowsProportionsProperty().Add(Proportion::Fill);
            layout->getRowsProportionsProperty().Add(Proportion::Auto);
            break;
        case TabSelectorPosition::Left:
            Grid::SetColumn(*buttonsGrid_, 0);
            Grid::SetRow(*buttonsGrid_, 0);
            Grid::SetColumn(*contentPanel_, 1);
            Grid::SetRow(*contentPanel_, 0);
            layout->getColumnsProportionsProperty().Add(Proportion::Auto);
            layout->getColumnsProportionsProperty().Add(Proportion::Fill);
            break;
        }
        UpdateButtonsGrid();
        InvalidateMeasure();
    }

    void TabControl::RebuildButtons()
    {
        buttons_.clear();
        buttonsGrid_->ClearChildren();
        for (int index = 0; index < getItemsProperty().getCountProperty(); ++index)
        {
            const std::shared_ptr<TabItem> item = getItemsProperty().getItem(index);
            const auto image = std::make_shared<Image>();
            image->setRenderableProperty(item->getImageProperty());
            const auto button = std::make_shared<ListViewButton>();
            button->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
            button->setVerticalAlignmentProperty(VerticalAlignment::Stretch);
            button->setHeightProperty(item->getHeightProperty());
            button->setButtonsContainerProperty(buttonsGrid_.get());
            button->setContentProperty(image);
            button->Click += [this, item](void *, Events::MyraEventArgs &)
            {
                const int itemIndex = getItemsProperty().IndexOf(item);
                if (itemIndex >= 0)
                {
                    setSelectedIndexProperty(itemIndex);
                }
            };
            buttons_.push_back(button);
            buttonsGrid_->AddWidget(button);
        }
        UpdateButtonsGrid();
        OnSelectedItemChanged();
    }

    void TabControl::UpdateButtonsGrid()
    {
        const bool vertical =
            tabSelectorPosition_ == TabSelectorPosition::Left || tabSelectorPosition_ == TabSelectorPosition::Right;
        for (std::size_t index = 0; index < buttons_.size(); ++index)
        {
            Grid::SetColumn(*buttons_[index], vertical ? 0 : static_cast<int>(index));
            Grid::SetRow(*buttons_[index], vertical ? static_cast<int>(index) : 0);
        }
    }

    void TabControl::UpdateContent()
    {
        contentPanel_->ClearChildren();
        const std::shared_ptr<TabItem> selected = getSelectedItemProperty();
        if (selected && selected->getContentProperty())
        {
            contentPanel_->AddWidget(selected->getContentProperty());
        }
    }

    void TabControl::AddItemSubscription(const std::shared_ptr<TabItem> &item)
    {
        const auto existing = std::find_if(itemSubscriptions_.begin(), itemSubscriptions_.end(),
                                           [&item](const auto &subscription) { return subscription.item == item; });
        if (existing != itemSubscriptions_.end())
        {
            return;
        }
        TabItem *const itemAddress = item.get();
        const Events::MyraEventHandler::Token token = item->Changed.Add(
            [this, itemAddress](void *, Events::MyraEventArgs &)
            {
                if (getSelectedItemProperty().get() == itemAddress)
                {
                    UpdateContent();
                }
            });
        itemSubscriptions_.push_back({item, token});
    }

    void TabControl::RemoveItemSubscription(const TabItem *const item)
    {
        const auto iterator =
            std::find_if(itemSubscriptions_.begin(), itemSubscriptions_.end(),
                         [item](const auto &subscription) { return subscription.item.get() == item; });
        if (iterator == itemSubscriptions_.end())
        {
            return;
        }
        static_cast<void>(iterator->item->Changed.Remove(iterator->token));
        itemSubscriptions_.erase(iterator);
    }

    void TabControl::ClearItemSubscriptions()
    {
        for (ItemSubscription &subscription : itemSubscriptions_)
        {
            static_cast<void>(subscription.item->Changed.Remove(subscription.token));
        }
        itemSubscriptions_.clear();
    }
} // namespace Myra::Graphics2D::UI
