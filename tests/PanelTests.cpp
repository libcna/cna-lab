// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::Widget;

    class FixedWidget final : public Widget
    {
    public:
        explicit FixedWidget(const Point desiredSize) : desiredSize_(desiredSize) {}

    protected:
        [[nodiscard]] Point InternalMeasure(Point) override
        {
            return desiredSize_;
        }

    private:
        Point desiredSize_;
    };

    TEST(PanelTests, DefaultsToStretchAndExposesTheSharedWidgetCollection)
    {
        Panel panel;
        EXPECT_EQ(panel.getHorizontalAlignmentProperty(), HorizontalAlignment::Stretch);
        EXPECT_EQ(panel.getVerticalAlignmentProperty(), VerticalAlignment::Stretch);
        EXPECT_TRUE(panel.getWidgetsProperty().empty());
    }

    TEST(PanelTests, MeasuresLargestVisibleChildAndArrangesChildrenInActualBounds)
    {
        Panel panel;
        auto small = std::make_shared<FixedWidget>(Point(30, 20));
        auto large = std::make_shared<FixedWidget>(Point(60, 40));
        panel.AddWidget(small);
        panel.AddWidget(large);

        EXPECT_EQ(panel.Measure(Point(200, 100)), Point(60, 40));
        panel.Arrange(Rectangle(10, 20, 200, 100));

        EXPECT_EQ(panel.getBoundsProperty(), Rectangle(10, 20, 200, 100));
        EXPECT_EQ(small->getContainerBoundsProperty(), Rectangle(0, 0, 200, 100));
        EXPECT_EQ(small->getBoundsProperty(), Rectangle(0, 0, 30, 20));
        EXPECT_EQ(large->getBoundsProperty(), Rectangle(0, 0, 60, 40));

        large->setVisibleProperty(false);
        EXPECT_EQ(panel.Measure(Point(200, 100)), Point(30, 20));
    }
}
