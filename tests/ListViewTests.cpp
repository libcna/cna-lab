// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/ListView.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::ListView;
    using Myra::Graphics2D::UI::ListViewButton;
    using Myra::Graphics2D::UI::SelectionMode;
    using Myra::Graphics2D::UI::Widget;

    TEST(ListViewTests, WrapsWidgetsAndUpdatesSingleSelectionFromButtonClicks)
    {
        ListView listView;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        int selectedChanges = 0;
        listView.SelectedIndexChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &listView);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectedIndexChanged);
            ++selectedChanges;
        };

        listView.AddWidget(first);
        listView.AddWidget(second);
        ASSERT_EQ(listView.getWidgetsProperty().size(), 2U);
        EXPECT_EQ(listView.getWidgetsProperty()[0], first);
        EXPECT_EQ(listView.getWidgetsProperty()[1], second);

        auto *const firstButton = dynamic_cast<ListViewButton *>(first->getParentProperty());
        auto *const secondButton = dynamic_cast<ListViewButton *>(second->getParentProperty());
        ASSERT_NE(firstButton, nullptr);
        ASSERT_NE(secondButton, nullptr);
        firstButton->DoClick();
        EXPECT_EQ(listView.getSelectedItemProperty(), first);
        EXPECT_EQ(listView.getSelectedIndexProperty(), 0);
        EXPECT_TRUE(firstButton->getIsPressedProperty());

        secondButton->DoClick();
        EXPECT_EQ(listView.getSelectedItemProperty(), second);
        EXPECT_EQ(listView.getSelectedIndexProperty(), 1);
        EXPECT_FALSE(firstButton->getIsPressedProperty());
        EXPECT_TRUE(secondButton->getIsPressedProperty());
        EXPECT_EQ(selectedChanges, 2);

        EXPECT_TRUE(listView.RemoveWidget(second.get()));
        EXPECT_EQ(listView.getSelectedItemProperty(), nullptr);
        EXPECT_FALSE(listView.getSelectedIndexProperty().has_value());
        EXPECT_EQ(selectedChanges, 3);
    }

    TEST(ListViewTests, SupportsOrderedInsertionMultipleModeAndExactTypeCloning)
    {
        ListView source;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        source.AddWidget(first);
        source.InsertWidget(0, second);
        EXPECT_EQ(source.getWidgetsProperty()[0], second);
        EXPECT_EQ(source.getWidgetsProperty()[1], first);

        source.setSelectionModeProperty(SelectionMode::Multiple);
        auto *const secondButton = dynamic_cast<ListViewButton *>(second->getParentProperty());
        ASSERT_NE(secondButton, nullptr);
        secondButton->DoClick();
        EXPECT_EQ(source.getSelectedItemProperty(), nullptr);
        EXPECT_TRUE(secondButton->getIsPressedProperty());

        const auto clone = std::dynamic_pointer_cast<ListView>(source.Clone());
        ASSERT_NE(clone, nullptr);
        ASSERT_EQ(clone->getWidgetsProperty().size(), 2U);
        EXPECT_NE(clone->getWidgetsProperty()[0], second);
        EXPECT_NE(clone->getWidgetsProperty()[1], first);
        EXPECT_EQ(clone->getSelectionModeProperty(), SelectionMode::Multiple);
        EXPECT_EQ(clone->getSelectedItemProperty(), nullptr);
    }
} // namespace
