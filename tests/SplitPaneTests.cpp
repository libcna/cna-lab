// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/SplitPane.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::HorizontalSplitPane;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::VerticalSplitPane;
    using Myra::Graphics2D::UI::Widget;

    TEST(SplitPaneTests, BuildsTheLogicalWidgetsAndInterspersedHandleLayout)
    {
        HorizontalSplitPane splitPane;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        const auto third = std::make_shared<Widget>();
        int changed = 0;
        splitPane.ProportionsChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &splitPane);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ProportionChanged);
            ++changed;
        };

        splitPane.AddWidget(first);
        splitPane.AddWidget(second);
        splitPane.InsertWidget(1, third);
        ASSERT_EQ(splitPane.getWidgetsProperty().size(), 3U);
        EXPECT_EQ(splitPane.getWidgetsProperty()[0], first);
        EXPECT_EQ(splitPane.getWidgetsProperty()[1], third);
        EXPECT_EQ(splitPane.getWidgetsProperty()[2], second);
        EXPECT_EQ(splitPane.getChildrenProperty().size(), 5U);
        EXPECT_EQ(splitPane.getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(splitPane.GetProportion(0), 1.0F);
        EXPECT_EQ(splitPane.GetProportion(1), 1.0F);
        EXPECT_EQ(splitPane.GetProportion(2), 1.0F);
        EXPECT_EQ(splitPane.GetProportion(-1), 0.0F);
        EXPECT_EQ(splitPane.GetProportion(3), 0.0F);
        EXPECT_EQ(changed, 3);
    }

    TEST(SplitPaneTests, UpdatesAdjacentSplitterProportionsAndRejectsInvalidIndices)
    {
        HorizontalSplitPane splitPane;
        splitPane.AddWidget(std::make_shared<Widget>());
        splitPane.AddWidget(std::make_shared<Widget>());
        splitPane.AddWidget(std::make_shared<Widget>());

        EXPECT_FLOAT_EQ(splitPane.GetSplitterPosition(0), 1.0F / 3.0F);
        splitPane.SetSplitterPosition(0, 0.25F);
        EXPECT_FLOAT_EQ(splitPane.GetProportion(0), 0.75F);
        EXPECT_FLOAT_EQ(splitPane.GetProportion(1), 1.25F);
        EXPECT_FLOAT_EQ(splitPane.GetProportion(2), 1.0F);
        EXPECT_FLOAT_EQ(splitPane.GetSplitterPosition(0), 0.25F);

        EXPECT_THROW(static_cast<void>(splitPane.GetSplitterPosition(-1)), std::out_of_range);
        EXPECT_THROW(static_cast<void>(splitPane.GetSplitterPosition(2)), std::out_of_range);
        EXPECT_THROW(splitPane.SetSplitterPosition(2, 0.5F), std::out_of_range);
    }

    TEST(SplitPaneTests, SupportsVerticalCollectionsRemovalAndExactTypeCloning)
    {
        VerticalSplitPane source;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        source.AddWidget(first);
        source.AddWidget(second);
        EXPECT_EQ(source.getOrientationProperty(), Orientation::Vertical);
        EXPECT_TRUE(source.RemoveWidget(first.get()));
        EXPECT_FALSE(source.RemoveWidget(first.get()));
        EXPECT_EQ(source.getWidgetsProperty().size(), 1U);
        EXPECT_EQ(source.getChildrenProperty().size(), 1U);

        source.AddWidget(first);
        const auto clone = std::dynamic_pointer_cast<VerticalSplitPane>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getWidgetsProperty().size(), 2U);
        EXPECT_NE(clone->getWidgetsProperty()[0], second);
        EXPECT_NE(clone->getWidgetsProperty()[1], first);
        EXPECT_EQ(clone->getChildrenProperty().size(), 3U);

        source.ClearWidgets();
        EXPECT_TRUE(source.getWidgetsProperty().empty());
        EXPECT_TRUE(source.getChildrenProperty().empty());
    }
} // namespace
