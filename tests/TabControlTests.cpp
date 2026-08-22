// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/TabControl.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::Grid;
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
} // namespace
