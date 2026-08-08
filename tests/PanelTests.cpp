// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"

#include <gtest/gtest.h>

#include <functional>
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

    class CallbackWidget final : public Widget
    {
    public:
        explicit CallbackWidget(const Point desiredSize) : desiredSize_(desiredSize) {}

        std::function<void()> onMeasure;
        std::function<void()> onArrange;
        int arrangeCalls = 0;

    protected:
        [[nodiscard]] Point InternalMeasure(Point) override
        {
            if (onMeasure)
            {
                std::function<void()> callback = std::move(onMeasure);
                callback();
            }
            return desiredSize_;
        }

        void InternalArrange() override
        {
            ++arrangeCalls;
            if (onArrange)
            {
                std::function<void()> callback = std::move(onArrange);
                callback();
            }
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

        EXPECT_EQ(panel.getBoundsProperty(), Rectangle(0, 0, 200, 100));
        EXPECT_EQ(small->getContainerBoundsProperty(), Rectangle(0, 0, 200, 100));
        EXPECT_EQ(small->getBoundsProperty(), Rectangle(0, 0, 30, 20));
        EXPECT_EQ(large->getBoundsProperty(), Rectangle(0, 0, 60, 40));

        large->setVisibleProperty(false);
        EXPECT_EQ(panel.Measure(Point(200, 100)), Point(30, 20));
    }

    TEST(PanelTests, LayoutPassesRetainChildrenAcrossReentrantTreeRefresh)
    {
        Panel measuringPanel;
        auto mutatingMeasure = std::make_shared<CallbackWidget>(Point(10, 10));
        auto removedDuringMeasure = std::make_shared<CallbackWidget>(Point(60, 40));
        measuringPanel.AddWidget(mutatingMeasure);
        measuringPanel.AddWidget(removedDuringMeasure);
        mutatingMeasure->onMeasure = [&] {
            EXPECT_TRUE(measuringPanel.RemoveWidget(removedDuringMeasure.get()));
            static_cast<void>(measuringPanel.getChildrenCopyProperty());
        };

        EXPECT_EQ(measuringPanel.Measure(Point(200, 100)), Point(60, 40));
        EXPECT_EQ(removedDuringMeasure->getParentProperty(), nullptr);

        Panel arrangingPanel;
        auto mutatingArrange = std::make_shared<CallbackWidget>(Point(10, 10));
        auto removedDuringArrange = std::make_shared<CallbackWidget>(Point(20, 20));
        arrangingPanel.AddWidget(mutatingArrange);
        arrangingPanel.AddWidget(removedDuringArrange);
        mutatingArrange->onArrange = [&] {
            EXPECT_TRUE(arrangingPanel.RemoveWidget(removedDuringArrange.get()));
            static_cast<void>(arrangingPanel.getChildrenCopyProperty());
        };

        arrangingPanel.Arrange(Rectangle(0, 0, 100, 100));
        EXPECT_EQ(removedDuringArrange->arrangeCalls, 1);
        EXPECT_EQ(removedDuringArrange->getParentProperty(), nullptr);
    }
}
