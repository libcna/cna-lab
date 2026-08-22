// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/TabControl.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <vector>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"
#include "Myra/Graphics2D/UI/Selectors/Selector.hpp"
#include "Myra/Graphics2D/UI/Selectors/TabItem.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Placement of a TabControl's selector-button strip. */
    enum class TabSelectorPosition
    {
        Top,
        Right,
        Bottom,
        Left
    };

    /** @brief Selector that presents TabItem content below, above, or beside a button strip. */
    class TabControl final : public Selector<Grid, TabItem>
    {
      public:
        TabControl();
        ~TabControl() override;

        [[nodiscard]] TabSelectorPosition getTabSelectorPositionProperty() const noexcept;
        void setTabSelectorPositionProperty(TabSelectorPosition value);
        [[nodiscard]] std::shared_ptr<Grid> getButtonsGridProperty() const;
        [[nodiscard]] std::shared_ptr<Panel> getContentPanelProperty() const;

      protected:
        void Reset() override;
        void InsertItem(std::shared_ptr<TabItem> item, int index) override;
        void RemoveItem(const std::shared_ptr<TabItem> &item) override;
        void OnSelectedItemChanged() override;
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget &source) override;

      private:
        void UpdateSelectorPosition();
        void RebuildButtons();
        void UpdateButtonsGrid();
        void UpdateContent();
        void AddItemSubscription(const std::shared_ptr<TabItem> &item);
        void RemoveItemSubscription(const TabItem *item);
        void ClearItemSubscriptions();

        struct ItemSubscription
        {
            std::shared_ptr<TabItem> item;
            Events::MyraEventHandler::Token token = Events::MyraEventHandler::InvalidToken;
        };

        std::shared_ptr<Grid> buttonsGrid_;
        std::shared_ptr<Panel> contentPanel_;
        std::vector<std::shared_ptr<ListViewButton>> buttons_;
        std::vector<ItemSubscription> itemSubscriptions_;
        TabSelectorPosition tabSelectorPosition_ = TabSelectorPosition::Top;
    };
} // namespace Myra::Graphics2D::UI
