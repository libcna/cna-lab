// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/TabControl.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/Graphics2D/UI/Simple/Button.hpp"
#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::Button;
    using Myra::Graphics2D::UI::Grid;
    using Myra::Graphics2D::UI::HorizontalStackPanel;
    using Myra::Graphics2D::UI::ListViewButton;
    using Myra::Graphics2D::UI::TabControl;
    using Myra::Graphics2D::UI::TabItem;
    using Myra::Graphics2D::UI::TabSelectorPosition;
    using Myra::Graphics2D::UI::Widget;

    TEST(TabControlTests, SelectsTheFirstItemThenUpdatesContentFromButtonClicks)
    {
        TabControl tabs;
        const auto firstContent = std::make_shared<Widget>();
        const auto secondContent = std::make_shared<Widget>();
        const auto first = std::make_shared<TabItem>(std::string("One"), firstContent);
        const auto second = std::make_shared<TabItem>(std::string("Two"), secondContent);
        tabs.getItemsProperty().Add(first);
        tabs.getItemsProperty().Add(second);

        EXPECT_EQ(tabs.getSelectedItemProperty(), first);
        EXPECT_TRUE(first->getIsSelectedProperty());
        ASSERT_EQ(tabs.getButtonsGridProperty()->getWidgetsProperty().size(), 2U);
        ASSERT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty()[0], firstContent);

        const auto secondButton =
            std::dynamic_pointer_cast<ListViewButton>(tabs.getButtonsGridProperty()->getWidgetsProperty()[1]);
        ASSERT_NE(secondButton, nullptr);
        secondButton->DoClick();
        EXPECT_EQ(tabs.getSelectedItemProperty(), second);
        EXPECT_FALSE(first->getIsSelectedProperty());
        EXPECT_TRUE(second->getIsSelectedProperty());
        ASSERT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty()[0], secondContent);

        const auto replacementContent = std::make_shared<Widget>();
        second->setContentProperty(replacementContent);
        ASSERT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty().size(), 1U);
        EXPECT_EQ(tabs.getContentPanelProperty()->getWidgetsProperty()[0], replacementContent);
    }

    TEST(TabControlTests, RepositionsSelectorButtonsForEverySupportedPlacement)
    {
        TabControl tabs;
        const auto layout = std::dynamic_pointer_cast<Grid>(tabs.getChildrenProperty()[0]);
        ASSERT_NE(layout, nullptr);

        EXPECT_EQ(Grid::GetRow(*tabs.getButtonsGridProperty()), 0);
        EXPECT_EQ(Grid::GetRow(*tabs.getContentPanelProperty()), 1);
        tabs.setTabSelectorPositionProperty(TabSelectorPosition::Right);
        EXPECT_EQ(Grid::GetColumn(*tabs.getButtonsGridProperty()), 1);
        EXPECT_EQ(Grid::GetColumn(*tabs.getContentPanelProperty()), 0);
        tabs.setTabSelectorPositionProperty(TabSelectorPosition::Bottom);
        EXPECT_EQ(Grid::GetRow(*tabs.getButtonsGridProperty()), 1);
        EXPECT_EQ(Grid::GetRow(*tabs.getContentPanelProperty()), 0);
        tabs.setTabSelectorPositionProperty(TabSelectorPosition::Left);
        EXPECT_EQ(Grid::GetColumn(*tabs.getButtonsGridProperty()), 0);
        EXPECT_EQ(Grid::GetColumn(*tabs.getContentPanelProperty()), 1);
    }

    TEST(TabControlTests, RemovesItemsAndClonesThePortedItemAndLayoutState)
    {
        TabControl source;
        const auto first = std::make_shared<TabItem>(std::string("One"), std::make_shared<Widget>());
        const auto second = std::make_shared<TabItem>(std::string("Two"), std::make_shared<Widget>());
        source.getItemsProperty().Add(first);
        source.getItemsProperty().Add(second);
        source.setTabSelectorPositionProperty(TabSelectorPosition::Left);
        source.getItemsProperty().Remove(first);
        EXPECT_EQ(source.getSelectedItemProperty(), nullptr);
        ASSERT_EQ(source.getButtonsGridProperty()->getWidgetsProperty().size(), 1U);

        const auto clone = std::dynamic_pointer_cast<TabControl>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getTabSelectorPositionProperty(), TabSelectorPosition::Left);
        ASSERT_EQ(clone->getItemsProperty().getCountProperty(), 1);
        EXPECT_NE(clone->getItemsProperty().getItem(0), second);
        EXPECT_EQ(clone->getItemsProperty().getItem(0)->getTextProperty(), second->getTextProperty());
        EXPECT_EQ(clone->getSelectedIndexProperty(), 0);
    }

    TEST(TabControlTests, CloseableHeadersSelectAndRemoveTheirExactItems)
    {
        TabControl tabs;
        tabs.setCloseableTabsProperty(true);
        const auto first = std::make_shared<TabItem>(std::string("One"), std::make_shared<Widget>());
        const auto second = std::make_shared<TabItem>(std::string("Two"), std::make_shared<Widget>());
        tabs.getItemsProperty().Add(first);
        const auto firstHeaderBeforeRebuild =
            std::dynamic_pointer_cast<HorizontalStackPanel>(tabs.getButtonsGridProperty()->getWidgetsProperty()[0]);
        ASSERT_NE(firstHeaderBeforeRebuild, nullptr);
        const auto detachedFirstSelector =
            std::dynamic_pointer_cast<ListViewButton>(firstHeaderBeforeRebuild->getWidgetsProperty()[0]);
        ASSERT_NE(detachedFirstSelector, nullptr);
        tabs.getItemsProperty().Add(second);

        ASSERT_EQ(tabs.getButtonsGridProperty()->getWidgetsProperty().size(), 2U);
        const auto secondHeader =
            std::dynamic_pointer_cast<HorizontalStackPanel>(tabs.getButtonsGridProperty()->getWidgetsProperty()[1]);
        ASSERT_NE(secondHeader, nullptr);
        ASSERT_EQ(secondHeader->getWidgetsProperty().size(), 2U);
        const auto secondSelector = std::dynamic_pointer_cast<ListViewButton>(secondHeader->getWidgetsProperty()[0]);
        const auto secondClose = std::dynamic_pointer_cast<Button>(secondHeader->getWidgetsProperty()[1]);
        ASSERT_NE(secondSelector, nullptr);
        ASSERT_NE(secondClose, nullptr);

        secondSelector->DoClick();
        EXPECT_EQ(tabs.getSelectedItemProperty(), second);
        EXPECT_EQ(detachedFirstSelector->getButtonsContainerProperty(), nullptr);
        detachedFirstSelector->DoClick();
        EXPECT_EQ(tabs.getSelectedItemProperty(), second);
        secondClose->DoClick();
        ASSERT_EQ(tabs.getItemsProperty().getCountProperty(), 1);
        EXPECT_EQ(tabs.getItemsProperty().getItem(0), first);
        EXPECT_EQ(tabs.getSelectedItemProperty(), nullptr);
        EXPECT_EQ(tabs.getButtonsGridProperty()->getWidgetsProperty().size(), 1U);
    }

    TEST(TabControlTests, ClonePreservesCloseableStructureAndRetainedButtonsBecomeInert)
    {
        std::shared_ptr<ListViewButton> retainedSelector;
        std::shared_ptr<Button> retainedClose;
        std::weak_ptr<TabControl> releasedTabs;
        {
            auto tabs = std::make_shared<TabControl>();
            tabs->setCloseableTabsProperty(true);
            tabs->getItemsProperty().Add(std::make_shared<TabItem>(std::string("One"), std::make_shared<Widget>()));

            const auto clone = std::dynamic_pointer_cast<TabControl>(tabs->Clone());
            ASSERT_NE(clone, nullptr);
            EXPECT_TRUE(clone->getCloseableTabsProperty());
            ASSERT_EQ(clone->getButtonsGridProperty()->getWidgetsProperty().size(), 1U);
            EXPECT_NE(std::dynamic_pointer_cast<HorizontalStackPanel>(
                          clone->getButtonsGridProperty()->getWidgetsProperty()[0]),
                      nullptr);

            const auto header = std::dynamic_pointer_cast<HorizontalStackPanel>(
                tabs->getButtonsGridProperty()->getWidgetsProperty()[0]);
            ASSERT_NE(header, nullptr);
            retainedSelector = std::dynamic_pointer_cast<ListViewButton>(header->getWidgetsProperty()[0]);
            retainedClose = std::dynamic_pointer_cast<Button>(header->getWidgetsProperty()[1]);
            ASSERT_NE(retainedSelector, nullptr);
            ASSERT_NE(retainedClose, nullptr);
            releasedTabs = tabs;
        }

        EXPECT_TRUE(releasedTabs.expired());
        EXPECT_EQ(retainedSelector->getButtonsContainerProperty(), nullptr);
        retainedSelector->DoClick();
        retainedSelector->setIsPressedProperty(true);
        retainedClose->DoClick();
    }
} // namespace
