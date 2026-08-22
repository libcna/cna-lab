// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/ComboView.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::ComboView;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::SelectionMode;
    using Myra::Graphics2D::UI::Widget;

    TEST(ComboViewTests, SelectsTheFirstItemWhenExpandedAndForwardsTheListEvent)
    {
        ComboView comboView;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        int selectedChanges = 0;
        comboView.SelectedIndexChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, comboView.getListViewProperty().get());
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectedIndexChanged);
            ++selectedChanges;
        };

        EXPECT_EQ(comboView.getDropdownMaximumHeightProperty(), 300);
        comboView.AddWidget(first);
        comboView.AddWidget(second);
        comboView.getButtonProperty()->DoClick();
        EXPECT_TRUE(comboView.getIsExpandedProperty());
        EXPECT_EQ(comboView.getSelectedItemProperty(), first);
        EXPECT_EQ(comboView.getSelectedIndexProperty(), 0);
        ASSERT_NE(comboView.getButtonProperty()->getContentProperty(), nullptr);
        EXPECT_NE(comboView.getButtonProperty()->getContentProperty(), first);

        comboView.setSelectedIndexProperty(1);
        EXPECT_EQ(comboView.getSelectedItemProperty(), second);
        EXPECT_EQ(comboView.getSelectedIndexProperty(), 1);
        EXPECT_EQ(selectedChanges, 2);
    }

    TEST(ComboViewTests, DelegatesCollectionAndConfigurationAndClonesItsItems)
    {
        ComboView source;
        const auto child = std::make_shared<Widget>();
        source.AddWidget(child);
        source.setDropdownMaximumHeightProperty(73);
        source.setSelectionModeProperty(SelectionMode::Multiple);

        const auto clone = std::dynamic_pointer_cast<ComboView>(source.Clone());
        ASSERT_NE(clone, nullptr);
        ASSERT_EQ(clone->getWidgetsProperty().size(), 1U);
        EXPECT_NE(clone->getWidgetsProperty().front(), child);
        EXPECT_EQ(clone->getDropdownMaximumHeightProperty(), 73);
        EXPECT_EQ(clone->getSelectionModeProperty(), SelectionMode::Multiple);
        EXPECT_TRUE(clone->RemoveWidget(clone->getWidgetsProperty().front().get()));
        EXPECT_TRUE(clone->getWidgetsProperty().empty());
    }
} // namespace
